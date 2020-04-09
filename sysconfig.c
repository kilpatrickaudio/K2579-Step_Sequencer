/*
 * K2579 Step Sequencer - System Config
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 * sysconfig page map:
 *  0 - clock div
 *  1 - mod1 assign
 *  2 - mod2 assign
 *  3 - live audition
 *  4 - midi part 1 channel		- remote
 *  5 - midi part 2 channel		- remote
 *  6 - key transpose
 *  7 - key trigger
 *  8 - LCD contrast			- remote
 *  9 - clock speed 			- remote
 * 10 - reset song / sequence
 * 11 - current loaded song
 * 31 - configured
 *
 */
#include "sysconfig.h"
#include "eeprom.h"
#include "seq_midi.h"
#include "screen_handler.h"
#include "clock.h"

#define EEPROM_CONFIG_ADDR 0x4000
#define EEPROM_CONFIG_MARK 0x55

// flag for resaving the page
unsigned char dirty;
unsigned int save_timer;
#define SYSCONFIG_SAVE_TIME 312

// parameters
unsigned char params[32];
#define PARAM_CLOCK_DIV 0
#define PARAM_MOD1_ASSIGN 1
#define PARAM_MOD2_ASSIGN 2
#define PARAM_LIVE_AUDITION 3
#define PARAM_MIDI_PT1_CHAN 4
#define PARAM_MIDI_PT2_CHAN 5
#define PARAM_KEY_TRANSPOSE 6
#define PARAM_KEY_TRIGGER 7
#define PARAM_LCD_CONTRAST 8
#define PARAM_CLOCK_SPEED 9
#define PARAM_RESET_MODE 10
#define PARAM_CURRENT_SONG 11
#define PARAM_KEY_MAP 12
#define PARAM_CONFIGURED 31

// init the global config
void sysconfig_init(void) {
	int i;
	for(i = 0; i < 32; i ++) {
		params[i] = 0x00;
	}

	// load config from EEPROM
	eeprom_read_page(EEPROM_CONFIG_ADDR, params);
	// force parameters that are remote
	sysconfig_set_midi_channel(0, params[PARAM_MIDI_PT1_CHAN]);
	sysconfig_set_midi_channel(1, params[PARAM_MIDI_PT2_CHAN]);
	sysconfig_set_lcd_contrast(params[PARAM_LCD_CONTRAST]);
	sysconfig_set_clock_speed(params[PARAM_CLOCK_SPEED]);
	dirty = 0;  // clear the dirty flag

	// should we seed this for the first time?
	if(params[PARAM_CONFIGURED] != EEPROM_CONFIG_MARK) {
		sysconfig_reset_all();
	}
	save_timer = 0;
}

// run the sysconfig task
void sysconfig_task(void) {
	save_timer ++;
	if(save_timer >= SYSCONFIG_SAVE_TIME) {
		save_timer = 0;
		if(!dirty) return;
		eeprom_write_page(EEPROM_CONFIG_ADDR, params);
		dirty = 0;
	}
}

// reset all settings
void sysconfig_reset_all(void) {
	int i;
	for(i = 0; i < 32; i ++) {
		params[i] = 0xff;
	}
	sysconfig_set_clock_div(6);
	sysconfig_set_mod_assign(0, SYSCONFIG_MOD_NONE);
	sysconfig_set_mod_assign(1, SYSCONFIG_MOD_NONE);
	sysconfig_set_live_aud(1);
	sysconfig_set_midi_channel(0, 0);
	sysconfig_set_midi_channel(1, 1);
	sysconfig_set_key_transpose(SYSCONFIG_KEY_TRANSPOSE12);
	sysconfig_set_key_trigger(0);
	sysconfig_set_lcd_contrast(160);
	sysconfig_set_clock_speed(100);
	sysconfig_set_reset_mode(SYSCONFIG_RESET_MODE_SONG);
	sysconfig_set_current_song(0);
	sysconfig_set_key_map(SYSCONFIG_KEY_MAP_A);
	params[PARAM_CONFIGURED] = EEPROM_CONFIG_MARK;
	dirty = 1;  // mark this for storing on the next pass
}

// get the clock divider
unsigned char sysconfig_get_clock_div(void) {
	return params[PARAM_CLOCK_DIV];
}

// set the clock divider
void sysconfig_set_clock_div(unsigned char div) {
	if(div < 1) {
		params[PARAM_CLOCK_DIV] = 1;
	}
	else if(div > SYSCONFIG_MAX_CLOCK_DIV) {
		params[PARAM_CLOCK_DIV] = SYSCONFIG_MAX_CLOCK_DIV;
	}
	else params[PARAM_CLOCK_DIV] = div;
	dirty = 1;
}

// get a mod assignment
unsigned char sysconfig_get_mod_assign(unsigned char part) {
	if(part) return params[PARAM_MOD2_ASSIGN];
	return params[PARAM_MOD1_ASSIGN];
}

// set a mod assignment
void sysconfig_set_mod_assign(unsigned char part, unsigned char assign) {
	if(part) {
		if(assign > SYSCONFIG_MAX_MOD_ASSIGN) {
			params[PARAM_MOD2_ASSIGN] = SYSCONFIG_MAX_MOD_ASSIGN;
		}
		else {
			params[PARAM_MOD2_ASSIGN] = assign;
		}
	}
	else {
		if(assign > SYSCONFIG_MAX_MOD_ASSIGN) {
			params[PARAM_MOD1_ASSIGN] = SYSCONFIG_MAX_MOD_ASSIGN;
		}
		else {
			params[PARAM_MOD1_ASSIGN] = assign;
		}
	}
	dirty = 1;
}

// get the live audition state
unsigned char sysconfig_get_live_aud(void) {
	return params[PARAM_LIVE_AUDITION];
}

// set the live audition state
void sysconfig_set_live_aud(unsigned char aud) {
	if(aud) params[PARAM_LIVE_AUDITION] = 1;
	else params[PARAM_LIVE_AUDITION] = 0;
	dirty = 1;
}

// get a midi part channel
unsigned char sysconfig_get_midi_channel(unsigned char part) {
	if(part > 1) return 0;
	if(part == 1) return seq_midi_get_channel(1);
	return seq_midi_get_channel(0);
}

// set a midi part channel
void sysconfig_set_midi_channel(unsigned char part, unsigned char channel) {
	if(part > 1) return;
	if(channel > 15) seq_midi_set_channel(part, 15);
	else seq_midi_set_channel(part, channel);
	if(part == 1) params[PARAM_MIDI_PT2_CHAN] = seq_midi_get_channel(1);
	else params[PARAM_MIDI_PT1_CHAN] = seq_midi_get_channel(0);
	dirty = 1;
}

// get the key transpose assignment
unsigned char sysconfig_get_key_transpose(void) {
	return params[PARAM_KEY_TRANSPOSE];
}

// set the key transpose assignment
void sysconfig_set_key_transpose(unsigned char assign) {
	if(assign > 3) params[PARAM_KEY_TRANSPOSE] = 3;
	else params[PARAM_KEY_TRANSPOSE] = assign;
	dirty = 1;
}

// get the key trigger mode
unsigned char sysconfig_get_key_trigger(void) {
	return params[PARAM_KEY_TRIGGER];
}

// set the key trigger mode
void sysconfig_set_key_trigger(unsigned char mode) {
	if(mode > 1) params[PARAM_KEY_TRIGGER] = SYSCONFIG_KEY_TRIGGER_MOM;
	else params[PARAM_KEY_TRIGGER] = mode;
	dirty = 1;
}

// get the screen contrast
unsigned char sysconfig_get_lcd_contrast(void) {
	return screen_get_contrast();
}

// set the screen contrast
void sysconfig_set_lcd_contrast(unsigned char contrast) {
	screen_set_contrast(contrast);
	params[PARAM_LCD_CONTRAST] = contrast;
	dirty = 1;
}

// get the clock speed
unsigned char sysconfig_get_clock_speed(void) {
	return clock_get_speed();
}

// set the clock speed
void sysconfig_set_clock_speed(unsigned char speed) {
	clock_set_speed(speed);
	params[PARAM_CLOCK_SPEED] = speed;
	dirty = 1;
}

// get the reset mode
unsigned char sysconfig_get_reset_mode(void) {
	return params[PARAM_RESET_MODE];
}

// set the reset mode
void sysconfig_set_reset_mode(unsigned char reset_mode) {
	params[PARAM_RESET_MODE] = reset_mode;
	dirty = 1;
}

// get the current song
unsigned char sysconfig_get_current_song(void) {
	return params[PARAM_CURRENT_SONG];
}

// set the current song
void sysconfig_set_current_song(unsigned char current_song) {
	params[PARAM_CURRENT_SONG] = current_song;
	dirty = 1;
}

// get the key map
unsigned char sysconfig_get_key_map(void) {
	return params[PARAM_KEY_MAP];
}

// set the key map
void sysconfig_set_key_map(unsigned char key_map) {
	params[PARAM_KEY_MAP] = key_map;
	dirty = 1;
}

