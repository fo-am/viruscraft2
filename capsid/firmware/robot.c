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
//       vcc  =|    |=  gnd
// led <- b0  =|    |=  a0 -> servo a
//        b1  =|    |=  a1 -> servo b
//      reset =|    |=  a2 -> servo c
//        b2  =|    |=  a3 -> servo d
//        a7  =|    |=  a4 -> scl
// sda <- a6  =|____|=  a5
//

// registers
#define REG_ALIVE 0
#define REG_ADDR  1

// high level control i2c registers
#define REG_SHOW_ID  2 // 0,1,2,3 or anything else = hide all 
#define REG_SERVO_SPEED 3
#define REG_SERVO_INTERPOLATION 4
#define REG_SERVO_START 5

// high level controls i2c registers
#define REG_SERVO_ANGLE 0
#define REG_SERVO_ACTIVE 1     // power down to save current consumption
#define REG_SERVO_PULSE_MIN 2
#define REG_SERVO_PULSE_MAX 3
#define REG_SERVO_HIDE_ANGLE 4
#define REG_SERVO_SHOW_ANGLE 5
#define REG_SERVO_SIZE 6

#define REG_SERVO_A REG_SERVO_START
#define REG_SERVO_B (REG_SERVO_START+REG_SERVO_SIZE)
#define REG_SERVO_C (REG_SERVO_START+REG_SERVO_SIZE*2)
#define REG_SERVO_D (REG_SERVO_START+REG_SERVO_SIZE*3)
#define REG_SERVO_END (REG_SERVO_START+REG_SERVO_SIZE*4)

uint8_t alive_counter=0;

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

uint8_t read_servo(unsigned int servo_id, unsigned int i) {
  switch (i) {
  case REG_SERVO_ANGLE: return 0;
  case REG_SERVO_ACTIVE: return servo[servo_id].active;
  case REG_SERVO_PULSE_MIN:  return servo[servo_id].min_pulse;
  case REG_SERVO_PULSE_MAX:  return servo[servo_id].max_pulse;
  case REG_SERVO_HIDE_ANGLE:  return hide_angle[servo_id];
  case REG_SERVO_SHOW_ANGLE:  return show_angle[servo_id];;
  default: return 0;
  }
}

void write_servo(unsigned int servo_id, unsigned int i, uint8_t value) {
  switch (i) {
  case REG_SERVO_ANGLE: break;
  case REG_SERVO_ACTIVE: servo[servo_id].active=value; break;
  case REG_SERVO_PULSE_MIN: servo[servo_id].min_pulse=value; break;
  case REG_SERVO_PULSE_MAX: servo[servo_id].max_pulse=value; break;
  case REG_SERVO_HIDE_ANGLE: hide_angle[servo_id]=value; break;
  case REG_SERVO_SHOW_ANGLE: show_angle[servo_id]=value; break;
  default: break;
  }
}

uint8_t i2c_read(uint8_t reg) {
  switch (reg) {
  case REG_ALIVE: return alive_counter++; 
  case REG_ADDR:  return I2C_ADDR; 
  case REG_SHOW_ID: return current;
  case REG_SERVO_SPEED: return servo[0].speed;
  case REG_SERVO_INTERPOLATION: return servo[0].interpolation;
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
  switch (reg) {
  case REG_SHOW_ID: show(value); break;
  case REG_SERVO_SPEED:
    for (unsigned int s = 0; s<SERVO_NUM; s++)
      servo[s].speed=value;
    break;
  case REG_SERVO_INTERPOLATION:
    for (unsigned int s = 0; s<SERVO_NUM; s++)
      servo[s].interpolation=value;
    break;
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
  servo_init();
  usiTwiSlaveInit(I2C_ADDR, i2c_read, i2c_write);
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
    if (counter>10) {           
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

