// i2ccontrol.c
// modified hidapi_sendrecv

#include "i2ccontrol.h"

// ----------------------------------------------------------------------usbread
int usbread(hid_device* handle, uint8_t* b, size_t length, int ms) {
  int res = hid_read_timeout(handle, b, length, ms);
  if (res < 0) {
    printf("read error, res = %d\n", res);
    printf("sys: %ls\n", hid_error(handle));
  } else if (res == 0) {
    printf("read timeout\n");
  } else {
    printf("read %d bytes:\n  ", res);
    int i;
    for (i = 0; i < res; ++i) {
      printf("%d ", b[i]);
    }
    printf("\n");
  }
  return res;
}

// ---------------------------------------------------------------------usbwrite
void usbwrite(hid_device* handle, const uint8_t* b, size_t length) {
    int res = hid_write(handle, b, length);
    if (res < 0) {
    printf("write failure, res = %d\n", res);
    printf("system: %ls\n", hid_error(handle));
  } else {
    printf("written %d bytes:\n  ", res);
    int i;
    for (i = 1; i < res; ++i) {
      printf("%d ", b[i]);
    }
    printf("\n");
    // uint8_t buf[8];
    // usbread(handle, buf, sizeof(buf));
  }
}

// ------------------------------------------------------------------print_usage
void print_usage() {
  printf(" Usage: \n");
  printf(" hidapi_sendrecv [-list] [-read vid pid serial]\n");
  printf(" examples:\n");
  printf("   hidapi_sendrecv -read 0x5f 0x4f aap\n");
  printf("\n");
}

// -----------------------------------------------------------------list_devices
void list_devices() {
  printf("listing hid devices...\n");
  struct hid_device_info* devs;
  struct hid_device_info* cur_dev;
  devs = hid_enumerate(0x0, 0x0);
  cur_dev = devs;
  while (cur_dev) {
    printf("Device: VID:%04hx PID:%04hx\n", 
           cur_dev->vendor_id, cur_dev->product_id);
    // printf("  path: %s\n", cur_dev->path);
    printf("serial_number: %ls\n", cur_dev->serial_number);
    printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
		printf("  Product:      %ls\n\n", cur_dev->product_string);
		    
    cur_dev = cur_dev->next;
  }
  hid_free_enumeration(devs);
}

hid_device* open_dev_by_args(int* argc, char** argv) {
  if (*argc < 2) {
    print_usage();
    *argc = 0;
    return NULL;
  }
  unsigned short vid;
  unsigned short pid;
  wchar_t serial_chars[32];
  wchar_t* serial = NULL;
  
  int args_todo = 2;
  args_todo -= sscanf(argv[1], "0x%hX", &vid);
  args_todo -= sscanf(argv[2], "0x%hX", &pid);
  if (*argc > 2) {
    if (strlen(argv[3]) > 0) {
      ++args_todo;
      args_todo -= sscanf(argv[3], "%ls", serial_chars);
      serial = serial_chars;
    }
  }
  if (args_todo != 0) {
    printf("failed. got: %s, %s, %s\n", argv[1], argv[2], argv[3]);
    return NULL;
  }
  printf("opening 0x%04X, 0x%04X, %ls\n", vid, pid, serial);
  hid_device* handle = hid_open(vid, pid, serial);
  if (!handle) {
    printf("Failed to open device\n");
    return NULL;
  }
  return handle;
}

// ---------------------------------------------------------------------read_dev
void read_dev(int* argc, char** argv) {
  hid_device* handle = open_dev_by_args(argc, argv);
  if (handle) {
    uint8_t buf[8];
    usbread(handle, buf, sizeof(buf), 1000);
    hid_close(handle);
  }
  return;
}

// --------------------------------------------------------------------write_dev
void write_dev(int* argc, char** argv) {
  hid_device* handle = open_dev_by_args(argc, argv);
  if (handle) {
    uint8_t buf[9] = {0, 1,2,3,4,5,6,7,8};
    usbwrite(handle, buf, sizeof(buf));
    hid_close(handle);
  }
  return;
}

// --------------------------------------------------------------------setup_i2c
void setup_i2c(hid_device* handle) {
  uint8_t buf[9];
  memset(buf, 0, 9);
  // set TWCR, TWBR and TWAR
  buf[1] = (1<<SET_TWCR) | (1<<SET_TWBR) | (1<<SET_TWAR);
  buf[2] = (1<<TWEN) | (1<<TWEA);
  buf[4] = 17;  // bitrate
  buf[5] = 0x01 << 1 | 0x01 ;  // address 1
  usbwrite(handle, buf, sizeof(buf));
}

// -------------------------------------------------------------------send_start
void send_start(hid_device* handle) {
  uint8_t buf[9];
  memset(buf, 0, 9);
  buf[1] = (1<<SET_TWCR);
  buf[2] = (1<<TWINT) | (1<<TWSTA) | (1<<TWEA) | (1<<TWEN);
  usbwrite(handle, buf, sizeof(buf));
}

// --------------------------------------------------------------------send_stop
void send_stop(hid_device* handle) {
  uint8_t buf[9];
  memset(buf, 0, 9);
  buf[1] = (1<<SET_TWCR);
  buf[2] = (1<<TWINT) | (1<<TWEA) | (1<<TWEN) | (1<<TWSTO);
  usbwrite(handle, buf, sizeof(buf));
}

// ------------------------------------------------------------------update_twar
void update_twar(hid_device* handle, uint8_t twar) {
  uint8_t buf[9];
  memset(buf, 0, 9);
  buf[1] = (1<<SET_TWAR);
  buf[5] = twar;
  usbwrite(handle, buf, sizeof(buf));
}

// ------------------------------------------------------------------update_twbr
void update_twbr(hid_device* handle, uint8_t twbr) {
  uint8_t buf[9];
  memset(buf, 0, 9);
  buf[1] = (1<<SET_TWBR);
  buf[4] = twbr;
  usbwrite(handle, buf, sizeof(buf));
}

// --------------------------------------------------------------------send_twdr
void send_twdr(hid_device* handle, uint8_t twdr) {
  uint8_t buf[9];
  memset(buf, 0, 9);
  buf[1] = (1<<SET_TWDR) | (1<<SET_TWCR);
  buf[2] = (1<<TWINT) | (1<<TWEA) | (1<<TWEN);
  buf[3] = twdr;
  usbwrite(handle, buf, sizeof(buf));
}

// --------------------------------------------------------------------send_data
void send_data(hid_device* handle) {
  update_twcr(handle, (1<<TWINT) | (1<<TWEA) | (1<<TWEN));
}

// ------------------------------------------------------------------update_twcr
void update_twcr(hid_device* handle, uint8_t twcr) {
  uint8_t buf[9];
  memset(buf, 0, 9);
  buf[1] = (1<<SET_TWCR);
  buf[2] = twcr;
  usbwrite(handle, buf, sizeof(buf));
}

// -------------------------------------------------------------------query_devs
void query_devs(hid_device* handle) {
  uint8_t buf[9];
  memset(buf, 0, 9);
  buf[1] = 1 << 4;
  usbwrite(handle, buf, sizeof(buf));
}

// --------------------------------------------------------------set_report_mode
void set_report_mode(hid_device* handle, uint8_t mode) {
  uint8_t buf[9];
  memset(buf, 0, 9);
  buf[6] = mode;
  usbwrite(handle, buf, sizeof(buf));
}

// -------------------------------------------------------------send_data_to_dev
void send_data_to_dev(hid_device* handle, uint8_t addr) {
  uint8_t buf[9];
  memset(buf, 0, 9);
  buf[1] = 2 << 4;
  buf[7] = 1;
  buf[8] = addr;
  usbwrite(handle, buf, sizeof(buf));
}

// -----------------------------------------------------------------send_payload
void send_payload(hid_device* handle, uint8_t data) {
  uint8_t buf[9];
  memset(buf, 0, 9);
  buf[1] = data;
  usbwrite(handle, buf, sizeof(buf));
}

// ---------------------------------------------------------------wait_for_twint
void wait_for_twint(hid_device* handle, uint8_t* buf) {
  while (1) {
    if (usbread(handle, buf, 8, 500)<8) break;
    printf("waiting...\n");
    _delay1s();
    if (buf[1] & (1<<TWINT)) break;
  }
}

// ---------------------------------------------------------------------_delay1s
void _delay1s() {
#ifdef _WIN32
  Sleep(300);
#else
  sleep(1)
#endif
}

// ------------------------------------------------------------------print_state
void print_state(uint8_t* buf) {
  char s[512];
  // status register & 0xF8
  printf("----------------------------------------------------------\n");
  printf("status: 0x%02X\n", buf[0] & 0xF8);
  int n = sprintf(s, "TWCR: ");
  if (buf[1] & (1<<TWINT)) n += sprintf(s+n, "TWINT, ");
  else n += sprintf(s+n, "_____, ");
  if (buf[1] & (1<<TWEA)) n += sprintf(s+n, "TWEA , ");
  else n += sprintf(s+n, "_____, ");
  if (buf[1] & (1<<TWSTA)) n += sprintf(s+n, "TWSTA, ");
  else n += sprintf(s+n, "_____, ");
  if (buf[1] & (1<<TWSTO)) n += sprintf(s+n, "TWSTO, ");
  else n += sprintf(s+n, "_____, ");
  if (buf[1] & (1<<TWWC)) n += sprintf(s+n, "TWWC , ");
  else n += sprintf(s+n, "_____, ");
  if (buf[1] & (1<<TWEN)) n += sprintf(s+n, "TWEN, ");
  else n += sprintf(s+n, "____, ");
  n += sprintf(s+n, "  -  , ");
  if (buf[1] & (1<<TWIE)) n += sprintf(s+n, "TWIE");
  else n += sprintf(s+n, "____");
  printf("%s\n", s);
  printf("TWDR: ");
  if (buf[1] & (1<<TWINT)) printf("0x%02X\n", buf[2]);
  else printf("N/A\n");
  printf("TWBR: %hu\n", buf[3]);
  printf("TWAR: %hu\n", buf[4]);
  printf("----------------------------------------------------------\n");
  
}

// ----------------------------------------------------------------list_i2c_devs
void list_i2c_devs(hid_device* handle) {
  int i = 0;
  struct {
    uint8_t addr;
    uint8_t type;
    uint8_t bufsize;
    uint8_t rand;
  } devices[10];
  
  uint8_t buf[8];
  send_start(handle);
  wait_for_twint(handle, buf);
  send_twdr(handle, 0x00);  // GC
  wait_for_twint(handle, buf);
  send_twdr(handle, 0x01);  // addr
  wait_for_twint(handle, buf);
  send_twdr(handle, 0x01);  // type
  wait_for_twint(handle, buf);
  send_twdr(handle, 0x08);  // bufsize
  wait_for_twint(handle, buf);
  send_twdr(handle, 0x42);  // cmd
  wait_for_twint(handle, buf);
  send_stop(handle);
  // gc from slaves
  wait_for_twint(handle, buf);
  update_twcr(handle, (1<<TWEN) | (1<<TWEA) | (1<<TWINT));  // ack
  wait_for_twint(handle, buf);
  print_state(buf);
  update_twcr(handle, (1<<TWEN) | (1<<TWEA) | (1<<TWINT));  // ack
  wait_for_twint(handle, buf);
  print_state(buf);
  devices[i].addr = buf[2];
  update_twcr(handle, (1<<TWEN) | (1<<TWEA) | (1<<TWINT));  // ack
  wait_for_twint(handle, buf);
  print_state(buf);
  devices[i].type = buf[2];
  update_twcr(handle, (1<<TWEN) | (1<<TWINT));  // nack
  wait_for_twint(handle, buf); // we're at 0x98 now
  print_state(buf);
  devices[i].bufsize = buf[2];
  devices[i].rand = buf[2];
  ++i;
  // back to idle mode
  update_twcr(handle, (1<<TWEN) | (1<<TWEA) | (1<<TWINT));  // ack
  
  printf("device:\n\taddr %hu\n\ttype %hu\n\tbufsize %hu\n\trand %hu\n",
         devices[0].addr, devices[0].type, devices[0].bufsize, devices[0].rand);
  
  
}

// -------------------------------------------------------------------mastermode
void mastermode(int* argc, char** argv) {
  hid_device* handle = open_dev_by_args(argc, argv);
  if (handle) {
    // system("cls");
    uint8_t buf[8];
    char input[128];
    int i = 0;
    uint16_t read = 1;
    int args_found = 0;
    uint16_t data = 0;
    while(1) {
      //system("cls");
      if (read > 0) {
        --read;
        if (usbread(handle, buf, sizeof(buf), 500) < 8) break;
        print_state(buf);
        if ((buf[1] & (1<<TWINT))) read = 0;
        else if (read>0) {
          _delay1s();
        }
      } else {
        printf("1: setup_i2c\n");
        printf("2: read \n");
        printf("3: send START\n");
        printf("4: set TWAR\n");
        printf("5: set TWDR and send\n");
        printf("6: set TWINT, TWEN, TWEA\n");
        printf("7: send STOP\n");
        printf("8: set TWCR pure (0x84 = int+ea)\n");
        printf("9: list i2c devs\n");
        printf("10: set TWBR\n");
        printf("11: query devs\n");
        printf("12: set report mode\n");
        printf("13: send 1 byte to address\n");
        printf("14: send byte 0\n");
        gets(input);
        args_found = sscanf(input, "%i 0x%02hX", &i, &data);
        if (args_found > 0) {
          if (i == 1) {
            setup_i2c(handle);
            read = 3;
          } else if (i== 2) {
            read = 1;
          } else if (i == 3) {
            send_start(handle);
            read = 10;
          } else if (i == 4) {
            if (args_found == 2) {
              update_twar(handle, (uint8_t)data);
              _delay1s();
              read = 1;
            } else {
              printf("need hex data\n");
            }
          } else if (i == 5) {
            if (args_found == 2) {
              send_twdr(handle, (uint8_t)data);
              _delay1s();
              read = 1;
            } else {
              printf("need hex data\n");
            }
          } else if (i == 6) {
            send_data(handle);
            _delay1s();
            read = 10;
          } else if (i == 7) {
            send_stop(handle);
            _delay1s();
            read = 1;
          } else if (i == 8) {
            if (args_found == 2) {
              update_twcr(handle, (uint8_t)data);
              _delay1s();
              read = 1;
            } else {
              printf("need hex data\n");
            }
          } else if (i == 9) {
            list_i2c_devs(handle);
          } else if (i == 10) {
            if (args_found == 2) {
              update_twbr(handle, (uint8_t)data);
              _delay1s();
              read = 1;
            } else {
              printf("need hex data\n");
            }
          } else if (i == 11) {
            query_devs(handle);
            read = 10;
          } else if (i == 12) {
            if (args_found == 2) {
              set_report_mode(handle, (uint8_t)data);
              _delay1s();
              read = 1;
            } else {
              printf("expected mode as well\n");
            }
          } else if (i == 13) {
            if (args_found == 2) {
              send_data_to_dev(handle, (uint8_t)data);
              _delay1s();
              read = 1;
            } else {
              printf("expected address as well\n");
            }
          } else if (i == 14) {
            if (args_found == 2) {
              send_payload(handle, (uint8_t)data);
              _delay1s();
              read = 1;
            }else {
              printf("expected data as well\n");
            }
          }else {
            break;
          }
        }
      }
    }
  }
  hid_close(handle);
}

// -------------------------------------------------------------------------main
int main(int argc, char** argv) {
  printf("hidapi_sendrecv\n\n");
  if (argc < 2) {
    print_usage();
    return 0;
  }
  
  int init_result = hid_init();
  if (init_result) {
    printf("hid_init() failed\n");
    return 1;
  }
  
  while (argc--) {
     if (strcmp(*argv, "-list") == 0) list_devices();
     if (strcmp(*argv, "-read") == 0) read_dev(&argc, argv);
     if (strcmp(*argv, "-write") == 0) write_dev(&argc, argv);
     if (strcmp(*argv, "-go") == 0) mastermode(&argc, argv);
     ++argv;
  }
  int exit_result = hid_exit();
  if (exit_result) {
    printf("hid_exit() failed\n");
    return 1;
  }
  return 0;
}
