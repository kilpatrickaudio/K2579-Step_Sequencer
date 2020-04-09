/*
 * LCD Driver for EA DIPS082 / EA DOGM163W-A - PIC32 
 *
 * Copyright 2010: Kilpatrick Audio
 * Writeen by: Andrew Kilpatrick
 *
 */
// initialize the display
void lcd_init(void);

// handle LCD writes
void lcd_task(void);

// clear the screen
void lcd_clear_screen(void);

// go to X / Y position
void lcd_goto_xy(unsigned char x, unsigned char y);

// shift the display to the left
void lcd_shift_left(void);

// shift the display to the right
void lcd_shift_right(void);

// print a string at the current position
void lcd_print_str(char *str);

// print a character at the current position
void lcd_print_char(char ch);

// get the contrast
unsigned char lcd_get_contrast(void);

// set the contrast - for digitally controlled contrast only
void lcd_set_contrast(unsigned char cont);

