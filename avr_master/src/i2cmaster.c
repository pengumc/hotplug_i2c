// i2cmaster.c

#include "i2cmaster.h"

// create a new i2cmasterdata_t then call this
void init_i2cmaster(i2cmasterdata_t* data) {
  // zero everything
  uint8_t i;
  uint8_t* d = (uint8_t*)((void*)data);
  for(i = 0; i < sizeof(i2cmasterdata_t); ++i) {
    d[i] = 0;
  }
  // set type to 1
  data->type = 1;
  // starting state
  data->state = I2CSTATE_IDLE & I2C_MASK;
}

// call this regularly
void doi2cstuff(i2cmasterdata_t* data) {
  if (CHK(TWCR, TWINT)) {
    uint8_t status = (TWSR & 0xF8) | data->state;
    switch(status) {
      case I2CSTATE_DEVQUERY0a:
      case I2CSTATE_DEVQUERY0b: {
        TWDR = 0x00;
        data->state = I2CSTATE_DEVQUERY1 & I2C_MASK;
        goto i2c_en_ea;
      }
      case I2CSTATE_DEVQUERY1: {
        TWDR = TWAR >> 1;
        data->state = I2CSTATE_DEVQUERY2 & I2C_MASK;
        goto i2c_en_ea;
      }
      case I2CSTATE_DEVQUERY2: {
        TWDR = data->type;
        data->state = I2CSTATE_DEVQUERY3 & I2C_MASK;
        goto i2c_en_ea;
      }
      case I2CSTATE_DEVQUERY3: {
        TWDR = I2C_MASTER_BUFSIZE;
        data->state = I2CSTATE_DEVQUERY4 & I2C_MASK;
        goto i2c_en_ea;
      }
      case I2CSTATE_DEVQUERY4: {
        TWDR = I2C_MASTER_CMD_QUERY;
        data->state = I2CSTATE_DEVQUERY5 & I2C_MASK;
        goto i2c_en_ea;
      }
      case I2CSTATE_DEVQUERY5: {
        data->state = I2CSTATE_DEVQUERY6 & I2C_MASK;
        data->dev_n = 0;
        data->counter = 0;
        goto i2c_stop;
      }
      case I2CSTATE_DEVQUERY7: {
        data->state = I2CSTATE_DEVQUERY8 & I2C_MASK;
        data->counter = 0;
        goto i2c_en_ea;
      }
      case I2CSTATE_DEVQUERY8: {
        data->temp_addr = TWDR;
        data->state = I2CSTATE_DEVQUERY9 & I2C_MASK;
        goto i2c_en_ea;
      }
      case I2CSTATE_DEVQUERY9: {
        data->temp_type = TWDR;
        data->state = I2CSTATE_DEVQUERY10 & I2C_MASK;
        goto i2c_en_ea;
      }
      case I2CSTATE_DEVQUERY10: {
          data->temp_bufsize = TWDR;
          data->counter = 0;
          data->state = I2CSTATE_DEVQUERY11 & I2C_MASK;
          goto i2c_en;
      }
      case I2CSTATE_DEVQUERY11: {
        data->devices[data->dev_n].addr = data->temp_addr;
        data->devices[data->dev_n].type = data->temp_type;
        data->devices[data->dev_n++].bufsize = data->temp_bufsize;
        if (data->dev_n >= I2C_MAX_DEVCOUNT) {
          data->error = I2C_ERR_DEVOVF;
          data->dev_n = 0;
        }
        data->state = I2CSTATE_DEVQUERY6 & I2C_MASK;
        data->counter = 0;
        goto i2c_en_ea;
      }
      default: {
        data->error = status;
        return;
      }
    }
  } else {  // NO TWINT
    switch (data->state) {
      case I2CSTATE_IDLE & I2C_MASK: {
        TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWSTA) | (1<<TWINT);
        data->state = I2CSTATE_DEVQUERY0a & I2C_MASK;
        return;
      }
      case I2CSTATE_DEVQUERY6 & I2C_MASK: {
        if ((++data->counter) == 0) {
          data->state = I2CSTATE_DEVQUERY12 & I2C_MASK;
        }
        return;
      }
      case I2CSTATE_DEVQUERY12 & I2C_MASK: {
        // data available
        // keep i2c separate from other systems
        // data->state 2 indicates we're done anyway.
        //data->cmd_state = TL_DATA_AVAILABLE;
        return;
      }
      default: return;
    }
  }
  // labels 
  i2c_en:
  TWCR = (1<<TWEN) | (1<<TWINT);
  return;
  
  i2c_en_ea:
  TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWINT);
  return;
  
  i2c_stop:
  TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWSTO) | (1<<TWINT);
  return;
}
