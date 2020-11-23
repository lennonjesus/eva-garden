#include <Arduino.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <WiFi.h>

#include "credentials.h" // ATTENTION: look at credentials.sample.h file

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish potOneSoilMoisturePub = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/potOneSoilMoisture", MQTT_QOS_1);
Adafruit_MQTT_Publish potTwoSoilMoisturePub = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/potTwoSoilMoisture", MQTT_QOS_1);
Adafruit_MQTT_Publish potThreeSoilMoisturePub = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/potThreeSoilMoisture", MQTT_QOS_1);
Adafruit_MQTT_Publish uvValuePub = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/uvValue", MQTT_QOS_1);
Adafruit_MQTT_Publish uvIndexPub = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/uvIndex", MQTT_QOS_1);

#define POT_PWR_PIN     21
#define POT_ONE_APIN    33
#define POT_TWO_APIN    34
#define POT_THREE_APIN  35

#define UV_APIN         32

int potOneSoilMoisture = 0;
int potTwoSoilMoisture = 0;
int potThreeSoilMoisture = 0;

float uvValue;
int uvIndex;

void readSoilMoisture();
void readSensorUV();
void connectWiFi();
void checkWifi();
void connectMqtt();

void setup() {

  Serial.begin(115200);

  delay(1000);

  pinMode(POT_PWR_PIN, OUTPUT);

  connectWiFi();
}

void loop() {

  checkWifi();

  connectMqtt();
  mqtt.processPackets(5000);

  readSoilMoisture();

  delay(2000);

  readSensorUV();

  delay(2000);
  
}

void readSoilMoisture() {
  digitalWrite(POT_PWR_PIN, HIGH);
  delay(1000);

  potOneSoilMoisture = 100 - (analogRead(POT_ONE_APIN) * 100 ) / 4095;
  delay(20);

  potTwoSoilMoisture = 100 - (analogRead(POT_TWO_APIN) * 100 ) / 4095;
  delay(20);
  
  potThreeSoilMoisture = 100 - (analogRead(POT_THREE_APIN) * 100 ) / 4095;
  delay(20);

  Serial.println("Vaso 1: " + (String) potOneSoilMoisture);
  Serial.println("Vaso 2: " + (String) potTwoSoilMoisture);
  Serial.println("Vaso 3: " + (String) potThreeSoilMoisture);

  delay(500);
  digitalWrite(POT_PWR_PIN, LOW);

  if (! potOneSoilMoisturePub.publish(potOneSoilMoisture)) {
    delay(10);
    Serial.println("Falha ao enviar humidade do solo vaso 1.");
    delay(10);
  }

  delay(5000);

  if (! potTwoSoilMoisturePub.publish(potTwoSoilMoisture)) {
    delay(10);
    Serial.println("Falha ao enviar humidade do solo vaso 2.");
    delay(10);
  }

  delay(5000);

  if (! potThreeSoilMoisturePub.publish(potThreeSoilMoisture)) {
    delay(10);
    Serial.println("Falha ao enviar humidade do solo vaso 3.");
    delay(10);
  }

  delay(1000);
}

void readSensorUV() {

  delay(500);

  byte numOfReadings = 10;
  uvValue = 0.0;
  
  for (int idx = 0; idx < numOfReadings; idx++) {
    uvValue += analogRead(UV_APIN);
    delay (200);
  }
  
  uvValue /= numOfReadings;
  uvValue = (uvValue * (3.3 / 1023.0)) * 1000.0;
  
  Serial.println("UV: " + (String) uvValue);

  if (! uvValuePub.publish(uvValue)) {
    Serial.println("Falha ao enviar valor UV.");
  }
  
  if (uvValue < 227) uvIndex = 0;
  else if (227 <= uvValue && uvValue < 318) uvIndex = 1;
  else if (318 <= uvValue && uvValue < 408) uvIndex = 2;
  else if (408 <= uvValue && uvValue < 503) uvIndex = 3;
  else if (503 <= uvValue && uvValue < 606) uvIndex = 4;    
  else if (606 <= uvValue && uvValue < 696) uvIndex = 5;
  else if (696 <= uvValue && uvValue < 795) uvIndex = 6;
  else if (795 <= uvValue && uvValue < 881) uvIndex = 7; 
  else if (881 <= uvValue && uvValue < 976) uvIndex = 8;
  else if (976 <= uvValue && uvValue < 1079) uvIndex = 9;  
  else if (1079 <= uvValue && uvValue < 1170) uvIndex = 10;
  else uvIndex = 11;

  Serial.println("UV Index: " + (String) uvIndex);

  delay(1000);

  if (! uvIndexPub.publish(uvIndex)) {
    Serial.println("Falha ao enviar índice UV.");
  }
  
}

void connectMqtt() {
  int8_t ret;
 
  if (mqtt.connected()) {
    return;
  }
 
  Serial.println("Conectando-se ao broker mqtt...");
 
  uint8_t num_tentativas = 5;
  
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Falha ao se conectar. Tentando se reconectar em 5 segundos.");
    mqtt.disconnect();
    delay(5000);
    num_tentativas--;
    if (num_tentativas == 0) {
      Serial.println("Seu ESP será resetado.");
      ESP.restart();
    }
  }
 
  Serial.println("Conectado ao broker com sucesso.");
}

void checkWifi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(" Conexão não estabelecida. Reiniciando...");
    ESP.restart();
  } 
}

void connectWiFi() {

  delay(100);
  
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Conectando em ");
  Serial.print(WIFI_SSID);

  int timeout = 0;

  while (WiFi.status() != WL_CONNECTED && ++timeout <= 10) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    Serial.print("."); 
  }

  checkWifi();

  Serial.println("");
  Serial.print("Conectado em ");
  Serial.print(WIFI_SSID);
  Serial.print(" com o IP ");
  Serial.println(WiFi.localIP());
}
