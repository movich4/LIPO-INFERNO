#include "ElevadorMotosierra.h"

namespace {

constexpr int PULSO_SERVO_MINIMO_US = 500;
constexpr int PULSO_SERVO_MAXIMO_US = 2500;

constexpr int UMBRAL_EXTREMO_INFERIOR_US = 1200;
constexpr int UMBRAL_CENTRO_INFERIOR_US = 1350;
constexpr int UMBRAL_CENTRO_SUPERIOR_US = 1650;
constexpr int UMBRAL_EXTREMO_SUPERIOR_US = 1800;

}  // namespace

ElevadorMotosierra::ElevadorMotosierra(
  byte pin,
  int angulo_minimo,
  int angulo_maximo,
  int angulo_boca_arriba,
  int angulo_boca_abajo,
  int paso_grados,
  bool invertir_incrementos
) : pin_(pin),
    angulo_minimo_(constrain(angulo_minimo, 0, 180)),
    angulo_maximo_(constrain(angulo_maximo, 0, 180)),
    angulo_boca_arriba_(angulo_boca_arriba),
    angulo_boca_abajo_(angulo_boca_abajo),
    paso_grados_(constrain(paso_grados, 1, 180)),
    invertir_incrementos_(invertir_incrementos),
    iniciada_(false),
    orientacion_conocida_(false),
    robot_boca_abajo_anterior_(false),
    palanca_paso_por_centro_(false),
    angulo_actual_(90) {
  if (angulo_minimo_ > angulo_maximo_) {
    const int temporal = angulo_minimo_;
    angulo_minimo_ = angulo_maximo_;
    angulo_maximo_ = temporal;
  }

  angulo_boca_arriba_ = constrain(
    angulo_boca_arriba_,
    angulo_minimo_,
    angulo_maximo_
  );
  angulo_boca_abajo_ = constrain(
    angulo_boca_abajo_,
    angulo_minimo_,
    angulo_maximo_
  );
  angulo_actual_ = angulo_boca_arriba_;
}

void ElevadorMotosierra::iniciar() {
  servo_.attach(
    pin_,
    PULSO_SERVO_MINIMO_US,
    PULSO_SERVO_MAXIMO_US
  );
  iniciada_ = true;
  escribirAngulo(angulo_boca_arriba_);
}

void ElevadorMotosierra::actualizar(
  int pulso_control_us,
  bool robot_boca_abajo
) {
  if (!orientacion_conocida_ ||
      robot_boca_abajo != robot_boca_abajo_anterior_) {
    escribirAngulo(
      robot_boca_abajo ? angulo_boca_abajo_ : angulo_boca_arriba_
    );
    robot_boca_abajo_anterior_ = robot_boca_abajo;
    orientacion_conocida_ = true;
    palanca_paso_por_centro_ = false;
  }

  pulso_control_us = constrain(
    pulso_control_us,
    PULSO_SERVO_MINIMO_US,
    PULSO_SERVO_MAXIMO_US
  );

  if (pulso_control_us >= UMBRAL_CENTRO_INFERIOR_US &&
      pulso_control_us <= UMBRAL_CENTRO_SUPERIOR_US) {
    palanca_paso_por_centro_ = true;
    return;
  }

  if (!palanca_paso_por_centro_) {
    return;
  }

  int incremento = 0;
  if (pulso_control_us >= UMBRAL_EXTREMO_SUPERIOR_US) {
    incremento = paso_grados_;
  } else if (pulso_control_us <= UMBRAL_EXTREMO_INFERIOR_US) {
    incremento = -paso_grados_;
  } else {
    return;
  }

  if (invertir_incrementos_) {
    incremento = -incremento;
  }

  // Al estar el robot boca abajo, el mismo giro del servo produce el
  // movimiento físico contrario. Se invierte CH3 para que la palanca siga
  // significando subir/bajar en el mismo sentido para el piloto.
  if (robot_boca_abajo) {
    incremento = -incremento;
  }

  escribirAngulo(angulo_actual_ + incremento);
  palanca_paso_por_centro_ = false;
}

int ElevadorMotosierra::obtenerAngulo() const {
  return angulo_actual_;
}

void ElevadorMotosierra::escribirAngulo(int angulo) {
  angulo_actual_ = constrain(angulo, angulo_minimo_, angulo_maximo_);

  if (iniciada_) {
    servo_.write(angulo_actual_);
  }
}
