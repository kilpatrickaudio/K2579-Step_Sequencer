/*
 * K2579 Step Sequencer - Sequencer MIDI Handler
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 */
// initialize the MIDI handler
void seq_midi_init(void);

// get the MIDI channel for a part
unsigned char seq_midi_get_channel(unsigned char part);

// set the MIDI channel for a part
void seq_midi_set_channel(unsigned char part, unsigned char channel);


