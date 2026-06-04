#include "../portal/CaptivePortal.h"
#include "Config.h"
#include "Models.h"
#include "../scanner/WifiScanner.h"
#include "../api/ApiRoutes.h"
#include "../analysis/SecurityAnalysis.h"
#include "../analysis/ChannelAnalysis.h"
#include "../firebase/FirebaseManager.h"
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
void handleNoContent();

void startCaptivePortal() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(PORTAL_SSID, PORTAL_PASSWORD);

  delay(500);

  IPAddress apIP = WiFi.softAPIP();

  dnsServer.start(53, "*", apIP);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/login", HTTP_POST, handleLogin);
  server.on("/dashboard", HTTP_GET, handleDashboard);
  server.on("/api/networks", HTTP_GET, handleApiNetworks);
  server.on("/api/channels", HTTP_GET, handleApiChannels);
  setupApiRoutes(server);

  // Android
  server.on("/generate_204", HTTP_GET, handleRoot);
  server.on("/gen_204", HTTP_GET, handleRoot);

  // Windows
  server.on("/fwlink", HTTP_GET, handleRoot);
  server.on("/connecttest.txt", HTTP_GET, handleRoot);
  server.on("/ncsi.txt", HTTP_GET, handleRoot);

  // iOS/macOS
  server.on("/hotspot-detect.html", HTTP_GET, handleRoot);
  server.on("/library/test/success.html", HTTP_GET, handleRoot);

  // Navegadores
  server.on("/favicon.ico", HTTP_GET, handleNoContent);

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
<title>Portal UPE - Auditoria WiFi</title>
<meta name="viewport" content="width=device-width, initial-scale=1">

<style>

body {
  margin: 0;
  min-height: 100vh;
  font-family: 'Segoe UI', Arial, sans-serif;
  background: linear-gradient(135deg, #1e3c72, #2a5298);
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 20px;
}

.login-container {
  width: 100%;
  max-width: 430px;
}

.card {
  background: white;
  border-radius: 18px;
  overflow: hidden;
  box-shadow: 0 15px 35px rgba(0,0,0,0.35);
}

.header {
  background: #1e3c72;
  padding: 25px 20px;
  text-align: center;
}

.logo-upe {
  width: 180px;
  height: auto;
  margin-bottom: 15px;
}

.header h1 {
  margin: 0;
  color: white;
  font-size: 24px;
  font-weight: 600;
}

.header p {
  margin-top: 8px;
  color: rgba(255,255,255,0.85);
  font-size: 14px;
}

.content {
  padding: 30px;
}

input {
  width: 100%;
  padding: 13px;
  margin-top: 12px;
  box-sizing: border-box;
  border-radius: 10px;
  border: 1px solid #cbd5e1;
  font-size: 15px;
  outline: none;
}

input:focus {
  border-color: #1e3c72;
}

button {
  width: 100%;
  padding: 13px;
  margin-top: 18px;
  background: #1e3c72;
  color: white;
  border: none;
  border-radius: 10px;
  font-size: 15px;
  font-weight: bold;
  cursor: pointer;
  transition: 0.2s;
}

button:hover {
  background: #162d55;
}

.info {
  background: #f1f5f9;
  color: #334155;
  padding: 14px;
  margin-top: 22px;
  border-radius: 10px;
  font-size: 13px;
  line-height: 1.6;
}

.footer {
  text-align: center;
  margin-top: 18px;
  color: rgba(255,255,255,0.8);
  font-size: 12px;
}

</style>
</head>

<body>

<div class="login-container">

  <div class="card">

    <div class="header">

      <h1>Portal Auditoria</h1>

      <p>Auditoria Wi-Fi Educacional</p>

    </div>

    <div class="content">

      <form method="POST" action="/login">

        <input 
          name="username" 
          placeholder="Usuário" 
          required
        >

        <input 
          name="password" 
          type="password" 
          placeholder="Senha" 
          required
        >

        <button type="submit">
          Entrar
        </button>

      </form>

      <div class="info">
        <b>Usuário de teste:</b><br>

        admin / 123456<br>
      </div>

    </div>

  </div>

  <div class="footer">
    Universidade de Pernambuco - UPE
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

    server.sendHeader("Location", "/dashboard", true);
    server.send(302, "text/plain", "Redirecionando para dashboard...");
  } else {
    authenticated = false;

    sendAuditLogToFirebase("login_local_negado", "usuario_invalido");

    server.send(401, "text/html",
      "<!DOCTYPE html>"
      "<html><head><meta charset='UTF-8'>"
      "<meta name='viewport' content='width=device-width, initial-scale=1'>"
      "<title>Login invalido</title></head>"
      "<body style='font-family:Arial;text-align:center;padding:40px;'>"
      "<h2>Login invalido</h2>"
      "<p>Usuario ou senha incorretos.</p>"
      "<a href='/'>Voltar</a>"
      "</body></html>"
    );
  }
}

void handleDashboard() {
  if (!authenticated) {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "Login necessario");
    return;
  }

  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Dashboard NetScan</title>
<meta name="viewport" content="width=device-width, initial-scale=1">

<style>
body {
  margin: 0;
  font-family: Arial, sans-serif;
  background: #0f172a;
  color: white;
}

header {
  background: #1e293b;
  padding: 20px;
  text-align: center;
}

.container {
  padding: 20px;
}

.cards {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(160px, 1fr));
  gap: 15px;
}

.card {
  background: #1e293b;
  padding: 18px;
  border-radius: 14px;
}

.card h3 {
  margin: 0;
  color: #38bdf8;
}

.card p {
  font-size: 26px;
  margin: 8px 0 0;
  font-weight: bold;
}

.actions {
  margin-top: 20px;
  display: flex;
  flex-wrap: wrap;
  gap: 10px;
}

.button {
  background: #38bdf8;
  color: #0f172a;
  padding: 10px 14px;
  border-radius: 8px;
  text-decoration: none;
  font-weight: bold;
}

.grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(320px, 1fr));
  gap: 20px;
  margin-top: 20px;
}

.panel {
  background: #1e293b;
  padding: 18px;
  border-radius: 14px;
}

.bar-row {
  margin-bottom: 12px;
}

.bar-label {
  margin-bottom: 4px;
  font-size: 14px;
}

.bar-bg {
  background: #334155;
  height: 24px;
  border-radius: 8px;
  overflow: hidden;
}

.bar {
  height: 100%;
  background: #38bdf8;
  color: #0f172a;
  font-size: 13px;
  font-weight: bold;
  text-align: right;
  padding-right: 6px;
  box-sizing: border-box;
  line-height: 24px;
}

.rssi-bar {
  background: #22c55e;
}

.alert {
  padding: 12px;
  border-radius: 8px;
  margin-bottom: 8px;
  background: #7f1d1d;
}

.alert.medio {
  background: #78350f;
}

.alert.baixo {
  background: #14532d;
}

table {
  width: 100%;
  border-collapse: collapse;
  background: #1e293b;
  margin-top: 20px;
  border-radius: 12px;
  overflow: hidden;
}

th, td {
  padding: 10px;
  border-bottom: 1px solid #334155;
  font-size: 14px;
}

th {
  background: #0284c7;
}

.status {
  margin-top: 10px;
  color: #94a3b8;
  font-size: 13px;
}
</style>
</head>

<body>

<header>
  <h1>NetScan</h1>
  <p>Dashboard de Auditoria Wi-Fi em Tempo Real</p>
</header>

<div class="container">

  <div class="cards">
    <div class="card">
      <h3>Redes</h3>
      <p id="totalNetworks">0</p>
    </div>

    <div class="card">
      <h3>Canal sugerido</h3>
      <p id="recommendedChannel">-</p>
    </div>

    <div class="card">
      <h3>Score Wi-Fi</h3>
      <p id="qualityScore">0</p>
    </div>

    <div class="card">
      <h3>Nível</h3>
      <p id="qualityLevel">-</p>
    </div>
  </div>

  <div class="actions">
    <a class="button" href="/api/report/csv">Exportar CSV</a>
  </div>

  <p class="status" id="status">Carregando dados...</p>

  <div class="grid">
    <div class="panel">
      <h2>Redes por Canal</h2>
      <div id="channelChart"></div>
    </div>

    <div class="panel">
      <h2>Intensidade do Sinal</h2>
      <div id="rssiChart"></div>
    </div>
  </div>

  <h2>Alertas de Segurança</h2>
  <div id="alerts"></div>

  <h2>Redes Detectadas</h2>
  <table>
    <thead>
      <tr>
        <th>SSID</th>
        <th>BSSID</th>
        <th>RSSI</th>
        <th>Canal</th>
        <th>Segurança</th>
      </tr>
    </thead>
    <tbody id="networksTable"></tbody>
  </table>

</div>

<script>
async function loadStats() {
  const res = await fetch('/api/stats');
  const data = await res.json();

  document.getElementById('totalNetworks').innerText = data.totalNetworks;
  document.getElementById('recommendedChannel').innerText = data.recommendedChannel;
  document.getElementById('qualityScore').innerText = data.qualityScore + "/100";
  document.getElementById('qualityLevel').innerText = data.qualityLevel;
}

async function loadNetworks() {
  const res = await fetch('/api/networks');
  const networks = await res.json();

  const tbody = document.getElementById('networksTable');
  tbody.innerHTML = "";

  networks.forEach(net => {
    tbody.innerHTML += `
      <tr>
        <td>${net.ssid || "Oculta"}</td>
        <td>${net.bssid}</td>
        <td>${net.rssi}</td>
        <td>${net.channel}</td>
        <td>${net.encryption}</td>
      </tr>
    `;
  });

  renderRssiChart(networks);
}

async function loadChannels() {
  const res = await fetch('/api/channels');
  const data = await res.json();

  renderChannelChart(data.channels);
}

async function loadAlerts() {
  const res = await fetch('/api/alerts');
  const alerts = await res.json();

  const div = document.getElementById('alerts');
  div.innerHTML = "";

  if (alerts.length === 0) {
    div.innerHTML = "<p>Nenhum alerta crítico detectado.</p>";
    return;
  }

  alerts.forEach(alert => {
    div.innerHTML += `
      <div class="alert ${alert.level}">
        <b>${alert.level.toUpperCase()}</b> - ${alert.message}
      </div>
    `;
  });
}

function renderChannelChart(channels) {
  const div = document.getElementById('channelChart');
  div.innerHTML = "";

  let max = 1;

  channels.forEach(c => {
    if (c.networks > max) max = c.networks;
  });

  channels.forEach(c => {
    const width = (c.networks / max) * 100;

    div.innerHTML += `
      <div class="bar-row">
        <div class="bar-label">Canal ${c.channel}</div>
        <div class="bar-bg">
          <div class="bar" style="width:${width}%">${c.networks}</div>
        </div>
      </div>
    `;
  });
}

function renderRssiChart(networks) {
  const div = document.getElementById('rssiChart');
  div.innerHTML = "";

  if (networks.length === 0) {
    div.innerHTML = "<p>Nenhuma rede encontrada. Execute o scanner antes.</p>";
    return;
  }

  networks.slice(0, 12).forEach(net => {
    let rssi = net.rssi;

    let quality = Math.min(100, Math.max(5, 2 * (rssi + 100)));

    div.innerHTML += `
      <div class="bar-row">
        <div class="bar-label">${net.ssid || "Oculta"} (${rssi} dBm)</div>
        <div class="bar-bg">
          <div class="bar rssi-bar" style="width:${quality}%">${rssi}</div>
        </div>
      </div>
    `;
  });
}

async function refreshDashboard() {
  try {
    await loadStats();
    await loadNetworks();
    await loadChannels();
    await loadAlerts();

    document.getElementById('status').innerText =
      "Atualizado automaticamente a cada 5 segundos.";
  } catch (error) {
    document.getElementById('status').innerText =
      "Erro ao carregar dados da API. Verifique as rotas /api.";
  }
}

refreshDashboard();
setInterval(refreshDashboard, 5000);
</script>

</body>
</html>
)rawliteral";

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

void handleNoContent() {
  server.send(204, "text/plain", "");
}

void handleNotFound() {
  String uri = server.uri();

  Serial.println("Rota nao encontrada: " + uri);

  if (
    uri == "/generate_204" ||
    uri == "/gen_204" ||
    uri == "/hotspot-detect.html" ||
    uri == "/connecttest.txt" ||
    uri == "/ncsi.txt" ||
    uri == "/fwlink" ||
    uri == "/library/test/success.html"
  ) {
    handleRoot();
    return;
  }

  if (uri == "/favicon.ico") {
    server.send(204, "text/plain", "");
    return;
  }

  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "Redirecionando...");
}