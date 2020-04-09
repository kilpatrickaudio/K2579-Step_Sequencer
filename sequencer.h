/*
 * K2579 Step Sequencer - Sequencer Core
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 */
// initialize the sequencer
void sequencer_init(void);

// run the sequencer task every 256us
void sequencer_task(void);

//
// MIDI/analog clock handlers
//
// set song position
void sequencer_midi_song_pos(unsigned int pos);

// clock pulse was received
void sequencer_clock_tick(void);

// start song at beginning
void sequencer_clock_start(void);

// stop song
void sequencer_clock_stop(void);

// get the current clock div count
unsigned char sequencer_get_clock_div_count(void);

//
// external control
//
// MIDI key trigger
void sequencer_key_trigger(unsigned char sequence);

// MIDI key transpose
void sequencer_key_transpose(char transpose);

// MIDI/analog control change was received
void sequencer_control_change(unsigned char mod, unsigned char value);

// get a control override value
// returns values in the local range
unsigned char sequencer_get_control_override(unsigned char mod);

// restore the current CC overrides to the default value
void sequencer_control_restore(void);

// restore the run/stop override to panel control
void sequencer_control_run_stop_restore(void);

// get the current sequence
unsigned char sequencer_get_current_seq(void);

// set the current sequence
void sequencer_set_next_seq(unsigned char seq);

// get the current sequence step position
unsigned char sequencer_get_current_step_index(void);

// set a note for preview
void sequencer_play_audition_note(unsigned char part, unsigned char note);

// calibrate the CV outputs
void sequencer_cv_cal_start(void);

// set a CV calibration voltage
void sequencer_cv_set_cal(unsigned char part, char octave);

// song is loaded - need to reset the start position
void sequencer_new_song_loaded(void);
