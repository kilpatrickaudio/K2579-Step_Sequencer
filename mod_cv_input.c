/*
 * K2579 Step Sequencer - Mod CV Input Handler
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 * Hardware I/O:
 *
 *  RB8/AN8		- mod 1 in					- analog (via analog lib)
 *  RB9/AN9		- mod 2 in					- analog (via analog lib)
 *
 */
#include "mod_cv_input.h"
#include "analog_input.h"
#include "sequencer.h"
#include "song.h"
#include "sysconfig.h"

unsigned int mod1_val;
unsigned int mod2_val;

// init the mod CV input
void mod_cv_input_init(void) {
	mod1_val = 0;
	mod2_val = 0;
}

// run the mod CV input task
void mod_cv_input_task(void) {
	unsigned int temp;
	unsigned char assign;

	// mod1
	temp = analog_input_get_val(ANA_MOD1_IN);
	if(temp != mod1_val) {
		mod1_val = temp;
		assign = sysconfig_get_mod_assign(0);
		if(assign != SYSCONFIG_MOD_NONE) {
			sequencer_control_change(assign, mod1_val >> 3);
		}
	}

	// mod2
	temp = analog_input_get_val(ANA_MOD2_IN);
	if(temp != mod2_val) {
		mod2_val = temp;
		assign = sysconfig_get_mod_assign(1);
		if(assign != SYSCONFIG_MOD_NONE) {
			sequencer_control_change(assign, mod2_val >> 3);
		}
	}
}

