// i2ccontrol.h

#ifndef __I2CCONTROL_H__
#define __I2CCONTROL_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
  #include <windows.h>
#else
  #include <unistd.h>
#endif

#include "hidapi/hidapi.h"
#define TWIE 0
#define TWEN 2
#define TWWC 3
#define TWSTO 4
#define TWSTA 5
#define TWEA 6
#define TWINT 7

#define SET_TWCR 0
#define SET_TWDR 1
#define SET_TWBR 2
#define SET_TWAR 3

int usbread(hid_device* handle, uint8_t* b, size_t length, int ms);
void usbwrite(hid_device* handle, const uint8_t* b, size_t length);
void print_usage();
void list_devices();
void read_dev(int* argc, char** argv);
void write_dev(int* argc, char** argv);
hid_device* open_dev_by_args(int* argc, char** argv);
void print_state(uint8_t* buf);
void setup_i2c(hid_device* handle);
void send_start(hid_device* handle);
void send_stop(hid_device* handle);
void update_twar(hid_device* handle, uint8_t twar);
void update_twbr(hid_device* handle, uint8_t twbr);
void send_twdr(hid_device* handle, uint8_t twdr);
void send_data(hid_device* handle);
void update_twcr(hid_device* handle, uint8_t twcr);
void wait_for_twint(hid_device* handle, uint8_t* buf);
void _delay1s();

#endif  // __I2CCONTROL_H__