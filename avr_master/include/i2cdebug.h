// base_hid.h

#ifndef __BASE_HID_H__
#define __BASE_HID_H__

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "usbdrv/usbdrv.h"
#include "i2cmaster.h"
#include "default.h"

#define USB_REQ_DATA 2
#define USB_I2C_QUERY_DEVS 1

#define CMD_STATE_IDLE 0
#define CMD_STATE_REJECTED 1
#define CMD_STATE_BUSY 2
#define CMD_STATE_FAILED 3
#define CMD_STATE_DATA_WAITING 4
#define CMD_STATE_USER_DATA 5

usbMsgLen_t usbFunctionSetup(uchar data[8]);
uchar usbFunctionWrite(uchar * data, uchar len);
void setup_error_report();
void setup_dev_query_report(uint8_t index);
void ready_data(uint8_t a);

// global vars:
// -----------------------------------------------------------USB HID Descriptor
PROGMEM char usbHidReportDescriptor[32] = {
  0x06, 0x9c, 0xff,     // Usage Page (Vendor Defined)
  0x09, 0x01,           // Usage (Vendor Defined)
  0xa1, 0x01,           // Collection (Vendor Defined)
  0x09, 0x02,           //   Usage (Vendor Defined)
  0x75, 0x08,           //   Report Size (8)
  0x95, 0x08,           //   Report Count (8)
  0x15, 0x00,           //   Logical Minimum (0)
  0x25, 0xff,           //   Logical Maximum (255)
  0x81, 0x02,           //   Input (Data, Variable, Absolute)
  0x09, 0x03,           //   Usage (Vendor Defined)
  0x75, 0x08,           //   Report Size (8)
  0x95, 0x08,           //   Report Count (8)
  0x15, 0x00,           //   Logical Minimum (0)
  0x25, 0xff,           //   Logical Maximum (255)
  0x91, 0x02,           //   Output (Data, Variable, Absolute)
  0xc0                  // End Collection
};

uint8_t buffer_pos;
uint8_t bytes_remaining;
uint8_t recv[8];
uint8_t report_data[8];
i2cmasterdata_t masterdata;

uint8_t new_cmd;
uint8_t reporting_count;
uint8_t cmd_state;
uint8_t pages_waiting;



#endif  // __BASE_HID_H__