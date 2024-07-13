// Define the GPIO pin connected to the RF switch control input
const int RF_Switch_Pin = 26;  // GPIO 2 on ESP32

bool RF_Remote(){
  bool status = digitalRead(RF_Switch_Pin);
  return status;
}