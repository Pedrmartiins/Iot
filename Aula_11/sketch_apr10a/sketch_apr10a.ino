#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>


const char* ssid = "linksys";
const char* password = "";


const char* mqttServer = "mqtt.thingspeak.com";
const int mqttPort = 1883;
const char* channelID = "2877133";
const char* writeAPIKey = "RX3YEA1UFFVGJI5R";


WiFiClient espClient;
PubSubClient client(espClient);


#define TRIG_PIN 34
#define ECHO_PIN 35


#define ONE_WIRE_BUS 15
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


#define RELE_PIN 2
const float LIMITE_NIVEL = 30.0; 


float temperaturaLocal = 0, nivelLocal = 0;
float tempGRP5 = 0, nivelGRP5 = 0;
float tempGRP6 = 0, nivelGRP6 = 0;


void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0'; // Termina a string
  String msg = String((char*)payload);

  if (String(topic) == "GRP5/TEMPERATURA") tempGRP5 = msg.toFloat();
  else if (String(topic) == "GRP6/TEMPERATURA") tempGRP6 = msg.toFloat();
  else if (String(topic) == "GRP5/NIVEL") nivelGRP5 = msg.toFloat();
  else if (String(topic) == "GRP6/NIVEL") nivelGRP6 = msg.toFloat();
}


float lerNivel() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  float duracao = pulseIn(ECHO_PIN, HIGH);
  float distancia = duracao * 0.034 / 2;
  return distancia; 
}


float lerTemperatura() {
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
}


void conectarWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("WiFi conectado.");
}


void conectarMQTT() {
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      Serial.println("MQTT conectado.");
      client.subscribe("GRP5/TEMPERATURA");
      client.subscribe("GRP5/NIVEL");
      client.subscribe("GRP6/TEMPERATURA");
      client.subscribe("GRP6/NIVEL");
    } else {
      Serial.print("Falha MQTT. Código: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void enviarDadosParaThingSpeak(float temperatura, float nivel) {
  String payload = "field1=" + String(temperatura, 2) + "&field2=" + String(nivel, 2);
  String topic = "channels/" + String(channelID) + "/publish/" + String(writeAPIKey);

  client.publish(topic.c_str(), payload.c_str());

  Serial.println("Enviado para ThingSpeak:");
  Serial.println("Tópico: " + topic);
  Serial.println("Payload: " + payload);
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELE_PIN, OUTPUT);
  digitalWrite(RELE_PIN, LOW);

  sensors.begin();
  conectarWiFi();

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  conectarMQTT();
}

void loop() {
  if (!client.connected()) {
    conectarMQTT();
  }
  client.loop();

 
  temperaturaLocal = lerTemperatura();
  nivelLocal = lerNivel();

  
  client.publish("GRP4/TEMPERATURA", String(temperaturaLocal).c_str());
  client.publish("GRP4/NIVEL", String(nivelLocal).c_str());

  
  float temperaturaMedia = (temperaturaLocal + tempGRP5 + tempGRP6) / 3.0;
  float nivelMedio = (nivelLocal + nivelGRP5 + nivelGRP6) / 3.0;

 
  if (nivelLocal > LIMITE_NIVEL) {
    digitalWrite(RELE_PIN, HIGH); 
  } else {
    digitalWrite(RELE_PIN, LOW);  
  }

 
  enviarParaThingSpeak(temperatura, temperaturaMedia, nivel, nivelMedio);

  delay(15000);
}
