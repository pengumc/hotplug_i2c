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
    if (recv[5] == 0) {
      // act on recv
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
      pages_waiting = 0;
    } else {
      ++pages_waiting;
      ready_data(15);
    }
    // } else {
      // if (recv[5] == USB_I2C_QUERY_DEVS) {
        // if (cmd_state == CMD_STATE_IDLE) {
          // cmd_state = CMD_STATE_BUSY;
          // masterdata.state = I2CSTATE_IDLE & I2C_MASK;
          // masterdata.cur_cmd = USB_I2C_QUERY_DEVS;
        // }
      // } else if (recv[5] == USB_REQ_DATA) {
        // if (cmd_state == CMD_STATE_FAILED) {
          // reporting_count = pages_waiting;
          // setup_error_report();  
        // } else if (cmd_state == CMD_STATE_DATA_WAITING) {
          // reporting_count = pages_waiting;
          // setup_dev_query_report(0);
        // }
      // }
    // }
    return 1;
  } else {
    return 0; 
  }
}

// -----------------------------------------------------------setup_error_report
void setup_error_report() {
  uint8_t i = 0;
  report_data[i++] = masterdata.cur_cmd;
  report_data[i++] = 0;
  report_data[i++] = masterdata.error;
  report_data[i++] = 0;
  report_data[i++] = 0;
  report_data[i++] = 0;
  report_data[i++] = 0;
  report_data[i++] = 0;
  usbSetInterrupt(report_data, 8);
}

// -------------------------------------------------------setup_dev_query_report
void setup_dev_query_report(uint8_t index) {
  if (cmd_state != CMD_STATE_DATA_WAITING) return;
  uint8_t i = 0;
  uint8_t j = index << 1;
  report_data[i++] = USB_I2C_QUERY_DEVS;
  report_data[i++] = index;
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
  usbSetInterrupt(report_data, 8);
}
 
// -------------------------------------------------------------------ready_data 
void ready_data(uint8_t a) {
  if (reporting_count == 0) {
    if (usbInterruptIsReady()) {
      // no special reports waiting
      report_data[0] = TWSR;
      report_data[1] = TWCR;
      report_data[2] = TWDR;
      report_data[3] = TWBR;
      report_data[4] = TWAR;
      report_data[5] = cmd_state;
      report_data[6] = masterdata.state;
      report_data[7] = a;
      usbSetInterrupt(report_data, 8);
    }
  } else {
    if (usbInterruptIsReady()) {
      // a report was sent
      if (--reporting_count == 0) {
        ready_data(0);
        pages_waiting = 0;
        cmd_state = CMD_STATE_IDLE;
        return;
      }
    }
    setup_dev_query_report(pages_waiting - reporting_count); 
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

  new_cmd = 0;
  cmd_state = 0;
  reporting_count = 0;
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
      if (masterdata.error) {
        cmd_state = CMD_STATE_FAILED;
        pages_waiting = 1;
      } else if (masterdata.state == (I2CSTATE_DEVQUERY12 & I2C_MASK)) {
        if (masterdata.dev_n >0) {
          cmd_state =  CMD_STATE_DATA_WAITING;
          pages_waiting = ++masterdata.dev_n >> 1;
        } else {  // cmd finished, no data
          pages_waiting = 0;
          cmd_state = CMD_STATE_IDLE;
        }
      }
    }
    ready_data(0);
  }
}

