/*
 * K2579 Step Sequencer - Panel
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 */
#define PANEL_MODE_SW 1
#define PANEL_ENTER_SW 2
#define PANEL_PAGE_DOWN_SW 3
#define PANEL_PAGE_UP_SW 4
#define PANEL_LIVE_SW 5
#define PANEL_RUN_STOP_SW 6
#define PANEL_RESET_SW 7

// initialize the panel
void panel_init(void);

// panel task - call this every 1ms
void panel_task(void);

// get the next switch in the queue
unsigned char panel_get_sw(void);

// set the clock LED
void panel_set_clock_led(unsigned char timeout);

// set the mod LED
void panel_set_mod_led(unsigned char timeout);

// set a CV/gate LED
void panel_set_cv_gate_led(unsigned char chan, unsigned char timeout);

// get a step pot
int panel_get_step_pot(unsigned char pot);

// get whether the LCD contrast magic button press has taken place
unsigned char panel_set_lcd_contrast(void);
