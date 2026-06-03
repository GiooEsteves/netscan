#ifndef WIFI_SCANNER_H
#define WIFI_SCANNER_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include "Models.h"

extern NetworkInfo networks[50];
extern int networkCount;
extern int channelUsage[14];

void scanNetworks();

void showFoundNetworks();

String getEncryptionName(uint8_t type);

#endif