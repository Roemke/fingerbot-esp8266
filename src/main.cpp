//main.cpp
#include <Arduino.h>
#include <Hash.h>
#include <ArduinoJson.h>
#include <ElegantOTA.h>

//#include <EEPROM.h> umgestellt auf littlefs
// Eigene Header
//#include "credentials.h" //fuer zuhause, eigentlich überflüssig mit wifi.h
#include "indexHtmlJS.h"
#include "myServo.h"
#include "wifi.h"
#include "main.h"
#include "logging.h"
#include "mqtt.h"

// ---- WebServer + WebSocket ----
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");


static unsigned long releaseButtonAt = 0;
static unsigned long releaseStopButtonAt = 0;

//fuer das speichern 
unsigned long lastSave = 0;
const unsigned long SAVE_INTERVAL = 2000; // 2 Sekunden
enum ButtonState { UP, DOWN, STOP, NONE }; //up ist rein, down ist raus
ButtonState buttonState = NONE;
enum LastAction {RAUS,REIN,STOPPEN,KEINE};
LastAction lastAction = KEINE;

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
  else if (var == "SERVO_STOP_ACTIVE") 
    result = String(servoData.stopActive);
  else if (var == "SERVO_STOP_INACTIVE") 
    result = String(servoData.stopInactive);
  else if (var == "TIME_PRESS") 
    result = String(servoData.timePress);
  else if (var == "SERVO_PIN_UPDOWN") 
    result = String(servoData.servoPinUpDown);
  else if (var == "SERVO_PIN_STOP") 
    result = String(servoData.servoPinStop); 
  else if (var == "WIFI_MAC_AP")
    result = wifiMacAp;
  else if (var == "WIFI_MAC_STA")
    result = wifiMacSta;
  else if (var == "WIFI_MAC_MODE")
    result = wifiMode;
  else if (var == "MQTT_BROKER")
    result = mqtt.broker;
  else if (var == "MQTT_PORT")
    result = String(mqtt.port);
  
  return result;
}



// ---- WebSocket Event Handler ----
// Nachricht an alle Clients senden
template <typename T>
void informClients(const String& action, T value) 
{
  JsonDocument doc;
  doc["action"] = action;
  doc["value"] = value;   // JsonVariant nimmt String oder int

  String msg;
  serializeJson(doc, msg);
  ws.textAll(msg);
}


void initialInformClient(AsyncWebSocketClient *client)
{  
  JsonDocument doc;
  doc["action"] = "init";
  doc["servoLeft"] = servoData.left;
  doc["servoMiddle"] = servoData.middle;
  doc["servoRight"] = servoData.right;
  doc["servoStopActive"] = servoData.stopActive;
  doc["servoStopInactive"] = servoData.stopInactive;
  doc["timePress"] = servoData.timePress;
  doc["servoPinUpDown"] = servoData.servoPinUpDown;
  doc["servoPinStop"] = servoData.servoPinStop;
  doc["buttonState"] = (buttonState == UP) ? "up" :
                        (buttonState == DOWN) ? "down" :
                        (buttonState == STOP) ? "stop" : "none";

  // Log-Array hinzufügen
  JsonArray logArr = doc["logs"].to<JsonArray>();
  for (uint8_t i = 0; i < logCount; i++) {
    uint8_t idx = (logIndex + LOG_BUFFER_SIZE - logCount + i) % LOG_BUFFER_SIZE;
    logArr.add(logBuffer[idx]);
  }
  String msg;
  serializeJson(doc, msg);
  client->text(msg);
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
  AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  logPrintf("ws-event, Heap: %d\n", ESP.getFreeHeap());
  if (type == WS_EVT_CONNECT) 
  {
    logPrintf("📡 Client #%u verbunden\n", client->id());
    // Aktuellen Status senden
    initialInformClient(client);
  }
  else if(type != WS_EVT_DATA) 
    return;

  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if(!info->final || info->index != 0 || info->len != len || info->opcode != WS_TEXT) return;

  
  

  // StaticJsonDocument mit ausreichendem Puffer, deprecated sollte reichen ein JsonDocument zu nehmen
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, data,len);
  if(error) 
  {
    Serial.print("JSON Fehler: ");
    logPrintln(error.c_str());
    return;
  }

  String action = doc["action"] | "";
  JsonVariant value = doc["value"];
  logPrintf("Aktion: %s, Wert: %s\n", action.c_str(), value.as<String>().c_str());
   
  if (action == "button") // buttons up/down/stop
  {
      String btn = value.as<String>();
      if(btn == "stop")
      {
          // Zuerst Up/Down-Servo auf Mittel
          servoUpDown.target = servoData.middle;
          releaseButtonAt = 0; // keine automatische Rücksetzung, er ist in der Mitte, drückt also nicht
          // Dann Stop-Servo aktivieren
          servoStop.target = servoData.stopActive;
          //releaseStopButtonAt = millis() + servoData.timePress; nein, dann spielt der Servo-Weg eine Rolle
          buttonState = STOP;
          lastAction = STOPPEN;
      }
      else // up oder down
      {
          // Stop-Servo vorher deaktivieren
          servoStop.target = servoData.stopInactive;
          releaseStopButtonAt = 0;
          servoUpDown.target = (btn == "up") ? servoData.right : servoData.left;
          buttonState = (btn == "up") ? UP : DOWN;
          lastAction = (btn == "up") ? REIN : RAUS;          
          //releaseButtonAt = millis() + servoData.timePress; - nein, das ist dann vom Weg des Servos abhängig
      }

      String lastActionString = (lastAction == RAUS) ? "raus" :
                                  (lastAction == REIN) ? "rein" :
                                  (lastAction == STOPPEN) ? "stop" : "keine";
      mqtt.publishLastCmd(lastActionString); // MQTT informieren
      informClients(action, value);

  }        
  else if (action == "wifiSetCredentials")
  {
      String ssid = value["ssid"] | "";
      String pass = value["password"] | "";
      logPrintf("Neue WiFi Daten: SSID=%s, PASS=%s\n", ssid.c_str(), pass.c_str());
      if (ssid.length() > 0)
      {
          wifiSetCredentials(ssid.c_str(), pass.c_str());
          informClients("wifiState", "saved");
          ESP.restart();  // Neustart mit neuen Daten
      }
  }
  else if (action == "mqttSet")
  {
      String broker = value["broker"] | "";
      int port = value["port"] | 1883;
      logPrintf("Neue MQTT Daten: BROKER=%s, PORT=%d\n", broker.c_str(), port);
      if (broker.length() > 0)
      {
          mqtt.broker = broker;
          mqtt.port = port;
          mqtt.saveConfig();          
          // Kein Neustart notwendig, MQTT-Verbindung wird neu initialisiert
      }
  }
  else //slider im Setup bewegt -> setup der Servopositionen
  {
    if (action == "servoLeft")      
      servoData.left   = value.as<int>();
    else if(action == "servoMiddle") 
      servoData.middle = value.as<int>();
    else if(action == "servoRight")
      servoData.right  = value.as<int>();
    else if(action == "servoStopActive")
      servoData.stopActive  = value.as<int>();
    else if(action == "servoStopInactive")
      servoData.stopInactive  = value.as<int>();
    else if(action == "servoPinUpDown")
      servoData.servoPinUpDown = value.as<int>();
    else if(action == "servoPinStop")
      servoData.servoPinStop = value.as<int>();
    else if(action == "timePress")
      servoData.timePress = value.as<int>();
    if (action == "servoLeft" || action == "servoMiddle" || action == "servoRight" )
      servoUpDown.target = value.as<int>();
    else if (action == "servoStopActive" || action == "servoStopInactive")
      servoStop.target = value.as<int>();
    informClients(action, value);
  }      
}


void setupOTA() {
  ElegantOTA.begin(&server); // Startet ElegantOTA  
  
  ElegantOTA.onStart([]() {
    logPrintln("OTA Start");
  });

  ElegantOTA.onEnd([](bool success) {
    success ? logPrintln("OTA Ende, Neustart...") : logPrintln("OTA Ende mit Fehler");
    success ? ESP.restart() : logPrintln("Kein Neustart");   
  });

  ElegantOTA.onProgress([](unsigned int progress, unsigned int total) {
    char buf[64];
    snprintf(buf, sizeof(buf), "OTA Fortschritt: %u/%u", progress, total);
    logPrintln(buf);
    informClients("ota", buf);
  });
  
  logBufferAdd("🌐 OTA Update verfügbar unter /update");
}

void setupWebsocket()
{
  // WebSocket einbinden
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  // lustig, ich bin alt,  [] leitet einen Lambda Ausdruck ein, also eine anonyme Funktion
  // die gabs frueher nicht :-), dafür mehr Lametta
  server.on("/favicon.ico", [](AsyncWebServerRequest *request)
            {
              request->send(204); // no content
            });

  // Hauptseite
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html, processor); });
  //jsonDaten abfragen - liefert hier zunächst letzter Stand, später evtl. reedKontakt abfragen
  server.on("/getData", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              JsonDocument doc;
              doc["lastAction"] = (lastAction == RAUS) ? "raus" :
                                  (lastAction == REIN) ? "rein" :
                                  (lastAction == STOPPEN) ? "stoppen" : "keine";

              String response;
              serializeJson(doc, response);
              request->send(200, "application/json", response);
            });
}

void onMqttCommand(const String& cmd)
{
    logPrintf("MQTT Befehl empfangen: %s\n", cmd.c_str());
    
    if (cmd == "up" || cmd == "rein" || cmd == "in")
    {
        servoUpDown.target = servoData.right;
        buttonState = UP;
        lastAction = REIN;
        informClients("button", "up");
    }
    else if (cmd == "down" || cmd == "raus" || cmd == "out")
    {
        servoUpDown.target = servoData.left;
        buttonState = DOWN;
        lastAction = RAUS;
        informClients("button", "down");
    }
    else if (cmd == "stop")
    {
        // Zuerst Up/Down-Servo auf Mittel  
        servoUpDown.target = servoData.middle;
        // Dann Stop-Servo aktivieren
        servoStop.target = servoData.stopActive;
        buttonState = STOP;
        lastAction = STOPPEN;
        informClients("button", "stop");
    }
    String lastActionString = (lastAction == RAUS) ? "raus" :
                                  (lastAction == REIN) ? "rein" :
                                  (lastAction == STOPPEN) ? "stop" : "keine"; 
    mqtt.publishLastCmd(lastActionString); // MQTT informieren

        
  }
// ---- Setup und Loop ----     
void setup()
{
  Serial.begin(115200);
  LittleFS.begin();
  loadServoData(); 


  initializeServos();
  wifiSetup();

  // OTA einrichten
   setupOTA();

   setupWebsocket();

   server.begin();
   logPrintln("HTTP-Server gestartet");

   // MQTT initialisieren
    mqtt.loadConfig();
    mqtt.begin();    
    mqtt.setOnCmd(onMqttCommand); // keine Befehle vom MQTT-Server
    
}

void loop() 
{
  
  updateServo(releaseButtonAt, releaseStopButtonAt);
  ws.cleanupClients();
  //ElegantOTA.loop(); nur wenn blockierend, hier nicht nötig da Async

  if (releaseButtonAt > 0 && millis() >= releaseButtonAt)
  {
      releaseButtonAt = 0;
      servoUpDown.target = servoData.middle;   
      buttonState = NONE;   
  }
  if (releaseStopButtonAt > 0 && millis() >= releaseStopButtonAt) 
  {
    releaseStopButtonAt = 0;
    servoStop.target = servoData.stopInactive;
    buttonState = NONE;
  }
  checkServoDataChanged();
  if (servoDataChanged && millis() - lastSave >= SAVE_INTERVAL) {
    lastSave = millis();
    saveServoData();
    servoDataChanged = false;
  }  

  mqtt.poll();
}