/*
 * K2579 Step Sequencer - Analog Input
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 */
#define ANA_PARAM1_POT 0
#define ANA_PARAM2_POT 1
#define ANA_MOD1_IN 2
#define ANA_MOD2_IN 3

// initialize the analog input
void analog_input_init(void);

// run the analog task - call this from the timer
void analog_input_task(void);

// get the most recent sample for a specific channel
int analog_input_get_val(unsigned char input);
