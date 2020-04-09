/*
 * K2579 Step Sequencer - EEPROM Driver for 24LC64 8Kbyte EEPROM
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 *  RG2/SCL1	- EEPROM SCL				- I2C clock
 *  RG3/SDA1	- EEPROM SDA				- I2C data
 *
 */
#include <plib.h>
#include "eeprom.h"
#include "TimeDelay.h"

// i2c settings
#define PBCLOCK_FREQ 80000000
#define I2C_CLOCK_FREQ 400000
#define EE_I2C I2C1

// commands
#define EE_ADDR 0xa0

unsigned char i2c_buf[64];

// initialize the EEPROM driver
void eeprom_init(void) {
	I2CConfigure(EE_I2C, I2C_ENABLE_HIGH_SPEED);
	I2CSetFrequency(EE_I2C, PBCLOCK_FREQ, I2C_CLOCK_FREQ);
 	I2CEnable(EE_I2C, TRUE);
}

// write a page of 32 bytes to the EEPROM
void eeprom_write_page(int addr, unsigned char buf[]) {
	int len = 32;

	// start
	StartI2C1();
	IdleI2C1();

	MasterWriteI2C1(EE_ADDR | 0);        // Write control byte
 	IdleI2C1();
 	if(I2C1STATbits.ACKSTAT) return;            // NACK'ed by slave ?

 	MasterWriteI2C1((addr & 0xffe0) >> 8);            // Address of operation
 	IdleI2C1();
 	if(I2C1STATbits.ACKSTAT) return;            // NACK'ed by slave ?

 	MasterWriteI2C1(addr & 0xffe0);  // Address of operation
 	IdleI2C1();
 	if(I2C1STATbits.ACKSTAT) return;            // NACK'ed by slave ?

 	while(len) {
 		MasterWriteI2C1(*buf++);
 		IdleI2C1();
 		if(I2C1STATbits.ACKSTAT) return;            // NACK'ed by slave ?
 		len--;
 	}
 	StopI2C1();
 	IdleI2C1();

 	// do acknowledge poll (indicating, that eeprom has completed flashing our bytes
 	while(1) {
 		StartI2C1();
 		IdleI2C1();

 		MasterWriteI2C1(EE_ADDR | 0);    // Call the eeprom
 		IdleI2C1();

 		if(I2C1STATbits.ACKSTAT == 0) break;
 		StopI2C1();
 		IdleI2C1();
 	}
 	StopI2C1();
 	IdleI2C1();
}    

// read a page of 32 bytes from the EEPROM
void eeprom_read_page(int addr, unsigned char buf[]) {
	int len = 32;
	// start
 	StartI2C1();
 	IdleI2C1();

 	MasterWriteI2C1(EE_ADDR | 0);  // write control byte
 	IdleI2C1();
 	if(I2C1STATbits.ACKSTAT) return;  // NACK'ed by slave ?
 	MasterWriteI2C1((addr & 0xffe0) >> 8);  // address of operation
 	IdleI2C1();
 	if(I2C1STATbits.ACKSTAT) return;  // NACK'ed by slave ?

	MasterWriteI2C1(addr & 0xffe0);  // address of operation
 	IdleI2C1();
 	if(I2C1STATbits.ACKSTAT) return;  // NACK'ed by slave ?
 	RestartI2C1();
 	IdleI2C1();
 	MasterWriteI2C1(EE_ADDR | 1);  // read command
 	IdleI2C1();
 	if(I2C1STATbits.ACKSTAT) return;  // NACK'ed by slave ?
 	while(len) {
 		I2C1CONbits.RCEN = 1;
 		while(!DataRdyI2C1());
 		*buf = I2C1RCV;
 		buf++;
 		len--;
 		if(len) {
 			// ACK the slave
 			I2C1CONbits.ACKDT = 0;
 			I2C1CONbits.ACKEN = 1;
 		}
 		else {
 			// NACK the slave
 			I2C1CONbits.ACKDT = 1;
 			I2C1CONbits.ACKEN = 1;
 		}
 		while(I2C1CONbits.ACKEN == 1);  /* wait till ACK/NACK sequence is over */
 	}
 	StopI2C1();
 	IdleI2C1();
}

