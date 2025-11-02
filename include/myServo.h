#ifndef MYSERVO_H
#define MYSERVO_H
#pragma once
#include <Arduino.h>
#include <Servo.h>

//servodata.h
struct ServoData {
    int left = 45;
    int middle =90 ;
    int right = 135;
    int stopActive=135;
    int stopInactive=90;
    int timePress = 1000; //Zeit Taste gedrückt halten (ms)    
    unsigned long moveInterval = 10; // Zeit Pause zwischen Servobewegungen (ms), long nicht nötg, aber sonst warning
    int angleMoveStep = 2; //Winkel Schrittweite pro Bewegung
    int servoPinUpDown = 14; // D5
    int servoPinStop = 12; // D6
  };
  struct ServoControl {
    int target = 90;     // Zielwinkel
    int current = 90;    // aktueller Winkel
    unsigned long lastMove = 0; // Zeit des letzten write
  };
  extern ServoData servoData;
  extern ServoControl servoUpDown,servoStop;
  extern Servo myServoUpDown;
  extern Servo myServoStop;
  extern bool servoDataChanged;

  // Function declarations extracted from the file
  void updateServo(unsigned long &releaseButtonAt, unsigned long &releaseStopButtonAt);
  void checkServoDataChanged();
  void saveServoData();
  void loadServoData();
  void initializeServos();
  

#endif
