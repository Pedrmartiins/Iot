#include "ESP8266WiFi.h"
#include <PubSubClient.h>

// Protótipos de métodos que serão definidos mais adiante
// estabelece conexão com o broker e retorna situação
bool connectMQTT();

// recebe as publicações do tópico informado no método e retorna
// via UART, aquilo que foi lido via subscrição.
void callback(char *topic, byte * payload, unsigned int length);

// Parâmetros de conexão WiFi
const char* ssid = "linksys"; // REDE
const char* password = ""; // SENHA

// Parâmetros de conexão ao MQTT Broker
const char* mqtt_broker = "test.mosquitto.org"; // host do broker
const char* topic = "MeuTopico/teste_topico"; // nome do tópico para assinatura e publicação
const char* mqtt_username = ""; // Usuário não necessário no host test.mosquitto.org
const char* mqtt_password = ""; // Senha Usuário não necessário no host test.mosquitto.org
const int mqtt_port = 1883; // Porta

// Variável de status de conexão MQTT
bool mqttStatus = 0;

// Objetos
WiFiClient espClient; // objeto responsável pela conexão WiFi
PubSubClient client(espClient); // Objeto responsável pela conexão com broker

// Definição do Setup
void setup(void)
{
    Serial.begin(9600); // Inicia conexão com monitor serial
    Serial.print("\n>>>> Iniciando Conexão...\n"); // Avisa início de conexão
    WiFi.begin(ssid, password); // Inicia conexão com WiFi

    // Inicia looping de tentativa de conexão ao serviço de WiFi
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\n\nWiFi conectada!!\n\t");
    Serial.print(WiFi.localIP()); // Envia IP através da UART
    mqttStatus = connectMQTT(); // Chama conexão MQTT com broker e retorna status
}

// Definição do looping do ESP
void loop()
{
    static long long pooling = 0; // define intervalo de pooling

    if (mqttStatus) { // Verifica se houve conexão
        client.loop();

        if (millis() > pooling + 5000) { // A cada período de 5 segundos publica info
            pooling = millis();
            client.publish(topic, "teste123,113007402022");
        }
    }
}

// Implementação do método de conexão com o broker
bool connectMQTT() {
    byte tentativa = 0; // variável byte que controla o número de tentativas de conexão
    client.setServer(mqtt_broker, mqtt_port); // chama método setServer passando url e porta do broker
    client.setCallback(callback); // Informa ao objeto client qual método deve ser chamado quando houver

    do {
        // Define o ID do cliente (a própria placa ESP)
        String client_id = "ESP-"; // Que usa o prefixo ESP-
        client_id += String(WiFi.macAddress()); // Concatenado com seu respectivo MAC address

        // O if tenta estabelecer a conexão com o broker
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            // Com sucesso da conexão, informa os dados do cliente (a placa)
            Serial.println("Exito na conexão");
            Serial.printf("Cliente %s conectado ao broker\n", client_id.c_str());

        } else { // Informa falha na conexão e aguarda 2 segundos para nova tentativa
            Serial.println("Falha ao conectar:");
            Serial.print(client.state());
            Serial.print("\nTentativa: ");
            Serial.println(tentativa);
            delay(2000);
        }
        tentativa++; // Incrementa número de tentativas
    } while (!client.connected() && tentativa < 5); // Limita número de tentativas

    if (tentativa < 5) {
        // Conexão realizada com sucesso
        client.publish(topic, "teste123,113007402022"); // Uma mensagem é publicada
        client.subscribe(topic); // Se inscreve no broker para receber mensagens
        return 1; // Retorna 1 com sucesso
    } else {
        // Caso contrário avisa falha e retorna 0
        Serial.println("Conexão falhou");
        return 0; // informa falha na conexão
    }
}

// Este método é chamado quando o client identifica nova mensagem no broker
void callback(char *topic, byte * payload, unsigned int length) {
    // char *topic identifica o tópico registrado
    // byte *payload conjunto de bytes que foram publicados
    // int length é o tamanho do vetor de bytes do payload

    Serial.print("Mensagem chegou no tópico: ");
    Serial.println(topic);
    Serial.print("Mensagem: ");

    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }

    Serial.println();
    Serial.println("----------------------");
}
