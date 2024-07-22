#include <ESP32PWM.h>
#include <ESP32Servo.h>

const int ServoPin = 25;
Servo myservo;
int pos = 0;


void Int_Servo() {
  ESP32PWM::allocateTimer(0);  // Allocate a timer for the PWM
  myservo.setPeriodHertz(50);  // Standard 50hz servo
  myservo.attach(ServoPin);
}

void MoveServo(uint8_t degrees, int steps, int del) {
  int position = last_pos_servo;
  if (degrees < 0 || degrees > 180) {
    degrees = 0;
  }  //
  else {
    if (last_pos_servo > degrees) {
      for (int pos = position; pos >= degrees; pos -= steps) {
        myservo.write(pos);
        delay(del);
        last_pos_servo = pos;
      }
      if (last_pos_servo > degrees) {
        myservo.write(degrees);
      }
    } else {
      for (int pos = position; pos <= degrees; pos += steps) {
        myservo.write(pos);
        delay(del);
        last_pos_servo = pos;
      }
      if (last_pos_servo < degrees) {
        myservo.write(degrees);
      }
    }
  }
}
