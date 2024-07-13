uint8_t m_pinClk;
uint8_t m_pinDIO;
uint8_t m_brightness;
unsigned int m_bitDelay;
#define SEG_A 0b00000001
#define SEG_B 0b00000010
#define SEG_C 0b00000100
#define SEG_D 0b00001000
#define SEG_E 0b00010000
#define SEG_F 0b00100000
#define SEG_G 0b01000000
#define SEG_DP 0b10000000

#define TM1637_I2C_COMM1 0x40
#define TM1637_I2C_COMM2 0xC0
#define TM1637_I2C_COMM3 0x80

//
//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---
//      D
/*------------------------------------------
// Segment Values in Hexadecimal and Binary:

// a segment:
// Hex: 0x01
// Binary: 0000 0001

// b segment:
// Hex: 0x02
// Binary: 0000 0010

// c segment:
// Hex: 0x04
// Binary: 0000 0100

// d segment:
// Hex: 0x08
// Binary: 0000 1000

// e segment:
// Hex: 0x10
// Binary: 0001 0000

// f segment:
// Hex: 0x20
// Binary: 0010 0000

// g segment:
// Hex: 0x40
// Binary: 0100 0000

// dp segment (decimal point):
// Hex: 0x80
// Binary: 1000 0000
----------------------------------------------*/

const uint8_t digitToSegment[] = {
  // XGFEDCBA
  0b00111111,  // 0
  0b00000110,  // 1
  0b01011011,  // 2
  0b01001111,  // 3
  0b01100110,  // 4
  0b01101101,  // 5
  0b01111101,  // 6
  0b00000111,  // 7
  0b01111111,  // 8
  0b01101111,  // 9
  0b01110111,  // A
  0b01111100,  // b
  0b00111001,  // C
  0b01011110,  // d
  0b01111001,  // E
  0b01110001   // F
};

static const uint8_t minusSegments = 0b01000000;

#define DEFAULT_BIT_DELAY 100

class TM1637Display {

public:

  //! Initialize a TM1637Display object, setting the clock and
  TM1637Display(uint8_t pinClk, uint8_t pinDIO, unsigned int bitDelay = DEFAULT_BIT_DELAY) {
    // Copy the pin numbers
    m_pinClk = pinClk;
    m_pinDIO = pinDIO;
    m_bitDelay = bitDelay;

    // Set the pin direction and default value.
    // Both pins are set as inputs, allowing the pull-up resistors to pull them up
    pinMode(m_pinClk, INPUT);
    pinMode(m_pinDIO, INPUT);
    digitalWrite(m_pinClk, LOW);
    digitalWrite(m_pinDIO, LOW);
  }

  //! Sets the brightness of the display.
  void setBrightness(uint8_t brightness, bool on = true) {
    m_brightness = (brightness & 0x7) | (on ? 0x08 : 0x00);
  }

  //! Display arbitrary data on the module
  void setSegments(const uint8_t segments[], uint8_t length = 4, uint8_t pos = 0) {
    // Write COMM1
    start();
    writeByte(TM1637_I2C_COMM1);
    stop();

    // Write COMM2 + first digit address
    start();
    writeByte(TM1637_I2C_COMM2 + (pos & 0x03));

    // Write the data bytes
    for (uint8_t k = 0; k < length; k++)
      writeByte(segments[k]);

    stop();

    // Write COMM3 + brightness
    start();
    writeByte(TM1637_I2C_COMM3 + (m_brightness & 0x0f));
    stop();
  }

  //! Clear the display
  void clear() {
    uint8_t data[] = { 0, 0, 0, 0 };
    setSegments(data);
  }

  //! Translate a single digit into 7 segment code
  static uint8_t encodeDigit(uint8_t digit) {
    return digitToSegment[digit & 0x0f];
  }

  void bitDelay() {
    delayMicroseconds(m_bitDelay);
  }

  void start() {
    pinMode(m_pinDIO, OUTPUT);
    bitDelay();
  }

  void stop() {
    pinMode(m_pinDIO, OUTPUT);
    bitDelay();
    pinMode(m_pinClk, INPUT);
    bitDelay();
    pinMode(m_pinDIO, INPUT);
    bitDelay();
  }

  bool writeByte(uint8_t b) {
    uint8_t data = b;

    // 8 Data Bits
    for (uint8_t i = 0; i < 8; i++) {
      // CLK low
      pinMode(m_pinClk, OUTPUT);
      bitDelay();

      // Set data bit
      if (data & 0x01)
        pinMode(m_pinDIO, INPUT);
      else
        pinMode(m_pinDIO, OUTPUT);

      bitDelay();

      // CLK high
      pinMode(m_pinClk, INPUT);
      bitDelay();
      data = data >> 1;
    }

    // Wait for acknowledge
    // CLK to zero
    pinMode(m_pinClk, OUTPUT);
    pinMode(m_pinDIO, INPUT);
    bitDelay();

    // CLK to high
    pinMode(m_pinClk, INPUT);
    bitDelay();
    uint8_t ack = digitalRead(m_pinDIO);
    if (ack == 0)
      pinMode(m_pinDIO, OUTPUT);


    bitDelay();
    pinMode(m_pinClk, OUTPUT);
    bitDelay();

    return ack;
  }
};


// Define the pins for TM1637
const int CLK = 19;  // CLK pin to D5 on TM1637
const int DIO = 18;  // DIO pin to D7 on TM1637

// Create a display object
TM1637Display display(CLK, DIO);

void show_on_led(uint8_t mode_1, uint8_t Temp) {
  // uint8_t segs[];
  uint8_t segs_1[] = { 0x00, display.encodeDigit(Temp / 10), display.encodeDigit(Temp % 10), 0x00 };
  if (Temp == 0) {
    segs_1[0] = 0x00;
    segs_1[1] = 0x00;
    segs_1[2] = 0x00;
    segs_1[3] = 0x00;
  } else if (Temp == 1) {
    segs_1[0] = 0x01;
    segs_1[1] = 0x3F;
    segs_1[2] = 0x71;
    segs_1[3] = 0x71;
  }
  switch (mode_1) {
    case 0:
      segs_1[0] = 0x00;
      // segs_1[1] = display.encodeDigit(Temp / 10);
      display.setSegments(segs_1);
      break;
    case 1:
      segs_1[0] = 0x01;
      display.setSegments(segs_1);
      break;
    case 2:
      segs_1[0] = 0x01 | 0x08;
      segs_1[1] = display.encodeDigit(Temp / 10);
      display.setSegments(segs_1);
      break;
    case 3:
      segs_1[0] = 0x01 | 0x08 | 0x40;
      display.setSegments(segs_1);
      break;
    default:
#ifndef DEBUG
      Serial.println("Mode Number 1 is incorrect");
#endif
      break;
  }
  //   switch (mode_2) {
  //     case 1:
  //       segs_1[3] = 0x39;
  //       display.setSegments(segs_1);
  //       break;
  //     case 2:
  //       segs_1[3] = 0x6D;
  //       display.setSegments(segs_1);
  //       break;
  //     default:
  // #ifndef DEBUG
  //       Serial.println("Mode Number 2 is incorrect");
  // #endif
  //       break;
  //   }
  // delay(2000);
}