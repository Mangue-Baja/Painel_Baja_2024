#include <Arduino.h>
#include <Ticker.h>
#include <string.h>
#include <TM1637Display.h>
#include <TM1637TinyDisplay6.h>
#include <U8g2lib.h>
#include "definitions.h"
#include "hard_defs.h"
#include "soft_defs.h"

HardwareSerial Receiver(2); // Define a Serial port instance called 'Receiver' using serial port 2

/* Interrupts routine */
void ticker2HzISR();
void ticker5HzISR();
void buttonInterruptISR();
void switchInterruptISR();
/* General Functions */
void Pinconfig();
void animacao();
void fourDigits();
void sixDigits();
void LedEmergency();
void temporizador();
void recebedor();
void Leds();
void debounceSpeed();
void Battery_box(int cor);
void LedFuel();
void LedEmergency();
void led_state();
void led_state(uint8_t pin[], uint8_t estado);
void transformador_time_current(Tempo* T);
void all_lines(int cor);
void doublelines(int x1,int y1,int x2,int y2,int quantidade);

//Setup
void setup()
{
  Serial.begin(115200);                                                     // Define and start serial monitor
  Receiver.begin(115200, SERIAL_8N1, Receiver_Txd_pin, Receiver_Rxd_pin);   // Define and start Receiver serial port

  //setup Six digits display
  Six.setBrightness(BRIGHT_HIGH);

  //setup OLED
  u8g2.begin();
  
  Var.velocidade = 0;
  Var.rpm = 0;
  Var.battery = 0;
  Var.combustivel = 0;
  Var.temp_cvt = 0;
  Var.temp_motor = 0;
  Var.SOT = false;

  Pinconfig();

  Switch.Pin = CHAVE;
  Button.Pin = BOTAO;
  
  Switch.buttonState = digitalRead(Switch.Pin);

  delay(100);
};

//Loop
void loop() 
{
  temporizador();   //relogio do enduro
  potenciometro = analogRead(POTENCIOMETRO);   //função para ler o potenciometro

  //Aqui tive que colocar em um if else pelo fato de aparecer um bug quando colocado o recebedor() no mesmo loop da animacao()
  if (Receiver.available()>0 )
  {    
    recebedor();      //recebe os dados da serial pela ECU dianteira
  } else {
    Leds();           //função que contem todos os leds
    animacao();       //animação do Oled
  }
  Var_0 = Var;
};

void Pinconfig()
{
  //Leds
  pinMode(combust_1, OUTPUT);
  pinMode(combust_2, OUTPUT);
  pinMode(combust_3, OUTPUT);
  pinMode(combust_4, OUTPUT);
  pinMode(combust_5, OUTPUT);

  pinMode(cvttemp_led, OUTPUT);
  pinMode(Bat_LED, OUTPUT);
  pinMode(mottemp_led, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
    
  //botões e potenciometros
  pinMode(BOTAO, INPUT_PULLUP);
  pinMode(CHAVE, INPUT);
  pinMode(POTENCIOMETRO, INPUT);

  //Tickers
  ticker2Hz.attach(0.5, ticker2HzISR);
  ticker5Hz.attach_ms(0.2, ticker5HzISR);

  //attachs
  attachInterrupt(digitalPinToInterrupt(BOTAO), buttonInterruptISR, RISING);
  attachInterrupt(digitalPinToInterrupt(CHAVE), switchInterruptISR, CHANGE);

  return;
};
//Funções que os tickers irão fazer
void ticker2HzISR() 
{  
  emergency_led_state = !emergency_led_state;

  if (emergency_led_state) boolean1HZ=true;
};

void ticker5HzISR()
{ 
  boolean5HZ = true;
};

//Declaração das funções para a maquina de estados fulera
void Leds()
{
  if (boolean5HZ)
  {
    LedFuel();
    fourDigits();
    sixDigits();
    LedEmergency();
    boolean5HZ = false;
  }
}; 
  
void animacao()
{                                                                                                                                                                          
  bool chaodetras = false;
  static char Speed[6];
  static char T_cvt[6];
  static char T_motor[6];

  debounceSpeed();
  strcpy(Speed, u8x8_u8toa(Var_0.velocidade, 2));    //variavel da velocidade que será exibida no Oled com 2 digitos
  strcpy(T_cvt, u8x8_u8toa(Var.temp_cvt, 3));        //variavel da temperatura do motor que será exibida no Oled com 3 digitos
  strcpy(T_motor, u8x8_u8toa(Var.temp_motor, 3));    //variavel da temperatura do motor que será exibida no Oled com 3 digitos

  u8g2.firstPage();
  do {

    //u8g2.setFontMode(0);
    u8g2.setFontMode(1);                //cor do background escura não recomendo trocar (esta já é a melhor combinação estetica)
    u8g2.setDrawColor(chaodetras);              
    u8g2.drawBox(0,0,DisplayWidth+4,DisplayHight);
    u8g2.setDrawColor(2);

    //Exibição temperatura do motor e temperatura da cvt
    u8g2.setFont(u8g2_font_crox2c_tn);          //Fonte com 13 pixels de altura
    u8g2.drawStr(79,DisplayHight-1,T_motor);    //valor do Motor
    u8g2.drawStr(0,DisplayHight-1,T_cvt);       //valor do CVT

    //graus celcius
    u8g2.setFont(u8g2_font_7x13_t_cyrillic);    //Fonte com 9 pixels de altura
    u8g2.drawStr(110,DisplayHight-1,"'C");      //motor
    u8g2.drawStr(30,DisplayHight-1,"'C");       //cvt
    u8g2.drawStr(86,33,"Km/h");
    
    (Var.SOT) ? u8g2.drawStr(10,20,"BOX") : u8g2.drawStr(10,20,"   ");
    
    //Velocimetro
    u8g2.setFont(u8g2_font_inb30_mn);           //Fonte com 27 pixels de altura
    u8g2.drawStr((DisplayWidth-u8g2.getStrWidth(Speed))/2,35,Speed);
  
    Battery_box(!chaodetras);

    
  } while (u8g2.nextPage());
}; 
  
//Funcões dos Leds
void LedFuel()
{
  byte intensity_led_brightness = map(potenciometro, 0, 4095, 1, 255); //controle de brilho dos led de emergencia

  //(*) -> ligado
  //( ) -> desligado
  //(*| ) -> pisca a luz de emergencia
  //bool gas_led1_state = true;        //(*)
  //bool gas_led2_state = true;        //(*)
  //bool gas_led3_state = true;        //(*)
  //bool gas_led4_state = true;        //(*)
  //bool gas_led5_state = true;        //(*)

  switch ((int)(Var.combustivel/10)) 
  {
    case (10):
      //todos os leds estão ligados (*)(*)(*)(*)(*)
      //gas_led1_state = emergency_led_state;        //(*)
      //gas_led2_state = emergency_led_state;        //(*)
      //gas_led3_state = emergency_led_state;        //(*)
      //gas_led4_state = emergency_led_state;        //(*)
      //gas_led5_state = emergency_led_state;        //(*)
      analogWrite(combust_1, intensity_led_brightness);
      analogWrite(combust_2, intensity_led_brightness);
      analogWrite(combust_3, intensity_led_brightness);
      analogWrite(combust_4, intensity_led_brightness);
      analogWrite(combust_5, intensity_led_brightness);
    break;

    case (5):
      //gas_led1_state = false;       //( )
      //Os outros 4 leds estão ligados(*)(*)(*)(*)
      analogWrite(combust_2, intensity_led_brightness);
      analogWrite(combust_3, intensity_led_brightness);
      analogWrite(combust_4, intensity_led_brightness);
      analogWrite(combust_5, intensity_led_brightness);
    break;

    case (3):

      //gas_led1_state = false;       //( )
      //gas_led2_state = false;       //( )
      //Os outros 3 leds estão ligados(*)(*)(*)
      analogWrite(combust_3, intensity_led_brightness);
      analogWrite(combust_4, intensity_led_brightness);
      analogWrite(combust_5, intensity_led_brightness);
    break;
    
    case (2):

      //gas_led1_state = false;       //( )
      //gas_led2_state = false;       //( )
      //gas_led3_state = false;       //( )
      //Os outros 2 leds estão ligados(*)(*)
      analogWrite(combust_4, intensity_led_brightness);
      analogWrite(combust_5, intensity_led_brightness);
    break;

    case (1):

      //gas_led1_state = false;       //( )
      //gas_led2_state = false;       //( )
      //gas_led3_state = false;       //( )
      //gas_led4_state = false;       //( )
      //gas_led5_state = emergency_led_state;   //(*| )pisca
      analogWrite(combust_5, emergency_led_state*intensity_led_brightness);
    break;
  }
};

void LedEmergency()
{
  //Leds de Emergência
  //O potenciometro foi feito uma gabiarra horrorosa colocando um poteciometro em paralelo com um outro resistor
  //Se possivel alterar a alimentação do potenciometro no deskboar para 3.3V
  
  //pelo resistor em paralelo tem um registro mais linear dos valores menores 
  //enquanto com o aumento do valor é quase que exponencial em um potenciometro do tipo B

  byte intensity_led_brightness = map(potenciometro, 0, 4095, 1, 255); //controle de brilho dos led de emergencia

  //controle da luz de emergencia da temperatura do Motor     
  if (Var.temp_motor < Alerta_TempMOT)
  {
    digitalWrite(mottemp_led,LOW);
  }  else {
    analogWrite(mottemp_led, emergency_led_state*intensity_led_brightness);
  }
  
  //controle da luz de emergencia da temperatura da CVT
  if (Var.temp_cvt < Alerta_TempCVT) 
  {
    digitalWrite(cvttemp_led,LOW);
  }  else {
    analogWrite(cvttemp_led, emergency_led_state*intensity_led_brightness);
  }

  //controle da luz de emergencia da bateria
  (Var.battery>20) ? digitalWrite(Bat_LED, LOW) : analogWrite(Bat_LED, emergency_led_state*intensity_led_brightness);

};
      
//Função dos displays de 7 segmentos  
void fourDigits() 
{
  //controle do display de 4 digitos e de 7 segmentos para visulização do RPM

  uint8_t intensity_brightness = map(potenciometro,0,4095,1,7);  //mapeia o valor do potenciometro com a intencidade do brilho que só vai de 0 a 8
  
  //setup Four digits display
  Four.setBrightness(intensity_brightness);   //controle de brilho do display
  Four.showNumberDecEx(Var.rpm, 0, true);  //valor exibido no display    o true é para se é para considerar os 0s

};
    
void sixDigits()
{
  /*
    Esses são os valores das posições dos digitos e 7 segmentos que é o ultimo valor colocado na função showNumberDec();

      5   4   3    2   1   0        
      _   _   _    _   _   _
    |_| |_| |_|  |_| |_| |_|
    |_| |_| |_|  |_| |_| |_|

    
  */

  uint8_t intensity_brightness = map(potenciometro,0,4095,1,7);   
  //mapeia o valor do potenciometro com a intencidade do brilho que só vai de 0 a 8

  Six.setBrightness(intensity_brightness);    //controle de brilho do display

  // a variavel dots é para comandar os pontos no display
  //não sei bem como funciona mas sei que ficou do jeito que eu queria essas combinações que eu fiz
  
  uint8_t dots = 0b01010100;      
  switch (six_digits_state) 
  {  
    case TEMPO_DE_ENDURO:
      //Loop do Six digits display

      Six.showNumberDec(Enduro.horas, dots, true, 2, 0);
      Six.showNumberDec(Enduro.minutos,dots, true, 2, 2);
      Six.showNumberDec(Enduro.segundos, 0, true, 2, 4);
      break;
      
    case CRONOMETRO:
      
      if (Button.mode >= 3 && (millis() - Button.lastDebounceTime)<30000) break; 
      
      transformador_time_current(&Volta);
      
      //Cronometrar volta
      dots = 0b01010100;

      Six.showNumber(0, false, 2, 0);
      Six.showNumberDec(Volta.minutos, dots, true , 2, 1);
      Six.showNumberDec(Volta.segundos, dots, true , 2, 3);
      Six.showNumberDec(0, dots, false, 2, 5);
      break;

    case DELTA_CRONOMETRO:
      //Comparação entre duas voltas
      dots = 0b01010100;
  
      if (ultima_volta.tempo_volta_ms >= penultima_volta.tempo_volta_ms)
      {  
        Delta.tempo_volta_ms = ultima_volta.tempo_volta_ms - penultima_volta.tempo_volta_ms; 
        
        Six.showNumberDec(penultima_volta.minutos, dots, true , 1, 0);
        Six.showNumberDec(penultima_volta.segundos, dots, true , 2, 1);
      
        Delta.segundos = Delta.tempo_volta_ms%60;

        Six.showNumber(0, false, 1, 3);
        Six.showNumberDec(Delta.segundos, dots, true , 2, 4);
      }

      else if(ultima_volta.tempo_volta_ms < penultima_volta.tempo_volta_ms)
      {  
        Delta.tempo_volta_ms = penultima_volta.tempo_volta_ms - ultima_volta.tempo_volta_ms; 

        //Six.showNumber(Delta.segundos, true ,3,3);

        Six.showNumberDec(penultima_volta.minutos, dots, true, 1, 0);
        Six.showNumberDec(penultima_volta.segundos, dots, true , 2, 1);
      
        Delta.segundos = Delta.tempo_volta_ms%60;
        
        Six.showString("-", 1, 3, 0);
        Six.showNumberDec(Delta.segundos, dots, true, 2, 4);
      }      
      six_digits_state = CRONOMETRO;
      break;
  }
};

//Função do temporizador
    
void transformador_time_current(Tempo* T)
{
  //transforma o tempo em millis e separa para segundos, minutos e horas;
  uint8_t corrector = ((millis() - T->time_current)/100)%10;

  T->tempo_volta_ms = (millis() - T->time_current)/1000;

  if (corrector > 4) T->segundos = T->tempo_volta_ms%60 + 1;
  else T-> segundos = T->tempo_volta_ms%60;
  

  T-> horas = T->tempo_volta_ms/3600;
  T-> minutos = (T->tempo_volta_ms/60)%60;
  T-> segundos = T->tempo_volta_ms%60;
};
  
void temporizador()
{
  if (boolean1HZ)
  {
    Enduro.segundos++;
    
    if (Enduro.segundos >= 60)
    {

      Enduro.segundos = 0;
      
      Enduro.minutos++;

      if (Enduro.minutos > 60)
      {

        Enduro.minutos = 0;
        
        Enduro.horas++;
            
        if (Enduro.horas > 24)
        {

          Enduro.horas = 0;

        }
      }
    }
    boolean1HZ = false;
  }
};

//função do que o botão
void buttonInterruptISR()
{
  if (Switch.buttonState)
  {
    if ((millis() - Button.lastDebounceTime) > debounceDelay) 
    {
      Button.mode++;

      penultima_volta = ultima_volta;
      ultima_volta = Volta;
      Volta.segundos = 0;
      Volta.minutos = 0;
      Volta.horas = 0;
      Volta.time_current = millis();
      

      if (Button.mode >= 2)
      {
      
        //six_digits_state = DELTA_CRONOMETRO;

      } else {

        six_digits_state = CRONOMETRO;
      
      };
    } 
    Button.lastDebounceTime = millis(); 
  }
};
    
//função do que do interruptor
void switchInterruptISR()
{
  if ((millis() - Switch.lastDebounceTime) > debounceDelay) 
  {
    
    Switch.buttonState = digitalRead(Switch.Pin);

    if (Switch.buttonState==0)
    {
      six_digits_state = TEMPO_DE_ENDURO;
    }

    if (Switch.buttonState==1)
    {
      six_digits_state = CRONOMETRO;
      Button.mode = 0;
    }
    Volta.time_current = millis();
  }
  Switch.lastDebounceTime = millis();
};

//Funcões para desing do Oled(espero que você saiba o basico de GA para editar isso)
//lembrando que se desenhar muitos objetos no Oled ele vai ficar cada vez mais lento para executar

void doublelines(int x1,int y1,int x2,int y2,int quantidade)
{  
  u8g2.drawLine(x1,y1,x2,y2);
  u8g2.drawLine(DisplayWidth-x1,y1,DisplayWidth-x2,y2);
  
  if (quantidade >= 2)
  {
  //linha da direita
  u8g2.drawLine(x1+1,y1,x2+1,y2);
  //linha da esquerda
  u8g2.drawLine(DisplayWidth-x1-1,y1,DisplayWidth-x2-1,y2);
  }

  if (quantidade >= 3)
  {
  //linha da direita
  u8g2.drawLine(x1+2,y1,x2+2,y2);
  //linha da esquerda
  u8g2.drawLine(DisplayWidth-x1-2,y1,DisplayWidth-x2-2,y2);
  }
};

void all_lines(int cor)
{
  u8g2.setDrawColor(cor);
  u8g2.drawBox(40, DisplayHight-20, 40, 10);

  u8g2.setDrawColor(2);
};

void Battery_box(int cor)
{  
  uint8_t x = 55;
  uint8_t y = 3;

  //icone  da bateria
  u8g2.setDrawColor(cor);
  u8g2.drawBox(x+7, DisplayHight-y-4*(y+1)-4, DisplayWidth - 2*(x+5),2);
  u8g2.drawBox(x, DisplayHight-y-4*(y+1)-2, DisplayWidth - 2*x+4, 5*(y+1)+2);
    
  u8g2.setDrawColor(!cor);
  u8g2.drawBox(x+1, DisplayHight-y-4*(y+1)-1, DisplayWidth - 2*x+2, 5*(y+1));
  
  //tracejado do icone da bateria
  if (Var.battery >= 0)
  {      
    u8g2.setDrawColor(cor);
    
    if (Var.battery >= 20)
    {
      if (Var.battery >= 40)
      {
        if (Var.battery >= 60)
        {
          if (Var.battery >= 80)
          {  
            u8g2.drawBox(x+2, DisplayHight-y-4*(y+1), DisplayWidth - 2*x, y);                
          }
          u8g2.drawBox(x+2, DisplayHight-y-3*(y+1), DisplayWidth - 2*x, y); 
        }
        u8g2.drawBox(x+2, DisplayHight-y-2*(y+1), DisplayWidth - 2*x, y);
      }
      u8g2.drawBox(x+2, DisplayHight-y-(y+1), DisplayWidth - 2*x, y);
    }
    u8g2.drawBox(x+2, DisplayHight-y, DisplayWidth - 2*x, y);
  }
  u8g2.setDrawColor(2);  
};

void debounceSpeed(){
  //filtro da velocidade para acelerações
  uint16_t deltaV;

  if (Var_0.velocidade < Var.velocidade)
  {
    deltaV = Var.velocidade - Var_0.velocidade;

    if (deltaV>2)
    {

      Var_0.velocidade+=3;
      
    } else {

      Var_0.velocidade+=1;
    
    }
  }

  if (Var_0.velocidade > Var.velocidade)
  {
 
    deltaV = Var_0.velocidade - Var.velocidade;

    if (deltaV>2)
    {

      Var_0.velocidade-=3;

    } else {
        
      Var_0.velocidade-=1;

    }
  }
};

//Função para receber os dados da ECU dianteira
void recebedor()
{ 
  uint8_t byteCount = 0;
  
  memset(dado_arr, 0, sizeof(Txtmng));  //o array e=é limpado com o valor 0 

  while(byteCount < sizeof(Txtmng))
  {

    dado_arr[byteCount] = Receiver.read(); //então os dados em bits que estão chegnado da serial é colocado no array

    if(byteCount==0)      Serial.printf("\r\nvelocidade = %d\r\n", Var.velocidade);
    else if(byteCount==1) Serial.printf("\r\nrpm = %d\r\n", Var.rpm);
    else if(byteCount==2) Serial.printf("\r\nSOC = %d\r\n", Var.rpm);
    else if(byteCount==3) Serial.printf("\r\nfuel level = %d\r\n", Var.combustivel);
    else if(byteCount==4) Serial.printf("\r\ntemp motor = %d\r\n", Var.temp_motor);
    else if(byteCount==5) Serial.printf("\r\ntemp CVT = %d\r\n", Var.temp_cvt);
    else if(byteCount==6) Serial.printf("\r\nSOT = %d\r\n", Var.SOT);
    
    byteCount++;
  }
  
  Serial.println("\n\n\n");
  memcpy(&Var, (Txtmng *)dado_arr, sizeof(Txtmng)); //então os valores em bits do array são colocados no Struct Var oara ser usado
};