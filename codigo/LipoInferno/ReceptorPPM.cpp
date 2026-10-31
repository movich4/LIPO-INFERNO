#include "ReceptorPPM.h"

namespace {

// Número máximo de canales que admite este módulo.
const byte MAX_CANALES = 8;

volatile unsigned int trama_en_proceso[MAX_CANALES];
volatile unsigned int ultima_trama[MAX_CANALES];
volatile byte numero_canales_configurado = 0;
volatile byte canal_actual = 0;
volatile unsigned int pausa_sincronismo_us = 3000;
volatile unsigned long tiempo_ultimo_flanco = 0;
volatile unsigned long tiempo_ultima_trama = 0;
volatile bool trama_nueva = false;

// Esta función se ejecuta cada vez que llega un flanco de la señal PPM.
void leer_interrupcion_PPM() {
  unsigned long tiempo_actual = micros();
  unsigned long diferencia = tiempo_actual - tiempo_ultimo_flanco;
  tiempo_ultimo_flanco = tiempo_actual;

  // Una pausa larga indica el inicio de una trama nueva.
  if (diferencia > pausa_sincronismo_us) {
    // Solo se guarda la trama si se han recibido todos los canales.
    if (canal_actual == numero_canales_configurado) {
      for (byte i = 0; i < numero_canales_configurado; i++) {
        ultima_trama[i] = trama_en_proceso[i];
      }

      tiempo_ultima_trama = millis();
      trama_nueva = true;
    }

    canal_actual = 0;
  } else if (canal_actual < numero_canales_configurado) {
    trama_en_proceso[canal_actual] = diferencia;
    canal_actual++;
  }
}

}  // namespace

namespace ReceptorPPM {

bool iniciar(byte pin_ppm, byte numero_canales, unsigned int pausa_sincronismo) {
  if (numero_canales == 0 || numero_canales > MAX_CANALES) {
    return false;
  }

  int numero_interrupcion = digitalPinToInterrupt(pin_ppm);
  if (numero_interrupcion == NOT_AN_INTERRUPT) {
    return false;
  }

  noInterrupts();
  numero_canales_configurado = numero_canales;
  canal_actual = 0;
  pausa_sincronismo_us = pausa_sincronismo;
  tiempo_ultimo_flanco = micros();
  tiempo_ultima_trama = 0;
  trama_nueva = false;
  interrupts();

  pinMode(pin_ppm, INPUT);
  attachInterrupt(numero_interrupcion, leer_interrupcion_PPM, RISING);

  return true;
}

bool leer(int *anchos_pulso) {
  if (anchos_pulso == NULL || numero_canales_configurado == 0) {
    return false;
  }

  bool hay_trama_nueva;

  // Se detienen brevemente las interrupciones para realizar una copia segura.
  noInterrupts();
  hay_trama_nueva = trama_nueva;

  if (hay_trama_nueva) {
    for (byte i = 0; i < numero_canales_configurado; i++) {
      anchos_pulso[i] = ultima_trama[i];
    }

    trama_nueva = false;
  }

  interrupts();
  return hay_trama_nueva;
}

bool hay_senal(unsigned int tiempo_maximo_sin_senal) {
  unsigned long ultima_recepcion;

  // La variable ocupa varios bytes; se copia con las interrupciones detenidas.
  noInterrupts();
  ultima_recepcion = tiempo_ultima_trama;
  interrupts();

  if (ultima_recepcion == 0) {
    return false;
  }

  return millis() - ultima_recepcion <= tiempo_maximo_sin_senal;
}

}  // namespace ReceptorPPM
