// i2cdef.h
#ifndef I2CDEF_H
#define I2CDEF_H

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

#define I2C_MASTER_TYPE_THRESHOLD 0x10
#define I2C_MASTER_BUFSIZE 32
#define I2C_MAX_DEVCOUNT 20

#define I2C_ERR_DEVOVF 1
#define I2C_ERR_UNHANDLED_STATE 2

#define I2C_MASTER_CMD_QUERY 0x42

#define AVRI2C_ERROR 0x00
#define AVRI2C_NOTHING 0xF8

// i2c statemachine
// #define I2CSTATE_IDLE       (AVRI2C_NOTHING | 0)
// #define I2CSTATE_DEVQUERY6  (AVRI2C_NOTHING | 1)
// #define I2CSTATE_DEVQUERY12 (AVRI2C_NOTHING | 2)

// #define I2CSTATE_DEVQUERY0a (AVRI2C_M_START_SENT | 3)
// #define I2CSTATE_DEVQUERY0b (AVRI2C_M_REP_START_SENT | 3)
// #define I2CSTATE_DEVQUERY1  (AVRI2C_M_SLA_W_SENT_AND_ACKED | 3)
// #define I2CSTATE_DEVQUERY2  (AVRI2C_M_DATA_SENT_AND_ACKED | 3) 
// #define I2CSTATE_DEVQUERY3  (AVRI2C_M_DATA_SENT_AND_ACKED | 4)
// #define I2CSTATE_DEVQUERY4  (AVRI2C_M_DATA_SENT_AND_ACKED | 5)
// #define I2CSTATE_DEVQUERY5  (AVRI2C_M_DATA_SENT_AND_NACKED | 3)

// #define I2CSTATE_DEVQUERY7  (AVRI2C_S_GC_RECEIVED_AND_ACKED | 1)
// #define I2CSTATE_DEVQUERY8  (AVRI2C_S_GC_DATA_RECEIVED_AND_ACKED | 3)
// #define I2CSTATE_DEVQUERY9  (AVRI2C_S_GC_DATA_RECEIVED_AND_ACKED | 4)
// #define I2CSTATE_DEVQUERY10 (AVRI2C_S_GC_DATA_RECEIVED_AND_ACKED | 5)
// #define I2CSTATE_DEVQUERY11 (AVRI2C_S_GC_DATA_RECEIVED_AND_NACKED | 3)

#define I2CSTATE_IDLE 0
#define I2CSTATE_ERROR 0xFF
#define I2CSTATE_DEVQUERY0 1
#define I2CSTATE_DEVQUERY1 2
#define I2CSTATE_DEVQUERY2 3
#define I2CSTATE_DEVQUERY3 4
#define I2CSTATE_DEVQUERY4 5
#define I2CSTATE_DEVQUERY5 6
#define I2CSTATE_DEVQUERY6 7
#define I2CSTATE_DEVQUERY7 8
#define I2CSTATE_DEVQUERY8 9
#define I2CSTATE_DEVQUERY9 10
#define I2CSTATE_DEVQUERY10 11
#define I2CSTATE_DEVQUERY11 12
#define I2CSTATE_DEVQUERY12 13



#endif  // I2CDEF_H
