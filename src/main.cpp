#include <Arduino.h>
#include <Adafruit_SleepyDog.h>

// Project Includes
#include "Version.h"
#include <pinout.h>
#include <audio.h>
#include <radio.h>

#define BALL_MAX 200
volatile uint16_t ball_count = 0;
uint16_t last_count = 0;
uint16_t current_count = 0;

const char startOfNumberDelimiter = '<';
const char endOfNumberDelimiter   = '>';

void incrementCount() {
  noInterrupts();
  ball_count++;
  interrupts();
}

void zeroCount() {
  noInterrupts();
  uint16_t prev_count = ball_count;
  ball_count = 0;
  interrupts();

  // Zero the pig dot LED display:
  Serial1.print (startOfNumberDelimiter);    
  Serial1.print (0);    // send the number
  Serial1.print (endOfNumberDelimiter);  
  Serial1.println ();
  delay(10);
  // Zero the pig dot LED display:
  Serial1.print (startOfNumberDelimiter);    
  Serial1.print (0);    // send the number
  Serial1.print (endOfNumberDelimiter);  
  Serial1.println ();


  Serial.printf("Count zeroed, was at: %d", prev_count);
}

void attachBallInterrupts() {
;
  attachInterrupt(TRACK_TRUCK, incrementCount, RISING);
  attachInterrupt(TRACK_BANK_TOP, incrementCount, RISING);
  attachInterrupt(TRACK_ELEVATOR, incrementCount, RISING);
}

void detachBallInterrupts() {
;
  detachInterrupt(TRACK_TRUCK);
  detachInterrupt(TRACK_BANK_TOP);
  detachInterrupt(TRACK_ELEVATOR);
}

void displayDots(int num) {
    Serial1.print (startOfNumberDelimiter);    
    Serial1.print (num);    // send the number
    Serial1.print (endOfNumberDelimiter);  
    Serial1.println ();
}

void empty_pig() {
  noInterrupts();
  uint16_t count = ball_count;
  interrupts();
  delay(100);
  detachBallInterrupts();
  startAudio();
  for (int i=0; i<6; i++) {
    displayDots(0);
    delay(230);
    displayDots(18);
    delay(230);
    Watchdog.reset();
  }
  digitalWrite(RELAY_PIN, HIGH);
  //stopAudio();
  delay(2100);
  Watchdog.reset();
  digitalWrite(RELAY_PIN, LOW);
  for (int i=0; i<4; i++) {
      Watchdog.reset();
      delay(1000);
  }
  //attachBallInterrupts();
  //detachBallInterrupts();
  Watchdog.reset();
  // Watchdog.disable();
  zeroCount();
  sendGoEvent(count); // Does not work inside VS1053 audio startPlayingFile!
  attachBallInterrupts();
  // Watchdog.enable();
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

  Watchdog.enable(8000);

  vsAudioSetup();
  Watchdog.reset();
  delay(100);
  radioSetup();
  Watchdog.reset();
  
  // RELAY SETUP
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  
  // Interrupt Setup
  attachBallInterrupts();


  srand (42);

  Serial.println("Starting Serial1");
  Serial1.begin(9600);
  delay(20);
  Serial.println ("Starting send on Serial1...");
  // for (int i = 0; i < 19; i++)
  //   {
  //     Watchdog.reset();
  //     Serial1.print (startOfNumberDelimiter);    
  //     Serial1.print (i);    // send the number
  //     Serial1.print (endOfNumberDelimiter);  
  //     Serial1.println ();
  //     delay(100);
  //   }  // end of for
  //   delay(1000);
    Serial1.print (startOfNumberDelimiter);    
    Serial1.print (0);    // send the number zero to clear the LED display
    Serial1.print (endOfNumberDelimiter);  
    Serial1.println ();


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
    int led_count = map(last_count, 0, 200, 0, 18);
    Serial.print("led_count: ");
    Serial.println(led_count);
    Serial1.print (startOfNumberDelimiter);    
    Serial1.print (led_count);    // send the number
    Serial1.print (endOfNumberDelimiter);  
    Serial1.println ();
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
    Serial.println("Flap-initiated emptying detected.");

    for (int i=0; i<6; i++) {
      Watchdog.reset();
      delay(1000);
    }
    detachBallInterrupts();
    sendGoEvent(last_count);
    attachBallInterrupts();
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
