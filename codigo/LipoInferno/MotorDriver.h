#pragma once

#include <Arduino.h>

// Controla un motor DC conectado a un módulo DRV8871.
class MotorDriver {
public:
  MotorDriver(byte pin_in1, byte pin_in2, bool invertido = false);

  void iniciar();

  // La potencia se limita automáticamente al intervalo -100..100.
  void mover(int potencia);

  // IN1 = LOW e IN2 = LOW: motor libre y DRV8871 en reposo.
  void parar();

  // IN1 = HIGH e IN2 = HIGH: frenado eléctrico.
  void frenar();

private:
  byte pin_in1_;
  byte pin_in2_;
  bool invertido_;
};
