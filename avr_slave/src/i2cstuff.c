// i2cstuff.c
#include "i2cstuff.h"

void start_I2C(i2cdata_t* data, uint8_t addr) {
  data->state = I2CSTATE_IDLE;
  // data->wait = OSCCAL;
  data->bc_queued = 0;
  // data->lastmode = 0;
  data->devtype = I2C_DEV_7SEG;
  data->once  = 0;
  uint8_t i;
  for (i=0; i<BUFSIZE; ++i) {
    data->buffer[i] = 0;
  }
  for (i=0; i<16; ++i) {
    data->active_addr[i] = 0;
  }
  set_I2C_address(data, addr);
  TWBR = 4; // with prescaler 1 and 8 MHz clk ==> 400 KHz
  TWCR = (1<<TWEN) | (1<<TWEA);
}

void set_I2C_address(i2cdata_t* data, uint8_t addr) {
  TWAR = addr<<1 | 0x01;
  data->address = addr;
}

void activate_received_address(i2cdata_t* data) {
  // have to help compiler a bit here, so it's not in one line
  uint8_t i = data->recv_addr & 0x7F;
  i = i >> 3;
  uint8_t bit = data->recv_addr & 0x07;
  SET(data->active_addr[i], bit);
}

void doi2cstuff(i2cdata_t* data) {
  if (CHK(TWCR, TWINT)) {
    uint8_t status = TWSR & 0xF8;
    switch(status) {
      case AVRI2C_ERROR: {
        TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWSTO) | (1<<TWINT);
        return;
      }
      case AVRI2C_M_DATA_RECEIVED_AND_NACKED: // sla+R is never sent
      case AVRI2C_M_DATA_RECEIVED_AND_ACKED: 
      case AVRI2C_M_SLA_R_SENT_AND_NACKED:
      case AVRI2C_M_SLA_R_SENT_AND_ACKED:
      case AVRI2C_S_ARB_LOST_AND_ACKED_GC: // not sending addressed transfers

      case AVRI2C_S_RECEIVED_SLA_R_AND_ACKED:
      case AVRI2C_S_ARB_LOST_AND_ACKED_OWN_SLA_R: // not possible yet
      case AVRI2C_S_DATA_SENT_AND_ACKED:
      case AVRI2C_S_DATA_SENT_AND_NACKED:
      case AVRI2C_S_LAST_DATA_SENT_AND_ACKED:
      {
        goto i2c_horror_exit;
      }
      // Master transmitter
      case AVRI2C_M_REP_START_SENT:
      case AVRI2C_M_START_SENT: {
        switch (data->state) {
          case I2CSTATE_BC1: {
            TWDR = 0x00;
            data->state = I2CSTATE_BC2;
            goto i2c_en_ea;
          }
          default: goto i2c_horror_exit;
        }
      }
      case AVRI2C_M_SLA_W_SENT_AND_ACKED: {  // send addr
        switch (data->state) {
          case I2CSTATE_BC2: {
            TWDR = data->address;
            data->state = I2CSTATE_BC3;
            goto i2c_en_ea;
          }
          default: goto i2c_horror_exit;
        }
      }
      case AVRI2C_M_SLA_W_SENT_AND_NACKED: {
        data->state = I2CSTATE_IDLE;
        goto i2c_en_ea;
      }
      case AVRI2C_M_DATA_SENT_AND_ACKED: {
        switch (data->state) {
          case I2CSTATE_BC3: {  // send type
            TWDR = data->devtype;
            data->state = I2CSTATE_BC4;
            goto i2c_en_ea;
          }
          case I2CSTATE_BC4: {  // send bufsize
            TWDR = data->bufsize;
            data->state = I2CSTATE_BC5;
            goto i2c_en_ea;
          }
          case I2CSTATE_BC5: {  // send wait remainder
            TWDR = data->wait;
            data->state = I2CSTATE_BC6;
            goto i2c_en_ea;
          }
          default: {
            TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWSTO) | (1<<TWINT);
            data->bc_queued = 0;
            data->state = I2CSTATE_IDLE;
            goto i2c_bad_exit;
          }
        }
      }
      case AVRI2C_M_DATA_SENT_AND_NACKED: {
        if (CHK(data->state, 7)) {
          // if transmitting, this was the last byte or things went wrong
          // in which case the master will handle things...
          // for both instances, stop is the correct action
          TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWSTO) | (1<<TWINT);
          data->bc_queued = 0;
          data->once = 0x80;
          data->state = I2CSTATE_IDLE;
          goto i2c_bad_exit;
        } else {
          goto i2c_horror_exit;
        }
      }
      case AVRI2C_M_ARBITRATION_LOST: {
        // during BC3 = during sending of address, so other sender's address 
        // is different, but it's still on TWDR so activate it.
        // during BC4, we send the same address and this dev
        // lost so the address is now taken by the winning dev
        // same for BC5 and BC 6
        switch (data->state) {
          case I2CSTATE_BC3: {
            data->recv_addr = TWDR;
          }
          case I2CSTATE_BC5:
          case I2CSTATE_BC6:
          case I2CSTATE_BC4: {
            data->recv_addr = data->address;
            activate_received_address(data);
            while (update_address(data)) {data->wait += 1;}
            set_I2C_address(data, data->address);
          }
          default: {
            data->state = I2CSTATE_BC0;
            goto i2c_en_ea;
          }
        }
      }
      
      // Slave receiver
      case AVRI2C_S_GC_RECEIVED_AND_ACKED: {
        switch (data->state) {
          case I2CSTATE_BC0:
          case I2CSTATE_IDLE: {
            data->state = I2CSTATE_RGC0;
            goto i2c_en_ea;
          }
          default: goto i2c_horror_exit;
        }
      }
      case AVRI2C_S_GC_DATA_RECEIVED_AND_ACKED: {
        switch (data->state) {
          // all general calls are 4 bytes long
          // addr, type, bufsize, rand/cmd
          case I2CSTATE_RGC0: {  // first data = address, ack next
            data->recv_addr = TWDR;
            data->state = I2CSTATE_RGC1;
            goto i2c_en_ea;
          }
          case I2CSTATE_RGC1: {  // second data = devtype
            data->recv_type = TWDR;
            data->state = I2CSTATE_RGC2;
            goto i2c_en_ea;
          }
          case I2CSTATE_RGC2: {  // third data = bufsize, don't ack fourth
            data->recv_bufsize = TWDR;
            data->state = I2CSTATE_RGC3;
            goto i2c_en;
          }
          default: goto i2c_horror_exit;
        }
      }
      case AVRI2C_S_GC_DATA_RECEIVED_AND_NACKED: {
        switch (data->state) {
          case I2CSTATE_RGC3: {  // last data from GC
            data->recv_last = TWDR;
            activate_received_address(data);
            while (update_address(data)) {++data->wait;}
            set_I2C_address(data, data->address);
            // if (devtype was master and it sent bc req) or bc was still queued
            if ((data->recv_last == I2C_QUERY &&
                data->recv_type < I2C_MASTER_TYPE_THRESHOLD) || 
                data->bc_queued > 0) {
                data->bc_queued = 1;
                data->state = I2CSTATE_BC0;
            } else data->state = I2CSTATE_IDLE;
            goto i2c_en_ea;
          }
          default: goto i2c_horror_exit;
        }
      }
      case AVRI2C_S_RECEIVED_STOP_OR_REP_START: {
        // premature abort of transfer
        data->state = I2CSTATE_IDLE;
        goto i2c_en_ea;
      }
    
      // slave addressed
      case AVRI2C_S_RECEIVED_SLA_W_AND_ACKED:
      case AVRI2C_S_ARB_LOST_AND_ACKED_OWN_SLA_W: {  // prepare
        switch (data->state) {
          case I2CSTATE_IDLE: {
            data->bufpos = 0;
            data->state = I2CSTATE_RECV0;
            goto i2c_en_ea;
          }
          default: goto i2c_horror_exit;
        }
      }
      case AVRI2C_S_DATA_RECEIVED_AND_ACKED: {
        switch (data->state) {
          case I2CSTATE_RECV0: {  // header byte
            data->lastmode = TWDR & 0x07;
            data->bufpos = TWDR >> 3;
            data->lastlen = data->bufpos;
            data->state = I2CSTATE_RECV1;
            goto i2c_bufpos;
          }
          case I2CSTATE_RECV1: {
            data->buffer[data->bufpos] = TWDR;
            goto i2c_bufpos;
          }
          default: goto i2c_horror_exit;
        }
      }
      case AVRI2C_S_DATA_RECEIVED_AND_NACKED: {
        switch (data->state) {
          case I2CSTATE_RECV1: {  // last byte
            data->buffer[data->bufpos] = TWDR;
            data->state = I2CSTATE_IDLE;
            i2c_receive();
            // TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWINT);
            // break;
            goto i2c_en_ea;
          }
          default: goto i2c_horror_exit;
        }
      }
      default: goto i2c_horror_exit;
    }  // TWSR cases
  } else { // TWINT = 0
    switch (data->state) {
      case I2CSTATE_BC0: {
        // wait for X and then start MT
        data->wait -= 1;
        if (data->wait == 0) {
          TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWSTA) | (1<<TWINT);
          data->state = I2CSTATE_BC1;
        }
        return;
      }
      default: {
        data->wait += 1;
        return;
      }
    }
  }
  
  goto i2c_bad_exit; 
  i2c_bufpos:
  if (--data->bufpos) goto i2c_en_ea;
  else goto i2c_en;
  
  i2c_en_ea:
  TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWINT);
  return;
  
  i2c_en:
  TWCR = (1<<TWEN) | (1<<TWINT);
  i2c_bad_exit:
  return;
  i2c_horror_exit:
  data->state = I2CSTATE_HORROR;
  return;
  // to beat: 1748

}  

// 0 if address is unchanged
uint8_t update_address(i2cdata_t* data) {
  if (CHK(data->active_addr[data->address>>3], data->address & 0x07)) {
    data->address += 1;
    if (data->address >= 120) data->address = 0x01;
    return 1;
  } else {
    return 0;
  }
}