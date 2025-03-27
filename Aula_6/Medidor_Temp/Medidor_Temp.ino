#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "linksys";
const char* password = ""; 

const char* apiKey = "LGLCLW8502LTIJEI";

const int sensorPin = A0; 

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  Serial.println("Conectando ao Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi conectado!");
}

void loop() {
  int sensorValue = analogRead(sensorPin); 

  float temperatura = (sensorValue * (3.3 / 1023.0)) * 100.0;  

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");

  if(WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    String url = "http://api.thingspeak.com/update?api_key=";
    url += apiKey;
    url += "&field1=" + String(temperatura);

    Serial.println("Enviando: " + url);
    
    http.begin(client, url);
    
    int httpResponseCode = http.GET();
    
    if(httpResponseCode > 0) {
      Serial.println("Resposta do servidor: " + String(httpResponseCode));
    } else {
      Serial.println("Erro ao enviar: " + http.errorToString(httpResponseCode));
    }

    http.end();
  }

  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);

  delay(15000); 
}