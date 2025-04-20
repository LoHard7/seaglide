#include <TFT_eSPI.h> // Biblioteca para displays TFT
#include <WiFi.h>
#include <HardwareSerial.h>

// Configuração do display TFT
TFT_eSPI tft = TFT_eSPI(240, 240); // Resolução 240x240

// Configuração do sensor ultrassônico
HardwareSerial Ultrasonic_Sensor(2); // Use UART2 (TX2: GPIO12, RX2: GPIO13)
const int pinRX = 18; // Pino RX conectado ao TX do sensor
const int pinTX = 19; // Pino TX conectado ao RX do sensor

// Buffer para o gráfico (tamanho igual à largura do display)
int graphBuffer[240] = {0};

unsigned char data_buffer[4] = {0};
float distance; // Variável para armazenar a distância
unsigned char CS; // Variável para checksum

void setup() {
  // Inicialização do monitor serial
  Serial.begin(115200);

  // Inicialização do hardware serial para o sensor
  Ultrasonic_Sensor.begin(9600, SERIAL_8N1, pinRX, pinTX);

  // Inicialização do TFT
  tft.init();
  tft.setRotation(2); // Rotação padrão
  tft.fillScreen(TFT_BLUE); // Define o fundo azul
  tft.setTextColor(TFT_WHITE, TFT_BLUE); // Cor do texto e fundo

  // Exibe mensagem inicial
  tft.setTextDatum(TC_DATUM); // Centraliza o texto
  tft.setTextSize(2);
  tft.drawString("Iniciando...", 120, 100); // Mensagem no centro
  delay(2000);
  tft.fillScreen(TFT_BLUE); // Limpa a tela para iniciar o loop com fundo azul
}

void loop() {
  // Verifica se há dados disponíveis no sensor
  if (Ultrasonic_Sensor.available() > 0) {
    delay(4);

    // Verifica o cabeçalho do pacote (0xFF)
    if (Ultrasonic_Sensor.read() == 0xFF) {
      data_buffer[0] = 0xFF;
      for (int i = 1; i < 4; i++) {
        data_buffer[i] = Ultrasonic_Sensor.read();
      }

      // Calcula o checksum
      CS = data_buffer[0] + data_buffer[1] + data_buffer[2];
      if (data_buffer[3] == CS) {
        distance = (data_buffer[1] << 8) + data_buffer[2];
        distance /= 10.0; // Converte para cm

        // Exibe no Serial
        Serial.print("Distance: ");
        if (distance > 450.0) {
          Serial.println("Out of range!");
          distance = 450.0; // Limita o valor máximo no gráfico
        } else {
          Serial.print(distance);
          Serial.println(" cm");
        }

        // Atualiza o buffer do gráfico
        int mappedDistance = map(distance, 0, 450, 31, 240);

        // Desloca os dados existentes para a esquerda
        for (int i = 0; i < 239; i++) {
          graphBuffer[i] = graphBuffer[i + 1];
        }
        graphBuffer[239] = mappedDistance; // Adiciona o novo valor na extremidade direita

        // Atualiza o display TFT
        tft.fillRect(0, 0, 240, 30, TFT_BLUE); // Limpa a área superior com azul
        tft.setTextDatum(TC_DATUM); // Centraliza o texto
        tft.setTextSize(2);
        tft.drawString(String(distance, 1) + " cm", 120, 10); // Exibe o valor numérico no topo

        // Desenha o gráfico
        tft.fillRect(0, 31, 240, 209, TFT_BLUE); // Limpa a área do gráfico com azul
        for (int x = 0; x < 239; x++) {
          tft.drawLine(x, graphBuffer[x], x + 1, graphBuffer[x + 1], TFT_GREEN);
        }
      }
    }
  }
}
