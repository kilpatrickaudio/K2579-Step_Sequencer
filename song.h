/*
 * K2579 Step Sequencer - Song
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 */
#define SONG_VERSION 0x01
#define SONG_CONFIGURE_MARK 0x55

// song parameters
#define SONG_NUM_STEPS 16
#define SONG_NUM_SEQ 16
#define SONG_MAX_LOOPS 15

// sequence directions
#define SONG_DIR_BACK 0
#define SONG_DIR_PONG 1
#define SONG_DIR_RAND 2
#define SONG_DIR_FWD 3
#define SONG_MAX_DIR 3

// sequence step values
#define SONG_STEP_RAND 253
#define SONG_STEP_NONE 254
#define SONG_STEP_REST 255

// intialize the song
void song_init(void);

// load a buffer into a sequence
void song_load_seq_buf(unsigned char seq, unsigned char buf[]);

// save a buffer from a sequence
void song_save_seq_buf(unsigned char seq, unsigned char buf[]);

// clear the song
void song_clear_song(void);

// clear a sequence
void song_clear_seq(unsigned char seq);

// copy a sequence
void song_copy_seq(unsigned char dest, unsigned char src);

// get the seq start
unsigned char song_get_seq_start(unsigned char seq);

// set the seq start
void song_set_seq_start(unsigned char seq, unsigned char start);

// get the seq len
unsigned char song_get_seq_len(unsigned char seq);

// set the seq len
void song_set_seq_len(unsigned char seq, unsigned char len);

// get a step length
unsigned char song_get_step_len(unsigned char seq, unsigned char step);

// set a step length
void song_set_step_len(unsigned char seq, unsigned char step, unsigned char len);

// get the seq dir
unsigned char song_get_seq_dir(unsigned char seq);

// set the seq dir
void song_set_seq_dir(unsigned char seq, unsigned char dir);

// get the seq loop
unsigned char song_get_seq_loop(unsigned char seq);

// set the seq loop
void song_set_seq_loop(unsigned char seq, unsigned char loop);

// get the seq next
unsigned char song_get_seq_next(unsigned char seq);

// set the seq next
void song_set_seq_next(unsigned char seq, unsigned char next);

// get a seq note
unsigned char song_get_note(unsigned char seq, unsigned char part, unsigned char step);

// set a seq note
//
// - note range should be 0-63 = 0-48 (notes) and 254-255 (extended note types)
// - notes saved with their raw values
// - prescale the value based on the span - for input control
//
void song_set_note(unsigned char seq, unsigned char part, unsigned char step, unsigned char note);

// get the seq gate
unsigned char song_get_gate(unsigned char seq, unsigned char part);

// set the seq gate
void song_set_gate(unsigned char seq, unsigned char part, unsigned char gate);

// set the seq scale
unsigned char song_get_scale(unsigned char seq, unsigned char part);

// get the seq scale
void song_set_scale(unsigned char seq, unsigned char part, unsigned char scale);

// get the seq span
unsigned char song_get_span(unsigned char seq, unsigned char part);

// set the seq span
void song_set_span(unsigned char seq, unsigned char part, unsigned char span);

// get the seq offset
char song_get_offset(unsigned char seq, unsigned char part);

// set the seq offset
void song_set_offset(unsigned char seq, unsigned char part, char offset);

// copy a part to the other part in the same seq
void song_part_copy(unsigned char seq, unsigned char part);

// invert the intervals in the selected part
void song_part_invert(unsigned char seq, unsigned char part);

// retrograde the selected part
void song_part_retrograde(unsigned char seq, unsigned char part);

// randomize the selected part
void song_part_randomize(unsigned char seq, unsigned char part);

// clear the selected part
void song_part_clear(unsigned char seq, unsigned char part);

// get a random note - used for RAND note type
unsigned char song_get_rand_note(void);
