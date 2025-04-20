#include <TFT_eSPI.h>
#include <SPI.h>
#include <math.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite horizon = TFT_eSprite(&tft);

int pitchPin = 34;  // Pino para o potenciômetro de pitch
int rollPin = 33;   // Pino para o potenciômetro de roll
int resetPin = 32;  // Pino para o botão de reset
float pitch = 0;
float roll = 0;
float pitchZero = 0;  // Ponto inicial de pitch (quando o botão for pressionado)
float rollZero = 0;   // Ponto inicial de roll (quando o botão for pressionado)
unsigned long lastUpdate = 0;
bool buttonPressed = false; // Variável para verificar o estado do botão
unsigned long buttonPressTime = 0; // Tempo que o botão foi pressionado

void setup() {
  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);

  horizon.setColorDepth(8);
  horizon.createSprite(240, 240);

  pinMode(pitchPin, INPUT);
  pinMode(rollPin, INPUT);
  pinMode(resetPin, INPUT_PULLUP);  // O botão será configurado com um resistor de pull-up interno
}

void loop() {
  if (millis() - lastUpdate > 50) {
    lastUpdate = millis();

    // Se o botão for pressionado
    if (digitalRead(resetPin) == LOW) {
      if (!buttonPressed) {
        // Marcar que o botão foi pressionado e registrar o tempo
        buttonPressed = true;
        buttonPressTime = millis();
        
        // Define o valor de pitch e roll como o ponto de zero ao iniciar o pressionamento
        pitchZero = map(analogRead(pitchPin), 0, 4095, -90, 90);  
        rollZero = map(analogRead(rollPin), 0, 4095, -90, 90);    
      }
      
      // Se o botão for pressionado por mais de 3 segundos, resetar para o estado original
      if (millis() - buttonPressTime >= 3000) {
        pitchZero = 0; // Resetando para o valor inicial
        rollZero = 0;  // Resetando para o valor inicial
      }
    }

    // Se o botão for solto
    if (digitalRead(resetPin) == HIGH && buttonPressed) {
      buttonPressed = false; // Marca que o botão foi solto
    }

    // Leitura dos potenciômetros, agora com relação ao valor de zero (ponto inicial)
    pitch = map(analogRead(pitchPin), 0, 4095, -90, 90) - pitchZero;
    roll = map(analogRead(rollPin), 0, 4095, -90, 90) - rollZero;

    drawHorizon(pitch, roll);  // Atualiza a tela
  }
}

void drawHorizon(float pitch, float roll) {
  int cx = 120;
  int cy = 120;

  float rollRad = roll * DEG_TO_RAD;
  float pitchOffset = pitch * 2.0;

  horizon.fillSprite(TFT_BLACK);

  // Céu e solo
  for (int y = 0; y < 240; y++) {
    for (int x = 0; x < 240; x++) {
      float dx = x - cx;
      float dy = y - cy;

      float rotX = cos(rollRad) * dx - sin(rollRad) * (dy + pitchOffset);
      float rotY = sin(rollRad) * dx + cos(rollRad) * (dy + pitchOffset);

      uint16_t color = (rotY < 0) ? TFT_BLUE : TFT_BROWN;
      horizon.drawPixel(x, y, color);
    }
  }

  // Pitch marks (mais espaçados e sem o central)
  for (int i = -3; i <= 3; i++) {
    if (i == 0) continue;

    float offset = i * 20;
    drawPitchLine(cx, cy, offset, rollRad);
  }

  // Indicador superior (triângulo)
  horizon.fillTriangle(cx, cy - 119, cx - 6, cy - 109, cx + 6, cy - 109, TFT_WHITE);

  // "V" central simulando o bico do avião
  int vSize = 10;
  int lineY = cy + vSize;

  // Desenha o V
  horizon.drawLine(cx - vSize, cy, cx, lineY, TFT_WHITE);
  horizon.drawLine(cx + vSize, cy, cx, lineY, TFT_WHITE);

  // Linha horizontal partindo das pontas do V até a borda
  horizon.drawLine(20, cy, cx - vSize, cy, TFT_WHITE);  // Esquerda
  horizon.drawLine(cx + vSize, cy, 220, cy, TFT_WHITE); // Direita

  // Moldura circular
  horizon.drawCircle(cx, cy, 119, TFT_WHITE);

  // Marcação de ângulos na moldura
  for (int angle = -60; angle <= 60; angle += 30) {
    float rad = angle * DEG_TO_RAD;
    int x1 = cx + cos(rad - HALF_PI) * 110;
    int y1 = cy + sin(rad - HALF_PI) * 110;
    int x2 = cx + cos(rad - HALF_PI) * 119;
    int y2 = cy + sin(rad - HALF_PI) * 119;
    horizon.drawLine(x1, y1, x2, y2, TFT_WHITE);
  }

  horizon.pushSprite(0, 0);
}

// Desenha uma linha horizontal de pitch com rotação
void drawPitchLine(int cx, int cy, float offset, float rollRad) {
  float length = 40;

  float x1 = -length / 2;
  float y1 = offset;

  float x2 = length / 2;
  float y2 = offset;

  int x1r = cx + cos(rollRad) * x1 - sin(rollRad) * y1;
  int y1r = cy + sin(rollRad) * x1 + cos(rollRad) * y1;

  int x2r = cx + cos(rollRad) * x2 - sin(rollRad) * y2;
  int y2r = cy + sin(rollRad) * x2 + cos(rollRad) * y2;

  horizon.drawLine(x1r, y1r, x2r, y2r, TFT_WHITE);
}
