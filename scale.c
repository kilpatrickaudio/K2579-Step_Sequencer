/*
 * K2579 Step Sequencer - Scale Processing
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 */
#include <stdio.h>
#include "scale.h"
#include "song.h"
#include "scale_tables.h"

// convert a note number to a name
void scale_note_to_name(unsigned char note, unsigned char scale, char *str) {
	unsigned char degree, octave;
	if(note == SONG_STEP_REST) sprintf(str, "REST");
	else if(note == SONG_STEP_NONE) sprintf(str, "NONE");
	else if(note == SONG_STEP_RAND) sprintf(str, "RAND");
	else {
		degree = note % 12;
		octave = note / 12;
		// minor scales
		if((scale == SCALE_NAT_MINOR) | (scale == SCALE_HAR_MINOR)) {
			if(degree == 0) sprintf(str, "C %d", (octave + 1));
			else if(degree == 2) sprintf(str, "D %d", (octave + 1));
			else if(degree == 3) sprintf(str, "Eb%d", (octave + 1));
			else if(degree == 5) sprintf(str, "F %d", (octave + 1));
			else if(degree == 7) sprintf(str, "G %d", (octave + 1));
			else if(degree == 8) sprintf(str, "Ab%d", (octave + 1));
			else if(degree == 10) sprintf(str, "Bb%d", (octave + 1));
			else if(degree == 11) sprintf(str, "B %d", (octave + 1));			
		}
		// diminished
		else if(scale == SCALE_DIM) {
			if(degree == 0) sprintf(str, "C %d", (octave + 1));
			else if(degree == 2) sprintf(str, "D %d", (octave + 1));
			else if(degree == 3) sprintf(str, "Eb%d", (octave + 1));
			else if(degree == 5) sprintf(str, "F %d", (octave + 1));			
			else if(degree == 6) sprintf(str, "F#%d", (octave + 1));
			else if(degree == 8) sprintf(str, "G#%d", (octave + 1));
			else if(degree == 9) sprintf(str, "A %d", (octave + 1));
			else if(degree == 11) sprintf(str, "B %d", (octave + 1));			
		}
		// level
		else if(scale == SCALE_LEVEL) {
			sprintf(str, " %02d", note);
		}
		// chromatic, major, pent, whole
		else {
			if(degree == 0) sprintf(str, "C %d", (octave + 1));
			else if(degree == 1) sprintf(str, "C#%d", (octave + 1));
			else if(degree == 2) sprintf(str, "D %d", (octave + 1));
			else if(degree == 3) sprintf(str, "D#%d", (octave + 1));
			else if(degree == 4) sprintf(str, "E %d", (octave + 1));
			else if(degree == 5) sprintf(str, "F %d", (octave + 1));
			else if(degree == 6) sprintf(str, "F#%d", (octave + 1));
			else if(degree == 7) sprintf(str, "G %d", (octave + 1));
			else if(degree == 8) sprintf(str, "G#%d", (octave + 1));
			else if(degree == 9) sprintf(str, "A %d", (octave + 1));
			else if(degree == 10) sprintf(str, "A#%d", (octave + 1));
			else if(degree == 11) sprintf(str, "B %d", (octave + 1));
		}
	}
}

// convert a scale type to a name
void scale_type_to_name(unsigned char scale, char *str) {
	if(scale == SCALE_CHROMATIC) sprintf(str, "chromatic");
	else if(scale == SCALE_MAJOR) sprintf(str, " major");
	else if(scale == SCALE_NAT_MINOR) sprintf(str, "nat minor");
	else if(scale == SCALE_HAR_MINOR) sprintf(str, "har minor");
	else if(scale == SCALE_WHOLE) sprintf(str, "whole");
	else if(scale == SCALE_PENT) sprintf(str, "pentatonic");
	else if(scale == SCALE_DIM) sprintf(str, "diminished");
	else if(scale == SCALE_LEVEL) sprintf(str, "level");
}

// quantize a note to the current scale
unsigned char scale_quantize(unsigned char note, unsigned char scale) {
	unsigned char not = note;
	int i;
	// quantize notes from 0-48
	if(not < 49) {
		if(scale == SCALE_MAJOR) {
			for(i = SCALE_LEN_MAJOR - 1; i >= 0; i --) {
				if(scale_major[i] <= not) {
					not = scale_major[i];
					break;
				}
			}
		}
		else if(scale == SCALE_NAT_MINOR) {
			for(i = SCALE_LEN_NAT_MINOR - 1; i >= 0; i --) {
				if(scale_nat_minor[i] <= not) {
					not = scale_nat_minor[i];
					break;
				}
			}
		}
		else if(scale == SCALE_HAR_MINOR) {
			for(i = SCALE_LEN_HAR_MINOR - 1; i >= 0; i --) {
				if(scale_har_minor[i] <= not) {
					not = scale_har_minor[i];
					break;
				}
			}
		}
		else if(scale == SCALE_WHOLE) {
			for(i = SCALE_LEN_WHOLE - 1; i >= 0; i --) {
				if(scale_whole[i] <= not) {
					not = scale_whole[i];
					break;
				}
			}
		}
		else if(scale == SCALE_PENT) {
			for(i = SCALE_LEN_PENT - 1; i >= 0; i --) {
				if(scale_pent[i] <= not) {
					not = scale_pent[i];
					break;
				}
			}
		}
		else if(scale == SCALE_DIM) {
			for(i = SCALE_LEN_DIM - 1; i >= 0; i --) {
				if(scale_dim[i] <= not) {
					not = scale_dim[i];
					break;
				}
			}
		}
	}
	return not;
}

// adjust a note to fit within the selected span
//
// - this is not for use to scale input values
// - notes are shifted down the minimum amount so that they
//   fall within the current span
//
unsigned char scale_span_adjust(unsigned char note, unsigned char span) {
	if(note < 49) {
		if(span == 3 && note > 36) return note - 12;
		else if(span == 2) {
//			if(note > 36) return note - 24;
//			else if(note > 24) return note - 12;
			if(note > 24) return (note - 24) + 12;
		}
		else if(span == 1) {
//			if(note > 36) return note - 36;
//			else if(note > 24) return note - 24;
//			else if(note > 12) return note - 12;
			if(note > 36) return (note - 36) + 12;
			else if(note > 24) return (note - 24) + 12;
			else if(note > 12) return (note - 12) + 12;
			else return note + 12;
		}
	}
	return note;
}

