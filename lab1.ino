#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define LED1_PIN 0    // D3
#define LED2 2    // D4
#define LED3 12   // D6
#define BUTTON 14 // D5

const char* ssid = "Redmi Note 10 Pro";
const char* password = "55555555";

ESP8266WebServer server(9999);

int ledOrder[3] = {LED1_PIN, LED2, LED3}; 
bool reverseDirection = false; 

const unsigned long doubleClickTime = 300; 
unsigned long lastButtonPress = 0;
int clickCount = 0;

void setup() {
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP-address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);
  server.begin();
}

void loop() {
  server.handleClient();
  handleButton();
  runAlgorithm();
}

void handleButton() {
  bool buttonState = digitalRead(BUTTON) == LOW;
  static bool lastButtonState = HIGH;
  unsigned long currentMillis = millis();

  if (buttonState && lastButtonState == HIGH) {
    if (currentMillis - lastButtonPress < doubleClickTime) {
      clickCount++;
    } else {
      clickCount = 1;
    }
    lastButtonPress = currentMillis;
  }

  if (clickCount == 2) {
    clickCount = 0;
    reverseDirection = !reverseDirection;
  }

  if (currentMillis - lastButtonPress > doubleClickTime) {
    clickCount = 0;
  }

  lastButtonState = buttonState;
}

void runAlgorithm() {
  static int lastActiveIndex = 0;
  static unsigned long lastUpdate = 0;
  const unsigned long interval = 300;

  if (millis() - lastUpdate >= interval) {
    digitalWrite(ledOrder[lastActiveIndex], LOW);

    if (reverseDirection) {
      lastActiveIndex = (lastActiveIndex == 0) ? 2 : lastActiveIndex - 1;
    } else {
      lastActiveIndex = (lastActiveIndex + 1) % 3;
    }

    digitalWrite(ledOrder[lastActiveIndex], HIGH);

    lastUpdate = millis();
  }
}

void handleRoot() {
  String html = "<html>"
  "<head>"
  "<meta charset='UTF-8'>"
  "<title>ESP8266 LED Control</title>"
  "<style>"
  "body { text-align:center; font-family:sans-serif; background-color: #f0f0f0; margin: 0; padding: 0; }"
  "h1 { color: #4CAF50; font-size: 36px; margin-top: 50px; }"
  "button { padding: 15px 30px; font-size: 20px; background-color: #4CAF50; border: none; color: white; border-radius: 12px; cursor: pointer; transition: background-color 0.3s, transform 0.3s; }"
  "button:hover { background-color: #45a049; transform: scale(1.1); }"
  "button:active { background-color: #388e3c; transform: scale(1.05); }"
  "</style>"
  "<script>"
  "function toggleDirection() { fetch('/toggle'); }"
  "</script>"
  "</head>"
  "<body>"
  "<h1>ESP8266 LED Control</h1>"
  "<button onclick='toggleDirection()'>Змінити напрямок</button>"
  "</body>"
  "</html>";

  server.send(200, "text/html", html);
}

void handleToggle() {
  reverseDirection = !reverseDirection;
  server.send(200, "text/plain", "OK");
}