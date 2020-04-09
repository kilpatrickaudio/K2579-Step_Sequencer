/*
 * K2579 Step Sequencer - Main
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 * Hardware I/O:
 *
 *  RB0/AN0		- step 1 pot				- analog
 *  RB1/AN1		- step 2 pot				- analog
 *  RB2			- n/c
 *  RB3			- mode switch				- input - active low
 *  RB4			- enter switch				- input - active low
 *  RB5			- page down switch			- input - active low	
 *  RB8/AN8		- mod 1 in					- analog
 *  RB9/AN9		- mod 2 in					- analog
 *  RB10		- page up switch			- input - active low
 *	RB11		- n/c
 *	RB12		- n/c
 *  RB13(TDI)	- test mode					- input - active low
 *  RB14		- gate 1 out				- output - active high
 *  RB15		- gate 2 out				- output - active high
 *
 *	RD0			- n/c
 *	RD1			- n/c
 *	RD2			- n/c
 *	RD3			- n/c
 *	RD4			- n/c
 *	RD5			- n/c
 *	RD6			- n/c
 *	RD7			- n/c
 *  RD8/INT1	- clock in					- input - int 1
 *  RD9/INT2	- reset in					- input - int 2
 *  RD10		- DAC !SS					- SPI1 chip select
 *	RD11		- n/c
 *	RD13		- n/c
 *	RD14 		- n/c
 *
 *  RE0			- clock LED					- output - active high
 *  RE1			- CV/gate LED 1				- output - active high
 *  RE2			- CV/gate LED 2				- output - active high
 *	RE3			- MOD LED					- output - active high
 *	RE4			- live switch				- input - active low
 *	RE5			- run/stop switch			- input - active low
 *	RE6			- reset switch				- input - active low
 *  RE7			- LCD serial RS				- output
 *
 *	RF0			- n/c
 *	RF1			- n/c
 *  RF2/SDI1	- not used					- SPI1 MISO (unused)
 *  RF3/SDO1	- DAC MOSI					- SPI1 MOSI
 *  RF4/U2RX	- MIDI RX					- UART2 RX
 *  RF5/U2TX	- MIDI TX					- UART2 TX
 *  RF6/SCK1	- DAC SCLK					- SPI1 clock
 *
 *  RG2/SCL1	- EEPROM SCL				- I2C clock
 *  RG3/SDA1	- EEPROM SDA				- I2C data
 *	RG6			- LCD SPI clock				- SPI2 clock
 *	RG7			- not used					- SPI2 MISO (unused)
 *	RG8			- LCD SPI MOSI				- SPI2 MOSI  
 *	RG9			- LCD SPI /SS				- SPI2 /SS
 *
 */
#include <plib.h>
#include "TimeDelay.h"
#include "song.h"
#include "song_file.h"
#include "panel.h"
#include "analog_input.h"
#include "mod_cv_input.h"
#include "clock.h"
#include "lcd.h"
#include "screen_handler.h"
#include "sequencer.h"
#include "midi.h"
#include "eeprom.h"
#include "sysconfig.h"
#include "cv_output.h"
#include "seq_midi.h"
#include "gui.h"

// Configuration Bit settings
// SYSCLK = 80 MHz (8MHz Crystal/ FPLLIDIV * FPLLMUL / FPLLODIV)
// PBCLK = 80 MHz
// Primary Osc w/PLL (XT+,HS+,EC+PLL)
// WDT OFF
// Other options are don't care
//
#pragma config FPLLMUL = MUL_20, FPLLIDIV = DIV_2, FPLLODIV = DIV_1, FWDTEN = OFF
#pragma config POSCMOD = HS, FNOSC = PRIPLL, FPBDIV = DIV_1

#define FOSC			80E6
#define PBCLOCK			80E6

unsigned char count;
unsigned char rand_nommer_count;

// main!
int main(void) {
	// Enable multi-vectored interrupts
	INTEnableSystemMultiVectoredInt();
		
	// Enable optimal performance
	SYSTEMConfigPerformance(FOSC);
	mOSCSetPBDIV(OSC_PB_DIV_1);				// Use 1:1 CPU Core:Peripheral clocks
	DelayMs(50);
	#if !defined(__MPLAB_DEBUGGER_PIC32MXSK) && !defined(__MPLAB_DEBUGGER_FS2)
		DDPCONbits.JTAGEN = 0;
	#endif

	// unused pins set to outputs
	PORTSetPinsDigitalOut(IOPORT_B, BIT_2 | BIT_11 | BIT_12);
	PORTSetPinsDigitalOut(IOPORT_D, BIT_0 | BIT_1 | BIT_2 | BIT_3 | \
		BIT_4 | BIT_5 | BIT_6 | BIT_7 | \
		BIT_11 | BIT_13 | BIT_14);
	PORTSetPinsDigitalOut(IOPORT_F, BIT_0 | BIT_1);

	// debug LED
    PORTSetPinsDigitalOut(IOPORT_F, BIT_1);
	PORTClearBits(IOPORT_F, BIT_1);

	// test mode input
	PORTSetPinsDigitalIn(IOPORT_B, BIT_13);

	// clock and reset inputs
	PORTSetPinsDigitalIn(IOPORT_D, BIT_8 | BIT_9);
	
	// MIDI port - UART2
	UARTConfigure(UART2, UART_ENABLE_PINS_TX_RX_ONLY);
    UARTSetFifoMode(UART2, UART_INTERRUPT_ON_RX_NOT_EMPTY);
	UARTSetLineControl(UART2, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
	UARTSetDataRate(UART2, PBCLOCK, 31250);
	UARTEnable(UART2, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));

	// configure timer - task timer
	OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_64, 312);
	ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_1);

    // enable interrupts
    INTDisableInterrupts();
    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);  // multi-vector mode
    INTSetVectorPriority(INT_TIMER_1_VECTOR, INT_PRIORITY_LEVEL_1);  // timer 1 prio 1
    INTSetVectorPriority(INT_EXTERNAL_1_VECTOR, INT_PRIORITY_LEVEL_1);  // INT1 prio 1
    INTSetVectorPriority(INT_EXTERNAL_2_VECTOR, INT_PRIORITY_LEVEL_1);  // INT2 prio 1
    INTSetVectorPriority(INT_UART_2_VECTOR, INT_PRIORITY_LEVEL_1);  // UART2 prio 1
	INTEnable(INT_SOURCE_TIMER(TMR1), INT_ENABLED);  // timer 1 interrupt
//	INTEnable(INT_SOURCE_EX_INT(1), INT_ENABLED);  // INT1 clock input
//	INTEnable(INT_SOURCE_EX_INT(2), INT_ENABLED);  // INT2 reset input
	ConfigINT1(EXT_INT_PRI_1 | RISING_EDGE_INT | EXT_INT_ENABLE);  // INT1 clock input
	ConfigINT2(EXT_INT_PRI_1 | RISING_EDGE_INT | EXT_INT_ENABLE);  // INT2 reset input
	INTEnable(INT_SOURCE_UART_RX(UART2), INT_ENABLED);  // USART2 RX interrupt

	rand_nommer_count = 255;

	// init modules
	srand(123456);
	eeprom_init();
	midi_init(0x42);  // K2579 device type
	cv_output_init();
	seq_midi_init();
	song_init();
	sequencer_init();
	lcd_init();
	screen_init();
	analog_input_init();
	mod_cv_input_init();
	clock_init();
	panel_init();
	sysconfig_init();  // this initializes things in previous modules
	song_file_init();  // this requires sysconfig to be initialized

	// startup delay
	DelayMs(100);
	gui_init();

	// bootup message
	screen_write_popup(1000, "K2579 SEQ", "ver. 1.02");
	DelayMs(100);
	
	// last thing before run
    INTEnableInterrupts();

	// main loop!
	while(1) {
		lcd_task();
		ClearWDT();
		midi_tx_task();
		ClearWDT();
	}
}

//
// INTERRUPT VECTORS
//
// timer interval - 256us interval
void __ISR(_TIMER_1_VECTOR, ipl1) Timer1Handler(void) {
	INTClearFlag(INT_T1);
	LATFbits.LATF1 = 1;
	// 16 ms
	if((count & 0x3f) == 0) {
		analog_input_task();
		mod_cv_input_task();
		gui_task();
		screen_task();
		sysconfig_task();
		song_file_task();
		if(rand_nommer_count) {
			rand_nommer_count --;
			if(rand_nommer_count == 0) {
				rand_nommer_count = (rand() & 0xff) | 0x01;
			}
		}
	}
	count ++;
	clock_task();
	midi_rx_task();
	panel_task();
	sequencer_task();
	LATFbits.LATF1 = 0;
}

// INT1 clock in interrupt
void __ISR(_EXTERNAL_1_VECTOR, ipl1) Int1Handler(void) {
	INTClearFlag(INT_INT1);
	clock_clock_input();
}

// INT2 reset in interrupt
void __ISR(_EXTERNAL_2_VECTOR, ipl1) Int2Handler(void) {
	INTClearFlag(INT_INT2);
	clock_reset_input();
}

// MIDI RX interrupt
void __ISR(_UART2_VECTOR, ipl1) IntUart2Handler(void) {
	// Is this an RX interrupt?
	if(INTGetFlag(INT_U2RX)) {
		INTClearFlag(INT_U2RX);
		midi_rx_byte(UARTGetDataByte(UART2));
		U2STAbits.OERR = 0;
	}
}
