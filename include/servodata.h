#pragma once
#include <Arduino.h>
//servodata.h
struct ServoData {
    uint8_t magic = 0x42;  // immer 0x42, um EEPROM-Daten zu validieren
    int left = 45;
    int middle =90 ;
    int right = 135;
    int timeDown = 1000; //Zeit Rollo runter
    int timeUp = 1000;   //Zeit Rollo hoch
    int servoPin = 14; // D5
  };

  extern ServoData servoData;
