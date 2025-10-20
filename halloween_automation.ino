#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h> // Add mDNS support

const char* ssid = "Black_FBI_Van";
const char* password = "dailylake789";

WebServer server(80);
const int LED_PIN = 2; // Onboard LED pin for most ESP32 boards

void handleAction() {
  String cmd = server.arg("cmd");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  if (cmd == "on") {
    digitalWrite(LED_PIN, HIGH); // Turn LED ON
    server.send(200, "text/plain", "Device ON");
  } else if (cmd == "off") {
    digitalWrite(LED_PIN, LOW); // Turn LED OFF
    server.send(200, "text/plain", "Device OFF");
  } else {
    server.send(400, "text/plain", "Unknown command");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT); // Initialize LED pin
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  // Start mDNS with a unique hostname for each ESP32
  if (!MDNS.begin("esp32-halloween1")) { // Change hostname for each device
    Serial.println("Error starting mDNS");
    return;
  }
  MDNS.addService("http", "tcp", 80);

  server.on("/action", handleAction);
  server.begin();
}

void loop() {
  server.handleClient();
}