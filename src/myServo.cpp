/*
 * File: servo.cpp
 * Description: Implementation of servo control for the Esp8266FingerBotWippe project.
 * Author: [Your Name]
 * Date: [Date]
 */

#include <Arduino.h>

#include <Servo.h>
#include "myServo.h"
#include "logging.h"
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

Servo myServoUpDown;
Servo myServoStop;
ServoData servoData;
ServoControl servoUpDown,servoStop;


//langsamere Servo Bewegung - hatte Probleme mit der Steuerung, kann aber sein, dass dies vor der 
//Nutzung der Servo-Klasse war. Ich weiß nicht, ob das noch sinnvoll ist, nimmt man es heraus, s. u. mit 
//dem direkten Setzen des current-Wertes, dann bewegt er sich schon sehr ruckartig.
//mache die Zeit mal konfigurierbar und behalte das so bei. ChatGPT findet ein paar punkte dafür, ein paar dagegen, 
//ich mache die Zeit einfach konfgurierbar. 
void updateServo(unsigned long &releaseButtonAt, unsigned long &releaseStopButtonAt) {
  unsigned long now = millis();  
  
  if (servoUpDown.current != servoUpDown.target && now - servoUpDown.lastMove >= servoData.moveInterval) {
      servoUpDown.lastMove = now;
      if (servoUpDown.target > servoUpDown.current) servoUpDown.current++;
      else if (servoUpDown.target < servoUpDown.current) servoUpDown.current--;      
      //servoUpDown.current=servoUpDown.target;//test, führt zum Absturz, hmm einmal, gibt eigentlich keinen Grund
      //logPrintf("write UpDown: %d -> %d\n", servoUpDown.current, servoUpDown.target);      
      myServoUpDown.write(servoUpDown.current);
      if (servoUpDown.current == servoUpDown.target)
        releaseButtonAt = millis() + servoData.timePress;
  
  }
  if (servoStop.current != servoStop.target && now - servoStop.lastMove >= servoData.moveInterval) {
      servoStop.lastMove = now;
      if (servoStop.target > servoStop.current) servoStop.current++;
      else if (servoStop.target < servoStop.current) servoStop.current--;      
      //logPrintf("Write Stop: %d -> %d\n", servoStop.current, servoStop.target);
      myServoStop.write(servoStop.current);      
      if (servoStop.current == servoStop.target)
        releaseStopButtonAt = millis() + servoData.timePress;
  }   
}


//Daten aus LittleFS laden
//speichern, jetzt aber nicht im EEPROM sondern in LittleFS
bool servoDataChanged = false;

void checkServoDataChanged() {
    static ServoData lastSaved = servoData; // Initialisierung beim Start

 // Speicherbereiche vergleichen (ganzer struct)
    if (memcmp(&servoData, &lastSaved, sizeof(ServoData)) != 0) 
    {
      memcpy(&lastSaved, &servoData, sizeof(ServoData)); // neuen Stand merken
      servoDataChanged = true;
    }
}
void saveServoData() {
  File f = LittleFS.open("/servodata.json", "w");
  if (!f) {
    logPrintln("❌ Fehler beim Öffnen von servodata.json zum Schreiben");
    return;
  }

  JsonDocument doc;
  doc["left"]         = servoData.left;
  doc["middle"]       = servoData.middle;
  doc["right"]        = servoData.right;
  doc["stopActive"]   = servoData.stopActive;
  doc["stopInactive"] = servoData.stopInactive;
  doc["timePress"]    = servoData.timePress;
  doc["moveInterval"]    = servoData.moveInterval;
  doc["servoPinUpDown"] = servoData.servoPinUpDown;
  doc["servoPinStop"]   = servoData.servoPinStop;

  if (serializeJson(doc, f) == 0) {
    logPrintln("❌ Fehler beim Serialisieren nach servodata.json");
  } else {
    logPrintln("✅ ServoData gespeichert");
  }
  f.close();
}


void loadServoData() {
  servoData = ServoData(); // Defaults setzen, falls was schiefgeht


  //littleFS mounten muss schon geschenen sein, mache es einmal in Setup
  if (!LittleFS.exists("/servodata.json")) {
    logPrintln("⚠️ Keine servodata.json gefunden – Standardwerte verwenden");
    saveServoData();
    return;
  }

  File f = LittleFS.open("/servodata.json", "r");
  if (!f) {
    logPrintln("❌ Fehler beim Öffnen von servodata.json zum Lesen");
    return;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();

  if (err) {
    logPrintf("❌ JSON Fehler: %s\n", err.c_str());
    return;
  }

  // Werte übernehmen (mit Defaults)
  servoData.left           = doc["left"]           | servoData.left;
  servoData.middle         = doc["middle"]         | servoData.middle;
  servoData.right          = doc["right"]          | servoData.right;
  servoData.stopActive     = doc["stopActive"]     | servoData.stopActive;
  servoData.stopInactive   = doc["stopInactive"]   | servoData.stopInactive;
  servoData.timePress      = doc["timePress"]      | servoData.timePress;
  servoData.moveInterval   = doc["moveInterval"]   | servoData.moveInterval;
  servoData.servoPinUpDown = doc["servoPinUpDown"] | servoData.servoPinUpDown;
  servoData.servoPinStop   = doc["servoPinStop"]   | servoData.servoPinStop;

  logPrintln("✅ ServoData geladen");
}

void initializeServos() {
  myServoUpDown.attach(servoData.servoPinUpDown, 500, 2500);
  myServoStop.attach(servoData.servoPinStop, 500, 2500);

  servoUpDown.target = servoUpDown.current = servoData.middle;
  servoStop.target   = servoStop.current   = servoData.stopInactive;

  myServoUpDown.write(servoUpDown.current);
  myServoStop.write(servoStop.current);

  logPrintln("✅ Servos initialisiert");
}


// Add your servo control declarations here


