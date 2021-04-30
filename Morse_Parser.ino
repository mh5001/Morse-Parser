#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#include "morse.h"

#define BUTTON_PIN 2
#define LED_PIN 3
#define FLUKE_THRESHOLD 15
#define SHORT_THRESHOLD 250

#define WORD_THRESHOLD 1200
#define SPACE_THRESHOLD 2400

unsigned MORSE_DATA_COUNT;

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(1000);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  MORSE_DATA_COUNT = sizeof(MORSE_DATA) / (sizeof(char) * 6);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  delay(1000);
  display.display();
}

unsigned long prevTime = 0;
unsigned long releaseTime;
bool isDown = false;
bool isIdle = false;
bool canPush = false;
bool isAwaitSpace = false;

void togglePinDown() {
  if (isDown) return;
  prevTime = millis();
  isDown = true;
}

unsigned long togglePinUp() {
  if (!isDown) return 0;
  isDown = false;
  return millis() - prevTime;
}

void blinkLed() {
  display.display();
  digitalWrite(LED_PIN, HIGH);
  delay(10);
  digitalWrite(LED_PIN, LOW);
}

char morseData[6];
short morseLocation = 0;
unsigned long wordEndTime;

void loop() {
  int data = !digitalRead(BUTTON_PIN);
  
  if (data) {
    togglePinDown();
    isAwaitSpace = false;
    wordEndTime = millis();
  } else {
    unsigned long timeHold = togglePinUp();
    if (timeHold) {
      if (timeHold > FLUKE_THRESHOLD) {
        isIdle = true;
        releaseTime = millis();
        if (timeHold <= SHORT_THRESHOLD) {
          morseData[morseLocation++] = '.';
        } else {
          morseData[morseLocation++] = '-';
        }
        morseData[morseLocation] = '\0';
        if (morseLocation == 5) canPush = true;
      }
    }
  }

  if (isIdle) {
    if ((millis() - releaseTime >= WORD_THRESHOLD) || canPush) {
      isIdle = false;
      canPush = false;

      char out = 'A';
      bool hasOut = false;
      for (int i = 0; i < MORSE_DATA_COUNT; i++) {
        if (!strcmp(morseData, MORSE_DATA[i])) {
          out += i;
          hasOut = true;
          break;
        }
      }
      if (!hasOut) {
        display.print("?");
        blinkLed();
        morseLocation = 0;
        return;
      }
      display.print(out);
      morseLocation = 0;
      wordEndTime = millis();
      isAwaitSpace = true;
      
      blinkLed();
    }
  }

  if (isAwaitSpace) {
    if (millis() - wordEndTime >= SPACE_THRESHOLD) {
      isAwaitSpace = false;
      display.print("_");
      
      blinkLed();
    }
  }
}
