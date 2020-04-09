/*
 * K2579 Step Sequencer - Song
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 */
#include <stdlib.h>
#include "song.h"
#include "scale.h"
#include "midi.h"

#define PADDING1_LEN 16
#define PADDING2_LEN 19
#define PADDING3_LEN 30

// sequence structure
typedef struct {
	// page 0 - 32 bytes
	unsigned char notes[2][SONG_NUM_STEPS];  // notes
	// page 1 - 32 bytes
	unsigned char step_len[SONG_NUM_STEPS];  // step lengths
	unsigned char padding1[PADDING1_LEN];  // page 1 padding
	// page 2 - 32 bytes	
	unsigned char start;  // 0-15 = start position
	unsigned char len;  // 1-16 = length 1-16
	unsigned char dir;  // 0-3 = direction types
	unsigned char loop;  // 0-15 = number of loop times
	unsigned char next;  // 0-15 = next sequence to play
	unsigned char gate1;  // 1-48 = clock pulses (not divided)
	unsigned char scale1;  // 0-5 = scale types
	unsigned char span1;  // 1-4 = 1-4 octaves
	char offset1;  // -12 to +12 = -12 to +12 semitones
	unsigned char gate2;  // 1-48 = clock pulses (not divided)
	unsigned char scale2;  // 0-5 = scale types
	unsigned char span2;  // 1-4 = 1-4 octaves
	char offset2;  // -12 to +12 = -12 to +12 semitones
	unsigned char padding2[PADDING2_LEN];  // page 2 padding
	// page 3 - 32 bytes
	unsigned char padding3[PADDING3_LEN];  // page 3 padding
	unsigned char version;  // the version number of the sequence data
	unsigned char configured;  // the configuration mark byte
} sequence;

sequence seqs[SONG_NUM_SEQ];

// intialize the song
void song_init(void) {
	song_clear_song();
}

// load a buffer into a sequence
void song_load_seq_buf(unsigned char seq, unsigned char buf[]) {
	int i;
	if(seq > 15) return;
	char *p = (char *)&seqs[seq];
	for(i = 0; i < 128; i ++) {
		*(p + i) = buf[i];
	}
}

// save a buffer from a sequence
void song_save_seq_buf(unsigned char seq, unsigned char buf[]) {
	int i;
	if(seq > 15) return;
	char *p = (char *)&seqs[seq];
	for(i = 0; i < 128; i ++) {
		buf[i] = *(p + i);
	}
}

// clear the song
void song_clear_song(void) {
	int i;
	for(i = 0; i < SONG_NUM_SEQ; i++) {
		song_clear_seq(i);
	}
}

// clear a sequence
void song_clear_seq(unsigned char seq) {
	if(seq > (SONG_NUM_SEQ - 1)) return;
	int i;
	seqs[seq].start = 0;  // start at pos 1
	seqs[seq].len = SONG_NUM_STEPS;  // 16 steps
	seqs[seq].dir = SONG_DIR_FWD;  // forward
	seqs[seq].loop = 0;  // loop 0 times
	seqs[seq].next = seq;  // play this again
	seqs[seq].gate1 = 5;  // just less than 16th note at 24ppq
	seqs[seq].scale1 = SCALE_CHROMATIC;  // chromatic
	seqs[seq].span1 = 4;  // 4 octaves
	seqs[seq].offset1 = 0;  // normal offset
	seqs[seq].gate2 = 5;  // 16th note at 24ppq
	seqs[seq].scale2 = SCALE_CHROMATIC;  // chromatic
	seqs[seq].span2 = 4;  // 4 octaves
	seqs[seq].offset2 = 0;  // normal offset
	// step 1 plays a low note
	seqs[seq].notes[0][0] = 0;  // base note
	seqs[seq].notes[1][0] = 0;  // base note
	seqs[seq].step_len[0] = 0;  // default
	// steps 2-16 are rests
	for(i = 1; i < SONG_NUM_STEPS; i ++) {
		seqs[seq].notes[0][i] = SONG_STEP_REST;  // rest
		seqs[seq].notes[1][i] = SONG_STEP_REST;  // rest
		seqs[seq].step_len[i] = 0;  // default
	}
	// padding
	for(i = 0; i < PADDING1_LEN; i ++) {
		seqs[seq].padding1[i] = 0xe1;
	}
	for(i = 0; i < PADDING2_LEN; i ++) {
		seqs[seq].padding2[i] = 0xe2;
	}
	for(i = 0; i < PADDING3_LEN; i ++) {
		seqs[seq].padding3[i] = 0xe3;
	}
	seqs[seq].version = SONG_VERSION;
	seqs[seq].configured = SONG_CONFIGURE_MARK;
}

// copy a sequence
void song_copy_seq(unsigned char dest, unsigned char src) {	
	int i;
	if(src > (SONG_NUM_SEQ - 1)) return;
	if(dest > (SONG_NUM_SEQ - 1)) return;
	seqs[dest].start = seqs[src].start;
	seqs[dest].len = seqs[src].len;
	seqs[dest].dir = seqs[src].dir;
	seqs[dest].loop = seqs[src].loop;
	seqs[dest].next = seqs[src].next;
	seqs[dest].gate1 = seqs[src].gate1;
	seqs[dest].scale1 = seqs[src].scale1;
	seqs[dest].span1 = seqs[src].span1;
	seqs[dest].offset1 = seqs[src].offset1;
	seqs[dest].gate2 = seqs[src].gate2;
	seqs[dest].scale2 = seqs[src].scale2;
	seqs[dest].span2 = seqs[src].span2;
	seqs[dest].offset2 = seqs[src].offset1;
	for(i = 0; i < SONG_NUM_STEPS; i ++) {
		seqs[dest].notes[0][i] = seqs[src].notes[0][i];
		seqs[dest].notes[1][i] = seqs[src].notes[1][i];
	}
}

// get the seq start
unsigned char song_get_seq_start(unsigned char seq) {
	if(seq > (SONG_NUM_STEPS - 1)) return 0;
	return seqs[seq].start;
}

// set the seq start
void song_set_seq_start(unsigned char seq, unsigned char start) {
	if(seq > (SONG_NUM_SEQ - 1)) return;
	if(start > (SONG_NUM_STEPS - 1)) seqs[seq].start = (SONG_NUM_STEPS - 1);
	else seqs[seq].start = start;
}

// get the seq len
unsigned char song_get_seq_len(unsigned char seq) {
	if(seq > (SONG_NUM_SEQ - 1)) return 0;
	return seqs[seq].len;
}

// set the seq len
void song_set_seq_len(unsigned char seq, unsigned char len) {
	if(seq > (SONG_NUM_SEQ - 1)) return;
	if(len > SONG_NUM_STEPS) seqs[seq].len = SONG_NUM_STEPS;
	else seqs[seq].len = len;
}

// get a step length
unsigned char song_get_step_len(unsigned char seq, unsigned char step) {
	if(seq > (SONG_NUM_SEQ - 1)) return 0;
	if(step > (SONG_NUM_STEPS - 1)) return 0;
	return seqs[seq].step_len[step];
}

// set a step length
void song_set_step_len(unsigned char seq, unsigned char step, unsigned char len) {
	if(seq > (SONG_NUM_SEQ - 1)) return;
	if(step > (SONG_NUM_STEPS - 1)) return;
	if(len > 31) seqs[seq].step_len[step] = 31;
	seqs[seq].step_len[step] = len;
}

// get the seq dir
unsigned char song_get_seq_dir(unsigned char seq) {
	if(seq > (SONG_NUM_SEQ - 1)) return 0;
	return seqs[seq].dir;
}

// set the seq dir
void song_set_seq_dir(unsigned char seq, unsigned char dir) {
	if(seq > (SONG_NUM_SEQ - 1)) return;
	if(dir > SONG_MAX_DIR) seqs[seq].dir = SONG_MAX_DIR;
	else seqs[seq].dir = dir;
}

// get the seq loop
unsigned char song_get_seq_loop(unsigned char seq) {
	if(seq > (SONG_NUM_SEQ - 1)) return 0;
	return seqs[seq].loop;
}

// set the seq loop
void song_set_seq_loop(unsigned char seq, unsigned char loop) {
	if(seq > (SONG_NUM_SEQ - 1)) return;
	if(loop > SONG_MAX_LOOPS) seqs[seq].loop = SONG_MAX_LOOPS;
	else seqs[seq].loop = loop;
}

// get the seq next
unsigned char song_get_seq_next(unsigned char seq) {
	if(seq > (SONG_NUM_SEQ - 1)) return 0;
	return seqs[seq].next;
}

// set the seq next
void song_set_seq_next(unsigned char seq, unsigned char next) {
	if(seq > (SONG_NUM_SEQ - 1)) return;
	if(next > SONG_NUM_SEQ - 1) seqs[seq].next = SONG_NUM_SEQ - 1;
	else seqs[seq].next = next;
}

// get a seq note
unsigned char song_get_note(unsigned char seq, unsigned char part, unsigned char step) {
	if(seq > (SONG_NUM_SEQ - 1)) return 0;
	if(part > 1) return 0;
	if(step > (SONG_NUM_STEPS - 1)) return 0;
	return seqs[seq].notes[part][step];
}

// set a seq note
//
// - note range should be 0-63 = 0-48 (notes) and 254-255 (extended note types)
// - notes saved with their raw values
// - prescale the value based on the span - for input control
//
void song_set_note(unsigned char seq, unsigned char part, unsigned char step, unsigned char note) {
	if(seq > (SONG_NUM_SEQ - 1)) return;
	if(part > 1) return;
	if(step > (SONG_NUM_STEPS - 1)) return;
	if(note < 49) seqs[seq].notes[part][step] = note;
	else if(note == SONG_STEP_RAND) seqs[seq].notes[part][step] = SONG_STEP_RAND;
	else if(note == SONG_STEP_NONE) seqs[seq].notes[part][step] = SONG_STEP_NONE;
	else if(note == SONG_STEP_REST) seqs[seq].notes[part][step] = SONG_STEP_REST;
}

// get the seq gate
unsigned char song_get_gate(unsigned char seq, unsigned char part) {
	if(seq > (SONG_NUM_SEQ - 1)) return 0;
	if(part > 1) return 0;
	if(part == 1) return seqs[seq].gate2;
	return seqs[seq].gate1;
}

// set the seq gate
void song_set_gate(unsigned char seq, unsigned char part, unsigned char gate) {
	unsigned char gat = gate;
	if(gat > 48) gat = 48;
	else if(gat < 1) gat = 1;
	if(seq > (SONG_NUM_SEQ - 1)) return;
	if(part > 1) return;
	if(part == 1) seqs[seq].gate2 = gat;
	else seqs[seq].gate1 = gat;
}

// set the seq scale
unsigned char song_get_scale(unsigned char seq, unsigned char part) {
	if(seq > (SONG_NUM_SEQ - 1)) return 0;
	if(part > 1) return 0;
	if(part == 1) return seqs[seq].scale2;
	return seqs[seq].scale1;
}

// get the seq scale
void song_set_scale(unsigned char seq, unsigned char part, unsigned char scale) {
	unsigned char scl = scale;
	if(scl > 7) scl = 7;
	if(seq > (SONG_NUM_SEQ - 1)) return;
	if(part > 1) return;
	if(part == 1) seqs[seq].scale2 = scl;
	else seqs[seq].scale1 = scl;
}

// get the seq span
unsigned char song_get_span(unsigned char seq, unsigned char part) {
	if(seq > (SONG_NUM_SEQ - 1)) return 0;
	if(part > 1) return 0;
	if(part == 1) return seqs[seq].span2;
	return seqs[seq].span1;
}

// set the seq span
void song_set_span(unsigned char seq, unsigned char part, unsigned char span) {
	unsigned char spn = span;
	if(spn < 1) spn = 1;
	else if(spn > 4) spn = 4;
	if(seq > (SONG_NUM_SEQ - 1)) return;
	if(part > 1) return;
	if(part == 1) seqs[seq].span2 = spn;
	else seqs[seq].span1 = spn;
}

// get the seq offset
char song_get_offset(unsigned char seq, unsigned char part) {
	if(seq > (SONG_NUM_SEQ - 1)) return 0;
	if(part > 1) return 0;
	if(part == 1) return seqs[seq].offset2;
	return seqs[seq].offset1;
}

// set the seq offset
void song_set_offset(unsigned char seq, unsigned char part, char offset) {
	char offst = offset;
	if(offst < -12) offst = -12;
	else if(offst > 12) offst = 12;
	if(seq > (SONG_NUM_SEQ - 1)) return;
	if(part > 1) return;
	if(part == 1) seqs[seq].offset2 = offst;
	else seqs[seq].offset1 = offst;
}

// copy a part to the other part in the same seq
void song_part_copy(unsigned char seq, unsigned char part) {
	int i;
	if(part > 1) return;
	for(i = 0; i < SONG_NUM_STEPS; i ++) {
		seqs[seq].notes[(part + 1) & 0x01][i] = seqs[seq].notes[part][i];
	}
}

// invert the intervals in the selected part
void song_part_invert(unsigned char seq, unsigned char part) {
	if(part > 1) return;
	int i;
	for(i = 0; i < SONG_NUM_STEPS; i ++) {
		if(seqs[seq].notes[part][i] < 49) {
			seqs[seq].notes[part][i] = 48 - seqs[seq].notes[part][i];
		}	
	}	
}

// retrograde the selected part
void song_part_retrograde(unsigned char seq, unsigned char part) {
	if(part > 1) return;
	int i, j;
	unsigned char temp;
	j = SONG_NUM_STEPS - 1;
	for(i = 0; i < (SONG_NUM_STEPS >> 1); i ++) {
		temp = seqs[seq].notes[part][i];
		seqs[seq].notes[part][i] = seqs[seq].notes[part][j];
		seqs[seq].notes[part][j] = temp;
		j --;
	}	
}

// randomize a part
void song_part_randomize(unsigned char seq, unsigned char part) {
	if(part > 1) return;
	int i;
	for(i = 0; i < SONG_NUM_STEPS; i ++) {
		song_set_note(seq, part, i, rand() & 0x3f);
	}
}

// clear the selected part
void song_part_clear(unsigned char seq, unsigned char part) {
	if(part > 1) return;
	int i;
	for(i = 0; i < SONG_NUM_STEPS; i ++) {
		seqs[seq].notes[part][i] = SONG_STEP_REST;  // rest
	}
}

// get a random note - used for RAND note type
unsigned char song_get_rand_note(void) {
	return (rand() & 0x3f) % 48;
}

