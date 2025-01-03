#define BLYNK_TEMPLATE_ID "";
#define BLYNK_TEMPLATE_NAME "N";
#define BLYNK_AUTH_TOKEN "";
/////////////////////////////////////

#define BLYNK_PRINT Serial 
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char ssid[] = ""; // Nama Wifi
char pass[] = "";  // Password Wifi
char auth[] = BLYNK_AUTH_TOKEN;

#define Relay1 5 // Pin D1 
#define Relay2 4 // Pin D2
#define Relay3 0 // Pin D3
#define Relay4 2 // Pin D4

BLYNK_WRITE(V0)
{ 
  int value1 = param.asInt();
  digitalWrite(Relay1,value1);  
}
BLYNK_WRITE(V1)
{ 
  int value2 = param.asInt();
  digitalWrite(Relay2,value2);  
}
BLYNK_WRITE(V2)
{ 
  int value3 = param.asInt();
  digitalWrite(Relay3,value3);  
}
BLYNK_WRITE(V3)
{ 
  int value4 = param.asInt();
  digitalWrite(Relay4,value4);  
}

void setup()
{
  Serial.begin(9600); 
  Blynk.begin(auth, ssid, pass);
  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  pinMode(Relay3, OUTPUT);
  pinMode(Relay4, OUTPUT);
}

void loop()
{
  Blynk.run(); // Initiates Blynk 
}