/// @file i2cdebug.c
/// @author Michiel van der Coelen
/// @date 2015-05
/// @desc control TWCR TWDR TWBR TWAR and TWSR over usb

// F_CPU 20000
// hfuse DF
// lfuse E6

#include "i2cdebug.h"

// -------------------------------------------------------------usbFunctionSetup
usbMsgLen_t usbFunctionSetup(uchar data[8]) {
  usbRequest_t    *rq = (void *)data;
  if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
    if (rq->bRequest == USBRQ_HID_SET_REPORT) {
      buffer_pos = 0;
      bytes_remaining = rq->wLength.word;
      return USB_NO_MSG;
    }
  }
  return 0;
}

// -------------------------------------------------------------usbFunctionWrite
uchar usbFunctionWrite(uchar * data, uchar len) {
  if (bytes_remaining < len) {
    len = bytes_remaining;
  }
  uint8_t i;
  for (i = 0; i < len; ++i){
    recv[buffer_pos++] = data[i];
  }
  bytes_remaining -= len;
  
  if (bytes_remaining == 0) {
    if ((recv[0] & 0xF0) == 0) {
      // act on first 4 bits of recv[0]
      if (CHK(recv[0], 1)) {
        if (CHK(TWCR, TWINT)) TWDR = recv[2];
      }
      if (CHK(recv[0], 2)) {
        TWBR = recv[3];
      }
      if (CHK(recv[0], 3)) {
        TWAR = recv[4];
      }
      if (CHK(recv[0], 0)) {
        TWCR = recv[1];
      }
    } else {
      // set a new command (if possible)
      if ((recv[0] & 0xF0) == (USB_I2C_QUERY_DEVS<<4)) {
        if (cmd_state == CMD_STATE_IDLE ||
            cmd_state == CMD_STATE_DATA_WAITING) {
          if (masterdata.state == I2CSTATE_IDLE) {
            masterdata.cur_cmd = USB_I2C_QUERY_DEVS;
            cmd_state = CMD_STATE_BUSY;
          }
        }
      }
    }
    // always accept report mode on byte 5
    report_mode = recv[5];
    

    
    
    
    // prepare for immediate report request
    setup_next_report(recv[6]);
    usbSetInterrupt(report_data, 8);
    return 1;
  } else {
    return 0; 
  }
}

void setup_next_report(uint8_t page) {
  register uint8_t i = 0;
  switch(report_mode) {
    case REPORT_MODE_DATA: {
      if (page < pages_waiting) {
        uint8_t j = page << 1;
        report_data[i++] = USB_I2C_QUERY_DEVS;
        report_data[i++] = page;
        report_data[i++] = masterdata.devices[j].addr;
        report_data[i++] = masterdata.devices[j].type;
        report_data[i++] = masterdata.devices[j++].bufsize;
        if (j >= masterdata.dev_n) {
          report_data[i++] = 0;
          report_data[i++] = 0;
          report_data[i++] = 0;
        } else {
          report_data[i++] = masterdata.devices[j].addr;
          report_data[i++] = masterdata.devices[j].type;
          report_data[i++] = masterdata.devices[j].bufsize;
        }
        break;
      }
    }
    default:
    case REPORT_MODE_NORMAL: {
      report_data[i++] = TWSR;
      report_data[i++] = TWCR;
      report_data[i++] = TWDR;
      report_data[i++] = TWBR;
      report_data[i++] = TWAR;
      report_data[i++] = cmd_state;
      report_data[i++] = masterdata.state;
      report_data[i++] = masterdata.dev_n;
      break;
    }
    case REPORT_MODE_ERROR: {
      report_data[i++] = masterdata.cur_cmd;
      report_data[i++] = 0;
      report_data[i++] = masterdata.error[0];
      report_data[i++] = masterdata.error[1];
      report_data[i++] = masterdata.error[2];
      report_data[i++] = 0;
      report_data[i++] = 0;
      report_data[i++] = 0;
      break;
    }
  }
}

// -------------------------------------------------------------------------main
int main() {
  wdt_enable(WDTO_1S);
  usbInit();
  usbDeviceDisconnect();
  uint8_t i = 0;
  while(--i){           
    wdt_reset();
    _delay_ms(1);
  }

  cmd_state = 0;
  report_mode = 0;
  pages_waiting = 0;
  init_i2cmaster(&masterdata);

  usbDeviceConnect();
  sei();
  
  
  while(1) {
    wdt_reset();
    usbPoll();
      if (cmd_state == CMD_STATE_BUSY) {
      doi2cstuff(&masterdata);
      // 0 = idle
      // 1 3 4 5 = busy
      // 2 = done
      if (masterdata.error[0]) {
        cmd_state = CMD_STATE_FAILED;
        pages_waiting = 1;
      } else if (masterdata.state == I2CSTATE_DEVQUERY12) {
        // if (masterdata.dev_n >0) {
          cmd_state =  CMD_STATE_DATA_WAITING;
          pages_waiting = masterdata.dev_n;
        // } else {  // cmd finished, no data
          // pages_waiting = 0;
          // cmd_state = CMD_STATE_IDLE;
        // }
      }
    }
    setup_next_report(recv[6]);
    if (usbInterruptIsReady()) {
      usbSetInterrupt(report_data, 8);
    }
  }
}