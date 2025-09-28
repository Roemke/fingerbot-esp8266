#include "wifi.h"
#include "defines.h"
#include <EEPROM.h>

WifiData wifiData;

String wifiMode;
String wifiMacAp;
String wifiMacSta;

static void saveWifiData() {
    EEPROM.put(EEPROM_WIFI_ADDR, wifiData);
    EEPROM.commit();
}

bool loadWifiData() {      
    EEPROM.get(EEPROM_WIFI_ADDR, wifiData);
    if (wifiData.magic != 0x43) {
        Serial.println("WiFi EEPROM leer, starte AP...");
        return false;
    }
    Serial.println("WiFi-Daten geladen.");
    return true;
}

void wifiSetCredentials(const char* ssid, const char* password) {
    strncpy(wifiData.ssid, ssid, sizeof(wifiData.ssid) - 1);
    strncpy(wifiData.password, password, sizeof(wifiData.password) - 1);
    wifiData.ssid[sizeof(wifiData.ssid)-1] = 0;
    wifiData.password[sizeof(wifiData.password)-1] = 0;
    wifiData.magic = 0x43;
    saveWifiData();
}

void wifiSetup() {
    // Alle Verbindungen sauber trennen
    WiFi.mode(WIFI_AP_STA); // beide aktiv, damit wir auf beide MACs zugreifen können
    wifiMacAp = WiFi.softAPmacAddress();
    wifiMacSta = WiFi.macAddress();
    delay(1000);
    Serial.println("=== WiFi Reset ===")    ;
    
    // Kompletter Hardware-Reset des WiFi
    WiFi.disconnect(true);  // disconnect + disable STA
    WiFi.softAPdisconnect(true); // disconnect + disable AP
    delay(1000);
    
    // WiFi komplett ausschalten
    WiFi.mode(WIFI_OFF);
    delay(2000);
    
    // Speicher bereinigen
    yield();
    ESP.wdtFeed(); // Watchdog reset
    
    Serial.printf("Heap nach Reset: %d\n", ESP.getFreeHeap());   
    delay(1000);
    if (loadWifiData()) {
        Serial.printf("Verbinde mit WLAN %s ...\n", wifiData.ssid);
        WiFi.mode(WIFI_STA);
        WiFi.begin(wifiData.ssid, wifiData.password);

        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
            delay(500);
            Serial.print(".");
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("\nVerbunden, IP: %s\n", WiFi.localIP().toString().c_str());
            wifiMode = "client-Modus, IP " + WiFi.localIP().toString();
            return;
        }
    }

    // AP-Modus mit sauberem Start
    Serial.println("Starte AP...");
    WiFi.mode(WIFI_OFF);
    delay(1000);
    Serial.println("Vor dem setzen des AP-Modus");
    WiFi.mode(WIFI_AP);
    Serial.println("AP-Modus gesetzt");
    delay(1000);
    
    // Vereinfachte AP-Konfiguration
    bool success = WiFi.softAP("Fingerbot");
    Serial.printf("AP Erfolg: %d\n", success);
    delay(2000);
    Serial.printf("AP IP: %s\n", WiFi.softAPIP().toString().c_str());
    wifiMode = "AP-Modus, IP " + WiFi.softAPIP().toString();
}

bool wifiIsConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String wifiGetIP() {
    if (wifiIsConnected())
        return WiFi.localIP().toString();
    else
        return WiFi.softAPIP().toString();
}
