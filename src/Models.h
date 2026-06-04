#ifndef MODELS_H
#define MODELS_H

#include <Arduino.h>

struct NetworkInfo {
  String ssid;
  String bssid;
  int rssi;
  int channel;
  String encryption;
  String detectedAt;
};

struct LocalUser {
  String username;
  String password;
};

struct Alert {
  String type;
  String message;
  String level;
};

struct WiFiQuality {
  int score;
  String level;
  String description;
};

#endif