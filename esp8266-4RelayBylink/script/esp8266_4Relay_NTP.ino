#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800; // GMT+5:30
const int daylightOffset_sec = 0;

const int relayPins[4] = {5, 4, 14, 12}; // Pin GPIO untuk NodeMCU
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);

struct RelayConfig {
  int onHour;
  int onMinute;
  int offHour;
  int offMinute;
  bool state;
};

RelayConfig relayConfigs[4] = {
  {21, 0, 21, 1, false},
  {21, 5, 21, 6, false},
  {21, 10, 21, 11, false},
  {21, 15, 21, 16, false}
};

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Initializing...");
  display.display();
  delay(1000);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  for (int i = 0; i < 4; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], LOW);
  }

  timeClient.begin();
  timeClient.update();
}

void loop() {
  timeClient.update();
  time_t currentTime = timeClient.getEpochTime();
  struct tm * timeinfo = localtime(&currentTime);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Time: ");
  display.println(timeClient.getFormattedTime());
  display.println("--------------");

  for (int i = 0; i < 4; i++) {
    display.print("Relay ");
    display.print(i + 1);
    display.print(" ON: ");
    display.print(relayConfigs[i].onHour);
    display.print(":");
    display.println(relayConfigs[i].onMinute);

    display.print("Relay ");
    display.print(i + 1);
    display.print(" OFF: ");
    display.print(relayConfigs[i].offHour);
    display.print(":");
    display.println(relayConfigs[i].offMinute);

    if (timeinfo->tm_hour == relayConfigs[i].onHour && timeinfo->tm_min == relayConfigs[i].onMinute) {
      digitalWrite(relayPins[i], HIGH);
      relayConfigs[i].state = true;
      Serial.print("Relay ");
      Serial.print(i + 1);
      Serial.println(" ON");
    } 
    else if (timeinfo->tm_hour == relayConfigs[i].offHour && timeinfo->tm_min == relayConfigs[i].offMinute) {
      digitalWrite(relayPins[i], LOW);
      relayConfigs[i].state = false;
      Serial.print("Relay ");
      Serial.print(i + 1);
      Serial.println(" OFF");
    }
  }

  display.display();
  delay(1000);
}
