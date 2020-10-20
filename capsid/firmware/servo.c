// Penelopean Robotics Copyright (C) 2019 FoAM Kernow
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

#include "servo.h"
#include "math.h"

// initial pulse times in us for servo 0, 1, 2...7 (last value 6000 is the synchro gap)
unsigned int servo_pulse[9] = {SERVO_MID, SERVO_MID, SERVO_MID, SERVO_MID, 1500, 1500, 1500, 1500, 6000};
unsigned int servo_active[8] = {1,1,1,1,1,1,1,1};

void servo_init() {
#ifndef UNIT_TEST
  //SERVO_DDR = SERVO_MASK;
  SERVO_DDR = 0x0F; 
  TCCR1B |= (1<<WGM12) | (1<<CS11);  // pwm mode 4,CTC, prescale=8
  TIMSK1 |= (1<<OCIE1A);             // enable T1_compareA interrupt 
  TCNT1 = 65530;
#endif
}

// currently overwrites all of PORTA (if output)
void servo_pulse_update() {
#ifndef UNIT_TEST
  static unsigned char servo_num = 0;
  if (servo_num < SERVO_NUM) { // leave space for i2c
    // end pulse for servo (n), start pulse for servo (n+1)      
    SERVO_PORT &= 0xf0;
    if (servo_active[servo_num]==1) {
      SERVO_PORT |= (1<<servo_num);
    }
  } else {
    SERVO_PORT &= 0xf0;
  }
  
  OCR1A = servo_pulse[servo_num];  // set width of pulse
  servo_num++;                     // prepare next servo 
  if(servo_num > 8) servo_num = 0; // again from servo 0;
#endif
}

////////////////////////////////////////////////////////////

void servo_state_init(servo_state *state, unsigned char id) {
  state->id = id;
  state->active = 1;
  state->start_pulse = SERVO_MID;
  state->end_pulse = SERVO_MID;
  state->time = 0;
  state->speed = 0;
  state->amplitude = MAKE_FIXED(1.0);
  state->bias_degrees = 0;
  state->smooth = MAKE_FIXED(0.2);
  state->interpolation = SERVO_INTERPOLATION_LINEAR;
  state->min_pulse = SERVO_MIN;
  state->max_pulse = SERVO_MAX;
}

unsigned int servo_degrees_to_pulse(servo_state *state, int degrees) {
  unsigned int range = SERVO_DEG_MAX-SERVO_DEG_MIN;
  //unsigned int t = ((degrees-SERVO_DEG_MIN)*FIXED)/range;
  //return SERVO_MIN+(t*(SERVO_MAX-SERVO_MIN))/FIXED;
  float t = ((degrees-SERVO_DEG_MIN))/(float)range;
  return state->min_pulse+(t*(state->max_pulse-state->min_pulse));
} 

// start moving (or interrupt a current move) with 
// a new target position and speed
void servo_modify(servo_state *state, int target_degrees, unsigned int speed) {
  state->start_pulse = state->end_pulse;
    
  state->end_pulse = servo_degrees_to_pulse(state,target_degrees)*
    FIXED_TO_FLOAT(state->amplitude)+state->bias_degrees;

  state->time = 0;
  state->speed = speed;
}

int lerp(int a, int b, float v) {
  return a*(1.0-v)+b*v;
}

#define PI 3.141592

int cosi(int a, int b, float v) {
  return lerp(b,a,0.5+cos(PI*v)/2);
}

void servo_update(servo_state *state) {
  if (state->time >= MAKE_FIXED(1.0)) {
    state->time = MAKE_FIXED(1.0);
    state->active = 0;
  } else {
    state->active = 1;
    state->time += state->speed;
  }

  switch (state->interpolation) {
  case SERVO_INTERPOLATION_LINEAR: {
    servo_pulse[state->id] = lerp(state->start_pulse,state->end_pulse,FIXED_TO_FLOAT(state->time));
  } break;
  case SERVO_INTERPOLATION_COSINE: {
    servo_pulse[state->id] = cosi(state->start_pulse,state->end_pulse,FIXED_TO_FLOAT(state->time));
  } break;
    default: break;
  }
    
  state->time += state->speed;
  servo_active[state->id] = state->active;
}



