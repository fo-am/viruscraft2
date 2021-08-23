/* This example shows how to use continuous mode to take
range measurements with the VL53L0X. It is based on
vl53l0x_ContinuousRanging_Example.c from the VL53L0X API.

The range readings are in units of mm. */

#include <Wire.h>
#include <SoftWire.h>
#include <avr/wdt.h>
#include "VL53L0X.h"

int sdaPin=3;
int sclPin=4;
char swTxBuffer[16];
char swRxBuffer[16];
SoftWire sw(sdaPin, sclPin);

VL53L0X sensor(&sw);

void setup()
{
//  Serial.begin(9600);
  wdt_enable(WDTO_4S);

  sw.setTxBuffer(swTxBuffer, sizeof(swTxBuffer));
  sw.setRxBuffer(swRxBuffer, sizeof(swRxBuffer));
  sw.setDelay_us(5);
  sw.setTimeout(1000);
  sw.begin();
  
  pinMode(2,OUTPUT);

  digitalWrite(2,HIGH);
  delay(40);
  digitalWrite(2,LOW);
  
  sensor.setTimeout(500);
  if (!sensor.init())
  {
    while (1) {
      digitalWrite(2,HIGH);
      delay(100);
      digitalWrite(2,LOW);
      delay(100);
      // will be reset by wdt as i2c hasn't started up yet
    }
  }
  
// 0x70 = square
// 0x71 = guitar
// 0x72 = triangle
// 0x73 = circle
  
  Wire.begin(0x70); 
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
    
  // Start continuous back-to-back mode (take readings as
  // fast as possible).  To use continuous timed mode
  // instead, provide a desired inter-measurement period in
  // ms (e.g. sensor.startContinuous(100)).
  sensor.startContinuous();
}

unsigned char state = 0;
unsigned char thresh = 100;
unsigned short value = 0;
unsigned char debounce = 0;
float fv = 0;
float f = 0.3;
unsigned int alive=0;
void loop()
{
  value=sensor.readRangeContinuousMillimeters();
  fv = value*f+fv*(1-f);
  
//  Serial.println(fv);
  if (fv<thresh) {
    digitalWrite(2,HIGH);
    state=1;
    debounce=1;
  } else {
    digitalWrite(2,LOW);
    state=0;
  }
  alive++;
}

unsigned char addr = 0x00;
void receiveEvent(int howMany) {
    addr=Wire.read();
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void requestEvent()
{
  // use the wdt so if we don't receive a message for 4 seconds, we reset
    wdt_reset();
    switch (addr) {
      case 0x00: Wire.write(0xfa); break;
      case 0x01: Wire.write(state); break;
      case 0x02: Wire.write(thresh); break;
      case 0x03: Wire.write(value&0xff); break;
      case 0x04: Wire.write((value>>8)&0xff); break;
      case 0x05: Wire.write(debounce); break;
      case 0x06:
        Wire.begin(0x73); 
        Wire.onReceive(receiveEvent);
        Wire.onRequest(requestEvent);
        Wire.write(0xee);
      break;
      case 0x07: Wire.write(sensor.init()); break;
      case 0x08: Wire.write(alive); break;
      case 0x09: 
        sw.setTxBuffer(swTxBuffer, sizeof(swTxBuffer));
        sw.setRxBuffer(swRxBuffer, sizeof(swRxBuffer));
        sw.setDelay_us(5);
        sw.setTimeout(1000);
        sw.begin();
        Wire.write(0xef);
      break;   
      default: Wire.write(state); break;
  }
  debounce=0;
}
