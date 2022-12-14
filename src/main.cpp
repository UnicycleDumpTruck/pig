#include <Arduino.h>
#include <Adafruit_SleepyDog.h>

// Project Includes
#include "Version.h"
#include <pinout.h>
#include <audio.h>
#include <radio.h>

// Button Header
// Include the Bounce2 library found here :
// https://github.com/thomasfredericks/Bounce2
#include <Bounce2.h>

#define BALL_MAX 100

// INSTANTIATE A Bounce OBJECT
Bounce bounce = Bounce();

volatile uint16_t ball_count = 0;
uint16_t last_count = 0;
uint16_t current_count = 0;

void incrementCount() {
  noInterrupts();
  ball_count++;
  interrupts();
}

void zeroCount() {
  noInterrupts();
  ball_count = 0;
  interrupts();
}

void empty_pig() {
  sendGoEvent(1); // Does not work inside VS1053 audio startPlayingFile!
  delay(100);
  //startAudio();
  delay(1000);
  digitalWrite(RELAY_PIN, HIGH);
  delay(500);
  Watchdog.reset();
  digitalWrite(RELAY_PIN, LOW);
  for (int i=0; i<4; i++) {
      Watchdog.reset();
      delay(1000);
  }
  Serial.print("About to zero count, currently at: ");
  Serial.print(ball_count);
  zeroCount();
}

// ███████╗███████╗████████╗██╗   ██╗██████╗
// ██╔════╝██╔════╝╚══██╔══╝██║   ██║██╔══██╗
// ███████╗█████╗     ██║   ██║   ██║██████╔╝
// ╚════██║██╔══╝     ██║   ██║   ██║██╔═══╝
// ███████║███████╗   ██║   ╚██████╔╝██║
// ╚══════╝╚══════╝   ╚═╝    ╚═════╝ ╚═╝

void setup()
{
  Serial.begin(9600);
  // while (!Serial)
  // {
  //   ; // wait for serial port to connect. Needed for native USB port only
  // }
  Serial.printf("\nProject version v%s, built %s\n", VERSION, BUILD_TIMESTAMP);
  Serial.println("Setup function commencing...");
  //vsAudioSetup();
  delay(100);
  radioSetup();

  // BOUNCE SETUP

  // SELECT ONE OF THE FOLLOWING :
  // 1) IF YOUR INPUT HAS AN INTERNAL PULL-UP
  bounce.attach(BOUNCE_PIN, INPUT_PULLUP); // USE INTERNAL PULL-UP
  // 2) IF YOUR INPUT USES AN EXTERNAL PULL-UP
  //bounce.attach( BOUNCE_PIN, INPUT ); // USE EXTERNAL PULL-UP

  // DEBOUNCE INTERVAL IN MILLISECONDS
  bounce.interval(5); // interval in ms

  // LED SETUP
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // RELAY SETUP
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  
  // Interrupt Setup
  attachInterrupt(TRACK_TRUCK, incrementCount, RISING);
  attachInterrupt(TRACK_BANK_TOP, incrementCount, RISING);
  attachInterrupt(TRACK_ELEVATOR, incrementCount, RISING);
  
  Watchdog.enable(4000);
  Serial.println("Setup Complete");
}

// ██╗      ██████╗  ██████╗ ██████╗
// ██║     ██╔═══██╗██╔═══██╗██╔══██╗
// ██║     ██║   ██║██║   ██║██████╔╝
// ██║     ██║   ██║██║   ██║██╔═══╝
// ███████╗╚██████╔╝╚██████╔╝██║
// ╚══════╝ ╚═════╝  ╚═════╝ ╚═╝

void loop()
{
  if (ball_count != last_count) {
    Serial.println(ball_count);
    last_count = ball_count;
  }
  
  // If empty request came from network, empty pig.
  // if (receiveFromCube()) {
  //   Serial.println("Emptying because of command from LAN.");
  //   empty_pig();
  // }
  
  // If pig is emptying itself due to inner flap trigger,
  // send telemetry and wait for stability.
  int val = analogRead(BALL_FULL);
  if (val < 200) {
    sendGoEvent(9);
    Serial.println("Flap-initiated emptying detected.");
    for (int i=0; i<4; i++) {
      Watchdog.reset();
      delay(1000);
    }
    zeroCount();
  }
  
  // If enough balls have entered from tracks, empty pig.
  noInterrupts();
  current_count = ball_count;
  interrupts();
  if (current_count > BALL_MAX) {
    Serial.println(current_count);
    Serial.println("Emptying because ball count exceeded threshold.");
    empty_pig();
  }

  // Pet the dog.
  Watchdog.reset();
}
