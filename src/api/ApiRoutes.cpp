#include "ApiRoutes.h"
#include "../scanner/WifiScanner.h"
#include "../analysis/ChannelAnalysis.h"
#include "../analysis/SecurityAnalysis.h"
#include <ArduinoJson.h>

void setupApiRoutes(WebServer& server) {
  server.on("/api/stats", HTTP_GET, [&server]() {
    StaticJsonDocument<1024> doc;

    doc["totalNetworks"] = networkCount;
    doc["recommendedChannel"] = getBestChannel();
    doc["hasOpenNetwork"] = hasOpenNetwork();
    doc["evilTwinDetected"] = detectEvilTwin();

    WiFiQuality quality = calculateWiFiQuality();

    doc["qualityScore"] = quality.score;
    doc["qualityLevel"] = quality.level;
    doc["qualityDescription"] = quality.description;

    String json;
    serializeJson(doc, json);

    server.send(200, "application/json", json);
  });

  server.on("/api/alerts", HTTP_GET, [&server]() {
    generateSecurityAlerts();

    StaticJsonDocument<4096> doc;
    JsonArray arr = doc.to<JsonArray>();

    for (int i = 0; i < alertCount; i++) {
      JsonObject obj = arr.createNestedObject();
      obj["type"] = alerts[i].type;
      obj["message"] = alerts[i].message;
      obj["level"] = alerts[i].level;
    }

    String json;
    serializeJson(doc, json);

    server.send(200, "application/json", json);
  });

  server.on("/api/quality", HTTP_GET, [&server]() {
    WiFiQuality quality = calculateWiFiQuality();

    StaticJsonDocument<1024> doc;

    doc["score"] = quality.score;
    doc["level"] = quality.level;
    doc["description"] = quality.description;

    String json;
    serializeJson(doc, json);

    server.send(200, "application/json", json);
  });

  server.on("/api/evil-twin", HTTP_GET, [&server]() {
    server.send(200, "application/json", getEvilTwinJson());
  });

  server.on("/api/report/csv", HTTP_GET, [&server]() {
    String csv = "SSID,BSSID,RSSI,Canal,Seguranca,Hora\n";

    for (int i = 0; i < networkCount; i++) {
      csv += networks[i].ssid + ",";
      csv += networks[i].bssid + ",";
      csv += String(networks[i].rssi) + ",";
      csv += String(networks[i].channel) + ",";
      csv += networks[i].encryption + ",";
      csv += networks[i].detectedAt + "\n";
    }

    server.sendHeader("Content-Disposition", "attachment; filename=relatorio_wifi.csv");
    server.send(200, "text/csv", csv);
  });
}