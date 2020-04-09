/*
 * K2579 Step Sequencer - User Interface
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 */
#include <stdio.h>
#include <string.h>
#include "gui.h"
#include "panel.h"
#include "song.h"
#include "song_file.h"
#include "sequencer.h"
#include "eeprom.h"
#include "scale.h"
#include "sysconfig.h"
#include "clock.h"
#include "screen_handler.h"

// menu modes
char menu_mode;
#define MENU_SEQ 0
#define MENU_PART1 1
#define MENU_PART2 2
#define MENU_SYSTEM 3
#define MENU_MAX_MENU 3
char live_menu_override;

// main page
char seq_page;
#define SEQ_START 0
#define SEQ_LEN 1
#define SEQ_STEP_LEN 2
#define SEQ_LOOP 3
#define SEQ_DIR 4
#define SEQ_NEXT 5
#define SEQ_CPY 6
#define SEQ_CLR 7
#define SEQ_MAX_PAGE 7

// part pages
char part1_page;
char part2_page;
#define PART_NOTE_SET 0
#define PART_GATE_LEN 1
#define PART_SCALE 2
#define PART_SPAN 3
#define PART_OFFSET 4
#define PART_COPY 5
#define PART_TRANS 6
#define PART_MAX_PAGE 6

// system page
char system_page;
#define SYSTEM_SONG_LOAD 0
#define SYSTEM_SONG_SAV 1
#define SYSTEM_SONG_CLR 2
#define SYSTEM_CONTROL_CANCEL 3
#define SYSTEM_CLK_DIV 4
#define SYSTEM_RESET_MODE 5
#define SYSTEM_MOD1_ASSN 6
#define SYSTEM_MOD2_ASSN 7
#define SYSTEM_LIVE_AUD 8
#define SYSTEM_MIDI_PT1 9
#define SYSTEM_MIDI_PT2 10
#define SYSTEM_KEY_TRANSPOSE 11
#define SYSTEM_KEY_TRIGGER 12
#define SYSTEM_KEY_MAP 13
#define SYSTEM_LCD_CONT 14
#define SYSTEM_CV_CAL 15
#define SYSTEM_FACTORY_RESET 16
#define SYSTEM_MAX_PAGE 16

// live page
char live_page;
#define LIVE_PLAY 0
#define LIVE_SEQ 1
#define LIVE_STEP 2
#define LIVE_MAX_PAGE 2

char current_edit_seq;
char str[256];
char temp;
char temp2;
unsigned char utemp;
unsigned char utemp2;
unsigned char pot1_val;
unsigned char pot2_val;

// event flags
unsigned char startup_delay;
unsigned char playback_updated;
unsigned char control_override_updated;
unsigned char song_load_updated;
unsigned char song_save_updated;

// UI events
#define EVENT_NONE 0
#define EVENT_PAGE_UP 1
#define EVENT_PAGE_DOWN 2
#define EVENT_PAGE_CLICK 3
#define EVENT_ENTER_CLICK 4
#define EVENT_POT1_CHANGE 5
#define EVENT_POT2_CHANGE 6
#define EVENT_REFRESH 7

//
// local functions
//
// page mode increment
void gui_mode_inc(void);
void gui_mode_live(void);
// page dispatchers
void gui_set_seq(char evnt);
void gui_set_part(char evnt);
void gui_set_system(char evnt);
void gui_set_live(char evnt);
//
// page handlers
//
// global
void gui_play_bar(void);
// sequence
void gui_seq_start(char event);
void gui_seq_len(char event);
void gui_seq_step_len(char event);
void gui_seq_loop(char event);
void gui_seq_dir(char event);
void gui_seq_next(char event);
void gui_seq_cpy(char event);
void gui_seq_clr(char event);
// part
void gui_part_note_set(char event);
void gui_part_gate_len(char event);
void gui_part_scale(char event);
void gui_part_span(char event);
void gui_part_offset(char event);
void gui_part_copy(char event);
void gui_part_trans(char event);
// system
void gui_system_song_load(char event);
void gui_system_song_sav(char event);
void gui_system_song_clr(char event);
void gui_system_midi_control_cancel(char event);
void gui_system_clock_div(char event);
void gui_system_reset_mode(char event);
void gui_system_mod1_assign(char event);
void gui_system_mod2_assign(char event);
void gui_system_live_audition(char event);
void gui_system_midi_pt1(char event);
void gui_system_midi_pt2(char event);
void gui_system_key_transpose(char event);
void gui_system_key_trigger(char event);
void gui_system_key_map(char event);
void gui_system_lcd_cont(char event);
void gui_system_cv_cal(char event);
void gui_system_system_reset(char event);
// live
void gui_live_play(char event);
void gui_live_seq(char event);
void gui_live_step(char event);

// initialize the GUI
void gui_init(void) {
	menu_mode = MENU_SEQ;
	live_menu_override = 0;
	seq_page = SEQ_START;
	part1_page = PART_NOTE_SET;
	part2_page = PART_NOTE_SET;
	system_page = SYSTEM_SONG_LOAD;
	live_page = LIVE_PLAY;
	gui_set_seq(EVENT_REFRESH);
	gui_play_bar();

	// events
	startup_delay = 10;
	playback_updated = 0;
	control_override_updated = 0;
	song_load_updated = 0;
	song_save_updated = 0;
}

// run this every 16ms
void gui_task(void) {
	unsigned char key;
	unsigned char val;

	// prevent panel controls from affecting system at startup
	if(startup_delay) {
		key = panel_get_sw();
		pot1_val = panel_get_step_pot(0);
		pot2_val = panel_get_step_pot(1);
		startup_delay --;
		return;
	}

	//
	// PANEL CONTROLS
	// 
	key = panel_get_sw();
	if(key == PANEL_MODE_SW) {
 		gui_mode_inc();
	}
	else if(key == PANEL_ENTER_SW) {
		if(live_menu_override) {
			gui_set_live(EVENT_ENTER_CLICK);
		}
		else {
			// sequence
			if(menu_mode == MENU_SEQ) {
				gui_set_seq(EVENT_ENTER_CLICK);
			}
			// part 1/2
			else if(menu_mode == MENU_PART1 ||
					menu_mode == MENU_PART2) {
				gui_set_part(EVENT_ENTER_CLICK);
			}
			// system
			else if(menu_mode == MENU_SYSTEM) {
				gui_set_system(EVENT_ENTER_CLICK);
			}
		}
	}
	else if(key == PANEL_PAGE_UP_SW) {
		if(live_menu_override) {
			gui_set_live(EVENT_PAGE_UP);
		}
		else {
			// system
			if(menu_mode == MENU_SEQ) {
				gui_set_seq(EVENT_PAGE_UP);
			}
			// part 1/2
			else if(menu_mode == MENU_PART1 ||
					menu_mode == MENU_PART2) {
				gui_set_part(EVENT_PAGE_UP);
			}
			// system
			else if(menu_mode == MENU_SYSTEM) {
				gui_set_system(EVENT_PAGE_UP);
			}
		}	
	}
	else if(key == PANEL_PAGE_DOWN_SW) {
		if(live_menu_override) {
				gui_set_live(EVENT_PAGE_DOWN);
		}
		else {
			// system
			if(menu_mode == MENU_SEQ) {
				gui_set_seq(EVENT_PAGE_DOWN);
			}
			// part 1/2
			else if(menu_mode == MENU_PART1 ||
					menu_mode == MENU_PART2) {
				gui_set_part(EVENT_PAGE_DOWN);
			}
			// system
			else if(menu_mode == MENU_SYSTEM) {
				gui_set_system(EVENT_PAGE_DOWN);
			}
		}
	}
	else if(key == PANEL_LIVE_SW) {
		gui_mode_live();
	}
	else if(key == PANEL_RUN_STOP_SW) {
		clock_run_stop_toggle();
	}
	else if(key == PANEL_RESET_SW) {
		clock_reset_input();
	}

	// value pots
	val = panel_get_step_pot(0);
	if(val != pot1_val) {
		pot1_val = val;
		if(live_menu_override) {
			gui_set_live(EVENT_POT1_CHANGE);
		}
		else {
			// system
			if(menu_mode == MENU_SEQ) {
				gui_set_seq(EVENT_POT1_CHANGE);
			}
			// part 1/2
			else if(menu_mode == MENU_PART1 ||
					menu_mode == MENU_PART2) {
				gui_set_part(EVENT_POT1_CHANGE);
			}
			// system
			else if(menu_mode == MENU_SYSTEM) {
				gui_set_system(EVENT_POT1_CHANGE);
			}
		}
	}

	val = panel_get_step_pot(1);
	if(val != pot2_val) {
		pot2_val = val;
		if(live_menu_override) {
			gui_set_live(EVENT_POT2_CHANGE);
		}
		else {
			// system
			if(menu_mode == MENU_SEQ) {
				gui_set_seq(EVENT_POT2_CHANGE);
			}
			// part 1/2
			else if(menu_mode == MENU_PART1 ||
					menu_mode == MENU_PART2) {
				gui_set_part(EVENT_POT2_CHANGE);
			}
			// system
			else if(menu_mode == MENU_SYSTEM) {
				gui_set_system(EVENT_POT2_CHANGE);
			}
		}
	}

	// check if we're asking for LCD contrast set
	if(panel_set_lcd_contrast()) {
		menu_mode = MENU_SYSTEM;
		system_page = SYSTEM_LCD_CONT;
		gui_set_system(EVENT_REFRESH);
	}

	//
	// UPDATE EVENTS
	//
	if(playback_updated) {
		gui_play_bar();
		playback_updated = 0;
	}

	if(control_override_updated) {
		if(live_menu_override) {
			gui_set_live(EVENT_NONE);
		}
		else if(menu_mode == MENU_SYSTEM && system_page == SYSTEM_KEY_MAP) {
			gui_set_system(EVENT_REFRESH);
		}
		control_override_updated = 0;
	}

	if(song_load_updated) {
		// current song name
		if(!live_menu_override) {
			if(menu_mode == MENU_SEQ) {
				gui_set_seq(EVENT_REFRESH);
			}
			else if(menu_mode == MENU_SYSTEM) {
				gui_set_system(EVENT_REFRESH);
			}
		}
		sprintf(str, "song loaded %02d", (sysconfig_get_current_song() + 1));
		screen_write_popup(2000, "", str);
		song_load_updated = 0;
	}

	if(song_save_updated) {
		// current song name
		if(!live_menu_override) {
			if(menu_mode == MENU_SEQ) {
				gui_set_seq(EVENT_REFRESH);
			}
			else if(menu_mode == MENU_SYSTEM) {
				gui_set_system(EVENT_REFRESH);
			}
		}
		sprintf(str, "song saved %02d", (sysconfig_get_current_song() + 1));
		screen_write_popup(2000, "", str);
		song_save_updated = 0;
	}
}

// notify the GUI that a playback value has changed
void gui_playback_updated(void) {
	playback_updated = 1;
}

// notify the GUI that a control override has changed
void gui_control_override_updated(void) {
	control_override_updated = 1;
}

// notify the GUI that a song has just been loaded
void gui_song_load_updated(void) {
	song_load_updated = 1;
}

// notify the GUI that a song has just been saved
void gui_song_save_updated(void) {
	song_save_updated = 1;
}

// change menu modes
void gui_mode_inc(void) {
	// cancel live mode without advancing to a new menu
	if(live_menu_override) {
		live_menu_override = 0;
	}
	else {
		menu_mode ++;
		if(menu_mode > MENU_MAX_MENU) menu_mode = 0;
	}
	if(menu_mode == MENU_SEQ) {
		gui_set_seq(EVENT_REFRESH);
	}
	else if(menu_mode == MENU_PART1) {
		gui_set_part(EVENT_REFRESH);
	}
	else if(menu_mode == MENU_PART2) {
		gui_set_part(EVENT_REFRESH);
	}
	else if(menu_mode == MENU_SYSTEM) {
		gui_set_system(EVENT_REFRESH);
	}
}

// change to the live mode
void gui_mode_live(void) {
	live_menu_override = 1;
	gui_set_live(EVENT_REFRESH);
}

// set the seq page
void gui_set_seq(char evnt) {
	char event = evnt;
	if(menu_mode != MENU_SEQ) return;

	// change pages
	if(event == EVENT_PAGE_UP) {
		seq_page ++;
		if(seq_page > SEQ_MAX_PAGE) seq_page = SEQ_MAX_PAGE;
		event = EVENT_REFRESH;
	}
	else if(event == EVENT_PAGE_DOWN) {
		seq_page --;
		if(seq_page < 0) seq_page = 0;
		event = EVENT_REFRESH;
	}

	// dispatch page events
	if(seq_page == SEQ_START) {
		gui_seq_start(event);
	}
	else if(seq_page == SEQ_LEN) {
		gui_seq_len(event);
	}
	else if(seq_page == SEQ_STEP_LEN) {
		gui_seq_step_len(event);
	}
	else if(seq_page == SEQ_LOOP) {
		gui_seq_loop(event);
	}
	else if(seq_page == SEQ_DIR) {
		gui_seq_dir(event);
	}
	else if(seq_page == SEQ_NEXT) {
		gui_seq_next(event);
	}
	else if(seq_page == SEQ_CPY) {
		gui_seq_cpy(event);
	}
	else if(seq_page == SEQ_CLR) {
		gui_seq_clr(event);
	}
}

// set the part page and value
void gui_set_part(char evnt) {
	char event = evnt;
	char page;
	if(menu_mode != MENU_PART1 &&
		menu_mode != MENU_PART2) return;

	// change pages
	if(event == EVENT_PAGE_UP) {
		if(menu_mode == MENU_PART2) {
			part2_page ++;
			if(part2_page > PART_MAX_PAGE) part2_page = PART_MAX_PAGE;
			event = EVENT_REFRESH;
		}
		else {
			part1_page ++;
			if(part1_page > PART_MAX_PAGE) part1_page = PART_MAX_PAGE;
			event = EVENT_REFRESH;
		}
	}
	else if(event == EVENT_PAGE_DOWN) {
		if(menu_mode == MENU_PART2) {
			part2_page --;
			if(part2_page < 0) part2_page = 0;
			event = EVENT_REFRESH;
		}
		else {
			part1_page --;
			if(part1_page < 0) part1_page = 0;
			event = EVENT_REFRESH;
		}
	}

	if(menu_mode == MENU_PART2) {
		page = part2_page;
	}
	else {
		page = part1_page;
	}

	// dispatch page events
	if(page == PART_NOTE_SET) {
		gui_part_note_set(event);
	}
	else if(page == PART_GATE_LEN) {
		gui_part_gate_len(event);
	}
	else if(page == PART_SCALE) {
		gui_part_scale(event);
	}
	else if(page == PART_SPAN) {
		gui_part_span(event);
	}
	else if(page == PART_OFFSET) {
		gui_part_offset(event);
	}
	else if(page == PART_COPY) {
		gui_part_copy(event);
	}
	else if(page == PART_TRANS) {
		gui_part_trans(event);
	}
}

// set the system page and value
void gui_set_system(char evnt) {
	char event = evnt;
	if(menu_mode != MENU_SYSTEM) return;

	// change pages
	if(event == EVENT_PAGE_UP) {
		system_page ++;
		if(system_page > SYSTEM_MAX_PAGE) system_page = SYSTEM_MAX_PAGE;
		event = EVENT_REFRESH;
	}
	else if(event == EVENT_PAGE_DOWN) {
		system_page --;
		if(system_page < 0) system_page = 0;
		event = EVENT_REFRESH;
	}

	// dispatch page events
	if(system_page == SYSTEM_SONG_LOAD) {
		gui_system_song_load(event);
	}
	else if(system_page == SYSTEM_SONG_SAV) {
		gui_system_song_sav(event);
	}
	else if(system_page == SYSTEM_SONG_CLR) {
		gui_system_song_clr(event);
	}
	else if(system_page == SYSTEM_CONTROL_CANCEL) {
		gui_system_midi_control_cancel(event);
	}
	else if(system_page == SYSTEM_CLK_DIV) {
		gui_system_clock_div(event);
	}
	else if(system_page == SYSTEM_RESET_MODE) {
		gui_system_reset_mode(event);
	}
	else if(system_page == SYSTEM_MOD1_ASSN) {
		gui_system_mod1_assign(event);
	}
	else if(system_page == SYSTEM_MOD2_ASSN) {
		gui_system_mod2_assign(event);
	}
	else if(system_page == SYSTEM_LIVE_AUD) {
		gui_system_live_audition(event);
	}
	else if(system_page == SYSTEM_MIDI_PT1) {
		gui_system_midi_pt1(event);
	}
	else if(system_page == SYSTEM_MIDI_PT2) {
		gui_system_midi_pt2(event);
	}
	else if(system_page == SYSTEM_KEY_TRANSPOSE) {
		gui_system_key_transpose(event);
	}
	else if(system_page == SYSTEM_KEY_TRIGGER) {
		gui_system_key_trigger(event);
	}
	else if(system_page == SYSTEM_KEY_MAP) {
		gui_system_key_map(event);
	}
	else if(system_page == SYSTEM_LCD_CONT) {
		gui_system_lcd_cont(event);
	}
	else if(system_page == SYSTEM_CV_CAL) {
		gui_system_cv_cal(event);
	}
	else if(system_page == SYSTEM_FACTORY_RESET) {
		gui_system_system_reset(event);
	}
}

// set the live page and value 
void gui_set_live(char evnt) {
	char event = evnt;
	// change pages
	if(event == EVENT_PAGE_UP) {
		live_page ++;
		if(live_page > LIVE_MAX_PAGE) live_page = LIVE_MAX_PAGE;
		event = EVENT_REFRESH;
	}
	else if(event == EVENT_PAGE_DOWN) {
		live_page --;
		if(live_page < 0) live_page = 0;
		event = EVENT_REFRESH;
	}
	
	if(live_page == LIVE_PLAY) {
		gui_live_play(event);
	}
	else if(live_page == LIVE_SEQ) {
		gui_live_seq(event);
	}
	else if(live_page == LIVE_STEP) {
		gui_live_step(event);
	}
}

//
// PAGE HANDLERS
//
// lower play bar
void gui_play_bar(void) {
	unsigned char play_seq = sequencer_get_current_seq();
	unsigned char play_step_pos = sequencer_get_current_step_index();
	unsigned char play_playing = clock_get_song_playing();
	if(play_playing) {
		sprintf(str, "playing:  %02d:%02d", play_seq + 1, play_step_pos + 1);
	}
	else {
		sprintf(str, "stopped:  %02d:%02d", play_seq + 1, play_step_pos + 1);
	}
	screen_write_line(2, str);
}

// seq start
void gui_seq_start(char event) {
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "SEQ START");
	}
	else if(event == EVENT_POT1_CHANGE) {
		current_edit_seq = (pot1_val >> 4) & 0x0f;	
	}
	else if(event == EVENT_POT2_CHANGE) {
		song_set_seq_start(current_edit_seq, (pot2_val >> 4) & 0x0f);
	}

	sprintf(str, " %02d    step %02d", (current_edit_seq + 1), 
		song_get_seq_start(current_edit_seq) + 1);
	screen_write_line(1, str);
}

// seq length
void gui_seq_len(char event) {
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "SEQ LENGTH");
	}
	else if(event == EVENT_POT1_CHANGE) {
		current_edit_seq = (pot1_val >> 4) & 0x0f;		
	}
	else if(event == EVENT_POT2_CHANGE) {
		song_set_seq_len(current_edit_seq, ((pot2_val >> 4) & 0x0f) + 1);
	}

	sprintf(str, " %02d   steps %02d", (current_edit_seq + 1), 
		song_get_seq_len(current_edit_seq));
	screen_write_line(1, str);
}

// seq step length
void gui_seq_step_len(char event) {
	if(event == EVENT_REFRESH) {
		sprintf(str, "SEQ %02d STEP LEN", (current_edit_seq + 1));
		screen_write_line(0, str);
		temp = (pot1_val >> 4) & 0x0f;
	}
	else if(event == EVENT_POT1_CHANGE) {
		temp = (pot1_val >> 4) & 0x0f;
	}
	else if(event == EVENT_POT2_CHANGE) {
		song_set_step_len(current_edit_seq, temp, ((pot2_val >> 3) & 0x1f));
	}

	char len = song_get_step_len(current_edit_seq, temp);
	if(len == 0) {
		sprintf(str, "st %02d   len DIV", (temp + 1));
	}
	else {
		sprintf(str, "st %02d   len %02d", (temp + 1), len);
	}
	screen_write_line(1, str);
}

// seq loop
void gui_seq_loop(char event) {
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "SEQ LOOP");
	}
	else if(event == EVENT_POT1_CHANGE) {
		current_edit_seq = (pot1_val >> 4) & 0x0f;	
	}
	else if(event == EVENT_POT2_CHANGE) {
		song_set_seq_loop(current_edit_seq, (pot2_val >> 4) & 0x0f);
	}

	sprintf(str, " %02d  counts %02d", (current_edit_seq + 1), song_get_seq_loop(current_edit_seq));
	screen_write_line(1, str);
}

// seq direction
void gui_seq_dir(char event) {
	char dir;
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "SEQ DIRECTION");
	}
	else if(event == EVENT_POT1_CHANGE) {
		current_edit_seq = (pot1_val >> 4) & 0x0f;
	}
	else if(event == EVENT_POT2_CHANGE) {
		song_set_seq_dir(current_edit_seq, (pot2_val >> 6) & 0x03);
	}
	dir = song_get_seq_dir(current_edit_seq);
	
	if(dir == SONG_DIR_BACK) {
		sprintf(str, " %02d        bkwd", (current_edit_seq + 1));
	}
	else if(dir == SONG_DIR_PONG) {
		sprintf(str, " %02d        pong", (current_edit_seq + 1));
	}
	else if(dir == SONG_DIR_RAND) {
		sprintf(str, " %02d        rand", (current_edit_seq + 1));
	}
	else if(dir == SONG_DIR_FWD) {
		sprintf(str, " %02d        fwd", (current_edit_seq + 1));
	}
	screen_write_line(1, str);
}

// seq next
void gui_seq_next(char event) {
	char next;
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "SEQ NEXT");
	}
	else if(event == EVENT_POT1_CHANGE) {
		current_edit_seq = (pot1_val >> 4) & 0x0f;
	}
	else if(event == EVENT_POT2_CHANGE) {
		song_set_seq_next(current_edit_seq, (pot2_val >> 4) & 0x0f);
	}
	next = song_get_seq_next(current_edit_seq);
	
	if(next == current_edit_seq) {
		sprintf(str, " %02d  to seq %02d*", (current_edit_seq + 1), (next + 1));
	}
	else {
		sprintf(str, " %02d  to seq %02d", (current_edit_seq + 1), (next + 1));
	}
	screen_write_line(1, str);
}

// seq copy
void gui_seq_cpy(char event) {
	if(event == EVENT_REFRESH) {
		temp = current_edit_seq;
		temp2 = temp;
		screen_write_line(0, "SEQ COPY");
	}
	else if(event == EVENT_POT1_CHANGE) {
		temp = (pot1_val >> 4) & 0x0f;	
	}
	else if(event == EVENT_POT2_CHANGE) {
		temp2 = (pot2_val >> 4) & 0x0f;
	}
	else if(event == EVENT_ENTER_CLICK) {
		song_copy_seq(temp2, temp);
		screen_write_popup(750, "SEQ COPY", "copied");
	}

	sprintf(str, " %02d copy to %02d?", temp + 1, temp2 + 1);
	screen_write_line(1, str);
}

// seq clear
void gui_seq_clr(char event) {
	if(event == EVENT_REFRESH) {
		temp = current_edit_seq;
		screen_write_line(0, "SEQ CLEAR");
	}
	else if(event == EVENT_POT1_CHANGE) {
		temp = (pot1_val >> 4) & 0x0f;
	}
	else if(event == EVENT_ENTER_CLICK) {
		song_clear_seq(temp);
		screen_write_popup(750, "SEQ CLR", "cleared");
	}

	sprintf(str, " %02d      clear?", temp + 1);
	screen_write_line(1, str);
}

// part note set
void gui_part_note_set(char event) {
	unsigned char part;
	unsigned char note;
	unsigned char span;
	unsigned char audition;
	if(menu_mode == MENU_PART2) part = 1;
	else part = 0;

	audition = 0;
	if(event == EVENT_REFRESH) {
		if(part) {
			sprintf(str, "PART 2 SEQ %02d", (current_edit_seq + 1));
		}
		else {
			sprintf(str, "PART 1 SEQ %02d ", (current_edit_seq + 1));
		}
		screen_write_line(0, str);
		temp = 0;  // step note
	}
	else if(event == EVENT_POT1_CHANGE) {
		temp = (pot1_val >> 4) & 0x0f;
		audition = 1;
	}
	else if(event == EVENT_POT2_CHANGE) {
		note = (pot2_val >> 2) & 0x3f;
		span = song_get_span(current_edit_seq, part);
		if(note < 52) {
			// adjust span to reduce the pot resolution
			// span = 1 - 0-12
			if(span == 1) note = note / 4;
			// span = 2 - 0-24
			else if(span == 2) {
				note = note / 2;
				if(note > 24) note = 24;
			}
			// span = 3 - 0-36
			else if(span == 3) {
				note = (note / 4) * 2;
				if(note > 36) note = 36;
			}
		}
		else if(note > 51 && note < 55) note = SONG_STEP_RAND;
		else if(note > 55 && note < 60) note = SONG_STEP_NONE;
		else if(note > 57) note = SONG_STEP_REST;

		// scale quantize
		note = scale_quantize(note, song_get_scale(current_edit_seq, part));
		song_set_note(current_edit_seq, part, temp, note);
		audition = 1;
	}

	// get the note to display / audition
	note = scale_quantize(song_get_note(current_edit_seq, part, temp), 
		song_get_scale(current_edit_seq, part));
	note = scale_span_adjust(note, song_get_span(current_edit_seq, part));

	// audition the note
	if(audition && sysconfig_get_live_aud() && note < 49) {
		sequencer_play_audition_note(part, note);
	}

	char notename[16];
	scale_note_to_name(note, song_get_scale(current_edit_seq, part), notename);	
	sprintf(str, "st %02d  note %s", temp + 1, notename);
	screen_write_line(1, str);
}

// part gate len
void gui_part_gate_len(char event) {
	unsigned char part;
	if(menu_mode == MENU_PART2) part = 1;
	else part = 0;

	if(event == EVENT_REFRESH) {
		if(part) {
			sprintf(str, "PART 2 SEQ %02d", (current_edit_seq + 1));
		}
		else {
			sprintf(str, "PART 1 SEQ %02d ", (current_edit_seq + 1));
		}
		screen_write_line(0, str);
	}
	else if(event == EVENT_POT2_CHANGE) {
		song_set_gate(current_edit_seq, part, (pot2_val >> 2) & 0x3f);
	}

	sprintf(str, "gate length %02d", song_get_gate(current_edit_seq, part));
	screen_write_line(1, str);
}

// part scale
void gui_part_scale(char event) {
	unsigned char part;
	if(menu_mode == MENU_PART2) part = 1;
	else part = 0;

	if(event == EVENT_REFRESH) {
		if(part) {
			sprintf(str, "PART 2 SEQ %02d", (current_edit_seq + 1));
		}
		else {
			sprintf(str, "PART 1 SEQ %02d ", (current_edit_seq + 1));
		}
		screen_write_line(0, str);
	}
	else if(event == EVENT_POT2_CHANGE) {
		song_set_scale(current_edit_seq, part, (pot2_val >> 5) & 0x07);
	}

	char scalename[16];
	scale_type_to_name(song_get_scale(current_edit_seq, part), scalename);

	sprintf(str, "scale %s", scalename);
	screen_write_line(1, str);
}

// part span
void gui_part_span(char event) {
	unsigned char part;
	if(menu_mode == MENU_PART2) part = 1;
	else part = 0;

	if(event == EVENT_REFRESH) {
		if(part) {
			sprintf(str, "PART 2 SEQ %02d", (current_edit_seq + 1));
		}
		else {
			sprintf(str, "PART 1 SEQ %02d ", (current_edit_seq + 1));
		}
		screen_write_line(0, str);
	}
	else if(event == EVENT_POT2_CHANGE) {
		song_set_span(current_edit_seq, part, ((pot2_val >> 6) & 0x03) + 1);
	}

	char span = song_get_span(current_edit_seq, part);

	sprintf(str, "octave span %2d", span);
	screen_write_line(1, str);
}

// part offset
void gui_part_offset(char event) {
	unsigned char part;
	if(menu_mode == MENU_PART2) part = 1;
	else part = 0;

	if(event == EVENT_REFRESH) {
		if(part) {
			sprintf(str, "PART 2 SEQ %02d", (current_edit_seq + 1));
		}
		else {
			sprintf(str, "PART 1 SEQ %02d ", (current_edit_seq + 1));
		}
		screen_write_line(0, str);
	}
	else if(event == EVENT_POT2_CHANGE) {
		song_set_offset(current_edit_seq, part, (char)((pot2_val >> 3) & 0x1f) - 16);
	}

	char offset = song_get_offset(current_edit_seq, part);
	if(offset == 0) {
		sprintf(str, "note offset  0");		
	}
	else if(offset > -10 && offset < 10) {
		sprintf(str, "note offset %+2d", offset);
	}
	else {
		sprintf(str, "note offset %+2d", offset);
	}
	screen_write_line(1, str);
}

// part copy
void gui_part_copy(char event) {
	unsigned char part;
	if(menu_mode == MENU_PART2) part = 1;
	else part = 0;

	if(event == EVENT_REFRESH) {
		temp = 0;
		if(part) {
			sprintf(str, "PART 2 SEQ %02d", (current_edit_seq + 1));
		}
		else {
			sprintf(str, "PART 1 SEQ %02d ", (current_edit_seq + 1));
		}
		screen_write_line(0, str);
	}
	else if(event == EVENT_POT2_CHANGE) {
		temp = (pot2_val >> 7) & 0x01;
	}
	else if(event == EVENT_ENTER_CLICK) {
		song_part_copy(current_edit_seq, temp);
		screen_write_popup(750, "", "copied");
	}
	
	if(temp == 0) {
		sprintf(str, "part copy  1>2?");
	}
	else {
		sprintf(str, "part copy  2>1?");
	}
	screen_write_line(1, str);
}

// part trans
void gui_part_trans(char event) {
	unsigned char part;
	if(menu_mode == MENU_PART2) part = 1;
	else part = 0;

	if(event == EVENT_REFRESH) {
		temp = 0;
		if(part) {
			sprintf(str, "PART 2 SEQ %02d", (current_edit_seq + 1));
		}
		else {
			sprintf(str, "PART 1 SEQ %02d ", (current_edit_seq + 1));
		}
		screen_write_line(0, str);
	}
	else if(event == EVENT_POT2_CHANGE) {
		temp = (pot2_val >> 6) & 0x03;
	}
	else if(event == EVENT_ENTER_CLICK) {
		if(temp == 3) {
			song_part_clear(current_edit_seq, part);
			screen_write_popup(750, "", "cleared");
		}
		else if(temp == 2) {
			song_part_randomize(current_edit_seq, part);
			screen_write_popup(750, "", "randomized");
		}
		else if(temp == 1) {
			song_part_retrograde(current_edit_seq, part);
			screen_write_popup(750, "", "retrograded");
		}
		else {
			song_part_invert(current_edit_seq, part);
			screen_write_popup(750, "", "inverted");
		}
	}

	if(temp == 0) {
		sprintf(str, "part     invert?");
	}
	else if(temp == 1) {
		sprintf(str, "part retrograde?");
	}
	else if(temp == 2) {
		sprintf(str, "part  randomize?");
	}
	else {
		sprintf(str, "part      clear?");
	}
	screen_write_line(1, str);
}

// system song load
void gui_system_song_load(char event) {
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "SONG LOAD");
		temp = sysconfig_get_current_song();
	}
	else if(event == EVENT_POT2_CHANGE) {
		temp = (pot2_val >> 5) & 0x07;
	}
	if(event == EVENT_ENTER_CLICK) {
		song_file_load(temp);
	}

	if(temp == sysconfig_get_current_song()) {
		sprintf(str, "  load song %02d*?", (temp + 1));
	}
	else {
		sprintf(str, "  load song %02d ?", (temp + 1));
	}
	screen_write_line(1, str);
}

// system song save
void gui_system_song_sav(char event) {
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "SONG SAVE");
		temp = sysconfig_get_current_song();
	}
	else if(event == EVENT_POT2_CHANGE) {
		temp = (pot2_val >> 5) & 0x07;
	}
	else if(event == EVENT_ENTER_CLICK) {
		song_file_save(temp);
	}

	if(temp == sysconfig_get_current_song()) {
		sprintf(str, "  save song %02d*?", (temp + 1));
	}
	else {
		sprintf(str, "  save song %02d ?", (temp + 1));
	}
	screen_write_line(1, str);
}

// syste, song clear
void gui_system_song_clr(char event) {
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "SONG CLEAR");
	}
	else if(event == EVENT_ENTER_CLICK) {
		song_clear_song();
		screen_write_popup(750, "SONG CLEAR", "cleared");
	}
	screen_write_line(1, "     song clear?");
}

// system MIDI control cancel
void gui_system_midi_control_cancel(char event) {
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "MOD CTRL CANCEL");
	}
	else if(event == EVENT_ENTER_CLICK) {
		sequencer_control_restore();
		screen_write_popup(750, "", "canceled");
	}
	screen_write_line(1, " control cancel?");
}

// system clock divide
void gui_system_clock_div(char event) {
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "CLOCK DIVIDER");
	}
	else if(event == EVENT_POT2_CHANGE) {
		sysconfig_set_clock_div(((pot2_val >> 3) & 0x1f) + 1);
	}
	sprintf(str, "div ratio   1/%d", sysconfig_get_clock_div());
	screen_write_line(1, str);
}

// system reset mode
void gui_system_reset_mode(char event) {
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "RESET MODE");
	}
	else if(event == EVENT_POT2_CHANGE) {
		sysconfig_set_reset_mode((pot2_val >> 7) & 0x01);
	}
	if(sysconfig_get_reset_mode()) {
		sprintf(str, "reset mode  SEQ");
	}
	else {
		sprintf(str, "reset mode  SONG");
	}
	screen_write_line(1, str);

}

// system mod1 assign
void gui_system_mod1_assign(char event) {
	char assign;
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "MOD1 CV ASSIGN");
	}
	else if(event == EVENT_POT2_CHANGE) {
		sysconfig_set_mod_assign(0, (pot2_val >> 5) & 0x07);
	}

	assign = sysconfig_get_mod_assign(0);
	if(assign == SYSCONFIG_MOD_NONE) {
		sprintf(str, " 1 none");
	}
	else if(assign == SYSCONFIG_MOD_NEXT_SEQ) {
		sprintf(str, " 1 seq next");
	}
	else if(assign == SYSCONFIG_MOD_SEQ_START) {
		sprintf(str, " 1 seq start");
	}
	else if(assign == SYSCONFIG_MOD_SEQ_LEN) {
		sprintf(str, " 1 seq length");
	}
	else if(assign == SYSCONFIG_MOD_RUN_STOP) {
		sprintf(str, " 1 seq run/stop");
	}
	else if(assign == SYSCONFIG_MOD_GATE1) {
		sprintf(str, " 1 gate 1 time");
	}
	else if(assign == SYSCONFIG_MOD_GATE2) {
		sprintf(str, " 1 gate 2 time");
	}
	else if(assign == SYSCONFIG_MOD_SEQ_DIR) {
		sprintf(str, " 1 seq direction");
	}
	screen_write_line(1, str);
}

// system mod2 assign
void gui_system_mod2_assign(char event) {
	char assign;
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "MOD2 CV ASSIGN");
	}
	else if(event == EVENT_POT2_CHANGE) {
		sysconfig_set_mod_assign(1, (pot2_val >> 5) & 0x07);
	}

	assign = sysconfig_get_mod_assign(1);
	if(assign == SYSCONFIG_MOD_NONE) {
		sprintf(str, " 2 none");
	}
	else if(assign == SYSCONFIG_MOD_NEXT_SEQ) {
		sprintf(str, " 2 seq next");
	}
	else if(assign == SYSCONFIG_MOD_SEQ_START) {
		sprintf(str, " 2 seq start");
	}
	else if(assign == SYSCONFIG_MOD_SEQ_LEN) {
		sprintf(str, " 2 seq length");
	}
	else if(assign == SYSCONFIG_MOD_RUN_STOP) {
		sprintf(str, " 2 seq run/stop");
	}
	else if(assign == SYSCONFIG_MOD_GATE1) {
		sprintf(str, " 2 gate 1 time");
	}
	else if(assign == SYSCONFIG_MOD_GATE2) {
		sprintf(str, " 2 gate 2 time");
	}
	else if(assign == SYSCONFIG_MOD_SEQ_DIR) {
		sprintf(str, " 2 seq direction");
	}
	screen_write_line(1, str);
}

// system live audition
void gui_system_live_audition(char event) {
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "LIVE AUDITION");
	}
	else if(event == EVENT_POT2_CHANGE) {
		sysconfig_set_live_aud((pot2_val >> 7) & 0x01);
	}

	if(sysconfig_get_live_aud()) screen_write_line(1, "live aud    on");
	else screen_write_line(1, "live aud    off");
}

// system MIDI part 1
void gui_system_midi_pt1(char event) {
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "MIDI PART 1");
	}
	else if(event == EVENT_POT2_CHANGE) {
		sysconfig_set_midi_channel(0, (pot2_val >> 4) & 0x0f);
	}
	sprintf(str, "channel     %02d", sysconfig_get_midi_channel(0) + 1);
	screen_write_line(1, str);
}

// system MIDI part 2
void gui_system_midi_pt2(char event) {
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "MIDI PART 2");
	}
	else if(event == EVENT_POT2_CHANGE) {
		sysconfig_set_midi_channel(1, (pot2_val >> 4) & 0x0f);
	}
	sprintf(str, "channel     %02d", sysconfig_get_midi_channel(1) + 1);
	screen_write_line(1, str);
}

// system key transpose
void gui_system_key_transpose(char event) {
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "KEY TRANSPOSE");
	}
	else if(event == EVENT_POT2_CHANGE) {
		sysconfig_set_key_transpose((pot2_val >> 6) & 0x03);
	}
	char transpose = sysconfig_get_key_transpose();
	if(transpose == SYSCONFIG_KEY_TRANSPOSE1) sprintf(str, "transpose part 1");
	else if(transpose == SYSCONFIG_KEY_TRANSPOSE2) sprintf(str, "transpose part 2");
	else if(transpose == SYSCONFIG_KEY_TRANSPOSE12) sprintf(str, "transpose both");
	else sprintf(str, "transpose off");
	screen_write_line(1, str);
}

// system key trigger
void gui_system_key_trigger(char event) {
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "KEY TRIGGER");
	}
	else if(event == EVENT_POT2_CHANGE) {
		sysconfig_set_key_trigger(pot2_val >> 6);
	}
	if(sysconfig_get_key_trigger() == SYSCONFIG_KEY_TRIGGER_MOM) sprintf(str, "trigger    mom");
	else sprintf(str, "trigger    latch");
	screen_write_line(1, str);
}

// system key map
void gui_system_key_map(char event) {
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "KEY MAP");
	}
	else if(event == EVENT_POT2_CHANGE) {
		sysconfig_set_key_map(pot2_val >> 7);
	}
	if(sequencer_get_control_override(SYSCONFIG_MOD_KEY_MAP) == 1) {
		if(sysconfig_get_key_map() == SYSCONFIG_KEY_MAP_B) {
			sprintf(str, "key map      B>A");
		}
		else {
			sprintf(str, "key map      A>B");
		}
	}
	else {
		if(sysconfig_get_key_map() == SYSCONFIG_KEY_MAP_B) {
			sprintf(str, "key map      B");
		}
		else {
			sprintf(str, "key map      A");
		}
	}
	screen_write_line(1, str);
}

// system LCD contrast
void gui_system_lcd_cont(char event) {
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "LCD CONTRAST");
	}
	else if(event == EVENT_POT2_CHANGE) {
		sysconfig_set_lcd_contrast(pot2_val);
	}
	sprintf(str, "contrast    %d", ((sysconfig_get_lcd_contrast() >> 4) + 1));
	screen_write_line(1, str);
}

// system CV cal
void gui_system_cv_cal(char event) {
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "CV CALIBRATE");
		temp = 127;
		temp2 = 127;
	}
	else if(event == EVENT_POT1_CHANGE) {
		temp = (pot1_val >> 5) & 0x07;
		if(temp > 6) temp = 6;
		if(temp2 == 127) {
			sequencer_cv_cal_start();
			temp2 = (pot2_val >> 5) & 0x07;
			if(temp2 > 6) temp2 = 6;
			sequencer_cv_set_cal(1, temp2);	
		}
		sequencer_cv_set_cal(0, temp);
	}
	else if(event == EVENT_POT2_CHANGE) {
		temp2 = (pot2_val >> 5) & 0x07;
		if(temp2 > 6) temp2 = 6;
		if(temp == 127) {
			sequencer_cv_cal_start();
			temp = (pot1_val >> 5) & 0x07;
			if(temp > 6) temp = 6;
			sequencer_cv_set_cal(0, temp);	
		}
		sequencer_cv_set_cal(1, temp2);	
	}
	if(temp == 127 || temp2 == 127) {
		sprintf(str, "cv1  ?V cv2  ?V");
	}
	else {
		sprintf(str, "cv1 %+dV cv2 %+dV", (temp - 3), (temp2 - 3));
	}
	screen_write_line(1, str);
}

// system reset
void gui_system_system_reset(char event) {
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "SYSTEM RESET");
	}
	else if(event == EVENT_ENTER_CLICK) {
		sysconfig_reset_all();
		screen_write_popup(750, "", "done!");
		// this doesn't work correctly
//		system_page = SYSTEM_SONG_LOAD;  // force us out of here on the next entry
	}
	screen_write_line(1, " reset settings?");
}

// live clock control
void gui_live_play(char event) {
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "LIVE PLAY CTRL");
		temp = current_edit_seq;
	}
	else if(event == EVENT_POT1_CHANGE) {
		sysconfig_set_clock_speed(pot1_val);
	}
	else if(event == EVENT_POT2_CHANGE) {
		temp = (pot2_val >> 4) & 0x0f;
	}
	else if(event == EVENT_ENTER_CLICK) {
		sequencer_control_change(SYSCONFIG_MOD_NEXT_SEQ, ((temp << 3) & 0x7f));
		current_edit_seq = temp;
	}
	if(clock_get_speed() < 20) {
		sprintf(str, "EXT CLK  seq %02d?", (temp + 1));
	}
	else {
		sprintf(str, "%3d BPM  seq %02d?", sysconfig_get_clock_speed(), (temp + 1));
	}
	screen_write_line(1, str);
}

// live seq control
void gui_live_seq(char event) {
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "LIVE SEQ CTRL");
	}
	else if(event == EVENT_POT1_CHANGE) {
		sequencer_control_change(SYSCONFIG_MOD_GATE1, pot1_val >> 1);
		sequencer_control_change(SYSCONFIG_MOD_GATE2, pot1_val >> 1);
	}
	else if(event == EVENT_POT2_CHANGE) {
		sequencer_control_change(SYSCONFIG_MOD_SEQ_DIR, pot2_val >> 1);
	}
	else if(event == EVENT_ENTER_CLICK) {
		sequencer_control_restore();
	}
	utemp = sequencer_get_control_override(SYSCONFIG_MOD_GATE1);
	utemp2 = sequencer_get_control_override(SYSCONFIG_MOD_SEQ_DIR);
	char dir_str[16];
	if(utemp2 == 1) {
		sprintf(dir_str, "FLIP");
	}
	else {
		sprintf(dir_str, "NORM");
	}
	if(utemp == 255) {
		sprintf(str, "gate ?? dir %s", dir_str);
	}
	else {
		sprintf(str, "gate %02d dir %s", utemp, dir_str);
	}
	screen_write_line(1, str);
}

// live step control
void gui_live_step(char event) {
	if(event == EVENT_REFRESH) {
		screen_write_line(0, "LIVE STEP CTRL");
	}
	else if(event == EVENT_POT1_CHANGE) {
		sequencer_control_change(SYSCONFIG_MOD_SEQ_START, pot1_val >> 1);
	}
	else if(event == EVENT_POT2_CHANGE) {
		sequencer_control_change(SYSCONFIG_MOD_SEQ_LEN, pot2_val >> 1);
	}
	else if(event == EVENT_ENTER_CLICK) {
		sequencer_control_restore();
	}
	utemp = sequencer_get_control_override(SYSCONFIG_MOD_SEQ_START);
	utemp2 = sequencer_get_control_override(SYSCONFIG_MOD_SEQ_LEN);
	if(utemp == 255 && utemp2 == 255) {
		sprintf(str, "start ?? len ??");
	}
	else if(utemp == 255) {
		sprintf(str, "start ?? len %02d", utemp2);
	}
	else if(utemp2 == 255) {
		sprintf(str, "start %02d len ??", utemp + 1);
	}
	else {
		sprintf(str, "start %02d len %02d", utemp + 1, utemp2);
	}
	screen_write_line(1, str);
}
