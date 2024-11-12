#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Keypad.h>
#include <MIDI.h>

/*
~~~~~~~~~~~~~~~
HARDWARE PINOUT
~~~~~~~~~~~~~~~
SPI Display (128x160 RGB TFT LCD)
  SCL - D34 (GPIO34)
  SDA - D35 (GPIO35)
  DC - VN (GPIO39)
  CS - VP (GPIO36)

3.5mm TRS MIDI Jack
  Data - D17 (U2-TX0)

Rotary Encoder
  Click - D15 (GPIO15)
  Rotate Clockwise - D2 (GPIO2)
  Rotate Counter-Clockwise - D4 (GPIO4)

Linear Switch 1
  Click - D32 (GPIO32)

Linear Switch 2
  Click - D16 (GPIO16)

Keypad Button Matrix
  Column 1 - D5 (GPIO5)
  Column 2 - D18 (GPIO18)
  Column 3 - D19 (GPIO19)
  Column 4 - D21 (GPIO21)
  Row 1 - D23 (GPIO23)
  Row 2 - D22 (GPIO22)
  Row 3 - TX0 (GPIO1)
  Row 4 - RX0 (GPIO3)
*/

/*
~~~~~~~~~
USED PINS
~~~~~~~~~
DISPLAY
  D4 - RST
  D23 - SDA
  D18 - SCL
  D15 - CS
  D2 - RS
  D32 - LEDK

Left Button
  D35 - Click

Right Button
  D34 - Click

encoder
  D14 - Click
  D13 - Clockwise
  D12 - CounterClockwise
*/

#define MIDI_BAUDRATE 31250  // MIDI communication speed
#define MIDI_TX_PIN 1         // TX0 pin on ESP32

// TFT display instance
TFT_eSPI tft = TFT_eSPI();
// Create and bind the MIDI interface to the default hardware Serial port

// Define keypad button names (musical notes from C to B with sharps)
//const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B", "C", "C#", "D", "D#"};

// Keypad setup
const byte ROWS = 4;  // Four rows
const byte COLS = 4;  // Four columns
char keys[ROWS][COLS] = {
  {'D', 'E', 'F', 'G'},
  {'9', 'A', 'B', 'C'},
  {'5', '6', '7', '8'},
  {'1', '2', '3', '4'}
};
byte rowPins[ROWS] = {16, 17, 19, 21};  // Connect to the row pinouts of the keypad
byte colPins[COLS] = {27, 26, 25, 33};  // Connect to the column pinouts of the keypad

// Create keypad instance
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Helper function to convert keypad key to note index
int keyToNoteIndex(char key) {
  switch (key) {
    case '1': return 60;  // C3
    case '2': return 61;  // C#/Db
    case '3': return 62;  // D
    case '4': return 63;  // D#/Eb
    case '5': return 64;  // E
    case '6': return 65;  // F
    case '7': return 66;  // F#/Gb
    case '8': return 67;  // G
    case '9': return 68;  // G#/Ab
    case 'A': return 69;  // A
    case 'B': return 70; // A#/Bb
    case 'C': return 71; // B
    case 'D': return 72; // C4
    case 'E': return 73; // C#/Db
    case 'F': return 74; // D
    case 'G': return 75; // D#/Eb
    default: return -1;  // Invalid key
  }
}

void sendNoteOn(byte note, byte velocity) {
  Serial1.write(0x90);    // Note On message for channel 1
  Serial1.write(note);    // Note number (e.g., 60 for Middle C)
  Serial1.write(velocity); // Note velocity (e.g., 127 for max velocity)
}

void sendNoteOff(byte note, byte velocity) {
  Serial1.write(0x80);    // Note Off message for channel 1
  Serial1.write(note);    // Note number (e.g., 60 for Middle C)
  Serial1.write(velocity); // Note velocity (usually 0 for Note Off)
}

void setup() {
  // Initialize the TFT display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  Serial1.begin(MIDI_BAUDRATE, SERIAL_8N1, -1, MIDI_TX_PIN);

  // Print initial message
  tft.setCursor(0, 0);
  tft.println("Press a key:");
}

void loop() {
  char key = keypad.getKey();
  static char lastKey = NO_KEY;

  if (key != NO_KEY) {
    KeyState keyState = keypad.getState();
    int note = keyToNoteIndex(key);

    if (note != -1) {
      if (keyState == PRESSED) {
        // Play the note when the key is pressed
        sendNoteOn(note, 127);
        tft.fillScreen(TFT_BLACK);  // Clear screen
        tft.setCursor(0, 0);
        tft.println("MIDI Note:");
        tft.setCursor(0, 60);
        tft.setTextSize(4);
        tft.println(note);
      } else if (keyState == RELEASED) {
        // Stop the note when the key is released
        sendNoteOff(note, 0);
      }
    }
  }

  // Handle key release when no key is pressed anymore
  if (key == NO_KEY && lastKey != NO_KEY) {
    int lastNote = keyToNoteIndex(lastKey);
    if (lastNote != -1) {
      sendNoteOff(lastNote, 0);
    }
  }

  lastKey = key;
}
