/*
 * K2579 Step Sequencer - Clock / Reset Input Handler
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 * Hardware I/O:
 *
 *  RD8/INT1	- clock in					- input - int 1
 *  RD9/INT2	- reset in					- input - int 2
 *
 * Clock Handling
 *  - MIDI clock overrides analog clock for 1 second after a tick is received
 *  - MIDI song start resets the the playback to the start of the song
 *  - MIDI stop kills notes immediately (via sequencer)
 *  - MIDI song position selects correct playback position (via sequencer)
 *  - analog clock imposes a 8ms timeout after each pulse (max rate ~120Hz)
 *
 */
#include "clock.h"
#include "clock_table.h"
#include "sequencer.h"
#include "midi.h"
#include "gui.h"
#include "panel.h"

// control
unsigned int midi_override_timeout;		// midi takes over from analog input for 1 second
#define MIDI_OVERRIDE_TIME 3906     	// 1s at 256us per count
#define CLOCK_LED_TIMEOUT 2				// 32ms
unsigned char song_playing;				// 0 = song is stopped, 1 = song is playing
unsigned char clock_speed;				// <20 = ext, 20-250 = 20-255 BPM
unsigned int clock_interval;			// the clock interval
unsigned int clock_interval_count;		// counts the clock interval
// analog clock
#define CLOCK_IGNORE_TIME 32
#define RESET_IGNORE_TIME 400
unsigned int clock_ignore_timeout;
unsigned int reset_ignore_timeout;

// init the clock input
void clock_init(void) {
	clock_ignore_timeout = 0;
	reset_ignore_timeout = 0;
	clock_speed = 0;
	midi_override_timeout = 0;
	song_playing = 0;  // start with the song playing
	clock_interval = clock_table[20];
	clock_interval_count = 0;
}

// run a task to time out clock inputs
void clock_task(void) {
	// internal clock
	if(clock_speed) {
		clock_interval_count += 250;
		// we rolled over - time to make a clock pulse
		if(clock_interval_count >= clock_interval) {
			clock_interval_count -= clock_interval;
			_midi_tx_timing_tick();  // make MIDI timing tick
			if(song_playing) {
				if(!sequencer_get_clock_div_count()) {
					panel_set_clock_led(CLOCK_LED_TIMEOUT);  // blink the clock LED
				}
				sequencer_clock_tick();
			}
		}
	}

	// handle MIDI override timeout
	if(midi_override_timeout) {
		midi_override_timeout --;
	}

	// analog clock timeouts
	if(clock_ignore_timeout) {
		clock_ignore_timeout --;
	}
	if(reset_ignore_timeout) {
		reset_ignore_timeout --;
	}
}

// get the clock speed
unsigned char clock_get_speed(void) {
	return clock_speed;
}

// gets the song playing state
unsigned char clock_get_song_playing(void) {
	return song_playing;
}

// set the clock speed
void clock_set_speed(unsigned char speed) {
	clock_speed = speed;
	if(speed < 20) clock_speed = 0;
	if(speed > 250) clock_speed = 250;
	clock_interval = clock_table[clock_speed];
}

// MIDI clock tick received
void clock_midi_tick(void) {
	if(clock_speed) return;  // internal clock mode
	_midi_tx_timing_tick();  // echo MIDI timing tick
	if(song_playing) {
		if(!sequencer_get_clock_div_count()) {
			panel_set_clock_led(CLOCK_LED_TIMEOUT);  // blink the clock LED
		}
		sequencer_clock_tick();
	}
	midi_override_timeout = MIDI_OVERRIDE_TIME;
}

// MIDI clock start received
void clock_midi_start(void) {
	if(clock_speed) return;  // internal clock mode
	song_playing = 1;  // we are playing
	_midi_tx_start_song();  // send a MIDI clock start
	sequencer_clock_start();
	gui_playback_updated();
}

// MIDI clock continue received
void clock_midi_continue(void) {
	if(clock_speed) return;  // internal clock mode
	song_playing = 1;  // we are playing
	_midi_tx_continue_song();  // send a MIDI clock continue
	gui_playback_updated();
}

// MIDI clock stop received
void clock_midi_stop(void) {
	if(clock_speed) return;  // internal clock mode
	song_playing = 0;
	_midi_tx_stop_song();  // send a MIDI clock stop
	sequencer_clock_stop();
	gui_playback_updated();
}

// clock input triggered
void clock_clock_input(void) {
	if(clock_speed) return;  // internal clock mode
	if(midi_override_timeout) return;
	if(clock_ignore_timeout == 0) {
		_midi_tx_timing_tick();  // send a MIDI timing tick
		if(song_playing) {
			panel_set_clock_led(CLOCK_LED_TIMEOUT);  // blink the clock LED
			sequencer_clock_tick();
		}
		clock_ignore_timeout = CLOCK_IGNORE_TIME;
	}
}

// reset input triggered
void clock_reset_input(void) {
	if(reset_ignore_timeout == 0) {
		sequencer_clock_start();
		gui_playback_updated();
		reset_ignore_timeout = RESET_IGNORE_TIME;
	}
}

// clock run command
void clock_run_command(void) {
	if(!song_playing) {
		_midi_tx_continue_song();  // send a MIDI clock continue
		song_playing = 1;
		gui_playback_updated();
	}
}

// clock stop command
void clock_stop_command(void) {
	if(song_playing) {
		song_playing = 0;
		_midi_tx_stop_song();  // send a MIDI clock stop
		sequencer_clock_stop();
		gui_playback_updated();
	}
}

// clock run/stop toggle
void clock_run_stop_toggle(void) {
	sequencer_control_run_stop_restore();
	if(song_playing) {
		clock_stop_command();
	}
	else {
		clock_run_command();
	}
}
