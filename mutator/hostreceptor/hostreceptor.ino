/* This example shows how to use continuous mode to take
range measurements with the VL53L0X. It is based on
vl53l0x_ContinuousRanging_Example.c from the VL53L0X API.

The range readings are in units of mm. */

#include <Wire.h>
#include <SoftWire.h>
#include "VL53L0X.h"

int sdaPin=3;
int sclPin=4;
char swTxBuffer[16];
char swRxBuffer[16];
SoftWire sw(sdaPin, sclPin);

VL53L0X sensor(&sw);

void setup()
{
  Wire.begin(0x20); 
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  
  sw.setTxBuffer(swTxBuffer, sizeof(swTxBuffer));
  sw.setRxBuffer(swRxBuffer, sizeof(swRxBuffer));
  sw.setDelay_us(5);
  sw.setTimeout(1000);
  sw.begin();
  
  pinMode(2,OUTPUT);

  sensor.setTimeout(500);
  if (!sensor.init())
  {
    while (1) {
      digitalWrite(2,HIGH);
      delay(100);
      digitalWrite(2,LOW);
      delay(100);
    }
  }

  // Start continuous back-to-back mode (take readings as
  // fast as possible).  To use continuous timed mode
  // instead, provide a desired inter-measurement period in
  // ms (e.g. sensor.startContinuous(100)).
  sensor.startContinuous();
}

unsigned char state = 0;
unsigned char thresh = 100;

void loop()
{
  int v=sensor.readRangeContinuousMillimeters();

  if (v<thresh) {
    digitalWrite(2,HIGH);
    state=1;
  } else {
    digitalWrite(2,LOW);
    state=0;
  }
}

unsigned char addr = 0x00;
void receiveEvent(int howMany) {
    addr=Wire.read();
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void requestEvent()
{
    switch (addr) {
      case 0x00: Wire.write(0xfa);
      case 0x01: Wire.write(state);
      case 0x02: Wire.write(thresh);
      default: Wire.write(state);
  }
}
