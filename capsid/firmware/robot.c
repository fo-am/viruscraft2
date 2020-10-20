#include <avr/io.h>
#include <avr/interrupt.h>
//#include <avr/eeprom.h>
//#include <avr/wdt.h>
//#include <util/delay_basic.h>
#include <util/delay.h>
#include "usiTwiSlave.h"
#include "servo.h"
#include <stdint.h>

#define LED PB0
#define I2C_ADDR 0x0a

//            attiny84
//              ____
//       vcc  [\    \]  gnd
// led <- b0  [\    \]  a0 -> servo a
//        b1  [\    \]  a1 -> servo b
//      reset [\    \]  a2 -> servo c
//        b2  [\    \]  a3 -> servo d
//        a7  [\    \]  a4 -> scl
// sda <- a6  [\____\]  a5
//

// registers
#define REG_ALIVE 0
#define REG_ADDR  1

// high level controls
#define REG_SHOW  2 // 1,2,3,4 or 0 
#define REG_SERVO_SPEED 2
#define REG_SERVO_INTERPOLATION 2

// low level controls
#define REG_SERVO_A_ANGLE 2
#define REG_SERVO_A_ACTIVE 2
#define REG_SERVO_A_PULSE_MIN 2
#define REG_SERVO_A_PULSE_MAX 2
#define REG_SERVO_A_ANGLE_MIN 2
#define REG_SERVO_A_ANGLE_MAX 2

uint8_t counter=0;

int hide_angle[SERVO_NUM];
int show_angle[SERVO_NUM];
int current = 0;
servo_state servo[SERVO_NUM];

void show(int id) {
  servo_modify(&servo[current],hide_angle[current],10);
  if (id>=0 && id<SERVO_NUM) {
    current = id;
    servo_modify(&servo[current],show_angle[current],10);
  }
}

uint8_t i2c_read(uint8_t reg) {
  switch (reg) {
  case 0: return counter++; 
  case 1: return I2C_ADDR; 
  default:
    return 0xff;
  }
}

void i2c_write(uint8_t reg, uint8_t value) {
  switch (reg) {
  case 3: show(value);
  }
}

int main() {
  usiTwiSlaveInit(I2C_ADDR, i2c_read, i2c_write);
  servo_init();
  sei();
 
  hide_angle[0]=-35;
  hide_angle[1]=80;
  hide_angle[2]=-80;
  hide_angle[3]=90;

  show_angle[0]=20;
  show_angle[1]=15;
  show_angle[2]=10;
  show_angle[3]=35;

  for (unsigned int s = 0; s<SERVO_NUM; s++) { 
    servo_state_init(&servo[s],s);
    servo_modify(&servo[s],hide_angle[s],1000);
    servo[s].interpolation = SERVO_INTERPOLATION_COSINE;
  }
  
  DDRB |= _BV(LED); // led output  
  PORTB |= _BV(LED);

  int counter = 0;
  int state = 0;
  
  while (1) {    
    if (counter>100) {           
      //show(state%5);
      
      if (state%2==0) {
	PORTB |= _BV(LED);
      } else {
	PORTB &= ~_BV(LED);
      }
      
      state++;
      counter=0;
    }

    for (unsigned int s = 0; s<4; s++) { 
      servo_update(&servo[s]);
    }
    
    counter++;
    _delay_ms(10);    
  }
}

int i=0;

ISR(TIM1_COMPA_vect) {
  servo_pulse_update();
}

