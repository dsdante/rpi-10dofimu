#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdint.h>

int bcm_model();
int i2c_open(uint8_t device);
int i2c_read(uint16_t address, uint8_t reg, uint16_t length, uint8_t buff[]);
int i2c_write(uint16_t address, uint8_t reg, uint8_t value);
void i2c_close();

#endif  // HARDWARE_H
