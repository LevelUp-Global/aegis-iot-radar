#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- Configurações Wi-Fi e MQTT (HiveMQ Cloud) ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "ce918f0e6272473fa7f0a26b9b481604.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "RM562005";
const char* mqtt_password = "Jp281801_-";

WiFiClientSecure espClient;
PubSubClient client(espClient);

// --- Definições de Pinos e Componentes ---
const int trigPin = 5;      // Emissor do Radar (Ultrassom)
const int echoPin = 18;     // Receptor do Radar (Ultrassom)
const int btnPin = 12;      // Disparo Manual
const int ledPin = 2;       // LED de Alerta
const int servoPin = 15;    // Servo (Rede de Captura)

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Servo redeCaptura;

// Variáveis de Coordenadas
int coordX = 154;
int coordY = 87;

int distanciaAlvo = 0;
unsigned long lastMsg = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando na rede: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
}

void callback(char* topic, byte* payload, unsigned int length) {
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }
  
  if (String(topic) == "aegis/comando/rede" && messageTemp == "DISPARAR") {
    Serial.println("Comando remoto recebido! Lançando rede...");
    redeCaptura.write(90);
    client.publish("aegis/drone/status", "Captura Remota Executada!");
    delay(3000);
    redeCaptura.write(0);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    if (client.connect("ESP32AegisDrone", mqtt_user, mqtt_password)) {
      Serial.println("Conectado ao HiveMQ!");
      client.subscribe("aegis/comando/rede");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(trigPin, OUTPUT); // Configura o gatilho do sensor como saída
  pinMode(echoPin, INPUT);  // Configura o eco do sensor como entrada
  pinMode(btnPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  
  redeCaptura.attach(servoPin);
  redeCaptura.write(0);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Falha ao iniciar o OLED"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("Iniciando Aegis...");
  display.display();

  espClient.setInsecure();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // 1. Disparando o pulso do Radar Ultrassônico
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // 2. Calculando o Tempo de Voo e a Distância
  long duration = pulseIn(echoPin, HIGH);
  int distanciaRealCm = duration * 0.034 / 2; 
  
  // Mapeia os 400cm do sensor do Wokwi para a nossa escala de 1000m do Dashboard
  distanciaAlvo = distanciaRealCm * 2.5; 
  if (distanciaAlvo > 1000) distanciaAlvo = 1000;

  // Atualização do Display OLED
  display.clearDisplay();
  display.setCursor(0, 10);
  display.setTextSize(1);
  display.println("Radar Aegis Ativo");
  display.setCursor(0, 30);
  display.setTextSize(2);
  display.print("Dist: ");
  display.print(distanciaAlvo);
  display.println("m");
  display.display();

  // Lógica de Colisão e Atuação Física
  if (distanciaAlvo < 300) {
    digitalWrite(ledPin, HIGH);
    if (distanciaAlvo < 50) {
       redeCaptura.write(90); 
       static bool redeLancada = false;
       if(!redeLancada){
         client.publish("aegis/drone/status", "Rede Lancada (Autonomo)");
         redeLancada = true;
       }
    }
  } else {
    digitalWrite(ledPin, LOW);
    redeCaptura.write(0); 
  }

  // Disparo Físico de Emergência (Botão)
  if (digitalRead(btnPin) == LOW) {
    redeCaptura.write(90);
    client.publish("aegis/drone/status", "Captura Manual pelo Drone!");
    delay(1000);
  }

  // 5. Publicação da Telemetria via MQTT
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    
    // Simula a deriva do lixo espacial (move até 5 unidades para qualquer lado)
    coordX += random(-5, 6); 
    coordY += random(-5, 6);

    // Evita que as coordenadas fujam para números negativos no radar
    if(coordX < 0) coordX = 0;
    if(coordY < 0) coordY = 0;

    // Monta o JSON com as variáveis dinâmicas em vez de texto fixo
    String payload = "{\"distancia\": " + String(distanciaAlvo) + ", \"coordX\": " + String(coordX) + ", \"coordY\": " + String(coordY) + "}";
    
    client.publish("aegis/drone/telemetria", payload.c_str());
  }
}