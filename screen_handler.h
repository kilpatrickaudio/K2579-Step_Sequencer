/*
 * K2579 Step Sequencer - Screen Handler
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 */
// initialize the screen handler
void screen_init(void);

// call the screen task every 16ms
void screen_task(void);

// write text to a line
void screen_write_line(unsigned char line, char *str);

// write a popup message
void screen_write_popup(int timeout, char *str1, char *str2);

// get the contrast
unsigned char screen_get_contrast(void);

// adjust the screen contrast
void screen_set_contrast(unsigned char contrast);

