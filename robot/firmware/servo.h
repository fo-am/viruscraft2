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

#ifndef UNIT_TEST
#include <avr/io.h>
#endif
#include "fixed.h"

#ifndef PENELOPE_SERVO
#define PENELOPE_SERVO

#define SERVO_PORT PORTA
#define SERVO_DDR DDRA
#define SERVO_NUM 4
#define SERVO_MASK (0xff>>(8-SERVO_NUM))

#define SERVO_MIN (1100/2) //1000
#define SERVO_MAX (4500/2) //4500
#define SERVO_MID (2750/2) //2750
#define SERVO_DEG_MIN -90
#define SERVO_DEG_MAX 90

void servo_init();
void servo_pulse_update();

unsigned int servo_pulse[9];

#define SERVO_INTERPOLATION_LINEAR 0
#define SERVO_INTERPOLATION_COSINE 1

typedef struct {
  unsigned char id;
  unsigned char active;
  unsigned int start_pulse;
  unsigned int end_pulse;
  unsigned int time; // fixed point
  unsigned int speed; // fixed point
  unsigned int amplitude; // fixed
  unsigned int bias_degrees; // degrees
  unsigned int smooth; // fixed
  unsigned char interpolation;
  unsigned int min_pulse;
  unsigned int max_pulse;
} servo_state;

void servo_state_init(servo_state *state, unsigned char id);
unsigned int servo_degrees_to_pulse(servo_state *servo, int degrees);
void servo_modify(servo_state *state, int target_degrees, unsigned int speed);
void servo_update(servo_state *state);

#endif
