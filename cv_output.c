/*
 * K2579 Step Sequencer - CV Output
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 *  RB14		- gate 1 out				- output - active high
 *  RB15		- gate 2 out				- output - active high
 *
 *  RD10		- DAC !SS					- SPI1 chip select
 *
 *  RF2/SDI1	- not used					- SPI1 MISO (unused)
 *  RF3/SDO1	- DAC MOSI					- SPI1 MOSI
 *  RF6/SCK1	- DAC SCLK					- SPI1 clock
 *
 */
#include <plib.h>
#include "cv_output.h"
#include "note_lookup.h"
#include "TimeDelay.h"
#include "panel.h"

// hardware defines
#define DAC_SS LATDbits.LATD10
#define GATE1_OUT LATBbits.LATB14
#define GATE2_OUT LATBbits.LATB15

// intialize the CV output
void cv_output_init(void) {
	// gate outputs
	PORTSetPinsDigitalOut(IOPORT_B, BIT_14 | BIT_15);

	// DAC !SS output
	PORTSetPinsDigitalOut(IOPORT_D, BIT_10);

	DAC_SS = 1;
	GATE1_OUT = 0;
	GATE2_OUT = 0;

	// DAC SPI
	SpiChnOpen(SPI_CHANNEL1, SPI_OPEN_MSTEN | SPI_OPEN_CKP_HIGH | \
		SPI_OPEN_MODE32, 32);

	cv_output_note_on(0, 24);
	cv_output_note_on(1, 24);
	DelayMs(10);
	cv_output_note_off(0);
	cv_output_note_off(1);
}

// start a note on the CV output
void cv_output_note_on(unsigned char part, unsigned char note) {
	if(note > 72) return;
	unsigned int data = 0x300000;
	if(part) data = 0x310000;
	data |= (note_lookup[note] << 4) & 0xffff;

	// write to the DAC
	DAC_SS = 0;
	SpiChnPutC(SPI_CHANNEL1, data);
	while(SpiChnIsBusy(SPI_CHANNEL1)) ClearWDT();
	Delay10us(1);
	DAC_SS = 1;	
	panel_set_cv_gate_led(part, 255);

	if(part) GATE2_OUT = 1;
	else GATE1_OUT = 1;
}

// stop a note on the CV output
void cv_output_note_off(unsigned char part) {
	panel_set_cv_gate_led(part, 0);

	if(part) GATE2_OUT = 0;
	else GATE1_OUT = 0;
}




