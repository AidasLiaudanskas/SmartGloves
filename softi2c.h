#ifndef _SOFTI2C_H_
#define _SOFTI2C_H_
#include "mbed.h"


// Assert START condition on I2C bus
void softi2c_start();

// Assert STOP condition on I2C bus
void softi2c_stop();

// Write 1 byte, and return the slave ACK bit.
uint8_t softi2c_put(uint8_t byte);

// Read 1 byte, and return the slave ACK bit.
uint8_t softi2c_get(uint8_t *ack_ptr = NULL);

// Start writing to the address addr (MSB aligned, i.e. addr & 0x1 == 0)
// Return ACK
uint8_t softi2c_startrw(uint8_t addr, uint8_t r);

#endif _SOFTI2C_H_
