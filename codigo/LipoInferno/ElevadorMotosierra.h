#pragma once

#include <Arduino.h>
#include <Servo.h>

class ElevadorMotosierra {
public:
  ElevadorMotosierra(
    byte pin,
    int angulo_minimo,
    int angulo_maximo,
    int angulo_boca_arriba,
    int angulo_boca_abajo,
    int paso_grados,
    bool invertir_incrementos = false
  );

  void iniciar();

  // Cada llegada de CH3 a un extremo aplica un solo paso. Para volver a
  // aplicar otro paso, la palanca tiene que regresar antes a la zona central.
  void actualizar(int pulso_control_us, bool robot_boca_abajo);

  int obtenerAngulo() const;

private:
  void escribirAngulo(int angulo);

  byte pin_;
  int angulo_minimo_;
  int angulo_maximo_;
  int angulo_boca_arriba_;
  int angulo_boca_abajo_;
  int paso_grados_;
  bool invertir_incrementos_;
  bool iniciada_;
  bool orientacion_conocida_;
  bool robot_boca_abajo_anterior_;
  bool palanca_paso_por_centro_;
  int angulo_actual_;
  Servo servo_;
};
