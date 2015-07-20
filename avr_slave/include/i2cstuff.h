// i2cstuff.h
#ifndef I2CSTUFF_H
#define I2CSTUFF_H
#include <avr/io.h>
#include <avr/wdt.h>
#include <util/twi.h>
#include "default.h"

#define BUFSIZE 8

#define AVRI2C_M_START_SENT 0x08
#define AVRI2C_M_REP_START_SENT 0x10
#define AVRI2C_M_SLA_W_SENT_AND_ACKED 0x18
#define AVRI2C_M_SLA_W_SENT_AND_NACKED 0x20
#define AVRI2C_M_DATA_SENT_AND_ACKED 0x28
#define AVRI2C_M_DATA_SENT_AND_NACKED 0x30
#define AVRI2C_M_ARBITRATION_LOST 0x38
#define AVRI2C_M_SLA_R_SENT_AND_ACKED 0x40
#define AVRI2C_M_SLA_R_SENT_AND_NACKED 0x48
#define AVRI2C_M_DATA_RECEIVED_AND_ACKED 0x50
#define AVRI2C_M_DATA_RECEIVED_AND_NACKED 0x58

#define AVRI2C_S_RECEIVED_SLA_W_AND_ACKED 0x60
#define AVRI2C_S_ARB_LOST_AND_ACKED_OWN_SLA_W 0x68
#define AVRI2C_S_GC_RECEIVED_AND_ACKED 0x70
#define AVRI2C_S_ARB_LOST_AND_ACKED_GC 0x78
#define AVRI2C_S_DATA_RECEIVED_AND_ACKED 0x80
#define AVRI2C_S_DATA_RECEIVED_AND_NACKED 0x88
#define AVRI2C_S_GC_DATA_RECEIVED_AND_ACKED 0x90
#define AVRI2C_S_GC_DATA_RECEIVED_AND_NACKED 0x98
#define AVRI2C_S_RECEIVED_STOP_OR_REP_START 0xA0
#define AVRI2C_S_RECEIVED_SLA_R_AND_ACKED 0xA8
#define AVRI2C_S_ARB_LOST_AND_ACKED_OWN_SLA_R 0xB0
#define AVRI2C_S_DATA_SENT_AND_ACKED 0xB8
#define AVRI2C_S_DATA_SENT_AND_NACKED 0xC0
#define AVRI2C_S_LAST_DATA_SENT_AND_ACKED 0xC8

#define AVRI2C_ERROR 0x00

#define I2C_QUERY 0x42
// master types are 0x00 to 0x0F
#define I2C_MASTER_TYPE_THRESHOLD 0x10
#define I2C_DEV_7SEG 0x10

// bit 7 of state = broadcast mode
// unadressed slave mode, waiting
#define I2CSTATE_IDLE 0x01
// global call receiving
#define I2CSTATE_RGC0 0x02
#define I2CSTATE_RGC1 0x03
#define I2CSTATE_RGC2 0x04
#define I2CSTATE_RGC3 0x05
// personal data
#define I2CSTATE_RECV0 0x06
#define I2CSTATE_RECV1 0x07
// broadcasting own stuff
#define I2CSTATE_BC0 0x81
#define I2CSTATE_BC1 0x82
#define I2CSTATE_BC2 0x83
#define I2CSTATE_BC3 0x84
#define I2CSTATE_BC4 0x85
#define I2CSTATE_BC5 0x86
#define I2CSTATE_BC6 0x87

#define I2CSTATE_HORROR 0xFF;



typedef struct {
  uint8_t state;
  uint8_t address;
  uint8_t devtype;
  uint8_t bufsize;
  uint8_t buffer[BUFSIZE];
  uint8_t active_addr[16];
  uint8_t recv_addr;
  uint8_t recv_type;
  uint8_t recv_bufsize;
  uint8_t recv_last;
  uint8_t bc_queued;  // <- merge with state?
  uint8_t wait;
  uint8_t bufpos;
  uint8_t lastmode;
  uint8_t lastlen;
} i2cdata_t;

typedef struct {
  uint8_t address;
  uint8_t devtype;
  uint8_t bufsize;
} i2c_master_dev_t;

void start_I2C(i2cdata_t* data, uint8_t addr);
void set_I2C_address(i2cdata_t* data, uint8_t addr);
void doi2cstuff(i2cdata_t* data);
uint8_t update_address(i2cdata_t* data);
void activate_received_address(i2cdata_t* data);

// all info is in i2cdata
void i2c_receive();

#endif  // I2CSTUFF_H