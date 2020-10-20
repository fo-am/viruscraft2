
#include <avr/io.h>
#include <avr/interrupt.h>
//#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <util/delay_basic.h>
#include <util/delay.h>
//#include "usiTwiSlave.h"
//#include "servo.h"

#define LED PB0

// tried all sorts of nonsense to make i2c addresses writable so
// we can build different configurations of pattern matrix, but they
// are just to susceptable to noise - so hardcoding em for now...
/*#define I2C_ADDR 0x0a

uint8_t counter=0;

uint8_t i2c_read(uint8_t reg) {
  switch (reg) {
  case 0: return counter++; 
  case 1: return I2C_ADDR; 
  default:
    return 0xff;
  }
}

void i2c_write(uint8_t reg, uint8_t value) {}
*/

int main() {
  DDRB |= _BV(LED); // led output  
  PORTB |= _BV(LED);
  cli();
  //  servo_init();

  //usiTwiSlaveInit(I2C_ADDR, i2c_read, i2c_write);

  
  // Clear registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  // 1 Hz (1000000/((15624+1)*64))
  OCR1A = 15624;
  // CTC
  TCCR1B |= (1 << WGM12);
  // Prescaler 64
  TCCR1B |= (1 << CS11) | (1 << CS10);
  // Output Compare Match A Interrupt Enable
  TIMSK1 |= (1 << OCIE1A);

  //servo_state sa;
  //servo_state_init(&sa,0);
  
  
  sei();   
  /*  while (1) {
    PORTB |= _BV(LED);
    //servo_modify(&sa,0,200);
    _delay_ms(1000);    
    PORTB &= ~_BV(LED);
    //servo_modify(&sa,180,200);
    _delay_ms(1000);    
    }*/
}

ISR(TIMER1_COMPA_vect) {
    PORTB &= ~_BV(LED);
  //servo_pulse_update();
}

