#include "CaptivePortal.h"
#include "Config.h"
#include "Models.h"
#include "WifiScanner.h"
#include "ChannelAnalysis.h"
#include "FirebaseManager.h"
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ArduinoJson.h>

WebServer server(80);
DNSServer dnsServer;

bool authenticated = false;

LocalUser users[] = {
  {"admin", "123456"},
  {"professor", "upe2026"},
  {"aluno", "wifi123"}
};

const int userCount = 3;

void handleRoot();
void handleLogin();
void handleDashboard();
void handleApiNetworks();
void handleApiChannels();
void handleNotFound();

void startCaptivePortal() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(PORTAL_SSID, PORTAL_PASSWORD);

  delay(500);

  IPAddress apIP = WiFi.softAPIP();

  dnsServer.start(53, "*", apIP);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/login", HTTP_POST, handleLogin);
  server.on("/dashboard", HTTP_GET, handleDashboard);
  server.on("/api/networks", HTTP_GET, handleApiNetworks);
  server.on("/api/channels", HTTP_GET, handleApiChannels);

  server.on("/generate_204", HTTP_GET, handleRoot);
  server.on("/gen_204", HTTP_GET, handleRoot);
  server.on("/fwlink", HTTP_GET, handleRoot);
  server.on("/hotspot-detect.html", HTTP_GET, handleRoot);

  server.onNotFound(handleNotFound);

  server.begin();

  Serial.println("Portal iniciado.");
  Serial.println("SSID: " + String(PORTAL_SSID));
  Serial.println("Senha: " + String(PORTAL_PASSWORD));
  Serial.println("IP: " + apIP.toString());

  sendAuditLogToFirebase("portal_iniciado", apIP.toString());
}

void handleCaptivePortal() {
  dnsServer.processNextRequest();
  server.handleClient();
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Portal Educacional</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body{font-family:Arial;background:#1e3c72;padding:20px;}
.card{max-width:420px;margin:60px auto;background:white;padding:25px;border-radius:14px;}
input,button{width:100%;padding:12px;margin-top:10px;box-sizing:border-box;}
button{background:#1e3c72;color:white;border:0;border-radius:8px;}
.info{background:#f2f2f2;padding:10px;margin-top:20px;font-size:13px;}
</style>
</head>
<body>
<div class="card">
<h2>Portal Educacional UPE</h2>
<p>Auditoria Wi-Fi defensiva.</p>
<form method="POST" action="/login">
<input name="username" placeholder="Usuario local" required>
<input name="password" type="password" placeholder="Senha local" required>
<button>Entrar</button>
</form>
<div class="info">
<b>Usuarios de teste:</b><br>
admin / 123456<br>
professor / upe2026<br>
aluno / wifi123<br><br>
Nenhuma senha real e armazenada.
</div>
</div>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

void handleLogin() {
  String username = server.arg("username");
  String password = server.arg("password");

  bool valid = false;

  for (int i = 0; i < userCount; i++) {
    if (username == users[i].username && password == users[i].password) {
      valid = true;
      break;
    }
  }

  if (valid) {
    authenticated = true;
    sendAuditLogToFirebase("login_local_ok", username);
    server.sendHeader("Location", "/dashboard");
    server.send(302, "text/plain", "OK");
  } else {
    sendAuditLogToFirebase("login_local_negado", "usuario_invalido");
    server.send(401, "text/html", "<h2>Login invalido</h2><a href='/'>Voltar</a>");
  }
}

void handleDashboard() {
  if (!authenticated) {
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "Login necessario");
    return;
  }

  String html = "<html><head><meta charset='UTF-8'><title>Dashboard</title>";
  html += "<style>body{font-family:Arial;padding:20px;}table{width:100%;border-collapse:collapse;}td,th{border:1px solid #ddd;padding:8px;}th{background:#1e3c72;color:white;}</style>";
  html += "</head><body>";
  html += "<h1>Painel de Auditoria Wi-Fi</h1>";
  html += "<p>Redes detectadas: " + String(networkCount) + "</p>";
  html += "<p>Canal sugerido: " + String(getBestChannel()) + "</p>";
  html += "<p><a href='/api/networks'>API Redes</a> | <a href='/api/channels'>API Canais</a></p>";

  html += "<table><tr><th>SSID</th><th>RSSI</th><th>Canal</th><th>Seguranca</th><th>Hora</th></tr>";

  for (int i = 0; i < networkCount; i++) {
    html += "<tr>";
    html += "<td>" + networks[i].ssid + "</td>";
    html += "<td>" + String(networks[i].rssi) + "</td>";
    html += "<td>" + String(networks[i].channel) + "</td>";
    html += "<td>" + networks[i].encryption + "</td>";
    html += "<td>" + networks[i].detectedAt + "</td>";
    html += "</tr>";
  }

  html += "</table></body></html>";

  server.send(200, "text/html", html);
}

void handleApiNetworks() {
  StaticJsonDocument<8192> doc;
  JsonArray arr = doc.to<JsonArray>();

  for (int i = 0; i < networkCount; i++) {
    JsonObject obj = arr.createNestedObject();
    obj["ssid"] = networks[i].ssid;
    obj["rssi"] = networks[i].rssi;
    obj["bssid"] = networks[i].bssid;
    obj["channel"] = networks[i].channel;
    obj["encryption"] = networks[i].encryption;
    obj["detectedAt"] = networks[i].detectedAt;
  }

  String json;
  serializeJson(doc, json);

  server.send(200, "application/json", json);
}

void handleApiChannels() {
  StaticJsonDocument<2048> doc;

  JsonArray arr = doc.createNestedArray("channels");

  for (int i = 1; i <= 13; i++) {
    JsonObject obj = arr.createNestedObject();
    obj["channel"] = i;
    obj["networks"] = channelUsage[i];
  }

  doc["recommendedChannel"] = getBestChannel();

  String json;
  serializeJson(doc, json);

  server.send(200, "application/json", json);
}

void handleNotFound() {
  server.sendHeader("Location", "http://" + WiFi.softAPIP().toString() + "/");
  server.send(302, "text/plain", "Redirecionando...");
}