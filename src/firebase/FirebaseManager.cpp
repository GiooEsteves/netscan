#include "../firebase/FirebaseManager.h"
#include "Config.h"
#include "../time/TimeManager.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

void sendJsonToFirebase(String path, String json) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Firebase: WiFi nao conectado.");
    return;
  }

  HTTPClient http;

  String url = String(FIREBASE_DATABASE_URL) + path + ".json";

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  int code = http.POST(json);

  if (code > 0) {
    Serial.println("Firebase enviado: " + String(code));
  } else {
    Serial.println("Erro Firebase: " + String(code));
  }

  http.end();
}

void sendNetworkToFirebase(NetworkInfo network) {
  StaticJsonDocument<512> doc;

  doc["ssid"] = network.ssid;
  doc["rssi"] = network.rssi;
  doc["bssid"] = network.bssid;
  doc["channel"] = network.channel;
  doc["encryption"] = network.encryption;
  doc["detectedAt"] = network.detectedAt;

  String json;
  serializeJson(doc, json);

  sendJsonToFirebase("/wifi_audit/networks", json);
}

void sendAuditLogToFirebase(String action, String detail) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Firebase ignorado: WiFi externo nao conectado.");
    return;
  }
  
  StaticJsonDocument<256> doc;

  doc["action"] = action;
  doc["detail"] = detail;
  doc["time"] = getCurrentDateTime();

  String json;
  serializeJson(doc, json);

  sendJsonToFirebase("/wifi_audit/logs", json);
}