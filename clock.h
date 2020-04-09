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
 */
// init the clock input
void clock_init(void);

// run a task to time out clock inputs
void clock_task(void);

// get the clock speed
unsigned char clock_get_speed(void);

// set the clock speed
void clock_set_speed(unsigned char speed);

// gets the song playing state
unsigned char clock_get_song_playing(void);

// midi clock tick received
void clock_midi_tick(void);

// midi clock start received
void clock_midi_start(void);

// midi clock continue received
void clock_midi_continue(void);

// midi clock stop received
void clock_midi_stop(void);

// clock input triggered
void clock_clock_input(void);

// reset input triggered
void clock_reset_input(void);

// clock run command
void clock_run_command(void);

// clock stop command
void clock_stop_command(void);

// clock run/stop toggle
void clock_run_stop_toggle(void);

