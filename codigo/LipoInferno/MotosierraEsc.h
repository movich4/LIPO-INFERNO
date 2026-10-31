#pragma once

#include <Arduino.h>
#include <Servo.h>

class MotosierraEsc {
public:
  explicit MotosierraEsc(byte pin, bool invertir_canal_potencia = false);

  // Conecta el ESC y empieza enviando la señal neutra.
  void iniciar();

  // pulso_modo_us: 1000 = desarmada, 1500 = normal, 2000 = invertida.
  // pulso_potencia_us: 1000 = 0 % y 2000 = 100 %.
  void actualizar(int pulso_modo_us, int pulso_potencia_us);

  // Detiene el ESC y obliga a volver a pasar por la posición desarmada.
  void pararSeguro();

  // Datos de diagnóstico para comprobar el armado sin conectar la motosierra.
  int obtenerPulsoSalida() const;
  bool estaArmada() const;
  bool haPasadoPorDesarmada() const;

private:
  enum Modo {
    DESARMADA,
    NORMAL,
    INVERTIDA
  };

  Modo obtenerModo(int pulso_us) const;
  void escribirPulso(int pulso_us);

  byte pin_;
  bool invertir_canal_potencia_;
  bool iniciada_;
  bool paso_por_desarmada_;
  bool armada_;
  int pulso_salida_us_;
  Modo modo_anterior_;
  Servo esc_;
};
