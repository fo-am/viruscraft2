#include <Servo.h>

Servo servo_a;  // create servo object to control a servo
Servo servo_b;  // create servo object to control a servo
Servo servo_c;  // create servo object to control a servo
Servo servo_d;  // create servo object to control a servo


void set_servo_state(int state[]) {
  servo_a.write(90);
  servo_b.write(90);
  servo_c.write(90);
  servo_d.write(90);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  servo_a.attach(9); 
  servo_b.attach(10);  
  servo_c.attach(11);  
  servo_d.attach(12);  
  Serial.begin(9600);
}

//unsigned char v=0;
unsigned char s=0;
unsigned char d=0;

float v=0;
unsigned char moving=0;


float lerp(float a, float b, float t) {
  return a*(1-t)+b*t;
}

float t=0;

void hide_all() {
    servo_a.write(55); // hidden
    servo_b.write(170); // hidden
    servo_c.write(180); // hidden
    servo_d.write(10); // hidden
}

void loop() {  
  //v=0.01*analogRead(A0)+v*0.99;
   //myservo.write(v-150);

  //delay(5000);

  //stress test
  int c=0;
  while (1) {
    c++; 
    switch (c%4) {   
      case 0: servo_a.write(120); break;
      case 1: servo_b.write(105); break;
      case 3: servo_c.write(125); break; 
      case 2: servo_d.write(100) ; break;
    }
    delay(200);
    hide_all();
  }

  v=analogRead(A0);

  if (t>1 && v>200) {
    if (s==0) s=1;
    else s=0;
    t=0;
  }
   
  delay(15);                      
}
