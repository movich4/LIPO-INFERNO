#include "MotorDriver.h"

MotorDriver::MotorDriver(byte pin_in1, byte pin_in2, bool invertido)
  : pin_in1_(pin_in1),
    pin_in2_(pin_in2),
    invertido_(invertido) {
}

void MotorDriver::iniciar() {
  pinMode(pin_in1_, OUTPUT);
  pinMode(pin_in2_, OUTPUT);
  parar();
}

void MotorDriver::mover(int potencia) {
  potencia = constrain(potencia, -100, 100);

  if (invertido_) {
    potencia = -potencia;
  }

  const byte pwm = (abs(potencia) * 255L) / 100;

  if (potencia > 0) {
    // Desactiva primero la entrada que pudiera seguir generando PWM.
    digitalWrite(pin_in1_, LOW);
    analogWrite(pin_in2_, pwm);
  } else if (potencia < 0) {
    analogWrite(pin_in1_, pwm);
    digitalWrite(pin_in2_, LOW);   
  } else {
    parar();
  }
}

void MotorDriver::parar() {
  digitalWrite(pin_in1_, LOW);
  digitalWrite(pin_in2_, LOW);
}

void MotorDriver::frenar() {
  digitalWrite(pin_in1_, HIGH);
  digitalWrite(pin_in2_, HIGH);
}
