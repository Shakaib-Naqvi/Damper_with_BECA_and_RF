// #define DEBUG

#define RF_DATA_PIN 27

const int Buzzer_Pin = 23;
const int LED_Pin = 22;

int servo_open_pos = 135;
int servo_close_pos = 25;
int last_pos_servo;
int servo_delay = 35;
bool beca_status = false;
volatile bool rfDataReceived = false;  // Flag to indicate RF data received


unsigned long previousMillis = 0;
const long interval = 2000;  // Interval at which to check for changes in milliseconds
bool last_rf_signal = false;
bool beca_power = false;
int beca_mode;
int temp_by_beca;
int setpointt;
bool rf_signal;
uint16_t last_setpoint = 0;
int cfmbutton = 0;
int lst_cfmbutton = 0;
int bp = 0;
bool cfm_flag = false;
int save_setpointt = 0;
int CFM_s;
int minval, maxval;
#include "Servo.h"
// #include "ServerForWiFiCredentials.h"
#include "RES_MODBUS_V1_3.h"
#include <Preferences.h>
#include "TM1637_Display.h"

Preferences preferences;

void Pot_Calib(int min, int max) {
  if (min == 0 || max == 0) {
    if (min == 0) {
      myservo.write(servo_close_pos);
      delay(1000);
      int potmin = analogRead(34);
      delay(1000);
      preferences.begin("Pot", false);
      preferences.putInt("Min", potmin);
      preferences.end();
#ifdef DEBUG
      Serial.print("PotMin:");
      Serial.println(potmin);
#endif
    }
    if (max == 0) {
      myservo.write(servo_open_pos);
      delay(1000);
      int potmax = analogRead(34);
      delay(1000);
      preferences.begin("Pot", false);
      preferences.putInt("Max", potmax);
      preferences.end();
#ifdef DEBUG
      Serial.print("potmax:");
      Serial.println(potmax);
#endif
    }
  }
}


void Beep(int beepDelay, int numberOfBeeps) {
  for (int i = 0; i < numberOfBeeps; i++) {
    digitalWrite(Buzzer_Pin, HIGH);
    delay(beepDelay);
    digitalWrite(Buzzer_Pin, LOW);
    delay(beepDelay);
  }
}

int ReadPot(int potPin) {

  int minValue = minval;  // at servo_close_pos
  int maxValue = maxval;


  float potValue = analogRead(potPin);
  int mappedValue = map(potValue, minValue, maxValue, 0, 100);

  if (mappedValue < minValue) {
    minValue = mappedValue;
  } else if (mappedValue > maxValue) {
    maxValue = mappedValue;
  }
#ifdef DEBUG
  Serial.println("Pot Value");
  Serial.println(potValue);
  Serial.println("Mapped Value+");
  Serial.println(mappedValue);
#endif
  return mappedValue;
}

void setup() {
  Int_Servo();
  // preferences.begin("Pot", false);
  // preferences.putInt("Min", 0);
  // preferences.putInt("Max", 0);
  // preferences.end();

#ifdef DEBUG
  Serial.begin(115200);
#endif

  preferences.begin("Pot", false);
  minval = preferences.getInt("Min", 0);
  maxval = preferences.getInt("Max", 0);
  preferences.end();

  Pot_Calib(minval, maxval);

  int last_pot_value = ReadPot(34);
  last_pos_servo = map(last_pot_value, 0, 100, servo_close_pos, servo_open_pos);
  pinMode(Buzzer_Pin, OUTPUT);
  digitalWrite(Buzzer_Pin, LOW);

  MoveServo(servo_open_pos, 1, servo_delay);
  delay(1000);
  MoveServo(servo_close_pos, 1, servo_delay);
  RS485Serial.begin(MODBUS_BAUD_RATE, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
  pinMode(RS485_DE_RE_PIN, OUTPUT);
  digitalWrite(RS485_DE_RE_PIN, LOW);  // Set DE/RE pin low for receive mode
  Beep(200, 1);
  pinMode(RF_DATA_PIN, INPUT_PULLUP);  // Enable internal pull-up resistor
  display.setBrightness(0x0a);         // set the brightness (0x00 to 0x0f)
  int CFM;
  preferences.begin("CFM", false);
  CFM = preferences.getInt("cfm", 50);
  preferences.end();
  CFM_s = map(CFM, 0, 100, servo_close_pos, servo_open_pos);
  beca_check();
  lst_cfmbutton = cfmbutton;
}

void loop() {

  beca_check();

  if (cfmbutton != lst_cfmbutton) {
    if (bp < 3) {
      bp++;
      lst_cfmbutton = cfmbutton;
    } else {
      bp = 0;
      lst_cfmbutton = cfmbutton;
    }
    if (bp == 3 && cfm_flag == false) {
      cfm_flag = true;
      save_setpointt = setpointt * 10;
      Beep(50, 3);
    } else if (bp == 3 && cfm_flag == true) {
      cfm_flag = false;
      writeSingleRegister(1, 0x03, save_setpointt);
      Beep(1000, 1);
    }
  }
  if (cfm_flag == true && beca_power == true) {

    if (setpointt > 19 && setpointt < 31) {
      int cfm = (setpointt - 20) * 10;
      preferences.begin("CFM", false);
      preferences.putInt("cfm", cfm);
      preferences.end();
      CFM_s = map(cfm, 0, 100, servo_close_pos, servo_open_pos);
      MoveServo(CFM_s, 1, servo_delay);
      // Serial.println(cfm);
      // Serial.println(CFM_s);
    }
  }  //
  else {
    /*---------------------- Cool Mode ---------------------*/
    if (beca_mode == 0 || beca_mode == 2) {
      if (last_setpoint > temp_by_beca && beca_power == true) {
        MoveServo(servo_close_pos, 1, servo_delay);
      } else if (last_setpoint <= temp_by_beca && beca_power == true) {
        MoveServo(CFM_s, 1, servo_delay);
      }
    }

    /*---------------------- Heat Mode ---------------------*/
    else if (beca_mode == 1) {
      if (last_setpoint <= temp_by_beca && beca_power == true) {
        MoveServo(servo_close_pos, 1, servo_delay);
      } else if (last_setpoint > temp_by_beca && beca_power == true) {
        MoveServo(CFM_s, 1, servo_delay);
      }
    }
  }
  // rf_signal = digitalRead(RF_DATA_PIN);

  // if (rf_signal) {  //OFF
  //   Beep(50, 1);
  //   writeSingleRegister(1, 0x00, 0);  // Turn OFF BECA
  // } else {                            //ON
  //   Beep(50, 1);
  //   writeSingleRegister(1, 0x00, 1);  // Turn ON BECA
  // }

  if (beca_power == 0) {
    MoveServo(servo_close_pos, 1, servo_delay);
    show_on_led(0, 0);
  } else {
    unsigned long currentMillis = millis();
    // Check if the value of register[3] has changed
    if (setpointt != last_setpoint) {
      show_on_led(0, setpointt);  // Display setpoint
      last_setpoint = setpointt;
      previousMillis = currentMillis;
    } else if (currentMillis - previousMillis >= interval) {
      show_on_led(0, temp_by_beca);
    }
  }
}

void beca_check() {
  beca_status = readHoldingRegisters(MODBUS_SLAVE_ID, MODBUS_REGISTER_ADDRESS, MODBUS_REGISTER_COUNT, registers);
  if (beca_status) {
    cfmbutton = registers[1];
    beca_power = registers[0];
    beca_mode = registers[2];
    setpointt = registers[3] / 10;
    temp_by_beca = registers[8] / 10;
  } else {

    cfmbutton = 0;
    beca_power = 0;
    beca_mode = 0;
    setpointt = 0;
    temp_by_beca = 0;
    Beep(150, 5);
    beca_status = readHoldingRegisters(MODBUS_SLAVE_ID, MODBUS_REGISTER_ADDRESS, MODBUS_REGISTER_COUNT, registers);
    while (beca_status == false) {
      beca_status = readHoldingRegisters(MODBUS_SLAVE_ID, MODBUS_REGISTER_ADDRESS, MODBUS_REGISTER_COUNT, registers);
      show_on_led(4, 0);
      MoveServo(servo_close_pos, 1, servo_delay);
    }
  }
}
