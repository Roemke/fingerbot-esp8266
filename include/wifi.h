#pragma once
#include <Arduino.h>
#include <ESP8266WiFi.h>
//wifi.h
//struct fuer WLAN Daten im EEPROM
struct WifiData
{
    char ssid[32];
    char password[64];
    uint8_t magic = 0; //0x43 fuer valide Daten
} ;
extern WifiData wifiData;

extern String wifiMode;
extern String wifiMacAp;
extern String wifiMacSta;

// öffentliche API
void wifiSetup();
bool wifiIsConnected();
String wifiGetIP();
void wifiSetCredentials(const char* ssid, const char* password);
