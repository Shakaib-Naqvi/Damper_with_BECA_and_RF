#include <Preferences.h>
// Define the pin connected to the potentiometer
// const int potPin = 34;  // Define the pin connected to the potentiometer (ADC1_CH6)


float ReadPot(int potPin) {
  // Read the value from the potentiometer (analog input)
  int minValue = 1846;  // Initialize minValue to the highest possible mapped value
  int maxValue = 0;     // Initialize maxValue to the lowest possible mapped value
  Preferences preferences;
  // Print the initial min and max values
  preferences.begin("potvalues", false);
  minValue = preferences.getInt("minvalue", 0);
  maxValue = preferences.getInt("maxvalue", 0);
#ifndef DEBUG
  Serial.print("Initial Min Value: ");
  Serial.println(minValue);
  Serial.print("Initial Max Value: ");
  Serial.println(maxValue);
#endif
  float potValue = analogRead(potPin);
  // Map the potValue to a range of 0 - 1846
  int mappedValue = map(potValue, 0, 1846, 0, 100);

  // Update the min and max values
  if (mappedValue < minValue) {
    minValue = mappedValue;
    preferences.putInt("minvalue", minValue);
  }
  if (mappedValue > maxValue) {
    maxValue = mappedValue;
    preferences.putInt("maxvalue", maxValue);
  }

  // Print the potentiometer value, min value, and max value to the Serial Monitor
#ifndef DEBUG
  Serial.print("Potentiometer Value: ");
  Serial.print(potValue);
  Serial.print(" | Mapped Value: ");
  Serial.print(mappedValue);
  Serial.print(" | Min Value: ");
  Serial.print(minValue);
  Serial.print(" | Max Value: ");
  Serial.println(maxValue);
#endif

  return mappedValue;
}
