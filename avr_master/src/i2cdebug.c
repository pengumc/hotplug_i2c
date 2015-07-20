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
    if (recv[4] == 0) {
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
    } else {
      new_cmd = recv[4];
      new_cmd_data[0] = recv[5];
      new_cmd_data[1] = recv[6];
      new_cmd_data[2] = recv[7];
    }
    return 1;
  } else {
    return 0; 
  }
}

// ------------------------------------------------------------------toplevel_sm
void toplevel_sm(i2cmasterdata_t* data) {
  just step statemachine on usb int ready when in failed or data_available state
  or don't pretend it's a proper statemachine
  
  always keep 'normal_data' up to date
  update 'report_data'
  call statemachine when usbInterruptIsReady
  if free/busy set normal_data
  if failed, use data->report_remaining
  
  
  
  usb data modes:
    normal
      cur_cmd = 0, new_cmd = 0, cmd_sta
    report
  
  if (usbInterruptIsReady()){
    
    usbSetInterrupt(send, 8);
  }
}

/*
  set failed stuff
  ready data
  - data not read yet
*/

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
  usbDeviceConnect();
  sei();
  
  new_cmd = 0;
  i2cmasterdata_t masterdata;
  init_i2cmaster(&masterdata);
  
  while(1) {
    wdt_reset();
    usbPoll();
    doi2cstuff(&masterdata);
    if (usbInterruptIsReady()) {
      // determin what to send for the next tranfer
    }
  }
}

