#define ForkSpeed  20   // 포크의 회전 속도를 지정합니다. 최저 0 에서 최대 255 까지 지정합니다
#define pinMotor    6

const uint8_t MaxMotorSpeedDuty = 0;
const uint8_t MinMotorSpeedDuty = 255;
const uint8_t MotorSpeedDuty = MinMotorSpeedDuty - ForkSpeed;

void setup() {
  pinMode(SW_BUILTIN, INPUT_PULLUP);
  pinMode(pinMotor, OUTPUT);
  analogWrite(pinMotor, MinMotorSpeedDuty);
}

void loop() {
  if (digitalRead(SW_BUILTIN)) {
    analogWrite(pinMotor, MinMotorSpeedDuty);
  } else {
    analogWrite(pinMotor, MotorSpeedDuty);
  }
}