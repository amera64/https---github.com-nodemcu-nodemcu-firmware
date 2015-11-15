// Based on the 'roboface' example sketch for Adafruit I2C 8x8 LED 
// backpacks:
//
//  www.adafruit.com/products/870   www.adafruit.com/products/1049
//  www.adafruit.com/products/871   www.adafruit.com/products/1050
//  www.adafruit.com/products/872   www.adafruit.com/products/1051
//  www.adafruit.com/products/959   www.adafruit.com/products/1052
//
// Requires Adafruit_LEDBackpack and Adafruit_GFX libraries.
// For a simpler introduction, see the 'matrix8x8' example.
//
// This sketch demonstrates a couple of useful techniques:
// 1) Using a time-slice to animate eyes rather than using delay.
// 2) Displaying the same data on multiple matrices by sharing the
//    same I2C address.
//
// This example uses 2 matrices on the same address (two share an address)
// to animate a pair of eyes:
//
//     0     0
//
//      1 2 3 <--- mouth not included in this example
//
// The 'eyes' both display the same image (always looking the same
// direction -- can't go cross-eyed) and thus share the same address
// (0x70).
//
// Adafruit invests time and resources providing this open source code,
// please support Adafruit and open-source hardware by purchasing
// products from Adafruit!
//
// Original example written by P. Burgess for Adafruit Industries, 
// modified by Rick Winscot.
//
// BSD license, all text above must be included in any redistribution.
#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"


byte LED = 13;           // debug LED on the Arduino

unsigned long tme = 0;   // the last time we processed doBlink()
unsigned long slc = 20;  // milliseconds between animation calls
unsigned long ms = 0;    // a millis() time-slice

Adafruit_8x8matrix matrix = Adafruit_8x8matrix();

static uint8_t PROGMEM // Bitmaps are stored in program memory
const  blinkImg[][8] = {    // Eye animation frames
  { B00111100,         // Fully open eye
    B01111110,
    B11111111,
    B11111111,
    B11111111,
    B11111111,
    B01111110,
    B00111100 },
  { B00000000,
    B01111110,
    B11111111,
    B11111111,
    B11111111,
    B11111111,
    B01111110,
    B00111100 },
  { B00000000,
    B00000000,
    B00111100,
    B11111111,
    B11111111,
    B11111111,
    B00111100,
    B00000000 },
  { B00000000,
    B00000000,
    B00000000,
    B00111100,
    B11111111,
    B01111110,
    B00011000,
    B00000000 },
  { B00000000,         // Fully closed eye
    B00000000,
    B00000000,
    B00000000,
    B10000001,
    B01111110,
    B00000000,
    B00000000 } };
    
uint8_t
  blinkIndex[] = { 1, 2, 3, 4, 3, 2, 1 }, // Blink bitmap sequence
  blinkCountdown = 100, // Countdown to next blink (in frames)
  gazeCountdown  =  75, // Countdown to next eye movement
  gazeFrames     =  50, // Duration of eye movement (smaller = faster)
  mouthPos       =   0, // Current image number for mouth
  mouthCountdown =  10; // Countdown to next mouth change
int8_t
  eyeX = 3, eyeY = 3,   // Current eye position
  newX = 3, newY = 3,   // Next eye position
  dX   = 0, dY   = 0;   // Distance from prior to new position

void setup() {
  pinMode( LED, OUTPUT );// set Arduino LED as output - for debug blink
  
  randomSeed(analogRead(A0));
  matrix.begin(0x70);
}

void loop() {
  ms = millis();
  digitalWrite(LED, LOW); // slices less than 100ms are difficult to see
  
  if ( tme + slc < ms ) {
    tme = ms;
    digitalWrite(LED, HIGH); // set slc > 100ms to debug
    
    matrix.clear();    
    matrix.drawBitmap(0, 0,
    blinkImg[
      (blinkCountdown < sizeof(blinkIndex)) ? // Currently blinking?
      blinkIndex[blinkCountdown] :            // Yes, look up bitmap #
      0                                       // No, show bitmap 0
    ], 8, 8, LED_ON);
    
    // Decrement blink counter.  At end, set random time for next blink.
    if(--blinkCountdown == 0) blinkCountdown = random(5, 180);
  
    // Add a pupil (2x2 black square) atop the blinky eyeball bitmap.
    // Periodically, the pupil moves to a new position...
    if(--gazeCountdown <= gazeFrames) {
      // Eyes are in motion - draw pupil at interim position
      matrix.fillRect(
        newX - (dX * gazeCountdown / gazeFrames),
        newY - (dY * gazeCountdown / gazeFrames),
        2, 2, LED_OFF);
      if(gazeCountdown == 0) {    // Last frame?
        eyeX = newX; eyeY = newY; // Yes.  What's new is old, then...
        do { // Pick random positions until one is within the eye circle
          newX = random(7); newY = random(7);
          dX   = newX - 3;  dY   = newY - 3;
        } while((dX * dX + dY * dY) >= 10);      // Thank you Pythagoras
        dX            = newX - eyeX;             // Horizontal distance to move
        dY            = newY - eyeY;             // Vertical distance to move
        gazeFrames    = random(3, 15);           // Duration of eye movement
        gazeCountdown = random(gazeFrames, 120); // Count to end of next movement
      }
    } else {
      // Not in motion yet -- draw pupil at current static position
      matrix.fillRect(eyeX, eyeY, 2, 2, LED_OFF);
    }
  
    matrix.writeDisplay();
  }  
}
