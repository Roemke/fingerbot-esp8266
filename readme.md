# Fingerbot ESP8266  

Ein DIY-Projekt, um einen Servo über einen ESP8266 (z. B. Wemos D1 mini) als **Fingerbot** zu betreiben.  
Über ein Webinterface können die Servo-Positionen eingestellt werden. Die Servos
dienen als Finberbot für die Fernbedienung einer Haus und Heim(?) Markise, da 
die irgenden proprietäres Protokoll verwendet. 
Servo drückt auf Remote und damit rein/raus/stop möglich.
Integration in FHEM über MQTT.  
Gehäuse lässt sich selbst drucken (mein Blenderversuch liegt bei, geht so)
Servo ist Miuzei Micro Servo 9g MS18.


## ✨ Features

- Steuerung zweier Servos über Webinterface (3 Positionen: `left`, `middle`, `right`)
- Buttons für **Markise rein / raus / stop** inzwischen auch auf up / down
- Konfiguration von:
  - Servo-Endpositionen (`left`, `middle`, `right`)
  - Zeiten für Bewegungen (`timeDown`, `timeUp`), danach wieder middle-Position
  - Servo-Pin
  - Schrittwete der Bewegung 
- Persistente Speicherung aller Werte im **EEPROM**, nur noch WiFi, der Rest in littleFS
- WLAN-Konfiguration:
  - Verbindung mit bestehendem WLAN
  - Fallback: Startet Access Point (`Fingerbot`), wenn keine gültigen Daten gefunden werden auf 192.168.4.1
- Automatische Synchronisation des Status zwischen Clients (WebSocket), eine Spielerei, aber nett
- Erweiterbar um OTA-Updates für komfortable Software-Aktualisierung, passiert
- abonniert auch mqtt-nachrichten, hier von FHEM kommend, Broker konfigurierbar, getestet mit fhem
- published last_cmd per mqtt
---

## 📷 Hardware

- ESP8266 (z. B. Wemos D1 mini oder D1 mini Lite)
- Servo (z. B. SG90 oder MG90S), hier Miuzei Micro Servo 9g MS18
  für diesen Servo sind die Flanken auf 500 und 2500 gesetzt, dann erreicht man 0 bis 180 Grad
  (hätte ich konfigurierbar machen sollen)
- 5 V Stromversorgung (abhängig vom Servo)
- Blenderdatei für Bestandteile des Gehäuses, in Einzelteilen drucken :-)

---

## 🖥️ Webinterface

- Steuerung, Help, Setup, Netz (auch Setup)
- Slider für Servo-Positionen, Zeit, Schrittweite, Servopins
- Farbänderung der Buttons über CSS-Klassen (`active`)
- und ein paar Ergänzungen, sieht man in der indexHTML

---

## 📡 WLAN-Konfiguration

- WLAN-Daten werden in einer `WifiData`-Struktur im EEPROM gespeichert
- Falls keine gültigen Daten vorhanden sind → startet ESP im **Access-Point-Modus** (`Fingerbot`)
- Nach Eingabe von SSID & Passwort → werden Daten gespeichert, Gerät startet im STA-Modus neu

---

## 🔧 Installation

1. Repository klonen:
   ```bash
   git clone git@github.com:Roemke/fingerbot-esp8266.git
   cd fingerbot-esp8266
   ```

2. Mit PlatformIO bauen und flashen.

3. Beim ersten Start:
   - Gerät erstellt WLAN `Fingerbot`, liegt auf 192.168.4.1
   - Über Webinterface WLAN-Zugangsdaten eingeben
   - Danach verbindet sich der ESP automatisch mit deinem WLAN

---

## 🚀 Roadmap

- [x] Servo-Steuerung mit Slidern, zum einstellen
- [x] Buttons für Markise-Steuerung
- [x] EEPROM/FS-Persistenz (Servo- & WLAN-Daten), teils 
- [x] Access Point bei fehlender Konfiguration
- [x] OTA-Update via Webinterface
- [x] MQTT umsetzen 
- [x] in FHEM integrieren

---

## 📄 Lizenz

MIT License – freie Nutzung und Anpassung erlaubt.  
Bitte Credits geben, wenn du das Projekt weiterverwendest 😊

