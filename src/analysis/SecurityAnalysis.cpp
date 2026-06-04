#include "SecurityAnalysis.h"
#include "../scanner/WifiScanner.h"
#include "../analysis/ChannelAnalysis.h"

Alert alerts[30];
int alertCount = 0;

void addAlert(String type, String message, String level) {
  if (alertCount >= 30) return;

  alerts[alertCount].type = type;
  alerts[alertCount].message = message;
  alerts[alertCount].level = level;
  alertCount++;
}

bool isOpenNetwork(String encryption) {
  encryption.toLowerCase();

  return (
    encryption == "aberta" ||
    encryption == "open" ||
    encryption == "none" ||
    encryption == "sem senha"
  );
}

void generateSecurityAlerts() {
  alertCount = 0;

  for (int i = 0; i < networkCount; i++) {
    if (isOpenNetwork(networks[i].encryption)) {
      addAlert(
        "Rede aberta",
        "Rede em risco: " + networks[i].ssid +
        " | BSSID: " + networks[i].bssid +
        " | Canal: " + String(networks[i].channel) +
        " | Motivo: rede sem criptografia.",
        "alto"
      );
    }

    if (networks[i].rssi > -40) {
      addAlert(
        "Sinal muito forte",
        "Rede analisada: " + networks[i].ssid +
        " | BSSID: " + networks[i].bssid +
        " | RSSI: " + String(networks[i].rssi) +
        " dBm | Motivo: sinal muito forte, verificar proximidade do ponto de acesso.",
        "medio"
      );
    }
  }

  for (int ch = 1; ch <= 13; ch++) {
    if (channelUsage[ch] >= 5) {
      addAlert(
        "Canal congestionado",
        "Canal em risco: " + String(ch) +
        " | Quantidade de redes: " + String(channelUsage[ch]) +
        " | Motivo: muitas redes usando o mesmo canal.",
        "medio"
      );
    }
  }

  if (detectEvilTwin()) {
    addAlert(
      "Possível Evil Twin",
      "Possível Evil Twin detectado. Existem redes com o mesmo SSID e BSSID diferente. Verifique a página Evil Twin.",
      "alto"
    );
  }
}

bool hasOpenNetwork() {
  for (int i = 0; i < networkCount; i++) {
    if (isOpenNetwork(networks[i].encryption)) {
      return true;
    }
  }

  return false;
}

bool detectEvilTwin() {
  for (int i = 0; i < networkCount; i++) {
    for (int j = i + 1; j < networkCount; j++) {
      if (
        networks[i].ssid.length() > 0 &&
        networks[i].ssid == networks[j].ssid &&
        networks[i].bssid != networks[j].bssid
      ) {
        return true;
      }
    }
  }

  return false;
}

String getEvilTwinJson() {
  String json = "[";
  bool first = true;

  for (int i = 0; i < networkCount; i++) {
    for (int j = i + 1; j < networkCount; j++) {
      if (
        networks[i].ssid.length() > 0 &&
        networks[i].ssid == networks[j].ssid &&
        networks[i].bssid != networks[j].bssid
      ) {
        if (!first) json += ",";
        first = false;

        json += "{";
        json += "\"ssid\":\"" + networks[i].ssid + "\",";
        json += "\"bssid1\":\"" + networks[i].bssid + "\",";
        json += "\"bssid2\":\"" + networks[j].bssid + "\",";
        json += "\"channel1\":" + String(networks[i].channel) + ",";
        json += "\"channel2\":" + String(networks[j].channel);
        json += "}";
      }
    }
  }

  json += "]";
  return json;
}

WiFiQuality calculateWiFiQuality() {
  WiFiQuality quality;

  int score = 100;

  if (hasOpenNetwork()) score -= 25;
  if (detectEvilTwin()) score -= 30;

  for (int ch = 1; ch <= 13; ch++) {
    if (channelUsage[ch] >= 5) {
      score -= 10;
      break;
    }
  }

  if (networkCount > 20) score -= 10;

  if (score < 0) score = 0;

  quality.score = score;

  if (score >= 80) {
    quality.level = "Boa";
    quality.description = "Ambiente Wi-Fi com baixo risco.";
  } else if (score >= 50) {
    quality.level = "Media";
    quality.description = "Ambiente com alguns pontos de atencao.";
  } else {
    quality.level = "Ruim";
    quality.description = "Ambiente com riscos relevantes de seguranca ou interferencia.";
  }

  return quality;
}