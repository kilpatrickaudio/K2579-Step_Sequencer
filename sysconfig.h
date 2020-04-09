/*
 * K2579 Step Sequencer - System Config
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 */
#define SYSCONFIG_MAX_CLOCK_DIV 24

// mod input assignments
#define SYSCONFIG_MOD_NONE 0
#define SYSCONFIG_MOD_NEXT_SEQ 1
#define SYSCONFIG_MOD_SEQ_START 2
#define SYSCONFIG_MOD_SEQ_LEN 3
#define SYSCONFIG_MOD_RUN_STOP 4
#define SYSCONFIG_MOD_GATE1 5
#define SYSCONFIG_MOD_GATE2 6
#define SYSCONFIG_MOD_SEQ_DIR 7
#define SYSCONFIG_MOD_KEY_MAP 8
#define SYSCONFIG_MAX_MOD_ASSIGN 7
// the KEY_MAP mod is not assignable

// key transpose assignment
#define SYSCONFIG_KEY_TRANSPOSE_OFF 0
#define SYSCONFIG_KEY_TRANSPOSE1 1
#define SYSCONFIG_KEY_TRANSPOSE2 2
#define SYSCONFIG_KEY_TRANSPOSE12 3

// key trigger assignments
#define SYSCONFIG_KEY_TRIGGER_LATCH 0
#define SYSCONFIG_KEY_TRIGGER_MOM 1

// key map
#define SYSCONFIG_KEY_MAP_A 0
#define SYSCONFIG_KEY_MAP_B 1

// reset modes
#define SYSCONFIG_RESET_MODE_SONG 0
#define SYSCONFIG_RESET_MODE_SEQ 1

// init the global config
void sysconfig_init(void);

// run the sysconfig task
void sysconfig_task(void);

// reset all settings
void sysconfig_reset_all(void);

// get the clock divider
unsigned char sysconfig_get_clock_div(void);

// set the clock divider
void sysconfig_set_clock_div(unsigned char div);

// get a mod assignment
unsigned char sysconfig_get_mod_assign(unsigned char part);

// set a mod assign
void sysconfig_set_mod_assign(unsigned char part, unsigned char assign);

// get the live audition state
unsigned char sysconfig_get_live_aud(void);

// set the live audition state
void sysconfig_set_live_aud(unsigned char aud);

// get a midi part channel
unsigned char sysconfig_get_midi_channel(unsigned char part);

// set a midi part channel
void sysconfig_set_midi_channel(unsigned char part, unsigned char channel);

// get the key transpose assignment
unsigned char sysconfig_get_key_transpose(void);

// set the key transpose assignment
void sysconfig_set_key_transpose(unsigned char assign);

// get the key trigger mode
unsigned char sysconfig_get_key_trigger(void);

// set the key trigger mode
void sysconfig_set_key_trigger(unsigned char mode);

// get the screen contrast
unsigned char sysconfig_get_lcd_contrast(void);

// set the screen contrast
void sysconfig_set_lcd_contrast(unsigned char contrast);

// get the clock speed
unsigned char sysconfig_get_clock_speed(void);

// set the clock speed
void sysconfig_set_clock_speed(unsigned char speed);

// get the reset mode
unsigned char sysconfig_get_reset_mode(void);

// set the reset mode
void sysconfig_set_reset_mode(unsigned char reset);

// get the current song
unsigned char sysconfig_get_current_song(void);

// set the current song
void sysconfig_set_current_song(unsigned char current_song);

// get the key map
unsigned char sysconfig_get_key_map(void);

// set the key map
void sysconfig_set_key_map(unsigned char key_map);

