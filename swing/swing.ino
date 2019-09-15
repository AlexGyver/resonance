// серво-качели
// серво на D12
// потенциометр на A0

#include <Servo.h>
Servo servo;
boolean servoToggle;
uint32_t timer;
int period;

void setup() {
  servo.attach(12); // серво на D12
  pinMode(13, 1);   // питание потенциометра
  digitalWrite(13, 1);
}

void loop() {
  if (millis() - timer > period) {  // таймер
    timer = millis();
    servoToggle = !servoToggle;
    if (servoToggle) servo.write(20); // угол мин
    else servo.write(160);  // угол макс
    if (servoToggle) {
      long sum = 0;
      for (byte i = 0; i < 10; i++) {
        sum += analogRead(0);
      }
      sum /= 10;
      period = map(sum, 0, 1023, 300, 1000);
    }
  }
}
