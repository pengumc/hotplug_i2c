// i2cslave for hotplug i2c
// atmega88
// lfuse E2 (use internal 8 MHz)
// 1 1 1 0 0 0 1 0
// | | | | | | | |
// | | | | | | | +-- CKSEL0
// | | | | | | +-- CKSEL1
// | | | | | +-- CKSEL2
// | | | | +-- CKSEL3
// | | | +-- SUT0
// | | +-- SUT1
// | +-- CKOUT
// +-- CKDIV8

// hfuse D6

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

#include "default.h"
#include "i2cstuff.h"
// 7 seg
//    a
//  f   b
//    g
//  e   c
//    d dot
// PB0..5 =c, b, a, f, g, dot
// pd6, 7= e, d

//     PB3
//  PB4   PB2
//     PB5
//  PD6   PB0
//     PD7PB1

void clear_7seg(){
  // clear seg pins
  PORTB &= ~((1<<PB0) | (1<<PB1) | (1<<PB2) | (1<<PB3) | (1<<PB4) | (1<<PB5));
  PORTD &= ~((1<<PD6) | (1<<PD7));
}

void set_7seg(uint8_t number) {
  clear_7seg();
  if (number == 0) {
    PORTB |= (1<<PB0) | (0<<PB1) | (1<<PB2) | (1<<PB3) | (1<<PB4) | (0<<PB5);
    PORTD |= (1<<PD6) | (1<<PD7);
  } else if (number == 1) {
    PORTB |= (1<<PB0) | (0<<PB1) | (1<<PB2) | (0<<PB3) | (0<<PB4) | (0<<PB5);
    PORTD |= (0<<PD6) | (0<<PD7);
  } else if (number == 2) {
    PORTB |= (0<<PB0) | (0<<PB1) | (1<<PB2) | (1<<PB3) | (0<<PB4) | (1<<PB5);
    PORTD |= (1<<PD6) | (1<<PD7);
  } else if (number == 3) {
    PORTB |= (1<<PB0) | (0<<PB1) | (1<<PB2) | (1<<PB3) | (0<<PB4) | (1<<PB5);
    PORTD |= (0<<PD6) | (1<<PD7);
  } else if (number == 4) {
    PORTB |= (1<<PB0) | (0<<PB1) | (1<<PB2) | (0<<PB3) | (1<<PB4) | (1<<PB5);
    PORTD |= (0<<PD6) | (0<<PD7);
  } else if (number == 5) {
    PORTB |= (1<<PB0) | (0<<PB1) | (0<<PB2) | (1<<PB3) | (1<<PB4) | (1<<PB5);
    PORTD |= (0<<PD6) | (1<<PD7);
  } else if (number == 6) {
    PORTB |= (1<<PB0) | (0<<PB1) | (0<<PB2) | (1<<PB3) | (1<<PB4) | (1<<PB5);
    PORTD |= (1<<PD6) | (1<<PD7);
  } else if (number == 7) {
    PORTB |= (1<<PB0) | (0<<PB1) | (1<<PB2) | (1<<PB3) | (0<<PB4) | (0<<PB5);
    PORTD |= (0<<PD6) | (0<<PD7);
  } else if (number == 8) {
    PORTB |= (1<<PB0) | (0<<PB1) | (1<<PB2) | (1<<PB3) | (1<<PB4) | (1<<PB5);
    PORTD |= (1<<PD6) | (1<<PD7);
  } else if (number == 9) {
    PORTB |= (1<<PB0) | (0<<PB1) | (1<<PB2) | (1<<PB3) | (1<<PB4) | (1<<PB5);
    PORTD |= (0<<PD6) | (1<<PD7);
  } else {
    PORTB |= (0<<PB0) | (0<<PB1) | (0<<PB2) | (0<<PB3) | (0<<PB4) | (1<<PB5);
    PORTD |= (0<<PD6) | (0<<PD7);
  }
}

void set_pure(uint8_t X) {
  // 3 2 0 7 6 4 5
  uint8_t newB = 0x00;
  uint8_t newD = 0x00;
  newB |= (X & (1<<0))<<(PB3);
  newB |= (X & (1<<1))<<(PB2-1);
  newB |= (X & (1<<2))>>2;  // (PB0-2);
  newD |= (X & (1<<3))<<(PD7-3);
  newD |= (X & (1<<4))<<(PD6-4);
  newB |= (X & (1<<5))>>1;  // (PB4-5);
  newB |= (X & (1<<6))>>1; // (PB5-6);
  newB |= (X & (1<<7))>>6;  //(PB1-7)
  clear_7seg();
  PORTB |= newB;
  PORTD |= newD;
}
volatile uint8_t show = 0;
i2cdata_t i2cdata;

void i2c_receive(){
  // i2cdata.lastmode for mode
  // for (i = 0; i < data.lastlen; ++i) {
  //    this is your data
  // }
  switch (i2cdata.lastmode) {
    case 0: {
      CLR(TIMSK1, OCIE1A);
      set_7seg(i2cdata.buffer[0]);
      break;
    }
    default: {
      SET(TIMSK1, OCIE1A);
    }
  }
}


int main() {
  uint8_t reset = 0;
  wdt_enable(WDTO_1S);
  DDRB = 0x00 | (1<<PB0) | (1<<PB1) | (1<<PB2) | (1<<PB3) | (1<<PB4) | (1<<PB5);
  DDRD = (1<<PD6) | (1<<PD7);
  SET(PORTD, PD0);
  TCCR1A = 0;
  TCCR1B = (1<<WGM12) | 0b100;
  OCR1A = 31250;
  TIMSK1 |= (1<<OCIE1A);
  sei();
  
  uint8_t pressed = 0;
  uint16_t kanker = 0;
  
  start_I2C(&i2cdata, 32);
  TCNT1 = 0;
  while(1){
    doi2cstuff(&i2cdata);
    if (reset != 1) wdt_reset();
    if (!CHK(PIND, PD0)) {
      //down
      if (pressed == 0) {
        kanker++;
        if (kanker == 0) {
          pressed = 1;
          set_7seg(i2cdata.state);
          show = 0;
          reset = 1;
        }
      }
    } else {
      pressed = 0;
    }
    
  }
}

ISR(TIMER1_COMPA_vect){
  ++show;
  if (show == 0) {
    set_7seg(10);
  } else if (show == 1) {
    set_pure(TWCR);
  } else if (show == 2) {
    set_pure(TWSR & 0xF8);
  } else if (show == 3) {
    set_pure(i2cdata.state);
  } else if (show == 4) {
    set_pure(i2cdata.address);
    show = 0xFF;
  }
}
