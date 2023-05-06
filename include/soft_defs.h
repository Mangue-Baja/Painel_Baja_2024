#ifndef SOFT_DEF
#define SOFT_DEF

#include <Ticker.h>

//Variaveis
  //Srtucts
    typedef struct {
      uint16_t segundos = 0;
      uint8_t minutos = 0;
      uint8_t horas = 0;

      unsigned long time_current=0;
      unsigned long tempo_volta_ms = 0;
    }Tempo;
    
    Tempo Enduro;
    Tempo penultima_volta;
    Tempo ultima_volta;
    Tempo Volta;
    Tempo Delta;

    typedef struct {
      uint8_t Pin;
      int buttonState;            // the current reading from the input pin
      int lastButtonState = LOW;  // the previous reading from the input pin
      unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
      uint8_t mode = 0;
    }Perifericos;
    
    Perifericos Switch;
    Perifericos Button;
    

    //Struct para receber todos os dados do painel
    //
    //caso deseja adicionar um dado no painel  alterar não só na variavél quanto na ECU dianteira
    typedef struct {

      uint16_t velocidade;
      uint16_t rpm;
      uint16_t battery;
      uint16_t combustivel;
      uint16_t temp_cvt;
      uint16_t temp_motor;

    }Txtmng;

    Txtmng Var;
    Txtmng Var_0;
    uint16_t dado_arr[sizeof(Txtmng)];     //array que recebe os dados em Bits da ECU dianteira 
  //

  uint16_t potenciometro;
  
  bool emergency_led_state;
  
  bool boolean1HZ;
  bool boolean5HZ;

  unsigned long lastDebounceTime = 0;
  const int debounceDelay = 200;          //tempo em millisegundos para debounce dos botões
  
//Inicializadores 
  Ticker ticker2Hz;       //iniciando os tickers
  Ticker ticker5Hz;

  //File file

  typedef enum PROGMEM {TEMPO_DE_ENDURO, CRONOMETRO, DELTA_CRONOMETRO} tempo_six_digits;
  byte six_digits_state = TEMPO_DE_ENDURO;

#endif