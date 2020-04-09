/*
 * K2579 Step Sequencer - Sequencer Core
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 *  - clock divide behaviour:
 *    - all song position, gate and clock functions are based on 24ppq
 *    - steps are taken based on the clock divider
 *
 */
#include <plib.h>
#include <stdlib.h>
#include <stdio.h>
#include "sequencer.h"
#include "song.h"
#include "midi.h"
#include "cv_output.h"
#include "analog_input.h"
#include "clock.h"
#include "sysconfig.h"
#include "gui.h"
#include "seq_midi.h"
#include "scale.h"
#include "panel.h"

#define STEP_INVALID 127

// clock control
#define NOTE_KILL_TIME_RUN 39063		// ~10s at 256us per count
#define NOTE_KILL_TIME_STOP 796 		// 0.500ms at 256us per count
#define MOD_LED_TIMEOUT 2				// 32ms
unsigned int clock_tick_count;			// clock tick count
unsigned char clock_div_count;			// the current clock divide counter
int note_kill_timeout;					// the note timeout counter - for stopped clock

// sequencer internal
#define MIDI_NOTE_OFFSET 24
unsigned char next_cued_seq;			// the next seq to play or 255 if invalid
// these are used for display because the step is updated after each step is started
unsigned char current_seq_playing;		// the current sequence playing now
unsigned char current_step_index_playing;	// the current step index playing now
// current playback variables
unsigned char current_seq;				// the currently playing sequence
unsigned char current_loop_count;		// the number of loops taken
char current_step_count;				// current step count based on the dir and length - counts from 0 to len
unsigned char current_pingpong;			// the current ping pong count
unsigned char current_note[2];			// current note or 255
unsigned char gate_time_count[2];		// the current gate time counted
// control overrides
unsigned char control_start_override;	// 0-15 start override or 255 if disabled
unsigned char control_len_override;		// 0-15 length override or 255 if disabled
unsigned char control_gate_override[2];	// 1-48 length override or 255 disabled
unsigned char control_dir_override;		// 1 = override or 255 if disabled
char control_offset_override[2];  		// -12 to +12 overrides the offset
unsigned char control_run_override;		// 1 = run stopped or 255 if disabled
unsigned char control_key_map_override;  // 1 = swapped, 255 = normal

// local functions
// start a note
void sequencer_start_note(unsigned char part, unsigned char note);
// stop a note
void sequencer_stop_note(unsigned char part);
// the clock has changed
void sequencer_clock_changed(void);
// advance the sequencer 1 step
void sequencer_advance_step(void);
// the end of a loop is reached - figure out what to do next
void sequencer_loop_end(unsigned char len, unsigned char dir);
// compute and return the current step based on dir, len, etc.
char sequencer_compute_current_step(void);
// reset song position
void sequencer_reset_song_pos(void);

// initialize the sequencer
void sequencer_init(void) {
	// clock control
	clock_tick_count = 0;
	note_kill_timeout = 0;
	// sequencer internal
	sequencer_reset_song_pos();
	sequencer_control_restore();
}

// run the sequencer task every 256us
void sequencer_task(void) {
	// note kill
	if(note_kill_timeout) {
		note_kill_timeout --;
		if(note_kill_timeout == 0) {
			sequencer_stop_note(0);
			sequencer_stop_note(1);
		}
	}
}

// start a note
void sequencer_start_note(unsigned char part, unsigned char note) {
	unsigned char not;
	if(part > 1) return;
	if(control_offset_override[part]) {
		not = note + 12 + control_offset_override[part];  // 12-60 normal range
	}
	else {
		not = note + 12 + song_get_offset(current_seq, part);  // 12-60 normal range
	}
	if(not < 12 || not > 115) return;
	// send MIDI note
	current_note[part] = not;
	gate_time_count[part] = 0;
	_midi_tx_note_on(seq_midi_get_channel(part), current_note[part] + MIDI_NOTE_OFFSET, 100);
	// control analog output
	cv_output_note_on(part, current_note[part]);
	// reset the note timeout
	if(clock_get_song_playing()) note_kill_timeout = NOTE_KILL_TIME_RUN;
	else note_kill_timeout = NOTE_KILL_TIME_STOP;
}

// stop a note
void sequencer_stop_note(unsigned char part) {
	if(part > 1) return;
	if(current_note[part] == 255) return;
	// send MIDI note
	_midi_tx_note_off(seq_midi_get_channel(part), current_note[part] + MIDI_NOTE_OFFSET);
	current_note[part] = 255;
	// control analog output
	cv_output_note_off(part);
}

// the clock has changed
void sequencer_clock_changed(void) {
	int i;
	unsigned char gate;

	// gate time
	for(i = 0; i < 2; i ++) {
		if(current_note[i]) {
			gate_time_count[i] ++;
			gate = song_get_gate(current_seq, i);
			if(control_gate_override[i] != 255) gate = control_gate_override[i];
			if(gate_time_count[i] >= gate) {
				sequencer_stop_note(i);
			}
		}
	}

	// next step
	if(clock_div_count == 0) {
		int note;
		char step;

		// get the current step based on the start, len and random
		step = sequencer_compute_current_step();

		// control each note
		for(i = 0; i < 2; i ++) {
			note = song_get_note(current_seq, i, step);
			if(note == SONG_STEP_RAND) {
				note = song_get_rand_note();
			}
			note = scale_span_adjust(note, song_get_span(current_seq, i));
			note = scale_quantize(note, song_get_scale(current_seq, i));
			if(note == SONG_STEP_REST) {
				sequencer_stop_note(i);
			}
			else if(note == SONG_STEP_NONE) {
				// do nothing
			}
			else {
				sequencer_stop_note(i);
				sequencer_start_note(i, note);
			}
		}

		// update the display positions based on the step just played
		current_seq_playing = current_seq;
		current_step_index_playing = step;
		gui_playback_updated();

		// advance the sequencer for the next step
		sequencer_advance_step();
	}
	clock_div_count ++;
	// step time override
	unsigned char step_len = song_get_step_len(current_seq, current_step_index_playing);
	// a step len of >0 means use it for the clock div count
	if(step_len) {
		if(clock_div_count >= step_len) {
			clock_div_count = 0;
		}
	}
	// otherwise use the master clock div
	else if(clock_div_count >= sysconfig_get_clock_div()) {
		clock_div_count = 0;
	}
	note_kill_timeout = NOTE_KILL_TIME_RUN;
}

// advance the sequencer 1 step
void sequencer_advance_step(void) {
	// length
	unsigned char len = song_get_seq_len(current_seq);
	if(control_len_override != 255) {
		len = control_len_override;
	}

	// direction
	unsigned char dir = song_get_seq_dir(current_seq);
	if(control_dir_override == 1) {
		// flip directions for forward / backward
		if(dir == SONG_DIR_FWD) dir = SONG_DIR_BACK;
		else if(dir == SONG_DIR_BACK) dir = SONG_DIR_FWD;
	}	

	// the current step has not yet been initialized
	if(current_step_count == STEP_INVALID) {
		sequencer_loop_end(len, dir);
		return;
	}

	// pingpong
	if(dir == SONG_DIR_PONG) {
		// ponging (backwards)
		if(current_pingpong & 0x01) {
			current_step_count --;
			// start is reached
			if(current_step_count <= 0) {
				current_step_count = 0;
				current_pingpong = 0;
				sequencer_loop_end(len, dir);
			}
		}
		// pinging (forwards)
		else {
			current_step_count ++;
			// len is reached
			if(current_step_count >= len - 1) {
				current_step_count = len - 1;
				current_pingpong = 1;
				sequencer_loop_end(len, dir);
			}
		}
	}
	// go forwards / randomly
	else if(dir == SONG_DIR_FWD || dir == SONG_DIR_RAND) {
		current_step_count ++;
		// len is reached
		if(current_step_count > (len - 1)) {
			current_step_count = 0;
			sequencer_loop_end(len, dir);
		}
	}
	// go backwards
	else if(dir == SONG_DIR_BACK) {
		current_step_count --;
		// start is reached
		if(current_step_count < 0) {
			current_step_count = len - 1;
			sequencer_loop_end(len, dir);
		}
	}
}

// the end of a loop is reached - figure out what to do next
void sequencer_loop_end(unsigned char len, unsigned char dir) {
	// step is invalid from the reset - cause sequence to reload
	if(current_step_count == STEP_INVALID) {
		current_seq = 254;
	}
	// we've reached a loop end
	else {
		current_loop_count ++;
		// number of loops is exceeded
		if(current_loop_count > song_get_seq_loop(current_seq) && next_cued_seq == 255) {
			next_cued_seq = song_get_seq_next(current_seq);
		}
	}

	// if the cued seq is not the one we're on
	if((next_cued_seq != 255) & (next_cued_seq != current_seq)) {
		current_seq = next_cued_seq;
		current_pingpong = 0;
		current_loop_count = 0;
		
		// length
		unsigned char new_len = song_get_seq_len(current_seq);
		if(control_len_override != 255) {
			new_len = control_len_override;
		}

		// direction
		unsigned char new_dir = song_get_seq_dir(current_seq);
		if(control_dir_override == 1) {
			// flip directions for forward / backward
			if(new_dir == SONG_DIR_FWD) new_dir = SONG_DIR_BACK;
			else if(new_dir == SONG_DIR_BACK) new_dir = SONG_DIR_FWD;
		}
		// for backwards we start on the last step
		if(new_dir == SONG_DIR_BACK) {
			current_step_count = new_len - 1;
		}
		// for forwards we start on step 0
		else {
			current_step_count = 0;
		}
	}
	next_cued_seq = 255;
}

// compute and return the current step based on dir, len, etc.
char sequencer_compute_current_step(void) {
	char temp;
	// start
	unsigned char new_start = song_get_seq_start(current_seq);
	if(control_start_override != 255) {
		new_start = control_start_override;
	}

	// go randomly
	if(song_get_seq_dir(current_seq) == SONG_DIR_RAND) {
		// length
		unsigned char new_len = song_get_seq_len(current_seq);
		if(control_len_override != 255) {
			new_len = control_len_override;
		}
		temp = ((rand() >> 4) % new_len) + new_start;
	}
	// go sequentially
	else {
		temp = current_step_count + new_start;
	}

	// clamp the step index range because it wraps around for start/len offsets
	if(temp > (SONG_NUM_STEPS - 1)) temp -= SONG_NUM_STEPS;
	else if(temp < 0) temp += SONG_NUM_STEPS;

	return temp;
}

// reset song position
void sequencer_reset_song_pos(void) {
	clock_tick_count = 0;  // reset the song position
	clock_div_count = 0;
	// reset the sequence only
	if(sysconfig_get_reset_mode() == SYSCONFIG_RESET_MODE_SEQ) {
		next_cued_seq = current_seq;
	}
	// reset the entire song
	else {
		next_cued_seq = 0;  // next cued sequence is 0
	}
	current_seq_playing = next_cued_seq;
	current_step_count = STEP_INVALID;  // invalidate the current position
	current_loop_count = 0;
	current_pingpong = 0;
	sequencer_advance_step();
	current_step_index_playing = sequencer_compute_current_step();
	current_note[0] = 255;  // disabled
	current_note[1] = 255;  // disabled
	gate_time_count[0] = 0;
	gate_time_count[1] = 0;
	gui_playback_updated();
}

//
// MIDI / analog clock handlers
//
// set song position
void sequencer_midi_song_pos(unsigned int pos) {
	unsigned char clock_div = sysconfig_get_clock_div();
	_midi_tx_song_position(pos);  // send song position pointer

	// calculate the current position
	sequencer_reset_song_pos();  // reset the song
	clock_tick_count = pos * 6;  // calculate the desired clock tick offset
	unsigned int advance_tick_count = 0;
	unsigned char step_len = 0;
	unsigned char step = 0;
	while(1) {
		// get the current step based on the start, len and random
		step = sequencer_compute_current_step();
		step_len = song_get_step_len(current_seq, step);
		// step length is default (clock_div)
		if(step_len == 0) {
			step_len = clock_div;
		}
		// if we're past the SPP then break
		if(advance_tick_count + step_len > clock_tick_count) break;
		// add the step length
		advance_tick_count += step_len;
		sequencer_advance_step();  // move to the next step
		ClearWDT();
	}
	// deal with remainder
	clock_div_count = clock_tick_count - advance_tick_count;
	current_seq_playing = current_seq;
	current_step_index_playing = sequencer_compute_current_step();
	gui_playback_updated();
}

//
// MIDI/analog control handlers
//
// clock pulse was received
void sequencer_clock_tick(void) {
	if(control_run_override == 1) return;
	sequencer_clock_changed(); 
	clock_tick_count ++;
}

// start song at beginning
void sequencer_clock_start(void) {
	sequencer_stop_note(0);
	sequencer_stop_note(1);
	sequencer_reset_song_pos();
}

// stop song
void sequencer_clock_stop(void) {
	sequencer_stop_note(0);
	sequencer_stop_note(1);
	_midi_tx_control_change(seq_midi_get_channel(0), 123, 0);
	_midi_tx_control_change(seq_midi_get_channel(1), 123, 0);
}

// get the current clock div count
unsigned char sequencer_get_clock_div_count(void) {
	return clock_div_count;
}

//
// external control of sequence
//
// MIDI key trigger
void sequencer_key_trigger(unsigned char seq) {
	if(seq > SONG_NUM_SEQ) return;
	sequencer_set_next_seq(seq);
	clock_run_command();  // make sure we start if we're stopped
}

// MIDI key transpose
void sequencer_key_transpose(char transpose) {
	unsigned char assign = sysconfig_get_key_transpose();
	if(transpose < -12 || transpose > 12) return;

	if(assign == SYSCONFIG_KEY_TRANSPOSE12) {
		control_offset_override[0] = transpose;
		control_offset_override[1] = transpose;
	}
	else if(assign == SYSCONFIG_KEY_TRANSPOSE1) {
		control_offset_override[0] = transpose;
	}
	else if(assign == SYSCONFIG_KEY_TRANSPOSE2) {
		control_offset_override[1] = transpose;
	}
}

// MIDI/analog control change was received
// send values from 0-127 to here
void sequencer_control_change(unsigned char mod, unsigned char value) {
	if(value > 127) return;

	// next sequence
	if(mod == SYSCONFIG_MOD_NEXT_SEQ) {
		sequencer_set_next_seq(value >> 3);
	}
	// sequence start
	else if(mod == SYSCONFIG_MOD_SEQ_START) {
		control_start_override = (value >> 3);
	}
	// sequence len
	else if(mod == SYSCONFIG_MOD_SEQ_LEN) {
		control_len_override = (value >> 3) + 1;
	}
	// sequence run/stop
	else if(mod == SYSCONFIG_MOD_RUN_STOP) {
		// stop
		if(value < 64) {
			if(control_run_override == 255) {
				clock_stop_command();
				control_run_override = 1;
			}
		}
		else {
			if(control_run_override == 1) {
				clock_run_command();
				control_run_override = 255;
			}
		}
	}
	// gate 1 length
	else if(mod == SYSCONFIG_MOD_GATE1) {
		control_gate_override[0] = (value >> 2) + 1;
	}
	// gate 2 length
	else if(mod == SYSCONFIG_MOD_GATE2) {
		control_gate_override[1] = (value >> 2) + 1;
	}
	// dir
	else if(mod == SYSCONFIG_MOD_SEQ_DIR) {
		if(value < 64) control_dir_override = 255;
		else control_dir_override = 1;
	}
	// key map - this is special and not part of the normal group
	else if(mod == SYSCONFIG_MOD_KEY_MAP) {
		if(value < 64) control_key_map_override = 255;
		else control_key_map_override = 1;
	}
	// not recognized mod
	else {
		return;
	}

	// blink the mod led
	panel_set_mod_led(MOD_LED_TIMEOUT);

	// send event about the updated status
	gui_control_override_updated();
}

// get a control override value
// returns values in the local range
unsigned char sequencer_get_control_override(unsigned char mod) {
	// next sequence
	if(mod == SYSCONFIG_MOD_NEXT_SEQ) { 
		// not supported
	}
	// sequence start
	else if(mod == SYSCONFIG_MOD_SEQ_START) {
		if(control_start_override != 255) {
			return control_start_override;
		}
	}
	// sequence len
	else if(mod == SYSCONFIG_MOD_SEQ_LEN) {
		if(control_len_override != 255) {
			return control_len_override;
		}
	}
	// sequence run/stop
	else if(mod == SYSCONFIG_MOD_RUN_STOP) {
		if(control_run_override != 255) {
			return 0;
		}
	}
	// gate 1 length
	else if(mod == SYSCONFIG_MOD_GATE1) {
		if(control_gate_override[0]) {
			return control_gate_override[0];
		}
	}
	// gate 2 length
	else if(mod == SYSCONFIG_MOD_GATE2) {
		if(control_gate_override[1]) {
			return control_gate_override[1];
		}
	}
	// dir
	else if(mod == SYSCONFIG_MOD_SEQ_DIR) {
		if(control_dir_override != 255) {
			return control_dir_override;
		}
	}
	// keymap
	else if(mod == SYSCONFIG_MOD_KEY_MAP) {
		if(control_key_map_override != 255) {
			return control_key_map_override;
		}
	}
	return 255;
}

// restore the current CC / key overrides to the default value
void sequencer_control_restore(void) {
	control_start_override = 255;  // disabled
	control_len_override = 255;  // disabled
	control_gate_override[0] = 255;  // disabled
	control_gate_override[1] = 255;  // disabled
	control_dir_override = 255;  // disabled
	control_offset_override[0] = 0;  // disabled
	control_offset_override[1] = 0;  // disabled
	control_run_override = 255;  // disabled
	control_key_map_override = 255;  // disabled
	gui_control_override_updated();
}

// restore the run/stop override to panel control
void sequencer_control_run_stop_restore(void) {
	control_run_override = 255;  // disabled
	gui_control_override_updated();
}

// get the current sequence
unsigned char sequencer_get_current_seq(void) {
	return current_seq_playing;
}

// set the current sequence
void sequencer_set_next_seq(unsigned char seq) {
	if(seq > (SONG_NUM_SEQ - 1)) return;

	// if the song is playing, don't interrupt the current loop pass
	if(clock_get_song_playing()) {
		next_cued_seq = seq;  // cue up for the next pass
	}
	// otherwise reset everything and set it immediately
	else {
		sequencer_reset_song_pos();
		// if we have to force another sequence to change
		if(seq != current_seq) {
			next_cued_seq = seq;
			current_step_count = STEP_INVALID;  // invalidate the current position
			sequencer_advance_step();
		}
		// force correct display
		current_seq_playing = current_seq;
		current_step_index_playing = sequencer_compute_current_step();
		gui_playback_updated();
	}
}

// get the current sequence step index
unsigned char sequencer_get_current_step_index(void) {
	return current_step_index_playing;
}

// set a note for preview
void sequencer_play_audition_note(unsigned char part, unsigned char note) {
	if(part > 1) return;
	sequencer_stop_note(part);
	sequencer_start_note(part, note);
}

// calibrate the CV outputs
void sequencer_cv_cal_start(void) {
	clock_stop_command();
}

// set a CV calibration voltage
void sequencer_cv_set_cal(unsigned char part, char octave) {
	unsigned char note;
	if(part > 1) return;
	if(octave > 6) return;
	note = octave * 12;

	// don't multi-trigger
	if(current_note[part] == note) return;

	// turn off notes if already playing
	if(current_note[part] != 255) {
		_midi_tx_note_off(seq_midi_get_channel(part), current_note[part] + MIDI_NOTE_OFFSET);
		current_note[part] = 255;
	}

	// send MIDI note
	current_note[part] = note;
	gate_time_count[part] = 0;
	_midi_tx_note_on(seq_midi_get_channel(part), current_note[part] + MIDI_NOTE_OFFSET, 100);
	// control analog output
	cv_output_note_on(part, current_note[part]);
	// reset the note timeout
	if(clock_get_song_playing()) note_kill_timeout = NOTE_KILL_TIME_RUN;
	else note_kill_timeout = NOTE_KILL_TIME_STOP;	
}


// song is loaded - need to reset the start position
void sequencer_new_song_loaded(void) {
	// clock control
	clock_tick_count = 0;
	note_kill_timeout = 0;
	// sequencer internal
	sequencer_reset_song_pos();
	sequencer_control_restore();
}
