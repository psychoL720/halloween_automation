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

// Define RGBW LED pins (change as needed)
const int RED_PIN   = 4;  // use 220 ohm resistors for each channel
const int GREEN_PIN = 16; // use 220 ohm resistors for each channel
const int BLUE_PIN  = 17; // use 220 ohm resistors for each channel
const int WHITE_PIN = 18; // use 220 ohm resistors for each channel
// If using common cathode, connect the common of the LED to GND
// If using common anode, connect common pin to 3.3V and invert logic in code (set 0 = ON, 255 = OFF)
// LED labeled either CC or CA possibly
// Longest pin is the common pin
// TO identify which you have:
// 1. Connect common to GND, briefly touch each color pin to 3.3V through a 220 ohm resistor
//    If the LED lights up, it's common cathode
// 2. Connect common to 3.3V, briefly touch each color pin to GND through a 220 ohm resistor
//    If the LED lights up, it's common anode
const int LEDC_CHANNEL_R = 0;
const int LEDC_CHANNEL_G = 1;
const int LEDC_CHANNEL_B = 2;
const int LEDC_CHANNEL_W = 3;
const int LEDC_FREQ = 5000;
const int LEDC_RES = 8; // 8-bit PWM

const int ONBOARD_LED_PIN = 2; // Most ESP32 boards use GPIO2 for onboard LED

void setRGBW(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  ledcWrite(LEDC_CHANNEL_R, r);
  ledcWrite(LEDC_CHANNEL_G, g);
  ledcWrite(LEDC_CHANNEL_B, b);
  ledcWrite(LEDC_CHANNEL_W, w);
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
    // Fade out all channels together
    for (int duty = 255; duty >= 0; duty--) {
      setRGBW(duty, duty, duty, duty);
      delay(5); // Adjust for fade speed
    }
    server.send(200, "text/plain", "Faded out LED");
  } else if (cmd == "white") {
    setRGBW(0, 0, 0, 255); // Only white channel at full brightness
    server.send(200, "text/plain", "White full ON");
  } else if (cmd == "red") {
    // Fade out all channels together
    for (int duty = 255; duty >= 0; duty--) {
      setRGBW(duty, duty, duty, duty);
      delay(5); // Adjust for fade speed
    }
    delay(300); // Brief pause (300 ms)
    // Fade in red channel only
    for (int duty = 0; duty <= 255; duty++) {
      setRGBW(duty, 0, 0, 0);
      delay(5); // Adjust for fade speed
    }
    server.send(200, "text/plain", "White OUT, faded in Red");
  } else {
    server.send(400, "text/plain", "Unknown command");
  }
}

void setup() {
  Serial.begin(115200);

  // Setup PWM for each channel
  ledcSetup(LEDC_CHANNEL_R, LEDC_FREQ, LEDC_RES);
  ledcSetup(LEDC_CHANNEL_G, LEDC_FREQ, LEDC_RES);
  ledcSetup(LEDC_CHANNEL_B, LEDC_FREQ, LEDC_RES);
  ledcSetup(LEDC_CHANNEL_W, LEDC_FREQ, LEDC_RES);

  ledcAttachPin(RED_PIN,   LEDC_CHANNEL_R);
  ledcAttachPin(GREEN_PIN, LEDC_CHANNEL_G);
  ledcAttachPin(BLUE_PIN,  LEDC_CHANNEL_B);
  ledcAttachPin(WHITE_PIN, LEDC_CHANNEL_W);

  setRGBW(255, 255, 255, 255); // Start ON and white

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