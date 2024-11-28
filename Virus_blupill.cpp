#include "mbed.h"
BufferedSerial MATLAB(PA_2, PA_3, 9600); // tx, rx
// Pines para controlar el NEMA a través del driver
DigitalOut ENABLENEMA1(PB_12);
DigitalOut DireccionNema1(PB_13);
DigitalOut PULSO1(PB_14); // Definimos el pin de salida

DigitalOut ENABLENEMA2(PB_15);    // Habilitación para motor 2
DigitalOut DireccionNema2(PC_2); // Dirección para motor 2
DigitalOut PULSO2(PA_8);         // Pin de salida para motor 2
//.............................Leds caja ......................................
DigitalOut ledStart(PA_9);
DigitalOut ledStop(PB_5);
DigitalOut ledReset(PB_6);
//.................................finales de carrera...........................
InterruptIn FINX0(PB_7, PullUp);
InterruptIn FINX1(PB_8, PullUp);
InterruptIn FINZ0(PB_9, PullUp);
InterruptIn FINZ1(PB_11, PullUp);
//..................................Sensores moviles..........................
InterruptIn INDX0(PB_10, PullUp);
InterruptIn INDX1(PB_1, PullUp);
InterruptIn INDZ0(PA_7, PullUp);
InterruptIn INDZ1(PA_6, PullUp);
//..............................variable para el error de comunicacion........
volatile bool error_com =
    false; // Variable para manejar errores de comunicacion
//..................................PT100......................
AnalogIn PT100_1(PA_5); // Configura la primera entrada analoga
AnalogIn PT100_2(PA_4); // Configura la segunda entrada analoga
//..................leds pruebas
// DigitalOut leds(LED1);
// DigitalOut leds2(LED2);
//............................. Creamos un ticker para los pulsos
Ticker Pulsos1;
Ticker Pulsos2;

float Periodo; // Inicializamos la variable en la que se calcula la frecuencia a
               // través de los RPM
const int STEPS = 400; // Inicializamos la constante en la que se encuentran los
                       // microsteps del driver
int pasostotales1 = 0; // Pasos totales para motor 1
int pasostotales2 = 0; // Pasos totales para motor 2
int NUMVUELTAS1 = 0;   // Vueltas para motor 1
int NUMVUELTAS2 = 0;   // Vueltas para motor 2
bool Girando1 = true;  // Estado de giro para motor 1
bool Girando2 = true;  // Estado de giro para motor 2
volatile int64_t CantidadPulsos1 = 0; // Pulsos para motor 1
volatile int64_t CantidadPulsos2 = 0; // Pulsos para motor 2
volatile bool motorState1 = false;    // Estado motor 1
volatile bool motorState2 = false;    // Estado motor 2
char CADENA[600];
int LONGITUD, LONGITUD1;
int valor_PT1 = 0, valor_PT2 = 0;
int home = 0;
void FunGira1(void);
void FunGira2(void);
// Función para configurar los datos de los esclavos
void sensoresCarreraX1();
void sensoresCarreraX2();
void sensoresCarreraZ1();
void sensoresCarreraZ2();
void inductivoX1();
void inductivoX2();
void inductivoZ1();
void inductivoZ2();
void leer_datos();
void revisarError();
void actualizarEstadoLeds();
void manejarError();

void revisarError() {
  if (!MATLAB.writable()) { // Simulando error de comunicación
    error_com = true;
  }
}

void actualizarEstadoLeds() {
  // LED Start: Encendido si algún motor está girando
  ledStart =
      (CantidadPulsos1 < pasostotales1 || CantidadPulsos2 < pasostotales2);

  ledReset = (home == 1);
}

void manejarError() {
  if (error_com) {
    ledStop = 1; // LED rojo encendido
  } else {
    ledStop = 0; // LED rojo apagado
  }
}

int main(void) {
  // inicializamos los LEDs en estado de apagado
  ledStart = 0;
  ledStop = 0;
  ledReset = 0;

  // Configuraciones para la interrupciones
  FINX0.fall(&sensoresCarreraX1);
  FINX1.fall(&sensoresCarreraX2);
  FINZ0.fall(&sensoresCarreraZ1);
  FINZ1.fall(&sensoresCarreraZ2);
  INDX0.fall(&inductivoX1);
  INDX1.fall(&inductivoX2);
  INDZ0.fall(&inductivoZ1);
  INDZ1.fall(&inductivoZ2);
  // Inicializamos los pines de salida para parar los NEMA
  ENABLENEMA1 = 1;
  ENABLENEMA2 = 1;
  DireccionNema1 = 0;
  DireccionNema2 = 0;

  while (true) {

    leer_datos();
    if (MATLAB.readable()) { // Recibimos datos desde MATLAB
      ledReset = 1;
      char BufferIN[800]; // Creamos buffer para almacenar los datos recibidos
      int GIROS1 = 0, RPM1 = 0, Direccion1 = 0;
      int GIROS2 = 0, RPM2 = 0, Direccion2 = 0;
      int home = 0;

      CantidadPulsos1 = 0; // Reiniciamos el contador motor 1
      CantidadPulsos2 = 0;
      NUMVUELTAS1 = 0; // Reiniciamos el contador motor 1 CantidadPulsos2 = 0;
                       // // Reiniciamos el contador motor 2
      NUMVUELTAS2 = 0; // Reiniciamos el contador motor 2

      MATLAB.read(BufferIN,
                  sizeof(BufferIN)); // Recibimos el arreglo por el Serial
      // Creamos una lista del arreglo y le asignamos una variable a cada uno
      sscanf(BufferIN, "%d,%d,%d,%d,%d,%d,%d", &GIROS1, &RPM1, &Direccion1,
             &GIROS2, &RPM2, &Direccion2, &home);
      if (home == 1) {
        ledReset = 1; // Encender LED de reset durante el homing

        // Homing para el eje Z
        while (FINZ0.read() == 1) {
          FunGira2();
        }

        while (FINZ1.read() == 1) {
          FunGira2();
        }

        ENABLENEMA2 = 1;

        // Homing para el eje X
        while (FINX0.read() == 1) {
          FunGira1();
        }

        while (FINX1.read() == 1) {
          FunGira1();
        }

        ENABLENEMA1 = 1;
        ledReset = 0; // Apagar LED de reset después de completar el homing
      } else {
        ThisThread::sleep_for(50ms);
        Periodo =
            float(60) /
            (STEPS * RPM1); // Calculamos el periodo de los pasos para motor 1
        pasostotales1 =
            GIROS1 *
            STEPS; // Calculamos la cantidad de pasos totales para motor 1
        ENABLENEMA1 =
            0; // Habilitamos el ENABLE del driver para que gire el motor 1
        DireccionNema1 = Direccion1;
        Pulsos1.attach(&FunGira1, Periodo); // Creamos los pulsos para motor 1
        ledStart = 1;

        Periodo =
            float(60) /
            (STEPS * RPM2); // Calculamos el periodo de los pasos para motor 2
        pasostotales2 =
            GIROS2 *
            STEPS; // Calculamos la cantidad de pasos totales para motor 2
        ENABLENEMA2 =
            0; // Habilitamos el ENABLE del driver para que gire el motor 2
        DireccionNema2 = Direccion2;
        Pulsos2.attach(&FunGira2, Periodo); // Creamos los pulsos para motor 2
      }
    }
    ThisThread::sleep_for(50ms);
  }
}
void leer_datos() {
  valor_PT1 = PT100_1.read_u16();
  valor_PT2 = PT100_2.read_u16();

  ThisThread::sleep_for(100ms);
  LONGITUD1 =
      sprintf(CADENA, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", valor_PT1, valor_PT2,
              FINX0.read(), FINX1.read(), FINZ0.read(), FINZ1.read(),
              INDX0.read(), INDX1.read(), INDZ0.read(), INDZ1.read());
  MATLAB.write(CADENA, LONGITUD1);
  ThisThread::sleep_for(100ms);
  revisarError();
}
void FunGira1(void) {
  if (CantidadPulsos1 < pasostotales1) {
    PULSO1 = 1; // Ponemos en alto la salida para motor 1
    wait_us(10);
    PULSO1 = 0;
    CantidadPulsos1++; // Contamos los pulsos para motor 1
    NUMVUELTAS1 =
        CantidadPulsos1 /
        STEPS; // Calculamos el número de vueltas que llevamos para motor 1
    actualizarEstadoLeds();
  } else {
    Girando1 = true;
    Pulsos1.detach(); // Detenemos los pulsos para motor 1
    ENABLENEMA1 = 1;  // Deshabilitamos el ENABLE del motor 1 para que no gire
    DireccionNema1 = 0;
    ledStop = 1; // Encendemos el led de Stop para indicar que el enable ha sido
                 // puesto en 0
  }
}

void FunGira2(void) {
  if (CantidadPulsos2 < pasostotales2) {
    PULSO2 = 1; // Ponemos en alto la salida para motor 2
    wait_us(10);
    PULSO2 = 0;
    CantidadPulsos2++; // Contamos los pulsos para motor 2
    NUMVUELTAS2 =
        CantidadPulsos2 /
        STEPS; // Calculamos el número de vueltas que llevamos para motor 2
    actualizarEstadoLeds();
  } else {
    Girando2 = true;
    Pulsos2.detach(); // Detenemos los pulsos para motor 2
    ENABLENEMA2 = 1;  // Deshabilitamos el ENABLE del motor 2 para que no gire
    DireccionNema2 = 0;
    ledStop = 1;
  }
}
void sensoresCarreraX1() {
  if (FINX0.read() == 1) {
    // Sensor activado: detener motor y reportar error
    ENABLENEMA1 = 1; // Deshabilitar motor 1
    motorState1 = false;
    ledStop = 1;  // Encender LED rojo indicando error
    ledStart = 0; // Asegurarse de apagar el LED de actividad
  } else {
    // Si el sensor no está activado
    ledStop = 0;                        // Apagar LED rojo si no hay error
    if (!motorState1) {                 // Cambiar dirección si estaba detenido
      DireccionNema1 = !DireccionNema1; // Invertir dirección
    }
    ENABLENEMA1 = 0; // Habilitar motor 1 para seguir funcionando
    motorState1 = true;
  }
}

void sensoresCarreraX2() {
  if (FINX1.read() == 1) {
    ENABLENEMA1 = 1; // Deshabilitar motor 1
    motorState1 = false;
    ledStop = 1;  // Encender LED rojo indicando error
    ledStart = 0; // Apagar LED de actividad
  } else {
    ledStop = 0; // Apagar LED rojo si no hay error
    if (!motorState1) {
      DireccionNema1 = !DireccionNema1; // Invertir dirección
    }
    ENABLENEMA1 = 0; // Habilitar motor 1
    motorState1 = true;
  }
}

void sensoresCarreraZ1() {
  if (FINZ0.read() == 1) {
    ENABLENEMA2 = 1; // Deshabilitar motor 2
    motorState2 = false;
    ledStop = 1;  // Encender LED rojo indicando error
    ledStart = 0; // Apagar LED de actividad
  } else {
    ledStop = 0; // Apagar LED rojo si no hay error
    if (!motorState2) {
      DireccionNema2 = !DireccionNema2; // Invertir dirección
    }
    ENABLENEMA2 = 0; // Habilitar motor 2
    motorState2 = true;
  }
}

void sensoresCarreraZ2() {
  if (FINZ1.read() == 1) {
    ENABLENEMA2 = 1; // Deshabilitar motor 2
    motorState2 = false;
    ledStop = 1;  // Encender LED rojo indicando error
    ledStart = 0; // Apagar LED de actividad
  } else {
    ledStop = 0; // Apagar LED rojo si no hay error
    if (!motorState2) {
      DireccionNema2 = !DireccionNema2; // Invertir dirección
    }
    ENABLENEMA2 = 0; // Habilitar motor 2
    motorState2 = true;
  }
}

void inductivoX1() {
  if (INDX0.read() == 1) {
    ENABLENEMA1 = 1; // Deshabilitar motor 1
    motorState1 = false;
    ledStop = 1;  // Encender LED rojo indicando error
    ledStart = 0; // Apagar LED de actividad
  } else {
    ledStop = 0; // Apagar LED rojo si no hay error
    if (!motorState1) {
      DireccionNema1 = !DireccionNema1; // Invertir dirección
    }
    ENABLENEMA1 = 0; // Habilitar motor 1
    motorState1 = true;
  }
}

void inductivoX2() {
  if (INDX1.read() == 1) {
    ENABLENEMA1 = 1; // Deshabilitar motor 1
    motorState1 = false;
    ledStop = 1;  // Encender LED rojo indicando error
    ledStart = 0; // Apagar LED de actividad
  } else {
    ledStop = 0; // Apagar LED rojo si no hay error
    if (!motorState1) {
      DireccionNema1 = !DireccionNema1; // Invertir dirección
    }
    ENABLENEMA1 = 0; // Habilitar motor 1
    motorState1 = true;
  }
}

void inductivoZ1() {
  if (INDZ0.read() == 1) {
    ENABLENEMA2 = 1; // Deshabilitar motor 2
    motorState2 = false;
    ledStop = 1;  // Encender LED rojo indicando error
    ledStart = 0; // Apagar LED de actividad
  } else {
    ledStop = 0; // Apagar LED rojo si no hay error
    if (!motorState2) {
      DireccionNema2 = !DireccionNema2; // Invertir dirección
    }
    ENABLENEMA2 = 0; // Habilitar motor 2
    motorState2 = true;
  }
}

void inductivoZ2() {
  if (INDZ1.read() == 1) {
    ENABLENEMA2 = 1; // Deshabilitar motor 2
    motorState2 = false;
    ledStop = 1;  // Encender LED rojo indicando error
    ledStart = 0; // Apagar LED de actividad
  } else {
    ledStop = 0; // Apagar LED rojo si no hay error
    if (!motorState2) {
      DireccionNema2 = !DireccionNema2; // Invertir dirección
    }
    ENABLENEMA2 = 0; // Habilitar motor 2
    motorState2 = true;
  }
}
