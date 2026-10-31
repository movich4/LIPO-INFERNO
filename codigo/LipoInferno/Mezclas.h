#pragma once

#include <Arduino.h>

namespace Mezclas {

struct PotenciasMotores {
  int izquierda;
  int derecha;
};

// Convierte un pulso PPM de 1000..2000 us en una orden de -100..100.
// La zona muerta evita movimientos causados por el ruido alrededor de 1500 us.
int normalizarCanal(int pulso_us, int zona_muerta_us = 30);

// Mezcla el avance y el giro para una tracción diferencial.
PotenciasMotores traccionDiferencial(int avance, int giro);

// Un ajuste positivo (pote a la derecha) reduce el motor derecho.
// Un ajuste negativo (pote a la izquierda) reduce el motor izquierdo.
// ajuste debe estar entre -100 y 100.
PotenciasMotores aplicarAjuste(
  PotenciasMotores potencias,
  int ajuste,
  int correccion_maxima = 20
);

}  // namespace Mezclas
