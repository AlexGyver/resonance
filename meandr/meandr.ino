// скетч генератора меандра с олед дисплеем

#define CLK 4		// пин энкодера CLK (s1)
#define DT 5		// пин энкодера DT (s2)
#define SW 3		// пин энкодера SW (key)
#define ENC_VCC 2	// пин энкодера ENC_VCC
#define ENC_GND 6	// пин энкодера ENC_GND

// дисплей подключен к питанию GND-VCC, пин SDA->A4, SCL->A5

#define USE_MICRO_WIRE

#include <GyverOLED.h>
GyverOLED oled;

#include <GyverEncoder.h>
Encoder enc(CLK, DT, SW);

float freq = 800;

void setup() {
  enc.setType(TYPE2);

  pinMode(ENC_VCC, 1);
  pinMode(ENC_GND, 1);
  digitalWrite(ENC_VCC, 1);
  digitalWrite(ENC_GND, 0);

  pinMode(9, 1);
  pinMode(10, 1);

  oled.init(OLED128x32);
  oled.clear();
  oled.scale2X();
  oled.setCursor(2, 2);
  oled.print(freq);
  oled.print(" hz ");

  TCCR1A = 0b01010000;
  TCCR1B = 0b00001000;
  TCCR1C = 0b01000000;

  freq = setSquare(freq);
}

void loop() {
  enc.tick();
  if (enc.isTurn()) {     // если был совершён поворот (индикатор поворота в любую сторону)
    if (enc.isRight()) freq += 0.05;
    if (enc.isLeft()) freq -= 0.05;
    if (enc.isFastR()) freq += 1.0;
    if (enc.isFastL()) freq -= 1.0;
    if (enc.isRightH()) freq += 5.0;
    if (enc.isLeftH()) freq -= 5.0;
    
    oled.setCursor(2, 2);
    oled.print(setSquare(freq));
    oled.print(" hz  ");
  }
}

float setSquare(float frequency) {
  if (frequency < 2) frequency = 2;
  uint16_t top;
  float realF;
  if (frequency <= 16) {
    TCCR1B = 0b00001011;
    top = (float)250000L / frequency / 2 - 1;
    realF = (float)250000L / 2 / (1 + top);
  } else if (frequency <= 122) {
    TCCR1B = 0b00001010;
    top = (float)2000000L / frequency / 2 - 1;
    realF = (float)2000000L / 2 / (1 + top);
  } else {
    TCCR1B = 0b00001001;
    top = (float)16000000L / frequency / 2 - 1;
    realF = (float)16000000L / 2 / (1 + top);
  }
  OCR1A = top;
  OCR1B = top;
  return realF;
}
