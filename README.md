# 🛰️ Aegis - Radar IoT de Limpeza Espacial

Este repositório contém a maquete física de IoT desenvolvida para o projeto **Aegis**, uma plataforma de monitoramento e interceptação de lixo espacial. 

Este módulo atua como o "cérebro" embarcado dos Drones de Limpeza, responsável por ler a distância dos detritos, atuar fisicamente para captura e enviar a telemetria em tempo real para a Terra via MQTT.

## 👨‍🚀 Integrantes (Equipe)
* **João Pedro Camilo** - RM 562005
* **Lucas Matsubara** - RM 565020
* **Pamella Christiny** - RM 565206

---

## 📐 Arquitetura de Hardware
O circuito foi prototipado utilizando o simulador Wokwi com os seguintes componentes:
- **Microcontrolador:** ESP32 (Conexão Wi-Fi)
- **Sensor de Proximidade (Entrada 1):** HC-SR04 (Ultrassom). *Nota Arquitetural: Utilizado para simular a medição de Tempo de Voo (Time of Flight). Em um cenário espacial real no vácuo, este componente representa um sensor LIDAR.*
- **Acionamento de Emergência (Entrada 2):** Pushbutton.
- **Alerta de Risco (Saída 1):** LED Vermelho.
- **Rede de Captura (Saída 2):** Servomotor.
- **Interface Local:** Display OLED SSD1306 (Exibe status do radar e distância).

## 📡 Comunicação e Nuvem
A comunicação bidirecional ocorre através do broker **HiveMQ Cloud** em porta segura (8883 - TLS).
Foram estruturados três tópicos MQTT principais:
1. `aegis/drone/telemetria` (Publicação de JSON com distância e coordenadas dinâmicas X, Y)
2. `aegis/drone/status` (Publicação de alertas textuais de operação)
3. `aegis/comando/rede` (Assinatura para receber ordens de disparo manual a partir do Node-RED)

Os dados são consumidos por um painel **Node-RED**, que exibe um Dashboard para controle da missão e persiste as informações automaticamente em tabelas (LOG_COLISAO e LOG_STATUS_DRONE) em um banco de dados MySQL hospedado no **Clever Cloud**.

## 🚀 Como Executar
1. Acesse o projeto no Wokwi através do link: `[CLIQUE AQUI](https://wokwi.com/projects/465311269133858817)`
2. Dê o Play na simulação.
3. Clique no sensor HC-SR04 e deslize a barra de distância.
4. Distâncias menores que 300m acionam o alerta luminoso (LED). Menores que 50m disparam a captura autônoma (Servo).

🎥 **Link do Vídeo Pitch:** `[COLE O LINK DO SEU VÍDEO DO YOUTUBE AQUI]`
