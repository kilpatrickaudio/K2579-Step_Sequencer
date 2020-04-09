/*
 * K2579 Step Sequencer - CV Output
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 */
// intialize the CV output
void cv_output_init(void);

// start a note on the CV output
void cv_output_note_on(unsigned char part, unsigned char note);

// stop a note on the CV output
void cv_output_note_off(unsigned char part);


