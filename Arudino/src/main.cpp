//mosquitto_sub -h localhost -u weatheruser -P  Datait2025! -t "vejrstation/data"

#include <BME280I2C.h>
#include <Wire.h>
#include <WiFiS3.h>
#include <PubSubClient.h>

const char* ssid = "h4prog";
const char* password = "1234567890";

const char* mqtt_server = "192.168.106.11"; 
const int mqtt_port = 1883;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

#define SERIAL_BAUD 115200
BME280I2C bme;

void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("BME280Client", "weatheruser", "Datait2025!")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  while (!Serial) {}

  Wire.begin();

  if (!bme.begin()) {
    Serial.println("Could not find BME280 sensor!");
    while (1);
  }

  switch (bme.chipModel()) {
    case BME280::ChipModel_BME280:
      Serial.println("Found BME280 sensor! Success.");
      break;
    case BME280::ChipModel_BMP280:
      Serial.println("Found BMP280 sensor! No Humidity available.");
      break;
    default:
      Serial.println("Found UNKNOWN sensor! Error!");
  }

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();

  float temp(NAN), hum(NAN), pres(NAN);

  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_hPa);

  bme.read(pres, temp, hum, tempUnit, presUnit);

  // Build JSON message
  char payload[128];
  snprintf(payload, sizeof(payload),
           "{\"temp\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f}",
           temp, hum, pres);

  // Publish to MQTT
  client.publish("vejrstation/data", payload);

  // Also print to Serial
  Serial.println(payload);

  delay(5000);
}
