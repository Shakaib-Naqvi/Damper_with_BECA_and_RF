#include <ESP32PWM.h>
#include <ESP32Servo.h>
#include <Preferences.h>

// #define DEBUG

Servo myservo;  // create servo object to control a servo


int pos = 0;  // variable to store the servo position

void MoveServo(uint8_t degrees, int steps) {
  // delay(2000);
  Preferences preferences;
  preferences.begin("servo", false);
  ESP32PWM::allocateTimer(0);  // Allocate a timer for the PWM
  myservo.setPeriodHertz(50);  // Standard 50hz servo
  myservo.attach(25);          // attaches the servo on pin 25 to the servo object

  pos = preferences.getInt("position", 0);
  if (pos < 25 || pos > 135) {
    pos = 0;
  }

  if (pos > degrees) {
    // if (0 > 20) {
    for (pos = pos; pos >= degrees; pos -= steps) {
#ifndef DEBUG
      Serial.println(pos);
#endif
      myservo.write(pos);
      delay(15);  // tell servo to go to position in variable 'pos'
      // pos_2 = pos;
      preferences.putInt("position", pos);
    }
    if (pos < degrees) {
      pos = degrees;  // Ensure the final position is exactly the target
      myservo.write(pos);
      preferences.putInt("position", pos);
#ifndef DEBUG
      Serial.println(pos);
#endif
    }
  } else {
    for (pos = pos; pos <= degrees; pos += steps) {
#ifndef DEBUG
      Serial.println(pos);
#endif
      myservo.write(pos);  // tell servo to go to position in variable 'pos'
      delay(15);           // waits 15ms for the servo to reach the position
      // pos_2 = pos;  // waits 15ms for the servo to reach the position
      preferences.putInt("position", pos);
    }
    if (pos > degrees) {
      pos = degrees;  // Ensure the final position is exactly the target
      myservo.write(pos);
      preferences.putInt("position", pos);
#ifndef DEBUG
      Serial.println(pos);
#endif
    }
  }
}
