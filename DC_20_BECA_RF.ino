#include "Pot_Values.h"
#include "Servo.h"
#include "RF_Switch.h"
#include "ServerForWiFiCredentials.h"
#include "RES_MODBUS_V1_3.h"

#define RF_DATA_PIN 27

volatile bool rfDataReceived = false;  // Flag to indicate RF data received

void IRAM_ATTR handleInterrupt() {
  rfDataReceived = true;  // Set flag to true indicating data received
}


int temp_by_beca;
int setpoint;
unsigned long previousMillis = 0;
const long interval = 2000;  // Interval at which to check for changes in milliseconds


void control_with_beca_rf(){
  
  unsigned long currentMillis = millis();
  static uint16_t last_value = 0;
  static bool last_beca_power = false;
  static bool last_rf_signal = true;

  bool rf_signal = digitalRead(RF_DATA_PIN);

  bool beca_power = read_beca(0);

  // Control the servo based on Beca Power
  if (beca_power != last_beca_power) {
    if (beca_power) {
      MoveServo(135, 1);  // Move servo to 135 degrees if Beca power turns on
    } else {
      MoveServo(25, 1);  // Move servo to 25 degrees if Beca power turns off
    }

    // Update last known Beca power state
    last_beca_power = beca_power;
  }
  // Control the servo based on RF Remote
  if (rf_signal != last_rf_signal) {
    if (rf_signal) {
      MoveServo(25, 2);                 // Move servo to 25 degrees if RF remote signal is inactive
      writeSingleRegister(1, 0x00, 0);  // Set setpoint to 25.0°C on BECA device with ID 1
    } else {
      MoveServo(135, 2);                // Move servo to 135 degrees if RF remote signal is active
      writeSingleRegister(1, 0x00, 1);  // Set setpoint to 25.0°C on BECA device with ID 1
    }

    // Update last known RF remote signal state
    last_rf_signal = rf_signal;
  }

  // Control the servo based on beca_power status
  if (beca_power) {
    int register_value = read_beca(3) / 10;
    // // Check if the value of register[3] has changed
    if (register_value != last_value) {
      // Display the new value on the TM1637
      show_on_led(0, register_value);  // Display setpoint

      // Update the last_value and reset the timer
      last_value = register_value;
      previousMillis = currentMillis;
    } else if (currentMillis - previousMillis >= interval) {
      // Display the temperature from register[8] after the interval has passed
      temp_by_beca = read_beca(8) / 10;  // 8 parameter is for temperature
      show_on_led(0, temp_by_beca);      // Display temperature in Celsius
    }
  } else {
    show_on_led(0, 0);
  }

  if (digitalRead(23) == HIGH){
    handleDelete();
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  // Wire.begin();
  // // MoveServo(0, 1);
  setup_wifi_credentials();
  RS485Serial.begin(MODBUS_BAUD_RATE, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
  pinMode(RS485_DE_RE_PIN, OUTPUT);
  digitalWrite(RS485_DE_RE_PIN, LOW);  // Set DE/RE pin low for receive mode
  // writeSingleRegister(1, 0x03, 250);   // Set setpoint to 25.0°C on BECA device with ID 1

  pinMode(RF_DATA_PIN, INPUT_PULLUP);  // Enable internal pull-up resistor

  attachInterrupt(digitalPinToInterrupt(RF_DATA_PIN), handleInterrupt, CHANGE);


  // Initialize the display
  display.setBrightness(0x0a);  // set the brightness (0x00 to 0x0f)

  // Test display by showing '8888'
  // uint8_t segs_7[] = { 0x00, 0x80, 0x00, 0x00 };  // Turn on DP on digit 3
  // display.setSegments(segs_7);
  // delay(500);
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  control_with_beca_rf();

  // if (rfDataReceived) {
  //   int rfData = digitalRead(RF_DATA_PIN);

  //   if (rfData == LOW) {
  //     Serial.println("OFF");
  //     MoveServo(30, 1);
  //     ReadPot(34);

  //   } else {
  //     Serial.println("ON");
  //     MoveServo(125, 1);
  //     ReadPot(34);

  //     // Add your code for action when no signal is detected
  //   }

  //   rfDataReceived = false;  // Reset flag
  // }

  // // Other tasks can be performed here without blocking
  // delay(100);  // Adjust delay as necessary





  //  Add Status Flag





  // float temp_in_celsius = readDS18B20Temperature();
  // // #ifndef DEBUG
  // Serial.println(temp_in_celsius);
  // // #endif
  // delay(5000);

  // readDS18B20Temperature();

  // read_ADS1115();

  // ReadPot(34);

  // bool status = RF_Remote();
  // if (status == true) {
  //   Serial.println("TRUE");
  // } else {
  //   Serial.println("FALSE");
  // }
  // MoveServo(135, 2);
  // delay(3000);
  // MoveServo(25, 4);
  // delay(3000);
  // MoveServo(12,7);
  // MoveServo(180,5);
  // MoveServo(0,3);
}