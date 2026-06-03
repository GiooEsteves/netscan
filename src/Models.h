#ifndef MODELS_H
#define MODELS_H

#include <Arduino.h>

struct NetworkInfo {
  String ssid;
  int rssi;
  String bssid;
  int channel;
  String encryption;
  String detectedAt;
};

struct LocalUser {
  String username;
  String password;
};

#endif