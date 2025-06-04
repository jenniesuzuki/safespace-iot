#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ThingSpeak.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define RELAY 19
#define PULSE_PIN 35
#define alertLed 32

// WiFi e MQTT
const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* mqttTopic = "iot/esp32/heart";
const char* clientNameMqtt = "monitorBatimento";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int minHeartRate = 50;
int maxHeartRate = 120;

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

void connectToMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqttClient.connect(clientNameMqtt)) {
      mqttClient.subscribe(mqttTopic);
      Serial.println("Connected");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 2s");
      delay(2000);
    }
  }
}

void setup() {
  Wire.begin();
  Serial.begin(115200);

 $0
  pinMode(alertLed, OUTPUT);
  pinMode(RELAY, OUTPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 30);
  display.println("Heart Rate Monitor");
  display.display();

  connectToWiFi();
  mqttClient.setServer(mqttServer, mqttPort);
  ThingSpeak.begin(espClient);
}

void loop() {
  if (!mqttClient.connected()) {
    connectToMQTT();
  }
  mqttClient.loop();

  int pulseValue = analogRead(PULSE_PIN);
  float voltage = pulseValue * (3.3 / 4095.0);
  int heartRate = (voltage / 3.3) * 675;

  String jsonPayload = "{\"heartRate\":" + String(heartRate) + "}";
  mqttClient.publish(mqttTopic, jsonPayload.c_str());

  if (heartRate < minHeartRate) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Heart Rate LOW");
    display.setCursor(0, 20);
    display.print(String(heartRate) + " bpm");
    display.display();
    digitalWrite(RELAY, LOW);
    digitalWrite(alertLed, LOW);
  } else if (heartRate > maxHeartRate) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Heart Rate HIGH");
    display.setCursor(0, 20);
    display.print(String(heartRate) + " bpm");
    display.display();
    digitalWrite(alertLed, HIGH);
    digitalWrite(RELAY, HIGH);
  } else {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Heart Rate Normal");
    display.setCursor(0, 20);
$0
    display.print(String(heartRate) + " bpm");
    display.display();
    digitalWrite(RELAY, LOW);
    digitalWrite(alertLed, LOW);
  }

  delay(2000);
}
