#pragma once
#include "servodata.h"
#include "wifi.h"

// EEPROM-Adressen
#define EEPROM_SERVO_ADDR   0
#define EEPROM_WIFI_ADDR    (EEPROM_SERVO_ADDR + sizeof(ServoData))
#define EEPROM_SIZE         (EEPROM_WIFI_ADDR + sizeof(WifiData))
