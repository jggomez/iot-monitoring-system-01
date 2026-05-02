#include <WiFi.h>
#include <PubSubClient.h> // Librería para MQTT
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include "secrets.h"

const int mqtt_port = 1883;
const char* topic = "room1/sensors";

#define dhtpin 14
#define dhttype DHT22

DHT dht(dhtpin, dhttype);
LiquidCrystal_I2C lcd(0x27, 16, 2);

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);  
  dht.begin();
  lcd.init();
  lcd.backlight();

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void setup_wifi() {
  delay(10);
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado - IP: " + WiFi.localIP().toString());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("conectado");
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" reintentando en 5 segundos");
      delay(5000);
    }
  }
}

void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Error leyendo el sensor DHT");
    return;
  }

  showLCD("Humidity", h, "%");
  delay(2000);
  showLCD("Temperature", t, "C");

  sendMQTT(t, h);

  delay(2000);
}

void sendMQTT(float temp, float hum) {
  
  String jsonPayload = "{\"temperature\": " + String(temp) + ", \"humidity\": " + String(hum) + "}";
  
  Serial.print("Publish in topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(jsonPayload);

  if (client.publish(topic, jsonPayload.c_str())) {
    Serial.println("Publish succesfull");
  } else {
    Serial.println("Error publishing");
  }
}

void showLCD(String label, float valor, String unidad) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(label);
  lcd.setCursor(5, 1);
  lcd.print(valor);
  lcd.print(unidad);
}
