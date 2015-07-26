// i2cmaster.h

#ifndef I2CMASTER_H
#define I2CMASTER_H

#include <avr/io.h>

#include "default.h"
#include "i2cdef.h"


typedef struct {
  uint8_t addr;
  uint8_t type;
  uint8_t bufsize;
} dev_t;


typedef struct {
  uint8_t state;  // bit 0..2 i2c state
  uint8_t cur_cmd;
  uint8_t dev_n;
  uint8_t type;
  uint8_t temp_addr;
  uint8_t temp_type;
  uint8_t temp_bufsize;
  uint16_t counter;
  uint8_t error;
  dev_t devices[I2C_MAX_DEVCOUNT];
} i2cmasterdata_t;

void doi2cstuff(i2cmasterdata_t* data);
void init_i2cmaster(i2cmasterdata_t* data);

#endif  // I2CMASTER_H
