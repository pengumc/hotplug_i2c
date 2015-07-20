// base_hid.h

#ifndef __BASE_HID_H__
#define __BASE_HID_H__

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "usbdrv/usbdrv.h"

#define SET(x,y) (x|=(1<<y))
#define CLR(x,y) (x&=(~(1<<y)))
#define CHK(x,y) (x&(1<<y)) 
#define TOG(x,y) (x^=(1<<y))
#define Delay(x) (_delay_us(x))

usbMsgLen_t usbFunctionSetup(uchar data[8]);
uchar usbFunctionWrite(uchar * data, uchar len);
// global vars:
// -----------------------------------------------------------USB HID Descriptor
PROGMEM char usbHidReportDescriptor[32] = {
  0x06, 0x9c, 0xff,     /* Usage Page (Vendor Defined)                     */
  0x09, 0x01,           /* Usage (Vendor Defined)                          */
  0xa1, 0x01,           /* Collection (Vendor Defined)                     */
  0x09, 0x02,           /*   Usage (Vendor Defined)                        */
  0x75, 0x08,           /*   Report Size (8)                               */
  0x95, 0x08,           /*   Report Count (8)       */
  0x15, 0x00,           /*   Logical Minimum (0)                           */
  0x25, 0xff,           /*   Logical Maximum (255)                         */
  0x81, 0x02,           /*   Input (Data, Variable, Absolute)              */
  0x09, 0x03,           /*   Usage (Vendor Defined)                        */
  0x75, 0x08,           /*   Report Size (8)                               */
  0x95, 0x08,           /*   Report Count (8)       */
  0x15, 0x00,           /*   Logical Minimum (0)                           */
  0x25, 0xff,           /*   Logical Maximum (255)                         */
  0x91, 0x02,           /*   Output (Data, Variable, Absolute)             */
  0xc0                  /* End Collection                                  */
};

uint8_t buffer_pos;
uint8_t bytes_remaining;
volatile uint8_t recv[8];
volatile uint8_t send[8];
uint8_t new_cmd;
uint8_t new_cmd_data[3];



#endif  // __BASE_HID_H__