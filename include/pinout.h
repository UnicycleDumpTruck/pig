#ifndef PINOUT_H
#define PINOUT_H

#include <Arduino.h>

// Radio Pins
#define RFM69_CS 8
#define RFM69_INT 3
#define RFM69_RST 4

// Audio Featherwing Pins
#define SHIELD_RESET -1 // VS1053 reset pin (unused!)
#define SHIELD_CS 6     // VS1053 chip select pin (output)
#define SHIELD_DCS 10   // VS1053 Data/command select pin (output)
#define CARDCS 5        // Card chip select pin
#define DREQ 9          // VS1053 Data request, ideally an Interrupt pin

// Button Pins
#define BOUNCE_PIN A0
#define LED_PIN 13
#define RELAY_PIN 12

#define BALL_FULL A0      // Switch inside pig body triggered by side pressure
#define TRACK_TRUCK A1    // Break-beam sensor on track from armored truck
#define TRACK_BANK_TOP A2 // Break-beam sensor on track from top of bank
#define TRACK_ELEVATOR A3 // Break-beam sensor on track from crank elevators

#endif
