# Fingerbot ESP8266

Ein DIY-Projekt, um einen Servo über einen ESP8266 (z. B. Wemos D1 mini) als **Fingerbot** zu betreiben.  
Über ein Webinterface können die Servo-Positionen eingestellt, Markises rein/raus bewegt und WLAN-Einstellungen gespeichert werden.  
Gehäuse lässt sich selbst drucken (mein Blenderversuch ist nicht zum
weitergeben geeignet...), Servo ist Miuzei Micro Servo 9g MS18.
Ist gescheitert, servo war zu schwach, daher mal weiter verwendet und für eine Fernbedienung von Heim und Haus / Somfy IO verwendet, mal sehen, ob es reicht. 
BlenderVersuch neu gemacht, jetzt für die Markise
Konstruktionsfehler, servos zu eng ... :-(
---

## ✨ Features

- Steuerung eines Servos über Webinterface (3 Positionen: `left`, `middle`, `right`)
- Buttons für **Rollo hoch / runter / stop** inzwischen auf up / down
- Konfiguration von:
  - Servo-Endpositionen (`left`, `middle`, `right`)
  - Zeiten für Bewegungen (`timeDown`, `timeUp`), danach wieder middle-Position
  - Servo-Pin
- Persistente Speicherung aller Werte im **EEPROM**
- WLAN-Konfiguration:
  - Verbindung mit bestehendem WLAN
  - Fallback: Startet Access Point (`Fingerbot`), wenn keine gültigen Daten gefunden werden auf 192.168.4.1
- Automatische Synchronisation des Status zwischen Clients (WebSocket), eine Spielerei, aber nett
- Erweiterbar um OTA-Updates für komfortable Software-Aktualisierung

---

## 📷 Hardware

- ESP8266 (z. B. Wemos D1 mini oder D1 mini Lite)
- Servo (z. B. SG90 oder MG90S), hier Miuzei Micro Servo 9g MS18
  für diesen Servo sind die Flanken auf 500 und 2500 gesetzt, dann erreicht man 0 bis 180 Grad
- 5 V Stromversorgung (abhängig vom Servo)

---

## 🖥️ Webinterface

- 3 **Slider** für Servo-Positionen  
- Buttons für:
  - ⬆️ Hoch
  - ⬇️ Runter
  - ⏹️ Stop  
- Anzeige der aktuellen Werte direkt neben den Slidern  
- Farbänderung der Buttons über CSS-Klassen (`active`)

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

2. Mit PlatformIO oder Arduino IDE bauen und flashen.
   Getestet nur unter PlatformIO

3. Beim ersten Start:
   - Gerät erstellt WLAN `Fingerbot`
   - Über Webinterface WLAN-Zugangsdaten eingeben
   - Danach verbindet sich der ESP automatisch mit deinem WLAN

---

## ⚙️ Projektstruktur
Hatte ich generieren lassen, stimmt nicht... besser doch tree :-)
├── include
│   ├── credentials.h
│   ├── credentials_template.h
│   ├── indexHtmlJS.h
│   ├── logging.h
│   ├── main.h
│   ├── myServo.h
│   ├── README
│   └── wifi.h
├── platformio.ini
├── readme.md
├── src
│   ├── logging.cpp
│   ├── main.cpp
│   ├── myServo.cpp
│   └── wifi.cpp
usw

## 🚀 Roadmap

- [x] Servo-Steuerung mit Slidern, zum einstellen
- [x] Buttons für Markise-Steuerung
- [x] EEPROM/FS-Persistenz (Servo- & WLAN-Daten)
- [x] Access Point bei fehlender Konfiguration
- [x] OTA-Update via Webinterface

---

## 📄 Lizenz

MIT License – freie Nutzung und Anpassung erlaubt.  
Bitte Credits geben, wenn du das Projekt weiterverwendest 😊

