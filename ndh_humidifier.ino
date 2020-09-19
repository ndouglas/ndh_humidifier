#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP_EEPROM.h>
#include <DHT.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>

// Includes:
//  - WIFI_SSID
//  - WIFI_PASS
//  - OTA_PASSWORD
#include "secrets.h";

// DHT22
#define DHT_PIN D7
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

float temperature;
float humidity;
unsigned long lastSampled = millis();

// The pin that rocks the relay...
// is the pin that rules the world.
#define RELAY_PIN D6

// Web Server
AsyncWebServer webServer(80);

// EEPROM
struct eepromStruct {
  float humidityLowerBound; // 4 bytes
  float humidityUpperBound; // 4 bytes
  int unused0; // 4 bytes
  int unused1; // 4 bytes
} settings;

void setup() {

  // Serial.
  Serial.begin(115200);
  Serial.println();

  // EEPROM emulation.
  EEPROM.begin(sizeof(eepromStruct));

  // Settings.
  settings.humidityLowerBound = 75.0;
  settings.humidityUpperBound = 90.0;
  settings.unused0 = 0;
  settings.unused1 = 1;

  // Read from EEPROM if values have been set there.
  if (EEPROM.percentUsed() >= 0) {
    EEPROM.get(0, settings);
  }

  // Relay Pin.
  pinMode(RELAY_PIN, OUTPUT);

  // DHT22.
  dht.begin();

  // WiFi.
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { 
    Serial.print(".");
    delay(100); 
  }
 
  // WiFi Connected!
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  // HTTP GET /
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument data(1024);
    data["temperature"] = String(temperature, 3);
    data["humidity"] = String(humidity, 3);
    data["settings"]["humidityLowerBound"] = String(settings.humidityLowerBound, 3);
    data["settings"]["humidityUpperBound"] = String(settings.humidityUpperBound, 3);
    serializeJsonPretty(data, *response);
    request->send(response);
  });

  // HTTP GET /temperature
  webServer.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(temperature, 3));
  });

  // HTTP GET /humidity
  webServer.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(humidity, 3));
  });

  // HTTP GET /relay
  webServer.on("/relay", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", digitalRead(RELAY_PIN) == HIGH ? "true" : "false");
  });

  // HTTP GET /settings/humidityLowerBound
  webServer.on("/settings/humidityLowerBound", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(settings.humidityLowerBound, 3));
  });

  // HTTP GET /settings/humidityUpperBound
  webServer.on("/settings/humidityUpperBound", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(settings.humidityUpperBound, 3));
  });

  // HTTP POST /settings/humidityLowerBound
  webServer.on("/settings/humidityLowerBound", HTTP_POST, [](AsyncWebServerRequest *request) {
    if(request->hasParam("value", true)) {
      AsyncWebParameter* value = request->getParam("value", true);
      settings.humidityLowerBound = value->value().toFloat();
      EEPROM.put(0, settings);
      EEPROM.commitReset();
    }
    request->send(200, "text/plain", String(settings.humidityLowerBound, 3));
  });

  // HTTP POST /settings/humidityUpperBound
  webServer.on("/settings/humidityUpperBound", HTTP_POST, [](AsyncWebServerRequest *request) {
    if(request->hasParam("value", true)) {
      AsyncWebParameter* value = request->getParam("value", true);
      settings.humidityUpperBound = value->value().toFloat();
      EEPROM.put(0, settings);
      EEPROM.commitReset();
    }
    request->send(200, "text/plain", String(settings.humidityUpperBound, 3));
  });

  // Web Server.
  webServer.begin();

  // Arduino OTA.
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  
}

void loop() {
  if (millis() - lastSampled > 2000) {
    humidity = dht.readHumidity();
    temperature = dht.readTemperature(true);
    while (isnan(humidity)) {
      Serial.print('.');
      humidity = dht.readHumidity();
      delay(250);
    }
    lastSampled = millis();
    Serial.print("Humidity: ");
    Serial.println(String(humidity, 3));
    if (humidity > settings.humidityUpperBound) {
      digitalWrite(RELAY_PIN, LOW);
    }
    else if (humidity < settings.humidityLowerBound) {
      digitalWrite(RELAY_PIN, HIGH);
    }
  }
  ArduinoOTA.handle();
}
