#include <Arduino.h>
#include <Adafruit_SleepyDog.h>

// Project Includes
#include "Version.h"
#include <pinout.h>
#include <audio.h>
#include <radio.h>

// Create second Serial device on M0 sercom
// Uart Serial2 (&sercom1, RS485_RX, RS485_TX, SERCOM_RX_PAD_0, UART_TX_PAD_2);

// void SERCOM1_Handler()
// {
//   Serial2.IrqHandler();
// }

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

void empty_pig() {
  noInterrupts();
  uint16_t count = ball_count;
  interrupts();
  // Watchdog.disable();
  detachBallInterrupts();
  sendGoEvent(count); // Does not work inside VS1053 audio startPlayingFile!
  attachBallInterrupts();
  // Watchdog.enable();
  delay(100);
  //startAudio();
  //delay(1000);
  digitalWrite(RELAY_PIN, HIGH);
  delay(500);
  Watchdog.reset();
  digitalWrite(RELAY_PIN, LOW);
  for (int i=0; i<4; i++) {
      Watchdog.reset();
      delay(1000);
  }
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

  // // LED SETUP
  // pinMode(LED_PIN, OUTPUT);
  // digitalWrite(LED_PIN, LOW);
  
  // RELAY SETUP
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  
  // Interrupt Setup
  attachBallInterrupts();

  Watchdog.enable(4000);


  srand (42);


  Serial.println("Starting Serial1");
  Serial1.begin(9600);
  // pinPeripheral(RS485_RX, PIO_SERCOM);
  // pinPeripheral(RS485_TX, PIO_SERCOM);
  delay(20);
  Serial1.println("test");

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
  
  Serial.println ("Starting numbers send on Serial1...");
  for (int i = 0; i < 10; i++)
    {
    Serial1.print (startOfNumberDelimiter);    
    Serial1.print (rand ());    // send the number
    Serial1.print (endOfNumberDelimiter);  
    Serial1.println ();
    }  // end of for

  delay (3000);

  
  
  
  // Serial1.println("t");
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
