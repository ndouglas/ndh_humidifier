#include <ESP8266WiFi.h>
#include <DHT.h>

// Includes WIFI_SSID and WIFI_PASS.
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
#define RELAY_LOWER_BOUND 75.0
#define RELAY_UPPER_BOUND 90.0

void setup() {

  // Serial.
  Serial.begin(115200);
  Serial.println();

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

}

// the loop function runs over and over again forever
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
    if (humidity > RELAY_UPPER_BOUND) {
      Serial.println("Turning off relay...");
      digitalWrite(RELAY_PIN, LOW);
    }
    else if (humidity < RELAY_LOWER_BOUND) {
      Serial.println("Turning on relay...");
      digitalWrite(RELAY_PIN, HIGH);
    }
  }
}
