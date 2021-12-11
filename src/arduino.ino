/*
arduino.ino - Part of xmas_lights

Author: Derek Rowland (gx1400@gmail.com)
Initial creation: Dec 4th, 2021

Changelog:
- 2021-12-04: Initial creation

This file is part of xmas_lights.

xmas_lights is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

xmas_lights is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with xmas_lights.  If not, see <https://www.gnu.org/licenses/
*/

// includes
#include "Arduino.h"
#include "FastLED.h"

// LED Strip
#define NUM_LEDS 50
#define ledStrip1Pin 7
#define ledType WS2811
#define ledBrightness 200
#define ledSaturation 255



// constants
#define led1Pin 8
#define led2Pin 9
#define interruptPin 2

//globals
bool StateLed = false;              // state tracking for LED flip flop
bool stringComplete = false;        // string has been received on Serial line
volatile bool ModeEnabled = true;   // state tracking for enable mode

volatile unsigned long last_interrupt_time = 0; // interrupt debounce

String inputString = "";            // string to hold incoming data

int indexLED = 0;
CRGB color = CRGB::Green;

CRGB leds[NUM_LEDS];

/*
Setup and initialization routine
*/
void setup() {
  // initialize output pins
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);

  FastLED.addLeds<ledType, ledStrip1Pin>(leds, NUM_LEDS);

  attachInterrupt(digitalPinToInterrupt(interruptPin), isr, CHANGE);

  // initialize serial:
  Serial.begin(115200);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);

  Serial.println("Starting...");
  isr();
  ModeEnable();
}

/*
Main loop
*/
void loop() {

  // print the string when a newline arrives:
  if (stringComplete) { SerialInputHandler(); }

  // If mode is disabled, then exit loop
  if (!ModeEnabled) { return; }

  // LED flip flop
  //ledFlipFlop();
  //ledStripTest();
  ledStripRainbow();
}

/*
LED Strip Control Test
*/
void ledStripTest() {
  
  leds[indexLED] = color;
  FastLED.show();
  delay(1000);
  indexLED++;

  if(indexLED == NUM_LEDS) {
    indexLED = 0;
    CRGB c1 = CRGB::Green;
    CRGB c2 = CRGB::Black;

    if(color == c1) {
      color = c2;
    } else {
      color = c1;
    }
  }
}

/*
Rainbow Test
*/
void ledStripRainbow() {
  for (int j = 0; j < 255; j++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(i - (j * 2), ledSaturation, ledBrightness); /* The higher the value 4 the less fade there is and vice versa */ 
    }
    FastLED.show();
    delay(25); /* Change this to your hearts desire, the lower the value the faster your colors move (and vice versa) */
  }
}

/*
LED Flip Flop routing
*/
void ledFlipFlop() {
  if (StateLed) {
    digitalWrite(led1Pin, HIGH);
    digitalWrite(led2Pin, LOW);
  }
  else {
    digitalWrite(led1Pin, LOW);
    digitalWrite(led2Pin, HIGH);
  }

  delay(500);
  StateLed = !StateLed;
}

/*
Interrupt handler
*/
void isr() {

  // ISR debounce
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time < 50)  { return; }
  last_interrupt_time = interrupt_time;

  // check state of interrupt pin to determine if mode should be
  // enabled or disabled
  if (digitalRead(interruptPin)) { ModeEnable(); }
  else { ModeDisable(); }
}

/*
Assert Mode to enabled
*/
void ModeEnable() {
  ModeEnabled = true;
  Serial.println("Enabling mode.");
  digitalWrite(led1Pin, HIGH);
}

/*
Deassert Mode to disabled
*/
void ModeDisable() {
  ModeEnabled = false;
  Serial.println("Disabling mode.");
  digitalWrite(led1Pin, LOW);

  FastLED.clear();  // clear all pixel data
  FastLED.show();
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}

/*
Handler function for serial input strings
*/ 
void SerialInputHandler() {
    
    String strToParse = inputString; 

    // clear the string:
    inputString = ""; 
    stringComplete = false;

    Serial.println("ECHO:" + strToParse); // echo input

    // clean up input
    strToParse.toUpperCase();
    strToParse.trim();
    strToParse.replace(" ","");

    // command parsing
    String strCmd = "";
    String strParam = "";
    int indexEqual = strToParse.indexOf('=');  // find = character, 

    
    if( indexEqual > 0) { 
      Serial.println("Valid Command!"); 

      strCmd = strToParse.substring(0, indexEqual);
      strParam = strToParse.substring(indexEqual + 1);

      SerialCommand(strCmd, strParam);
    }
    else { 
      InvalidCommand(strToParse);
      return;
    }

    
    
}

/*
Serial Command processing
*/
void SerialCommand(String cmd, String param) {
  if (cmd == "ENABLE") {
    ParseEnable(param);
  }
  else if (cmd == "MODE") {
    ParseMode(param);
  }
  else {
    InvalidCommand(cmd);
  }
}




/*
helper function for invalid commands
*/
void InvalidCommand(String cmd) {
  Serial.println("Invalid command: " + cmd);
}

/*
helper function for invalid params
*/
void InvalidParam(String cmd, String param) {
  Serial.println("Invalid param: " + param + ", on command: " + cmd);
}

/*
Parse Enable commands
*/
void ParseEnable(String param) {
  Serial.println("Valid enable command: " + param);

  if (param == "ON") {
    ModeEnable();
  } else if (param == "OFF") {
    ModeDisable();
  } else {
    InvalidParam("ENABLE", param);
  }
}

/*
Parse Mode commands
*/
void ParseMode(String param) {
  Serial.println("Valid mode command: " + param);
}



