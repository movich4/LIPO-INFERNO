#include "MotosierraEsc.h"

namespace {

constexpr int PULSO_MINIMO_US = 1000;
constexpr int PULSO_NEUTRO_US = 1500;
constexpr int PULSO_MAXIMO_US = 2000;

constexpr int UMBRAL_MODO_DESARMADO_US = 1300;
constexpr int UMBRAL_MODO_INVERTIDO_US = 1700;

// Tolera el pequeño error habitual del canal en el extremo de la palanca.
constexpr int UMBRAL_POTENCIA_CERO_US = 1050;

}  // namespace

MotosierraEsc::MotosierraEsc(byte pin, bool invertir_canal_potencia)
  : pin_(pin),
    invertir_canal_potencia_(invertir_canal_potencia),
    iniciada_(false),
    paso_por_desarmada_(false),
    armada_(false),
    pulso_salida_us_(PULSO_NEUTRO_US),
    modo_anterior_(DESARMADA) {
}

void MotosierraEsc::iniciar() {
  esc_.attach(pin_, PULSO_MINIMO_US, PULSO_MAXIMO_US);
  iniciada_ = true;
  pararSeguro();
}

void MotosierraEsc::actualizar(int pulso_modo_us, int pulso_potencia_us) {
  const Modo modo_solicitado = obtenerModo(pulso_modo_us);

  int potencia_us = constrain(
    pulso_potencia_us,
    PULSO_MINIMO_US,
    PULSO_MAXIMO_US
  );

  if (invertir_canal_potencia_) {
    potencia_us = PULSO_MINIMO_US + PULSO_MAXIMO_US - potencia_us;
  }

  if (modo_solicitado == DESARMADA) {
    escribirPulso(PULSO_NEUTRO_US);
    paso_por_desarmada_ = true;
    armada_ = false;
    modo_anterior_ = DESARMADA;
    return;
  }

  // Un cambio de sentido con potencia corta inmediatamente la salida.
  if (modo_solicitado != modo_anterior_) {
    armada_ = false;
  }

  // Tras el arranque o un fail-safe hay que pasar por DESARMADA. Además,
  // cualquier armado o cambio de sentido exige tener la potencia al 0 %.
  if (paso_por_desarmada_ && potencia_us <= UMBRAL_POTENCIA_CERO_US) {
    armada_ = true;
  }

  if (!armada_) {
    escribirPulso(PULSO_NEUTRO_US);
    modo_anterior_ = modo_solicitado;
    return;
  }

  int salida_us;
  if (modo_solicitado == NORMAL) {
    salida_us = map(
      potencia_us,
      PULSO_MINIMO_US,
      PULSO_MAXIMO_US,
      PULSO_NEUTRO_US,
      PULSO_MAXIMO_US
    );
  } else {
    salida_us = map(
      potencia_us,
      PULSO_MINIMO_US,
      PULSO_MAXIMO_US,
      PULSO_NEUTRO_US,
      PULSO_MINIMO_US
    );
  }

  escribirPulso(salida_us);
  modo_anterior_ = modo_solicitado;
}

void MotosierraEsc::pararSeguro() {
  escribirPulso(PULSO_NEUTRO_US);
  paso_por_desarmada_ = false;
  armada_ = false;
  modo_anterior_ = DESARMADA;
}

int MotosierraEsc::obtenerPulsoSalida() const {
  return pulso_salida_us_;
}

bool MotosierraEsc::estaArmada() const {
  return armada_;
}

bool MotosierraEsc::haPasadoPorDesarmada() const {
  return paso_por_desarmada_;
}

MotosierraEsc::Modo MotosierraEsc::obtenerModo(int pulso_us) const {
  if (pulso_us <= UMBRAL_MODO_DESARMADO_US) {
    return DESARMADA;
  }

  if (pulso_us >= UMBRAL_MODO_INVERTIDO_US) {
    return INVERTIDA;
  }

  return NORMAL;
}

void MotosierraEsc::escribirPulso(int pulso_us) {
  pulso_salida_us_ = constrain(
    pulso_us,
    PULSO_MINIMO_US,
    PULSO_MAXIMO_US
  );

  if (!iniciada_) {
    return;
  }

  esc_.writeMicroseconds(pulso_salida_us_);
}
