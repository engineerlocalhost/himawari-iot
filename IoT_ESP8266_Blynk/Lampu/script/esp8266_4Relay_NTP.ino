#define BLYNK_TEMPLATE_ID "";
#define BLYNK_TEMPLATE_NAME "N";
#define BLYNK_AUTH_TOKEN "";

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// WiFi credentials
const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";

// Blynk credentials
char auth[] = "Your_Blynk_Auth_Token";

// NTP Server configuration
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 25200; // Adjust for WIB (GMT+7)
const int daylightOffset_sec = 0;

// Relay and manual switch pins
const int relayPins[4] = {5, 4, 14, 12};         // Pin untuk relay
const int manualSwitchPins[4] = {13, 0, 2, 15};  // Pin untuk saklar manual

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Relay configuration structure
struct RelayConfig {
  int onHour;
  int onMinute;
  int offHour;
  int offMinute;
  bool state;           // Status relay
  bool manualOverride;  // Status manual override
};

RelayConfig relayConfigs[4] = {
  {21, 0, 21, 1, false, false},   // Relay 1
  {21, 5, 21, 6, false, false},  // Relay 2
  {21, 10, 21, 11, false, false},// Relay 3
  {21, 15, 21, 16, false, false} // Relay 4
};

// NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, 60000);

// Blynk virtual pins for controlling relay states
BLYNK_WRITE(V1) {
  relayConfigs[0].state = param.asInt();
  digitalWrite(relayPins[0], relayConfigs[0].state ? HIGH : LOW);
}

BLYNK_WRITE(V2) {
  relayConfigs[1].state = param.asInt();
  digitalWrite(relayPins[1], relayConfigs[1].state ? HIGH : LOW);
}

BLYNK_WRITE(V3) {
  relayConfigs[2].state = param.asInt();
  digitalWrite(relayPins[2], relayConfigs[2].state ? HIGH : LOW);
}

BLYNK_WRITE(V4) {
  relayConfigs[3].state = param.asInt();
  digitalWrite(relayPins[3], relayConfigs[3].state ? HIGH : LOW);
}

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

  Blynk.begin(auth, ssid, password);

  for (int i = 0; i < 4; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], LOW);

    pinMode(manualSwitchPins[i], INPUT_PULLUP); // Saklar manual sebagai input
  }

  timeClient.begin();
  timeClient.update();
}

void loop() {
  timeClient.update();
  Blynk.run();

  time_t currentTime = timeClient.getEpochTime();
  struct tm *timeinfo = localtime(&currentTime);

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

    // Check manual switch state
    bool manualState = !digitalRead(manualSwitchPins[i]); // Saklar aktif jika low
    if (manualState) {
      digitalWrite(relayPins[i], HIGH);
      relayConfigs[i].manualOverride = true; // Aktifkan manual override
      Serial.print("Relay ");
      Serial.print(i + 1);
      Serial.println(" MANUAL ON");
    } else if (relayConfigs[i].manualOverride) {
      digitalWrite(relayPins[i], LOW);
      relayConfigs[i].manualOverride = false; // Nonaktifkan manual override
      Serial.print("Relay ");
      Serial.print(i + 1);
      Serial.println(" MANUAL OFF");
    }

    // Automatic schedule control
    if (!relayConfigs[i].manualOverride) { // Hanya jika tidak dalam manual override
      if (timeinfo->tm_hour == relayConfigs[i].onHour && timeinfo->tm_min == relayConfigs[i].onMinute) {
        digitalWrite(relayPins[i], HIGH);
        relayConfigs[i].state = true;
        Serial.print("Relay ");
        Serial.print(i + 1);
        Serial.println(" AUTO ON");
      } else if (timeinfo->tm_hour == relayConfigs[i].offHour && timeinfo->tm_min == relayConfigs[i].offMinute) {
        digitalWrite(relayPins[i], LOW);
        relayConfigs[i].state = false;
        Serial.print("Relay ");
        Serial.print(i + 1);
        Serial.println(" AUTO OFF");
      }
    }
  }

  display.display();
  delay(1000);
}
