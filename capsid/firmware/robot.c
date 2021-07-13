// Copyright (C) 2021 FoAM Kernow
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "usiTwiSlave.h"
#include "servo.h"
#include "crc.h"
#include <stdint.h>
#include <avr/eeprom.h>

#define LED PB0
#define I2C_ADDR 0x14
#define FIRMWARE_VERSION 5

// changlog: 5
// * crc checking all writes
// * new servo defaults
// * watchdog timer

#define CRC_NONCE 0x42

//            attiny84
//              ____
//       vcc  =|    |=  gnd
// led <- b0  =|    |=  a0 -> servo a
//        b1  =|    |=  a1 -> servo b
//      reset =|    |=  a2 -> servo c
//        b2  =|    |=  a3 -> servo d
//        a7  =|    |=  a4 -> scl
// sda <- a6  =|____|=  a5
//

// registers
#define REG_VERSION  0
#define REG_ALIVE 1
#define REG_ADDR  2
#define REG_MODE 3 
#define REG_LED 4
// high level control i2c registers
#define REG_SHOW_ID  5 // 0,1,2,3 or anything else = hide all 
#define REG_SERVO_SPEED 6
#define REG_SERVO_INTERPOLATION 7
#define REG_SERVO_START 8

#define REG_CRC_STATE 0xfa
#define REG_CRCL 0xfb
#define REG_CRCH 0xfc

#define REG_EEINIT 0xfd
#define REG_EESAVE 0xfe

// 0x08 Servo 0 angle
// 0x09 Servo 0 high_power
// 0x0a Servo 0 hide angle
// 0x0b Servo 0 show angle  --> 30

// 0x0c Servo 1 angle
// 0x0d Servo 1 high_power
// 0x0e Servo 1 hide angle
// 0x0f Servo 1 show angle

// 0x10 Servo 2 angle
// 0x11 Servo 2 high_power
// 0x12 Servo 2 hide angle
// 0x13 Servo 2 show angle

// 0x14 Servo 3 angle
// 0x15 Servo 3 high_power
// 0x16 Servo 3 hide angle
// 0x17 Servo 3 show angle

// high level controls i2c registers
#define REG_SERVO_ANGLE 0
#define REG_SERVO_HIGH_POWER 1     // power down to save current consumption
#define REG_SERVO_HIDE_ANGLE 2
#define REG_SERVO_SHOW_ANGLE 3
#define REG_SERVO_SIZE 4

#define REG_SERVO_A REG_SERVO_START                      
#define REG_SERVO_B (REG_SERVO_START+REG_SERVO_SIZE)     
#define REG_SERVO_C (REG_SERVO_START+REG_SERVO_SIZE*2)   
#define REG_SERVO_D (REG_SERVO_START+REG_SERVO_SIZE*3)   
#define REG_SERVO_END (REG_SERVO_START+REG_SERVO_SIZE*4) 

#define EE_SERVO_SIZE (REG_SERVO_END-REG_SERVO_START)
unsigned char EEMEM ee_servo[EE_SERVO_SIZE]={ [0 ... EE_SERVO_SIZE-1] = 0 };

uint8_t alive_counter=0;
// 0=powerdown
// 1=normal
// 2=calibration (all servos restracted position)
// 3=waggle test
uint8_t mode=0;
uint8_t led_state=0;
uint8_t update_ee=0;

uint8_t crc_state=0;
uint16_t crc_check=0;

int hide_angle[SERVO_NUM];
int show_angle[SERVO_NUM];
int current = 0;
servo_state servo[SERVO_NUM];

void init_defaults() {
  // defaults
  hide_angle[0]=20;
  hide_angle[1]=100;
  hide_angle[2]=5;
  hide_angle[3]=55;

  show_angle[0]=70;
  show_angle[1]=55;
  show_angle[2]=50;
  show_angle[3]=0;

  for (unsigned int s = 0; s<SERVO_NUM; s++) { 
    servo_state_init(&servo[s],s);
  }
}

void show(int id) {
  servo_modify(&servo[current],hide_angle[current],10);
  if (id>=0 && id<SERVO_NUM) {
    current = id;
    servo_modify(&servo[current],show_angle[current],10);
  }
}

uint8_t read_servo(unsigned int servo_id, unsigned int i) {
  switch (i) {
  case REG_SERVO_ANGLE: return 0;
  case REG_SERVO_HIGH_POWER: return servo[servo_id].high_power;
  case REG_SERVO_HIDE_ANGLE:  return hide_angle[servo_id];
  case REG_SERVO_SHOW_ANGLE:  return show_angle[servo_id];
  default: return 0;
  }
}

void write_servo(unsigned int servo_id, unsigned int i, uint8_t value) {
  switch (i) {
  case REG_SERVO_ANGLE: break;
  case REG_SERVO_HIGH_POWER: servo[servo_id].high_power=value; break;
  case REG_SERVO_HIDE_ANGLE: hide_angle[servo_id]=value; break;
  case REG_SERVO_SHOW_ANGLE: show_angle[servo_id]=value; break;
  default: break;
  }
}

uint8_t i2c_read(uint8_t reg) {
  switch (reg) {
  case REG_VERSION: return FIRMWARE_VERSION; 
  case REG_ALIVE: {
	// gives us a way to see if we've reset recently
	if (alive_counter<255) {
	  alive_counter++;
	}
  }
  case REG_ADDR:  return I2C_ADDR; 
  case REG_MODE: return mode; 
  case REG_LED: return led_state; 
  case REG_SHOW_ID: return current;
  case REG_SERVO_SPEED: return servo[0].speed;
  case REG_SERVO_INTERPOLATION: return servo[0].interpolation;
  case REG_CRC_STATE: return crc_state;
  case REG_CRCL: return crc_check; 
  case REG_CRCH: return crc_check<<8;
  default:
    if (reg>=REG_SERVO_A && reg<REG_SERVO_B) {
      return read_servo(0,reg-REG_SERVO_A);
    }
    if (reg>=REG_SERVO_B && reg<REG_SERVO_C) {
      return read_servo(1,reg-REG_SERVO_B);
    }
    if (reg>=REG_SERVO_C && reg<REG_SERVO_D) {
      return read_servo(2,reg-REG_SERVO_C);
    }
    if (reg>=REG_SERVO_D && reg<REG_SERVO_END) {
      return read_servo(3,reg-REG_SERVO_D);
    }    
    return 0xff;	
  }
}

void i2c_write(uint8_t reg, uint8_t value) {
  // write into crc registers directly
  if (reg==REG_CRCH) {
	uint16_t temp = value;
	crc_check = (crc_check & ~0xff00) | (temp << 8);
	return;
  }
  if (reg==REG_CRCL) {
	crc_check = (crc_check & ~0xff) | (value & 0xff);
	return;
  }

  // check crc
  uint8_t buf[3];
  buf[0]=CRC_NONCE;
  buf[1]=reg;
  buf[2]=value;
  if (crc16(buf,sizeof(buf))!=crc_check) {
	crc_state=1;
	return;
  }
  crc_state=0;
  // all matches, go ahead...
  
  switch (reg) {
  case REG_MODE: mode=value; break; 
  case REG_LED: led_state=value; break; 
  case REG_SHOW_ID: show(value); break;
  case REG_SERVO_SPEED:
    for (unsigned int s = 0; s<SERVO_NUM; s++)
      servo[s].speed=value;
    break;
  case REG_SERVO_INTERPOLATION:
    for (unsigned int s = 0; s<SERVO_NUM; s++)
      servo[s].interpolation=value;
    break;
  case REG_EEINIT:
	init_defaults();
	// fallthrough
  case REG_EESAVE: update_ee=1; break;	
  default:
    if (reg>=REG_SERVO_A && reg<REG_SERVO_B) {
      write_servo(0,reg-REG_SERVO_A,value);
    }
    if (reg>=REG_SERVO_B && reg<REG_SERVO_C) {
      write_servo(1,reg-REG_SERVO_B,value);
    }
    if (reg>=REG_SERVO_C && reg<REG_SERVO_D) {
      write_servo(2,reg-REG_SERVO_C,value);
    }
    if (reg>=REG_SERVO_D && reg<REG_SERVO_END) {
      write_servo(3,reg-REG_SERVO_D,value);
    }    
  }
}


int main() {
  init_defaults();

  /* turning this off, as it's too unreliable
  // read eeprom
  unsigned int pos = 0;
  for (unsigned int servo_id=0; servo_id<4; servo_id++) {
	for (unsigned int addr=0; addr<REG_SERVO_SIZE; addr++) {
	  write_servo(servo_id,addr,eeprom_read_byte(&ee_servo[pos++]));
	}
  }
  */
  
  for (unsigned int s = 0; s<SERVO_NUM; s++) { 
    servo_modify(&servo[s],hide_angle[s],1000);
    servo[s].interpolation = SERVO_INTERPOLATION_COSINE;
  }

  servo_init();
  usiTwiSlaveInit(I2C_ADDR, i2c_read, i2c_write);
  wdt_enable(WDTO_2S);
  sei();
  
  DDRB |= _BV(LED); // led output  
  PORTB |= _BV(LED);
  
  // startup flash
  PORTB |= _BV(LED);
  _delay_ms(100);
  PORTB &= ~_BV(LED);

  unsigned int counter = 0;
  unsigned int waggle = 0;
  
  while (1) {
	// calibration mode
	if (mode==2) {
	  for (unsigned int s = 0; s<SERVO_NUM; s++) { 
	  	servo_modify(&servo[s],hide_angle[s],10);
	  }
	}

	if (mode==3) {
	  if (counter>100) {
		unsigned int old_waggle = waggle;
		waggle=(waggle+1)%4;
		servo_modify(&servo[old_waggle],hide_angle[old_waggle],10);		
		servo_modify(&servo[waggle],show_angle[waggle],10);
		counter=0;
	  }
  	  counter++;
	}

	if (led_state==1) {
	  PORTB |= _BV(LED);
	} else {
	  PORTB &= ~_BV(LED);
	}
	
	for (unsigned int s = 0; s<4; s++) { 
	  servo_update(&servo[s]);
	}

	if (update_ee==1) {
	  update_ee=0;
	  unsigned int pos = 0;
	  for (unsigned int servo_id=0; servo_id<4; servo_id++) {
		for (unsigned int addr=0; addr<REG_SERVO_SIZE; addr++) {
		  eeprom_update_byte(&ee_servo[pos++],read_servo(servo_id,addr));
		}
	  }
	  PORTB |= _BV(LED);
	  _delay_ms(50);
	  PORTB &= ~_BV(LED);
	  _delay_ms(50);
	  PORTB |= _BV(LED);
	  _delay_ms(50);
	  PORTB &= ~_BV(LED);
	  _delay_ms(50);
	}
	
    counter++;
    _delay_ms(10);
	wdt_reset();
  }
}

int i=0;


ISR(TIM1_COMPA_vect) {
  servo_pulse_update(mode);
}

