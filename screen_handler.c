/*
 * K2579 Step Sequencer - Screen Handler
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 */
#include <stdio.h>
#include <string.h>
#include "screen_handler.h"
#include "lcd.h"
#include "midi.h"

//#define SYSEX_LCD_DEBUG

#define MAX_LINES 3
#define LINE_LEN 16

int popup_timer;
char lines[MAX_LINES][LINE_LEN + 1];

// local functions
void screen_write_raw(unsigned char line, char *str);

// initialize the screen handler
void screen_init(void) {
	popup_timer = 0;
	sprintf(lines[0], " ");
	sprintf(lines[1], " ");
	sprintf(lines[2], " ");
	lcd_clear_screen();
}

// call the screen task every 16ms
void screen_task(void) {
	if(popup_timer) {
		popup_timer -= 16;
		if(popup_timer <= 0) {
			popup_timer = 0;
			// restore old screen
			screen_write_raw(0, lines[0]);
			screen_write_raw(1, lines[1]);
//			screen_write_raw(2, lines[2]);
		}
	}
}

// write text to a line
void screen_write_line(unsigned char line, char *str) {
	if(line >= MAX_LINES) return;
	strncpy(lines[line], str, LINE_LEN);
	if(popup_timer) return;
	screen_write_raw(line, lines[line]);
}

// write a popup message
void screen_write_popup(int timeout, char *str1, char *str2) {
	if(strlen(str1) > LINE_LEN && strlen(str2) > LINE_LEN) return;
//	lcd_clear_screen();
	if(strlen(str1) > 0) screen_write_raw(0, str1);
	else screen_write_raw(0, lines[0]);
	if(strlen(str2) > 0) screen_write_raw(1, str2);
	else screen_write_raw(1, lines[1]);
	popup_timer = timeout;
}

// write to the display
void screen_write_raw(unsigned char line, char *str) {
	int i;
	int len = strlen(str);
	lcd_goto_xy(0, line);
	lcd_print_str(str);
	if(len < LINE_LEN) {
		for(i = len; i < LINE_LEN; i ++) {
			lcd_print_char(' ');
		}
	}
#ifdef SYSEX_LCD_DEBUG
	char debug_str[64];
	sprintf(debug_str, "%d%s", line, str);
	_midi_tx_debug(debug_str);
#endif
}

// get the contrast
unsigned char screen_get_contrast(void) {
	return lcd_get_contrast();
}

// adjust the screen contrast
void screen_set_contrast(unsigned char contrast) {
	lcd_set_contrast(contrast);
}

