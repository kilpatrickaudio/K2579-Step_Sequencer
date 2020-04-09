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
// init the mod CV input
void mod_cv_input_init(void);

// run the mod CV input task
void mod_cv_input_task(void);
