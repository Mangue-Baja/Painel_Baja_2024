#ifndef HARD_DEF
#define HARD_DEF

#include <Arduino.h>

//Four digits display
#define Fourdigits_DIO 15     //Data do display de 4 digitos
#define Fourdigits_CLK 4     //Clock do display de 4 digitos

//Six digits display
#define Sixdigits_DIO 18     //Data  do display de 6 digitos
#define Sixdigits_CLK 19     //Clock  do display de 6 digitos

//OLED
#define SCLK_OLED 21          //Clock do OLED
#define SDA_OLED 22           //Data do OLED

#define DisplayHight 64       //Altura do Oled
#define DisplayWidth 124      //Comprimento do Oled

//leds
#define combust_1 14          //1ºled amarelo de combustivel
#define combust_2 27          //2ºled amarelo de combustivel
#define combust_3 25          //3ºled amarelo de combustivel  
#define combust_4 33          //4ºled amarelo de combustivel 
#define combust_5 32          //led vermelho de emergencia do combustivel 

#define mottemp_led 13         // led de aviso de temperatura do motor
#define cvttemp_led 12         // led de aviso de temperatura do CVT
#define SOT_Led     26         // led de aviso da telemetria ativa(LOW) ou não(HIGH)

//botões e chaves para interações com o usuario no carro
#define BOTAO 23                //verde
#define CHAVE 35                //roxo
#define POTENCIOMETRO 34        //cinza

#define Alerta_TempCVT 90       //temperatura estimada de quando a correia da cvt pode dar problema
#define Alerta_TempMOT 105      //temperatura estimada para dar problema na temperatura do motor

//Comunicação Serial com a Ecu da frente
#define Receiver_Rxd_pin 16     //azul
#define Receiver_Txd_pin 17     //amarelo

#endif
