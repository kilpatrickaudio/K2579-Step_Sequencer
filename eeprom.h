/*
 * K2579 Step Sequencer - EEPROM Driver
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 */
// initialize the EEPROM driver
void eeprom_init(void);

// write a page of 32 bytes to the EEPROM
void eeprom_write_page(int addr, unsigned char buf[]);

// read a page of 32 bytes from the EEPROM
void eeprom_read_page(int addr, unsigned char buf[]);

