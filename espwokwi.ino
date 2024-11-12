#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN 8         // PIN untuk sensor DHT
#define DHTTYPE DHT22     // Tipe DHT (DHT11 atau DHT22)

#define LED_HIJAU 5      // PIN LED Hijau
#define LED_KUNING 10     // PIN LED Kuning
#define LED_MERAH 12      // PIN LED Merah
#define RELAY_PIN 7      // PIN Relay
#define BUZZER 9          // PIN Buzzer

const char* ssid = "Wokwi-GUEST";           // SSID WiFi
const char* password = "";                  // Password WiFi
const char* mqtt_server = "broker.hivemq.com"; // Broker MQTT

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);

  // Inisialisasi DHT sensor
  dht.begin();

  // Inisialisasi pin
  pinMode(LED_HIJAU, OUTPUT);
  pinMode(LED_KUNING, OUTPUT);
  pinMode(LED_MERAH, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi.");
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Connected to MQTT broker.");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Baca suhu dan kelembapan dari DHT
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Jika data valid, kirimkan melalui MQTT dengan topik 'uts'
  if (!isnan(h) && !isnan(t)) {
    String statusPompa;
    if (t > 35) {
      // Kondisi untuk suhu > 35 °C
      digitalWrite(LED_MERAH, HIGH);
      digitalWrite(RELAY_PIN, HIGH);   // Pompa ON
      digitalWrite(BUZZER, HIGH);
      digitalWrite(LED_HIJAU, LOW);
      digitalWrite(LED_KUNING, LOW);
      statusPompa = "ON";
    } else if (t > 30 && t <= 35) {
      // Kondisi untuk suhu antara 30 dan 35 °C
      digitalWrite(LED_KUNING, HIGH);
      digitalWrite(RELAY_PIN, LOW);   // Pompa OFF
      digitalWrite(BUZZER, LOW);
      digitalWrite(LED_HIJAU, LOW);
      digitalWrite(LED_MERAH, LOW);
      statusPompa = "OFF";
    } else {
      // Kondisi untuk suhu < 30 °C
      digitalWrite(LED_HIJAU, HIGH);
      digitalWrite(RELAY_PIN, LOW);   // Pompa OFF
      digitalWrite(BUZZER, LOW);
      digitalWrite(LED_KUNING, LOW);
      digitalWrite(LED_MERAH, LOW);
      statusPompa = "OFF";
    }

    // Buat payload MQTT yang mengandung suhu, kelembapan, dan status pompa
    String payload = "Temperature: " + String(t) + " °C, Humidity: " + String(h) + " %, Pump: " + statusPompa;

    client.publish("uts", payload.c_str());
  }
  delay(2000);  // Kirim data setiap 2 detik
}
