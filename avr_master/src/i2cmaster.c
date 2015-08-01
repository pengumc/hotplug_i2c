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
  data->state = I2CSTATE_IDLE;
}

// call this regularly
void doi2cstuff(i2cmasterdata_t* data) {
  if (data->state == I2CSTATE_ERROR) return;
  uint8_t status;
  if (CHK(TWCR, TWINT)) {
    status = TWSR & 0xF8;
    switch(status) {
      case AVRI2C_M_REP_START_SENT:
      case AVRI2C_M_START_SENT: {
        switch(data->state) {
          case I2CSTATE_DEVQUERY0: {
            TWDR = 0x00;
            data->state = I2CSTATE_DEVQUERY1;
            goto i2c_en_ea;
          }
          case I2CSTATE_SD0: {
            TWDR = data->temp_addr<<1;
            data->state = I2CSTATE_SD1;
            goto i2c_en_ea;
          }
          default: goto unhandled_state;
        }
      }
      case AVRI2C_M_SLA_W_SENT_AND_ACKED: {
        switch(data->state) {
          case I2CSTATE_DEVQUERY1: {
            TWDR = TWAR >> 1;
            data->state = I2CSTATE_DEVQUERY2;
            goto i2c_en_ea;
          }
          case I2CSTATE_SD1: {
            //header, 5 bits payload size, 3 bits mode
            TWDR = (data->dev_n << 3) | (data->temp_bufsize & 0x07);  
            data->state = I2CSTATE_SD2;
            goto i2c_en_ea;
          }
          default: goto unhandled_state;
        }
      }
      case AVRI2C_M_DATA_SENT_AND_ACKED: {
        switch(data->state) {
          case I2CSTATE_DEVQUERY2: {
            TWDR = data->type;
            data->state = I2CSTATE_DEVQUERY3;
            goto i2c_en_ea;
          }
          case I2CSTATE_DEVQUERY3: {
            TWDR = I2C_MASTER_BUFSIZE;
            data->state = I2CSTATE_DEVQUERY4;
            goto i2c_en_ea;
          }
          case I2CSTATE_DEVQUERY4: {
            TWDR = I2C_MASTER_CMD_QUERY;
            data->state = I2CSTATE_DEVQUERY5;
            goto i2c_en_ea;
          }
          case I2CSTATE_SD2: {
            TWDR = data->buffer[--data->dev_n];
            if (data->dev_n == 0) {
              data->state = I2CSTATE_SD3;
            }
            goto i2c_en_ea;
          }
          default: goto unhandled_state;
        }
      }
      case AVRI2C_M_DATA_SENT_AND_NACKED: {
        switch(data->state) {
          case I2CSTATE_DEVQUERY5: {
            data->dev_n = 0;
            data->counter = 0;
            data->state = I2CSTATE_DEVQUERY6;
            goto i2c_stop;
          }
          case I2CSTATE_SD3: {
            data->state = I2CSTATE_DEVQUERY12;
            goto i2c_stop;
          }
          default: goto unhandled_state;
        }
      }
      case AVRI2C_S_GC_RECEIVED_AND_ACKED: {
        switch(data->state) {
          case I2CSTATE_DEVQUERY6: {
            data->state = I2CSTATE_DEVQUERY8;
            goto i2c_en_ea;
          }
          default: goto unhandled_state;
        }
      }
      case AVRI2C_S_GC_DATA_RECEIVED_AND_ACKED: {
        switch(data->state) {
          case I2CSTATE_DEVQUERY8: {
            data->temp_addr = TWDR;
            data->state = I2CSTATE_DEVQUERY9;
            goto i2c_en_ea;
          }
          case I2CSTATE_DEVQUERY9: {
            data->temp_type = TWDR;
            data->state = I2CSTATE_DEVQUERY10;
            goto i2c_en_ea;
          }
          case I2CSTATE_DEVQUERY10: {
            data->temp_bufsize = TWDR;
            data->counter = 0;
            data->state = I2CSTATE_DEVQUERY11;
            goto i2c_en;
          }
          default: goto unhandled_state;
        }
      }
      case AVRI2C_S_GC_DATA_RECEIVED_AND_NACKED: {
        switch(data->state) {
          case I2CSTATE_DEVQUERY11: {
            data->devices[data->dev_n].addr = data->temp_addr;
            data->devices[data->dev_n].type = data->temp_type;
            data->devices[data->dev_n++].bufsize = data->temp_bufsize;
            if (data->dev_n >= I2C_MAX_DEVCOUNT) {
              data->error[0] = I2C_ERR_DEVOVF;
              data->error[1] = status;
              data->error[2] = data->state;
              data->dev_n = 0;
            }
            data->state = I2CSTATE_DEVQUERY6;
            data->counter = 0;
            goto i2c_en_ea;
          }
          default: goto unhandled_state;
        }
      }
      default: goto unhandled_state;

    }
  } else {  // NO TWINT
    switch (data->state) {
      case I2CSTATE_IDLE: {
        if (data->cur_cmd) {
          TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWSTA) | (1<<TWINT);
          data->state = data->cur_cmd;
        }
        return;
      }
      case I2CSTATE_DEVQUERY6: {
        if ((++data->counter) == 0) {
          data->state = I2CSTATE_DEVQUERY12;
        }
        return;
      }
      case I2CSTATE_DEVQUERY12: {
        // data available
        // keep i2c separate from other systems
        // data->state devquery12 indicates were done anyway
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

  unhandled_state:
  data->error[0] = I2C_ERR_UNHANDLED_STATE;
  data->error[1] = status;
  data->error[2] = data->state;
  data->state = I2CSTATE_ERROR;
  return;
}
