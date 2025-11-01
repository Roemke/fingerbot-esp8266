// mqtt.h
#pragma once
#include <Arduino.h>
#include <functional>
#include "credentials.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


class AwningMQTT{
    public:
        String broker = mqttBROKER;            // aus credentials.h
        uint16_t port = mqttPORT;              // aus credentials.h


        void saveConfig();
        void loadConfig();
        // MQTT-Lebenszyklus (nicht-blockierend)
        void begin(const char* userArg = nullptr, const char* passArg = nullptr, const char* clientIdArg = nullptr);
        void poll();                // in loop() aufrufen
        void disconnect();
        bool isConnected() { return mqtt.connected(); }

        // Kommunikation
        bool publishLastCmd(const String& cmd);   // retained
        bool publishStatus(const String& json);   // no retain
        void setOnCmd(void (*handler)(const String&)); // Callback für empfangene Kommandos    

    private:
        //awning = markise
        String baseTopic = "awning"; // Präfix
        String topicCmd  = "awning/cmd";         
        String topicLast = "awning/last_cmd" ;
        String topicAvail= "awning/availability"; //na ob wir das brauchen?
        bool enable = true;       // false => MQTT komplett aus
        String user;            
        String pass;            

        // Interna        
        String lastAppliedBroker; //falls neuverbindung auftaucht, sollte unnoetig sein
        uint16_t lastAppliedPort = 0; //wie oben
        unsigned long serverChangedAt = 0;
        const unsigned long connectCooldownMs = 1500; // 1.5s Ruhe nach Wechsel
        WiFiClient   net;
        PubSubClient mqtt{net}; //gleichbedeutend mit PubSubClient mqtt(net);
        String       clientIdAuto;
        static String makeAutoClientId();

        void (*cmdHandler)(const String&) = nullptr;

        unsigned long lastAttemptMs = 0;
        uint32_t      reconnectIntervalMs = 5000; // 5s

        // Callback-Brücke (eine Instanz), Technik um ein Member als C-Style Callback zu verwenden
        // auch Trampolin genannt, nur eine Instanz erlaubt
        static AwningMQTT* instance ;
        static void onMqttStatic(char* topic, byte* payload, unsigned int len); //statische signatur, also ohne this
        // tatsächliche Callback-Methode, kann ich mir doch eigenltich sparen, wenn ich beim initialisieren eine 
        // Funktion übergebe - so ist nur eine Instanz erlaubt
        void onMqtt(char* topic, byte* payload, unsigned int len);
        void attemptConnect();
};

extern AwningMQTT mqtt;





