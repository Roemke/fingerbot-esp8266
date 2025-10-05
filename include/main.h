#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>


// Function declarations
void updateServo();
void checkServoDataChanged();
void saveServoData();
void loadServoData();
void initializeServos();
String processor(const String& var);
template <typename T>
void informClients(const String& action, T value);
void initialInformClient(AsyncWebSocketClient *client);
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void setupOTA();
void setupWebsocket();


#endif // MAIN_H