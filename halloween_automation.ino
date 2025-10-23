#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h> // Add mDNS support

/*
platformio.ini settings:
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
*/

const char* ssid = "Black_FBI_Van";
const char* password = "dailylake789";

WebServer server(80);

// Define LED pins
const int WHITE_PIN = 4;  // White LED pin
const int RED_PIN   = 16; // Red LED pin  
const int UV_PIN    = 17; // Ultraviolet LED pin

const int LEDC_CHANNEL_WHITE = 0;
const int LEDC_CHANNEL_RED = 1;
const int LEDC_CHANNEL_UV = 2;
const int LEDC_FREQ = 5000;
const int LEDC_RES = 8; // 8-bit PWM

const int ONBOARD_LED_PIN = 2; // Most ESP32 boards use GPIO2 for onboard LED

// Current LED brightness values
uint8_t whiteBrightness = 0;
uint8_t redBrightness = 0;
uint8_t uvBrightness = 0;

void setLEDs(uint8_t white, uint8_t red, uint8_t uv) {
  ledcWrite(LEDC_CHANNEL_WHITE, white);
  ledcWrite(LEDC_CHANNEL_RED, red);
  ledcWrite(LEDC_CHANNEL_UV, uv);
  // Update current values
  whiteBrightness = white;
  redBrightness = red;
  uvBrightness = uv;
}

void fadeOutAllLEDs() {
  // Find the maximum current brightness to fade from
  uint8_t maxBrightness = max(whiteBrightness, max(redBrightness, uvBrightness));
  
  for (int duty = maxBrightness; duty >= 0; duty--) {
    uint8_t whiteVal = (whiteBrightness > 0) ? map(duty, 0, maxBrightness, 0, whiteBrightness) : 0;
    uint8_t redVal = (redBrightness > 0) ? map(duty, 0, maxBrightness, 0, redBrightness) : 0;
    uint8_t uvVal = (uvBrightness > 0) ? map(duty, 0, maxBrightness, 0, uvBrightness) : 0;
    
    setLEDs(whiteVal, redVal, uvVal);
    delay(5);
  }
  setLEDs(0, 0, 0); // Ensure all are off
}

void handleAction() {
  String cmd = server.arg("cmd");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  
  if (cmd == "on") {
    digitalWrite(ONBOARD_LED_PIN, HIGH); // Turn ONBOARD LED ON
    server.send(200, "text/plain", "Onboard LED ON");
  } else if (cmd == "off") {
    digitalWrite(ONBOARD_LED_PIN, LOW); // Turn ONBOARD LED OFF
    server.send(200, "text/plain", "Onboard LED OFF");
  } else if (cmd == "fade") {
    fadeOutAllLEDs();
    server.send(200, "text/plain", "All LEDs faded out");
  } else if (cmd == "white") {
    setLEDs(0, 0, 0); // Turn off all LEDs immediately
    setLEDs(255, 0, 0); // Turn on white LED
    server.send(200, "text/plain", "White LED ON");
  } else if (cmd == "red") {
    fadeOutAllLEDs(); // Fade out any LEDs that are on
    delay(300); // Brief pause
    setLEDs(0, 255, 0); // Turn on red LED
    server.send(200, "text/plain", "Red LED ON");
  } else if (cmd == "blue") {
    fadeOutAllLEDs(); // Fade out any LEDs that are on
    delay(300); // Brief pause
    setLEDs(0, 0, 255); // Turn on UV LED
    server.send(200, "text/plain", "UV LED ON");
  } else {
    server.send(400, "text/plain", "Unknown command");
  }
}

void setup() {
  Serial.begin(115200);

  // Setup PWM for each LED channel
  ledcSetup(LEDC_CHANNEL_WHITE, LEDC_FREQ, LEDC_RES);
  ledcSetup(LEDC_CHANNEL_RED, LEDC_FREQ, LEDC_RES);
  ledcSetup(LEDC_CHANNEL_UV, LEDC_FREQ, LEDC_RES);

  ledcAttachPin(WHITE_PIN, LEDC_CHANNEL_WHITE);
  ledcAttachPin(RED_PIN, LEDC_CHANNEL_RED);
  ledcAttachPin(UV_PIN, LEDC_CHANNEL_UV);

  setLEDs(0, 0, 0); // Start with all LEDs off

  pinMode(ONBOARD_LED_PIN, OUTPUT); // Initialize onboard LED

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