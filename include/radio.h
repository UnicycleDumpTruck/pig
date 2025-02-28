#ifndef RADIO_H
#define RADIO_H

#include <Arduino.h>

#include <pinout.h>
#include <RH_RF69.h>
#include <RHReliableDatagram.h>

#define MY_ADDRESS 62 // Moneypalooza Pig Serial test
#define RF69_FREQ 915.0

void radioSetup();
void sendGoEvent(uint8_t s);
bool receiveFromCube();
void ftoa(float f, char *str, uint8_t precision);

#endif
