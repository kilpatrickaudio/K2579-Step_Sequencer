/*
 * K2579 Step Sequencer - Analog Input
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 *  RB0/AN0		- step 1 pot				- analog (via analog lib)
 *  RB1/AN1		- step 2 pot				- analog (via analog lib)
 *  RB8/AN8		- mod 1 in					- analog
 *  RB9/AN9		- mod 2 in					- analog
 *
 */
#include <plib.h>
#include "analog_input.h"

#define HIST_MAX 4
unsigned char hist_count;
int an_hist[4][HIST_MAX];
int old_val[4];

// initialize the analog input
void analog_input_init(void) {
	int i, j;
	// set up the ADC
	OpenADC10(ADC_MODULE_ON | ADC_IDLE_STOP | ADC_FORMAT_INTG | ADC_CLK_AUTO | ADC_AUTO_SAMPLING_ON | ADC_SAMP_ON,
		ADC_VREF_AVDD_AVSS | ADC_OFFSET_CAL_DISABLE | ADC_SCAN_ON | ADC_SAMPLES_PER_INT_4 | ADC_BUF_16 | ADC_ALT_INPUT_OFF,
		ADC_SAMPLE_TIME_16 | ADC_CONV_CLK_PB | ADC_CONV_CLK_32Tcy,
		ENABLE_AN0_ANA | ENABLE_AN1_ANA | ENABLE_AN8_ANA | ENABLE_AN9_ANA,
		SKIP_SCAN_AN2 | SKIP_SCAN_AN3 | SKIP_SCAN_AN4 | SKIP_SCAN_AN5 | SKIP_SCAN_AN6 | SKIP_SCAN_AN7 | \
		SKIP_SCAN_AN10 | SKIP_SCAN_AN11 | SKIP_SCAN_AN12 | SKIP_SCAN_AN13 | SKIP_SCAN_AN14 | SKIP_SCAN_AN15);

	hist_count = 0;
	for(i = 0; i < 4; i ++) {
		old_val[i] = 0;
		for(j = 0; j < HIST_MAX; j ++) {
			an_hist[i][j] = 0;
		}
	}
} 

// run the analog task - call this from the timer
void analog_input_task(void) {
	int i, j;
	int new_val[4];

	an_hist[0][hist_count] = ReadADC10(0);
	an_hist[1][hist_count] = ReadADC10(1);
	an_hist[2][hist_count] = ReadADC10(2);
	an_hist[3][hist_count] = ReadADC10(3);
	hist_count = (hist_count + 1) & 0x03;

	for(i = 0; i < 4; i ++) {
		new_val[i] = 0;
		for(j = 0; j < HIST_MAX; j ++) {
			new_val[i] += an_hist[i][j];
		}
		new_val[i] = (new_val[i] >> 2);
		if(abs(new_val[i] - old_val[i]) > 5) {
			old_val[i] = new_val[i];
		}
	}
}

// get the most recent sample for a specific channel
int analog_input_get_val(unsigned char input) {
	if(input > 3) return 0;
	return old_val[input];
}

