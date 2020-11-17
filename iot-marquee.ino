#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_LEDMatrixArray.h>
#include <Adafruit_LEDMatrixArray.cpp>

#include "credentials.h"

#define FILL " ";

ESP8266WebServer server(80);
String message("Hello world!");
Adafruit_BicolorMatrix displays[] = {
  Adafruit_BicolorMatrix(), Adafruit_BicolorMatrix()
};
uint8_t addresses[] = {
  0x70, 0x71
};
Adafruit_LEDMatrixArray<1,2> matrix(displays, addresses);

void setup() {
  Serial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  for (int attempt = 1; WiFi.status() != WL_CONNECTED && attempt <= 20; attempt++) {
    delay(500);
    Serial.print("Connecting to "); Serial.println(WIFI_SSID); 
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected.");
    Serial.print(" IP Address: "); Serial.println(WiFi.localIP());
    Serial.print(" PIN       : "); Serial.println(PIN);

    message = message + " " + WiFi.localIP().toString();
    message = message + " " + PIN;
  } else {
    Serial.println("Unable to connect. Skipping for now.");
  }

  server.on("/get", handleGet);
  server.on("/set", handleSet);
  server.begin();

  displays[0].setRotation(3);
  displays[1].setRotation(3);
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setTextSize(1);
  matrix.setTextColor(LED_RED);
}

void loop() {
  server.handleClient();
  runMarquee();
}

void runMarquee() {
  static int off = 0;
  static unsigned long lastTick = 0;
  const unsigned long SHIFT_DELAY = 50;

  if (millis() - lastTick < SHIFT_DELAY) {
    return;
  }
  
  lastTick = millis();

  const String displayMessage = message + FILL;

  off--;
  if (off <= -matrix.textWidth(displayMessage)) {
    off = 0;
  }

  matrix.clear();

  for (int pos = off; pos < matrix.MATRIX_WIDTH; pos += matrix.textWidth(displayMessage)) {
    matrix.setCursor(pos, 0);
    matrix.print(message);
  }
  
  matrix.writeDisplay();
}

void handleGet() {
  Serial.println("Request at /get");
  server.send(200, "text/plain", message);
}

void handleSet() {
  if (!server.hasArg("pin") || server.arg("pin") != PIN) {
    Serial.println("Error: PIN missing or incorrect.");
    server.send(401);
    return;
  }

  if (!server.hasArg("message")) {
    Serial.println("Error: Message missing.");
    server.send(401);
    return;
  }

  message = server.arg("message");
  Serial.print("New message: "); Serial.println(message);
  
  server.send(200, "text/plain", "OK. Showing \"" + message + "\"");
}
