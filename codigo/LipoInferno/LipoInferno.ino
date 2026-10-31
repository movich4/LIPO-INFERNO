#include "ReceptorPPM.h"
#include "MotorDriver.h"
#include "Mezclas.h"
#include "MotosierraEsc.h"
#include "ElevadorMotosierra.h"

constexpr byte PIN_PPM = 2;
constexpr byte NUM_CANALES = 8;

constexpr byte PIN_M1_IN1 = 3;        //Pines motores
constexpr byte PIN_M1_IN2 = 5;
constexpr byte PIN_M2_IN1 = 6;
constexpr byte PIN_M2_IN2 = 11;

constexpr byte PIN_SERVO = 9;
constexpr byte PIN_ESC = 10;

constexpr byte CANAL_GIRO = 0;        //Canales RX ppm
constexpr byte CANAL_AVANCE = 1;
constexpr byte CANAL_POTENCIA_MOTOSIERRA = 2;
constexpr byte CANAL_ELEVADOR_MOTOSIERRA = 3;
constexpr byte CANAL_MODO_MOTOSIERRA = 5;
constexpr byte CANAL_AJUSTE_MOTORES = 6;
constexpr byte CANAL_MODO_INVERTIDO = 7;

constexpr bool INVERTIR_CANAL_POTENCIA_MOTOSIERRA = false;

// Ajustes iniciales del mecanismo elevador. 
constexpr int ANGULO_SERVO_MINIMO = 0;
constexpr int ANGULO_SERVO_MAXIMO = 180;
constexpr int ANGULO_SERVO_BOCA_ARRIBA = 90;
constexpr int ANGULO_SERVO_BOCA_ABAJO = 110;
constexpr int PASO_SERVO_GRADOS = 30;
constexpr bool INVERTIR_INCREMENTOS_SERVO = false;

constexpr int ZONA_MUERTA_US = 30;                //Zona muerta en posicion central del joystick
constexpr int CORRECCION_MAXIMA_MOTORES = 20;     //Correccion maxima entre motores con el pote de la emisora
constexpr int UMBRAL_MODO_NORMAL_US = 1300;
constexpr int UMBRAL_MODO_INVERTIDO_US = 1700;

constexpr bool INVERTIR_MOTOR_IZQUIERDO = false;
constexpr bool INVERTIR_MOTOR_DERECHO = false;

int anchos_pulso[NUM_CANALES];
bool modo_invertido = false;          //  Cuando el robot va en invertido hay algunos canales que se invierten para 
                                      //  facilitar la conduccion
MotorDriver motor_izquierdo(
  PIN_M1_IN1,
  PIN_M1_IN2,
  INVERTIR_MOTOR_IZQUIERDO
);

MotorDriver motor_derecho(
  PIN_M2_IN1,
  PIN_M2_IN2,
  INVERTIR_MOTOR_DERECHO
);

MotosierraEsc motosierra(
  PIN_ESC,
  INVERTIR_CANAL_POTENCIA_MOTOSIERRA
);

ElevadorMotosierra elevador_motosierra(
  PIN_SERVO,
  ANGULO_SERVO_MINIMO,
  ANGULO_SERVO_MAXIMO,
  ANGULO_SERVO_BOCA_ARRIBA,
  ANGULO_SERVO_BOCA_ABAJO,
  PASO_SERVO_GRADOS,
  INVERTIR_INCREMENTOS_SERVO
);

void motores_mover(int potencia_izquierda, int potencia_derecha) {
  motor_izquierdo.mover(potencia_izquierda);
  motor_derecho.mover(potencia_derecha);
}

void motores_parar() {
  motor_izquierdo.parar();
  motor_derecho.parar();
}

void setup() {
  Serial.begin(115200);

  motor_izquierdo.iniciar();
  motor_derecho.iniciar();
  motosierra.iniciar();
  elevador_motosierra.iniciar();

  // Inicia la lectura de la señal PPM en el pin indicado.
  if (!ReceptorPPM::iniciar(PIN_PPM, NUM_CANALES)) {
    Serial.println("Error al iniciar el receptor PPM");
    while (true) {
      // El programa queda detenido si el pin no admite interrupciones.
    }
  }
}

void loop() {
  if (!ReceptorPPM::hay_senal()) {
    fail_safe();
    return;
  }

  // Solo se recalculan las salidas cuando llega una trama PPM nueva.
  if (ReceptorPPM::leer(anchos_pulso)) {
    motosierra.actualizar(
      anchos_pulso[CANAL_MODO_MOTOSIERRA],
      anchos_pulso[CANAL_POTENCIA_MOTOSIERRA]
    );

    int avance = Mezclas::normalizarCanal(
      anchos_pulso[CANAL_AVANCE],
      ZONA_MUERTA_US
    );
    const int giro = Mezclas::normalizarCanal(
      anchos_pulso[CANAL_GIRO],
      ZONA_MUERTA_US
    );
    const int ajuste = Mezclas::normalizarCanal(
      anchos_pulso[CANAL_AJUSTE_MOTORES],
      ZONA_MUERTA_US
    );

    // Los valores intermedios de CH7 mantienen el modo actual.
    bool modo_solicitado = modo_invertido;
    const int pulso_modo = anchos_pulso[CANAL_MODO_INVERTIDO];

    if (pulso_modo >= UMBRAL_MODO_INVERTIDO_US) {
      modo_solicitado = true;
    } else if (pulso_modo <= UMBRAL_MODO_NORMAL_US) {
      modo_solicitado = false;
    }

    // Evita invertir bruscamente ambos motores mientras el robot se mueve.
    if (modo_solicitado != modo_invertido) {
      if (avance != 0) {
        motores_parar();
        return;
      }

      modo_invertido = modo_solicitado;
    }

    // Al estar boca abajo solo cambia el eje avance/retroceso.
    if (modo_invertido) {
      avance = -avance;
    }

    elevador_motosierra.actualizar(
      anchos_pulso[CANAL_ELEVADOR_MOTOSIERRA],
      modo_invertido
    );

    Mezclas::PotenciasMotores potencias = Mezclas::traccionDiferencial(avance, giro);

    potencias = Mezclas::aplicarAjuste(
      potencias,
      ajuste,
      CORRECCION_MAXIMA_MOTORES
    );

    motores_mover(potencias.izquierda, potencias.derecha);
    //Serial.print(potencias.izquierda);
    //Serial.print("\t");
    //Serial.println(potencias.derecha);
  }

  mostrar_valores_RX();
}

void mostrar_valores_RX() {
  static unsigned long tiempo_ultima_impresion = 0;
  const unsigned long tiempo_actual = millis();

  if (tiempo_actual - tiempo_ultima_impresion < 100) {
    return;
  }

  tiempo_ultima_impresion = tiempo_actual;

  for (int i = 0; i < NUM_CANALES; i++) {
    int valor_canal = constrain(anchos_pulso[i], 1000, 2000);
    Serial.print(valor_canal);
    Serial.print("\t");
  }

  Serial.print("ESC:");
  Serial.print(motosierra.obtenerPulsoSalida());
  Serial.print("\tARM:");
  Serial.print(motosierra.estaArmada() ? 1 : 0);
  Serial.print("\tSAFE:");
  Serial.print(motosierra.haPasadoPorDesarmada() ? 1 : 0);
  Serial.print("\tSERVO:");
  Serial.println(elevador_motosierra.obtenerAngulo());
}

void fail_safe() {
  motores_parar();
  motosierra.pararSeguro();

  for (byte i = 0; i < NUM_CANALES; i++) {
    anchos_pulso[i] = 1500;
  }

  anchos_pulso[CANAL_POTENCIA_MOTOSIERRA] = 1000;
  anchos_pulso[CANAL_MODO_MOTOSIERRA] = 1000;
  anchos_pulso[CANAL_MODO_INVERTIDO] = 1000;
}
