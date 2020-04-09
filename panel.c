/*
 * K2579 Step Sequencer - Panel
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 * Hardware I/O:
 *
 *  RB0/AN0		- step 1 pot				- analog (via analog lib)
 *  RB1/AN1		- step 2 pot				- analog (via analog lib)
 *  RB3			- mode switch				- input - active low
 *  RB4			- enter switch				- input - active low
 *  RB5			- page down switch			- input - active low	
 *  RB10		- page up switch			- input - active low
 *
 *  RE0			- clock LED					- output - active high
 *  RE1			- CV/gate LED 1				- output - active high
 *  RE2			- CV/gate LED 2				- output - active high
 *	RE3			- MOD LED					- output - active high
 *	RE4			- live switch				- input - active low
 *	RE5			- run/stop switch			- input - active low
 *	RE6			- reset switch				- input - active low
 *
 */
#include <plib.h>
#include "panel.h"
#include "analog_input.h"

// hardware defines
#define MODE_SW PORTBbits.RB3
#define ENTER_SW PORTBbits.RB4
#define PAGE_DOWN_SW PORTBbits.RB5
#define PAGE_UP_SW PORTBbits.RB10
#define LIVE_SW PORTEbits.RE4
#define RUN_STOP_SW PORTEbits.RE5
#define RESET_SW PORTEbits.RE6

#define CLOCK_LED LATEbits.LATE0
#define CV_GATE_LED1 LATEbits.LATE1
#define CV_GATE_LED2 LATEbits.LATE2
#define MOD_LED LATEbits.LATE3

#define LOCKOUT_TIME 50

unsigned char keyq[16];
unsigned char keyq_inp;
unsigned char keyq_outp;
#define KEYQ_IN_INC keyq_inp = (keyq_inp + 1) & 0x0f

unsigned char lcd_contrast;
unsigned char mode_sw_lockout;
unsigned char enter_sw_lockout;
unsigned char page_down_sw_lockout;
unsigned char page_up_sw_lockout;
unsigned char live_sw_lockout;
unsigned char run_stop_sw_lockout;
unsigned char reset_sw_lockout;
unsigned char panel_phase;
unsigned char clock_led_timeout;
unsigned char mod_led_timeout;
unsigned char cv_gate_led_timeout1;
unsigned char cv_gate_led_timeout2;
char param1_pot;
char param2_pot;

// initialize the panel
void panel_init(void) {
	// switch inputs
	PORTSetPinsDigitalIn(IOPORT_B, BIT_3 | BIT_4 | BIT_5 | BIT_10);
	PORTSetPinsDigitalIn(IOPORT_E, BIT_4 | BIT_5 | BIT_6);

	// LED outputs
	PORTSetPinsDigitalOut(IOPORT_E, BIT_0 | BIT_1 | BIT_2 | BIT_3);

	// reset stuff
	keyq_inp = 0;
	keyq_outp = 0;
	lcd_contrast = 0;
	mode_sw_lockout = 0;
	enter_sw_lockout = 0;
	page_down_sw_lockout = 0;
	page_up_sw_lockout = 0;
	live_sw_lockout = 0;
	run_stop_sw_lockout = 0;
	reset_sw_lockout = 0;
	clock_led_timeout = 0;
	mod_led_timeout = 0;
	cv_gate_led_timeout1 = 0;
	cv_gate_led_timeout2 = 0;
}

// panel task - call this every 256us
void panel_task(void) {
	//
	// decode switches
	//
	if(!MODE_SW && !mode_sw_lockout) {
		mode_sw_lockout = LOCKOUT_TIME;
		KEYQ_IN_INC;
		keyq[keyq_inp] = PANEL_MODE_SW;
	}
	else if(mode_sw_lockout && MODE_SW) {
		mode_sw_lockout --;
	}

	if(!ENTER_SW && !enter_sw_lockout) {
		enter_sw_lockout = LOCKOUT_TIME;
		KEYQ_IN_INC;
		keyq[keyq_inp] = PANEL_ENTER_SW;
	}
	else if(enter_sw_lockout && ENTER_SW) {
		enter_sw_lockout --;
	}

	if(!PAGE_DOWN_SW && !page_down_sw_lockout) {
		page_down_sw_lockout = LOCKOUT_TIME;
		KEYQ_IN_INC;
		keyq[keyq_inp] = PANEL_PAGE_DOWN_SW;
	}
	else if(page_down_sw_lockout && PAGE_DOWN_SW) {
		page_down_sw_lockout --;
	}

	if(!PAGE_UP_SW && !page_up_sw_lockout) {
		page_up_sw_lockout = LOCKOUT_TIME;
		KEYQ_IN_INC;
		keyq[keyq_inp] = PANEL_PAGE_UP_SW;
	}
	else if(page_up_sw_lockout && PAGE_UP_SW) {
		page_up_sw_lockout --;
	}

	if(!LIVE_SW && !live_sw_lockout) {
		live_sw_lockout = LOCKOUT_TIME;
		KEYQ_IN_INC;
		keyq[keyq_inp] = PANEL_LIVE_SW;
	}
	else if(live_sw_lockout && LIVE_SW) {
		live_sw_lockout --;
	}

	if(!RUN_STOP_SW && !run_stop_sw_lockout) {
		run_stop_sw_lockout = LOCKOUT_TIME;
		KEYQ_IN_INC;
		keyq[keyq_inp] = PANEL_RUN_STOP_SW;
	}
	else if(run_stop_sw_lockout && RUN_STOP_SW) {
		run_stop_sw_lockout --;
	}

	if(!RESET_SW && !reset_sw_lockout) {
		reset_sw_lockout = LOCKOUT_TIME;
		KEYQ_IN_INC;
		keyq[keyq_inp] = PANEL_RESET_SW;
	}
	else if(reset_sw_lockout && RESET_SW) {
		reset_sw_lockout --;
	}

	// every 16 ms
	if(panel_phase == 0) {
		// clock LED
		if(clock_led_timeout) {
			CLOCK_LED = 1;
			if(clock_led_timeout < 255) clock_led_timeout --;
		}
		else {
			CLOCK_LED = 0;
		}
		// mod LED
		if(mod_led_timeout) {
			MOD_LED = 1;
			if(mod_led_timeout < 255) mod_led_timeout --;
		}
		else {
			MOD_LED = 0;
		}
		// CV/GATE 1 LED
		if(cv_gate_led_timeout1) {
			CV_GATE_LED1 = 1;
			if(cv_gate_led_timeout1 < 255) cv_gate_led_timeout1 --;
		}
		else {
			CV_GATE_LED1 = 0;
		}
		// CV/GATE 2 LED
		if(cv_gate_led_timeout2) {
			CV_GATE_LED2 = 1;
			if(cv_gate_led_timeout2 < 255) cv_gate_led_timeout2 --;
		}
		else {
			CV_GATE_LED2 = 0;
		}
	}

	// LCD contrast?
	if(mode_sw_lockout == LOCKOUT_TIME && enter_sw_lockout == LOCKOUT_TIME) {
		lcd_contrast = 1;
	}

	// step pots
	param1_pot = (analog_input_get_val(ANA_PARAM1_POT) >> 2);
	param2_pot = (analog_input_get_val(ANA_PARAM2_POT) >> 2);

	panel_phase = (panel_phase + 1) & 0x3f;
}

// get the next switch in the queue
unsigned char panel_get_sw(void) {
	unsigned char temp;
	if(keyq_inp == keyq_outp) return 0;	
	keyq_outp = (keyq_outp + 1) & 0x0f;
	temp = keyq[keyq_outp];
	return temp;
}

// set the clock LED
void panel_set_clock_led(unsigned char timeout) {
	clock_led_timeout = timeout;
}

// set the mod LED
void panel_set_mod_led(unsigned char timeout) {
	mod_led_timeout = timeout;
}

// set a CV/gate LED
void panel_set_cv_gate_led(unsigned char chan, unsigned char timeout) {
	if(chan > 1) return;
	if(chan) cv_gate_led_timeout2 = timeout;
	else cv_gate_led_timeout1 = timeout;
}

// get a step pot
int panel_get_step_pot(unsigned char pot) {
	if(pot == 0) return param1_pot;
	if(pot == 1) return param2_pot;
	return 0;
}

// get whether the LCD contrast magic button press has taken place
unsigned char panel_set_lcd_contrast(void) {
	if(lcd_contrast) {
		lcd_contrast = 0;
		return 1;
	}
	return 0;
}

