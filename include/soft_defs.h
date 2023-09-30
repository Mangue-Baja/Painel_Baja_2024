#ifndef SOFT_DEFS_H
#define SOFT_DEFS_H

#ifdef SOFT_DEFS_H
  #include <Arduino.h>
  #include "hard_defs.h"
#endif

typedef struct {
  uint16_t segundos = 0;
  uint8_t minutos = 0;
  uint8_t horas = 0;

  unsigned long time_current=0;
  unsigned long tempo_volta_ms = 0;
} Tempo;

Tempo Enduro;
Tempo penultima_volta;
Tempo ultima_volta;
Tempo Volta;
Tempo Delta;

typedef struct {
  uint8_t Pin;
  int buttonState;                     // The current reading from the input pin
  int lastButtonState = LOW;           // Tthe previous reading from the input pin
  unsigned long lastDebounceTime = 0;  // The last time the output pin was toggled
  uint8_t mode = 0;
} Perifericos;

Perifericos Switch;
Perifericos Button;

//Struct to receive all panel data
//if you want to add data to the panel, change it not only in the variable but also in the front ECU
typedef struct {
  uint16_t velocidade;
  uint16_t rpm;
  uint16_t temp_motor;
  uint16_t combustivel;
  uint16_t battery;
  uint16_t temp_cvt;
  uint16_t telemetry;
} Txtmng;

bool emergency_led_state;
bool boolean1HZ;
bool boolean5HZ;

unsigned long lastDebounceTime = 0;
const int debounceDelay = 200;          //Time in milliseconds for button debounce

typedef enum PROGMEM {TEMPO_DE_ENDURO, CRONOMETRO, DELTA_CRONOMETRO} tempo_six_digits;

byte six_digits_state = TEMPO_DE_ENDURO;

#endif