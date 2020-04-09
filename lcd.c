/*
 * LCD Driver for EA DIPS082 / EA DOGM163W-A - PIC32 
 *
 * Copyright 2011: Kilpatrick Audio
 * Writeen by: Andrew Kilpatrick
 *
 * Hardware I/O:
 *
 * // 4 bit mode:
 * // RD0			- LCD D0					- output
 * // RD1			- LCD D1					- output
 * // RD2			- LCD D2					- output
 * // RD3			- LCD D3					- output
 * // RD4			- LCD E						- output
 * // RD5			- LCD R/!W					- output
 * // RD6			- LCD RS					- output
 *
 * SPI mode:
 *  RE7			- LCD serial RS				- output
 *	RG6			- LCD SPI clock				- SPI2 clock
 *	RG7			- LCD SPI MISO				- SPI2 MISO (unused)
 *	RG8			- LCD SPI MOSI				- SPI2 MOSI  
 *	RG9			- LCD SPI /SS				- SPI2 /SS
 *
 */
#include <plib.h>
#include "TimeDelay.h"
#include "lcd.h"

// LCD panel selection
//#define LCD_4BIT
#define LCD_SPI
#define LCD_16X3
//#define LCD_8X2

// hardware defines
#define LCD_PORT LATD
#define LCD_E LATDbits.LATD4
#define LCD_RW LATDbits.LATD5
#define LCD_RS LATDbits.LATD6
#define LCD_SRS LATEbits.LATE7
#define LCD_SS LATGbits.LATG9

// commands
#define LCD_CLEAR_SCREEN 0x80
#define LCD_GOTO_XY 0x81
#define LCD_SHIFT_LEFT 0x82
#define LCD_SHIFT_RIGHT 0x83
#define LCD_CONTRAST 0x84

// control state machine
#define CMD_MODE_NORMAL 0
#define CMD_MODE_XY_X 1
#define CMD_MODE_XY_Y 2
#define CMD_MODE_CONTRAST 3
unsigned char cmd_mode;
unsigned char temp_x;
unsigned char temp_y;

unsigned char contrast;

// local functions
void lcd_write_cmd(unsigned char spi, unsigned char cmd);
void lcd_write_data(unsigned char spi, unsigned char data);
void lcd_write(unsigned char spi, unsigned char data);

unsigned char cmd_buf[256];
unsigned char cmd_buf_in;
unsigned char cmd_buf_out;

// initialize the display
void lcd_init(void) {
	DelayMs(100);	
	cmd_buf_in = 0;
	cmd_buf_out = 0;
	cmd_mode = CMD_MODE_NORMAL;
	contrast = 0x0e;

#ifdef LCD_4BIT
	// LCD pins
	PORTSetPinsDigitalOut(IOPORT_D, BIT_0 | BIT_1 | BIT_2 | \
		BIT_3 | BIT_4 | BIT_5 | BIT_6);
	LCD_E = 0;
	LCD_RW = 0;

	// init 4 bit mode
	lcd_write_cmd(0, 0x33);
	lcd_write_cmd(0, 0x32);
	lcd_write_cmd(0, 0x29);

	// init the display
	lcd_write_cmd(0, 0x15);  // bias set - fixed on high
	lcd_write_cmd(0, 0x53);  // power control - internal follower
	lcd_write_cmd(0, 0x6c);  // follower control - on, rab2
	lcd_write_cmd(0, 0x7f);  // contrast set
	lcd_write_cmd(0, 0x28);  // instruction set 0 - 4 bit, 2 line, normal instruction
	lcd_write_cmd(0, 0x0c);  // display on - display on, cursor off, blink off
//	lcd_write_cmd(0, 0x0f);  // display on - display on, cursor on, blink on
	lcd_write_cmd(0, 0x01);  // clear display
	DelayMs(2);
	lcd_write_cmd(0, 0x06);  // entry mode - increment on, shift off
#endif

#ifdef LCD_SPI
	PORTSetPinsDigitalOut(IOPORT_E, BIT_7);
	PORTSetPinsDigitalOut(IOPORT_G, BIT_9);
	LCD_SRS = 1;
	LCD_SS = 1;

	SpiChnOpen(SPI_CHANNEL2, SPI_OPEN_MSTEN | SPI_OPEN_SMP_END | SPI_OPEN_MODE8, 32);
	DelayMs(2);
	lcd_write_cmd(1, 0x38);  // function set
	lcd_write_cmd(1, 0x39);  // function set
	lcd_write_cmd(1, 0x15);  // bias set - 1/5, 3 line LCD
	lcd_write_cmd(1, 0x7e);  // contrast set
	lcd_write_cmd(1, 0x54);  // power control - booster on, contrast C5, set C4
	lcd_write_cmd(1, 0x6e);  // follower control - set voltage follower and gain
	DelayMs(250);
	lcd_write_cmd(1, 0x38);  // instruction set 0
	lcd_write_cmd(1, 0x0c);  // display on - display on, cursor off, blink off
//	lcd_write_cmd(1, 0x0f);  // display on - display on, cursor on, blink on
	lcd_write_cmd(1, 0x01);  // clear display
	DelayMs(2);
	lcd_write_cmd(1, 0x06);  // entry mode - increment on, shift off

#endif

}

// handle LCD writes
void lcd_task(void) {
	unsigned char cmd;
	if(cmd_buf_in == cmd_buf_out) return;

	// get a new command
	INTDisableInterrupts();
	cmd = cmd_buf[cmd_buf_out];
	cmd_buf_out ++;
	INTEnableInterrupts();

	// handle multi-byte commands
	if(cmd_mode == CMD_MODE_XY_X) {
		temp_x = cmd;
		cmd_mode = CMD_MODE_XY_Y;
		return;
	}
	if(cmd_mode == CMD_MODE_XY_Y) {
		temp_y = cmd;
#ifdef LCD_SPI
#ifdef LCD_16X3
		lcd_write_cmd(1, 0x80 | ((temp_y & 0x03) << 4) | (temp_x & 0x04));
#elif LCD_8x2
		lcd_write_cmd(1, 0x80 | ((temp_y & 0x01) << 6) | (temp_x & 0x03));
#endif
#endif
#ifdef LCD_4BIT
#ifdef LCD_16X3
		lcd_write_cmd(0, 0x80 | ((temp_y & 0x03) << 4) | (temp_x & 0x04));
#elif LCD_8X2
		lcd_write_cmd(0, 0x80 | ((temp_y & 0x01) << 6) | (temp_x & 0x03));
#endif
#endif
		cmd_mode = CMD_MODE_NORMAL;
		return;
	}
	if(cmd_mode == CMD_MODE_CONTRAST) {
#ifdef LCD_SPI
		lcd_write_cmd(1, 0x21);  // instruction table 1
		lcd_write_cmd(1, 0x54 | ((cmd & 0x30) >> 4));
		lcd_write_cmd(1, 0x70 | (cmd & 0x0f));
		lcd_write_cmd(1, 0x20);  // instruction table 0
#endif
#ifdef LCD_4BIT
		lcd_write_cmd(0, 0x0c);  // display on - display on, cursor off, blink off
		lcd_write_cmd(0, 0x54 | ((cmd & 0x30) >> 4));
		lcd_write_cmd(0, 0x70 | (cmd & 0x0f));
		lcd_write_cmd(0, 0x0c);  // instruction table 0
#endif
		cmd_mode = CMD_MODE_NORMAL;
		return;
	}

	// commands
	if(cmd & 0x80) {
		if(cmd == LCD_CLEAR_SCREEN) {
#ifdef LCD_SPI
			lcd_write_cmd(1, 0x01);  // clear display
			DelayMs(2);
			lcd_write_cmd(1, 0x02);  // return home
			DelayMs(2);
#endif
#ifdef LCD_4BIT
			lcd_write_cmd(0, 0x01);  // clear display
			DelayMs(2);
			lcd_write_cmd(0, 0x02);  // return home
			DelayMs(2);
#endif
		}
		else if(cmd == LCD_GOTO_XY) {
			cmd_mode = CMD_MODE_XY_X;
		}
		else if(cmd == LCD_CONTRAST) {
			cmd_mode = CMD_MODE_CONTRAST;
		}
		else if(cmd == LCD_SHIFT_LEFT) {
#ifdef LCD_SPI
			lcd_write_cmd(1, 0x18);
#endif
#ifdef LCD_4BIT
			lcd_write_cmd(0, 0x18);
#endif
		}
		else if(cmd == LCD_SHIFT_RIGHT) {
#ifdef LCD_SPI
			lcd_write_cmd(1, 0x1c);	
#endif
#ifdef LCD_4BIT
			lcd_write_cmd(0, 0x1c);	
#endif
		}
	}
	// print data
	else {
#ifdef LCD_SPI
		lcd_write_data(1, cmd);
#endif
#ifdef LCD_4BIT
		lcd_write_data(0, cmd);
#endif
	}
}

// clear the screen
void lcd_clear_screen(void) {
	cmd_buf[cmd_buf_in] = LCD_CLEAR_SCREEN;
	cmd_buf_in ++;
}

// go to X / Y position
void lcd_goto_xy(unsigned char x, unsigned char y) {
	cmd_buf[cmd_buf_in] = LCD_GOTO_XY;
	cmd_buf_in ++;
	cmd_buf[cmd_buf_in] = (x & 0x7f);
	cmd_buf_in ++;
	cmd_buf[cmd_buf_in] = (y & 0x7f);
	cmd_buf_in ++;
}

// shift the display to the left
void lcd_shift_left(void) {
	cmd_buf[cmd_buf_in] = LCD_SHIFT_LEFT;
	cmd_buf_in ++;
}

// shift the display to the right
void lcd_shift_right(void) {
	cmd_buf[cmd_buf_in] = LCD_SHIFT_RIGHT;
	cmd_buf_in ++;
}

// print a string at the current position
void lcd_print_str(char *str) {
	while(*str) {
		cmd_buf[cmd_buf_in] = *str;
		cmd_buf_in ++;
		*str++;
	}
}

// print a character at the current position
void lcd_print_char(char ch) {
	cmd_buf[cmd_buf_in] = ch;
	cmd_buf_in ++;
}

// get the contrast
unsigned char lcd_get_contrast(void) {
	return contrast;
}

// set the contrast - for digitally controlled contrast only
void lcd_set_contrast(unsigned char cont) {
	contrast = cont;
	cmd_buf[cmd_buf_in] = LCD_CONTRAST;
	cmd_buf_in ++;
	cmd_buf[cmd_buf_in] = (contrast >> 4) + 6;;
	cmd_buf_in ++;
}

//
// HARDWARE CONTROL
//
// write a command to the display
void lcd_write_cmd(unsigned char spi, unsigned char cmd) {
	if(spi) LCD_SRS = 0;
	else LCD_RS = 0;
	lcd_write(spi, cmd);
}


// write data to the display
void lcd_write_data(unsigned char spi, unsigned char data) {
	if(spi) LCD_SRS = 1;
	else LCD_RS = 1;
	lcd_write(spi, data);
}

// write to the display
void lcd_write(unsigned char spi, unsigned char data) {
	if(spi) {
		LCD_SS = 0;
		SpiChnPutC(SPI_CHANNEL2, data);
		while(SpiChnIsBusy(SPI_CHANNEL2)) ClearWDT();
		Delay10us(1);
		LCD_SS = 1;		
	}
	else {
		// high nibble
		unsigned char nibble = (data & 0xf0) >> 4;
		LCD_PORT &= 0xfff0;
		LCD_PORT |= nibble;
		Delay10us(1);
		LCD_E = 1;
		Delay10us(1);
		LCD_E = 0;
		
		// low nibble
		nibble = data & 0x0f;
		LCD_PORT &= 0xfff0;
		LCD_PORT |= nibble;
		Delay10us(1);
		LCD_E = 1;
		Delay10us(1);
		LCD_E = 0;
	}
	Delay10us(3);
}

