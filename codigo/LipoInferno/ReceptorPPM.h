#ifndef RECEPTOR_PPM_H
#define RECEPTOR_PPM_H

#include <Arduino.h>

namespace ReceptorPPM {

// Inicia el receptor PPM.
// En un Arduino Nano clásico se puede usar el pin 2 o el pin 3.
bool iniciar(byte pin_ppm, byte numero_canales, unsigned int pausa_sincronismo = 3000);

// Copia la última trama completa en el vector indicado.
// Devuelve verdadero cuando hay valores nuevos.
bool leer(int *anchos_pulso);

// Devuelve verdadero si se está recibiendo correctamente la señal PPM.
bool hay_senal(unsigned int tiempo_maximo_sin_senal = 100);

}  // namespace ReceptorPPM

#endif
