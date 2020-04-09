/*
 * K2579 Step Sequencer - Scale Processing
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 */
#define SCALE_CHROMATIC 0
#define SCALE_MAJOR 1
#define SCALE_NAT_MINOR 2
#define SCALE_HAR_MINOR 3
#define SCALE_WHOLE 4
#define SCALE_PENT 5
#define SCALE_DIM 6
#define SCALE_LEVEL 7

// convert a note number to a name
void scale_note_to_name(unsigned char note, unsigned char scale, char *str);

// convert a scale type to a name
void scale_type_to_name(unsigned char scale, char *str);

// quantize a note to the selectec scale
unsigned char scale_quantize(unsigned char note, unsigned char scale);

// adjust a note to fit within the selected span
//
// - this is not for use to scale input values
// - notes are shifted down so they fall within the current span
//
unsigned char scale_span_adjust(unsigned char note, unsigned char span);

