//main.cpp
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Hash.h>
#include <Servo.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

// Eigene Header
//#include "credentials.h" //fuer zuhause, eigentlich überflüssig mit wifi.h
#include "indexHtmlJS.h"
#include "servodata.h"
#include "wifi.h"
#include "defines.h"



// ---- WebServer + WebSocket ----
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");


static unsigned long rolloRollbackAt = 0;

Servo myservo;


ServoData servoData;

struct ServoControl {
  int target = 90;     // Zielwinkel
  int current = 90;    // aktueller Winkel
  unsigned long lastMove = 0; // Zeit des letzten write
  int moveInterval = 10;      // Millisekunden zwischen zwei Bewegungen
} servo; 

//langsamere Servo Bewegung
void updateServo() {
  unsigned long now = millis();
  if (servo.current != servo.target && now - servo.lastMove >= servo.moveInterval) {
      servo.lastMove = now;
      if (servo.target > servo.current) servo.current++;
      else if (servo.target < servo.current) servo.current--;
      myservo.write(servo.current);
  }
}

// Laden der Daten aus EEPROM
void loadServoData() 
{
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(EEPROM_SERVO_ADDR, servoData);
  if (servoData.magic != 0x42) 
  {
      Serial.println("EEPROM ungültig, Standardwerte setzen");
      servoData = {0x42, 45, 90, 135, 1000, 1000, 14};
      EEPROM.put(EEPROM_SERVO_ADDR, servoData);
      EEPROM.commit();
  } 
  else 
  {
      Serial.println("EEPROM-Daten geladen");
  }
}

//Prozessor um ggf. Werte zu setzen (webseite)
String processor(const String& var)
{
  String result = "";
 
  if (var == "COPYRIGHT")
    result = "2025 by Roemke";
  else if (var == "SERVO_LEFT") 
    result = String(servoData.left);
  else if (var == "SERVO_MIDDLE") 
    result = String(servoData.middle);
  else if (var == "SERVO_RIGHT") 
    result = String(servoData.right);
  else if (var == "SERVO_PIN") 
    result = String(servoData.servoPin);
  else if (var == "TIME_DOWN") 
    result = String(servoData.timeDown);
  else if (var == "TIME_UP") 
    result = String(servoData.timeUp); 
  else if (var == "WIFI_MAC_AP")
    result = wifiMacAp;
  else if (var == "WIFI_MAC_STA")
    result = wifiMacSta;
  else if (var == "WIFI_MAC_MODE")
    result = wifiMode;
  
  return result;
}

// ---- WebSocket Event Handler ----
// Nachricht an alle Clients senden
template <typename T>
void informClients(const String& action, T value) 
{
  StaticJsonDocument<256> doc;
  doc["action"] = action;
  doc["value"] = value;   // JsonVariant nimmt String oder int

  String msg;
  serializeJson(doc, msg);
  ws.textAll(msg);
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
  AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  Serial.printf("ws-event, Heap: %d\n", ESP.getFreeHeap());
  if(type != WS_EVT_DATA) 
    return;

  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if(!info->final || info->index != 0 || info->len != len || info->opcode != WS_TEXT) return;

  data[len] = 0; // Nullterminator
  String msg = (char*)data;

  // StaticJsonDocument mit ausreichendem Puffer
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, msg);
  if(error) 
  {
    Serial.print("JSON Fehler: ");
    Serial.println(error.c_str());
    return;
  }

  String action = doc["action"];
  JsonVariant value = doc["value"];

  if (action == "servoLeft")      
    servoData.left   = value.as<int>();
  else if(action == "servoMiddle") 
    servoData.middle = value.as<int>();
  else if(action == "servoRight")
    servoData.right  = value.as<int>();
  else if(action == "servoPin")
    servoData.servoPin = value.as<int>();
  else if(action == "timeDown")
    servoData.timeDown = value.as<int>();
  else if(action == "timeUp")
    servoData.timeUp = value.as<int>();
  else if (action == "rollo") //buttons up/down
  {
      if (value.as<String>() == "up")
      {
          servo.target = servoData.right;
          informClients("rolloState", "up");
          rolloRollbackAt = millis() + servoData.timeUp;
      }
      else if (value.as<String>() == "down")
      {
          servo.target = servoData.left;
          informClients("rolloState", "down");
          rolloRollbackAt = millis() + servoData.timeDown;
      }
      else if (value.as<String>() == "stop" || value.as<String>() == "middle")
      {
          servo.target = servoData.middle;
          informClients("rolloState", "middle");
          rolloRollbackAt = millis() + servoData.timeDown;
      }
  }
  else if (action == "wifiSetCredentials")
  {
      String ssid = value["ssid"] | "";
      String pass = value["password"] | "";
      Serial.printf("Neue WiFi Daten: SSID=%s, PASS=%s\n", ssid.c_str(), pass.c_str());
      if (ssid.length() > 0)
      {
          wifiSetCredentials(ssid.c_str(), pass.c_str());
          informClients("wifiState", "saved");
          ESP.restart();  // Neustart mit neuen Daten
      }
  }
      

  if (action == "servoLeft" || action == "servoMiddle" || action == "servoRight") 
    servo.target = value.as<int>();
  // Hier EEPROM schreiben
  EEPROM.put(EEPROM_SERVO_ADDR, servoData);
  EEPROM.commit();
  informClients(action, value);
}



void setup() {
  Serial.begin(115200);
  loadServoData(); 
  wifiSetup();

  
  // WebSocket einbinden
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  //lustig, ich bin alt,  [] leitet einen Lambda Ausdruck ein, also eine anonyme Funktion
  //die gabs frueher nicht :-), dafür mehr Lametta
  server.on("/favicon.ico", [](AsyncWebServerRequest *request)
  {
    request->send(204);//no content
  });

  // Hauptseite
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html,processor);
  });

  server.begin();
  Serial.println("HTTP-Server gestartet");
  myservo.attach(servoData.servoPin, 500, 2500); // Pin, min, max
  Serial.println("Servo initialisiert"); 
}

void loop() 
{
  
  updateServo();
  ws.cleanupClients();
  if (rolloRollbackAt > 0 && millis() >= rolloRollbackAt)
  {
      rolloRollbackAt = 0;
      servo.target = servoData.middle;
      informClients("rolloState", "middle");
  }   
}