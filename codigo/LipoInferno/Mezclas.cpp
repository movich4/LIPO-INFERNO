#include "Mezclas.h"

namespace {

constexpr int PULSO_MINIMO_US = 1000;
constexpr int PULSO_CENTRO_US = 1500;
constexpr int PULSO_MAXIMO_US = 2000;

int limitarPotencia(int potencia) {
  return constrain(potencia, -100, 100);
}

}  // namespace

namespace Mezclas {

int normalizarCanal(int pulso_us, int zona_muerta_us) {
  pulso_us = constrain(pulso_us, PULSO_MINIMO_US, PULSO_MAXIMO_US);
  zona_muerta_us = constrain(zona_muerta_us, 0, 499);

  const int limite_inferior = PULSO_CENTRO_US - zona_muerta_us;
  const int limite_superior = PULSO_CENTRO_US + zona_muerta_us;

  if (pulso_us >= limite_inferior && pulso_us <= limite_superior) {
    return 0;
  }

  if (pulso_us > limite_superior) {
    return map(
      pulso_us,
      limite_superior,
      PULSO_MAXIMO_US,
      0,
      100
    );
  }

  return map(
    pulso_us,
    PULSO_MINIMO_US,
    limite_inferior,
    -100,
    0
  );
}

PotenciasMotores traccionDiferencial(int avance, int giro) {
  avance = limitarPotencia(avance);
  giro = limitarPotencia(giro);

  long izquierda = static_cast<long>(avance) + giro;
  long derecha = static_cast<long>(avance) - giro;
  const long valor_maximo = max(abs(izquierda), abs(derecha));

  // Conserva la proporción entre las ruedas sin superar el 100 %.
  if (valor_maximo > 100) {
    izquierda = (izquierda * 100L) / valor_maximo;
    derecha = (derecha * 100L) / valor_maximo;
  }

  PotenciasMotores resultado = {
    static_cast<int>(izquierda),
    static_cast<int>(derecha)
  };
  return resultado;
}

PotenciasMotores aplicarAjuste(
  PotenciasMotores potencias,
  int ajuste,
  int correccion_maxima
) {
  potencias.izquierda = limitarPotencia(potencias.izquierda);
  potencias.derecha = limitarPotencia(potencias.derecha);
  ajuste = limitarPotencia(ajuste);
  correccion_maxima = constrain(correccion_maxima, 0, 100);

  const int reduccion =
    (static_cast<long>(abs(ajuste)) * correccion_maxima) / 100L;
  const int factor = 100 - reduccion;

  if (ajuste > 0) {
    potencias.derecha =
      (static_cast<long>(potencias.derecha) * factor) / 100L;
  } else if (ajuste < 0) {
    potencias.izquierda =
      (static_cast<long>(potencias.izquierda) * factor) / 100L;
  }

  return potencias;
}

}  // namespace Mezclas
