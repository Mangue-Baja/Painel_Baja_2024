#include <Arduino.h>
#include <Ticker.h>
#include <TM1637Display.h>
#include <TM1637TinyDisplay6.h>
#include <U8g2lib.h>
#include "definitions.h"
#include "hard_defs.h"
#include "soft_defs.h"

/* ESP Tools */
HardwareSerial Receiver(2); // Defines a Serial port instance called 'Receiver' using serial port 2
Ticker ticker2Hz;
Ticker ticker5Hz;
/* Global Variables */
Txtmng Var, Var_0;
uint16_t potenciometro;
uint16_t dado_arr[sizeof(Txtmng)];     // array that receives data in Bits from the front ECU (MMI)

/* Interrupts routine */
void buttonInterruptISR();
void switchInterruptISR();
void ticker2HzISR();
void ticker5HzISR();
/* General Functions */
void SetupPacket();
void Pinconfig();
void temporizador();
void recebedor(); // Receiver Function
// Leds Functions 
void Leds();
void animacao(); // Animation Function
void LedFuel();
void LedEmergency();
// Display Functions
void fourDigits();
void sixDigits();
void transformador_time_current(Tempo* T);
void debounceSpeed();
void Battery_box(int cor);
//void all_lines(int cor);
//void doublelines(int x1,int y1,int x2,int y2,int quantidade);

void setup()
{
  Serial.begin(115200);                                                     // Define and start serial monitor
  Receiver.begin(115200, SERIAL_8N1, Receiver_Txd_pin, Receiver_Rxd_pin);   // Define and start Receiver serial port

  //setup Six digits display
  Six.setBrightness(BRIGHT_HIGH);

  //setup OLED
  u8g2.begin();

  SetupPacket();
  Pinconfig();

  Switch.Pin = CHAVE;
  Button.Pin = BOTAO;
  
  Switch.buttonState = digitalRead(Switch.Pin);

  delay(100);
}

void loop() 
{
  temporizador();   //Enduro watch
  potenciometro = analogRead(POTENCIOMETRO);   // Function to read the potentiometer

  // Here I had to put it in an if else because a bug appeared when placing the receiver() in the same loop as the animation()
  if(Receiver.available()>0 )
  {    
    recebedor();      // Receives serial data from the front ECU
  } else {
    Leds();           // Function that contains all LEDs
    animacao();       // Oled animation
  }
  Var_0 = Var;
}

/* Setup functions */
void SetupPacket()
{
  Var.velocidade = 0;
  Var.rpm = 0;
  Var.battery = 0;
  Var.combustivel = 0;
  Var.temp_cvt = 0;
  Var.temp_motor = 0;
  Var.telemetry = 0;
}

void Pinconfig()
{
  //Leds
  /*
  pinMode(combust_1, OUTPUT);
  pinMode(combust_2, OUTPUT);
  pinMode(combust_3, OUTPUT);
  pinMode(combust_4, OUTPUT);
  pinMode(combust_5, OUTPUT);
  */

  pinMode(cvttemp_led, OUTPUT);
  pinMode(Bat_LED, OUTPUT);
  pinMode(mottemp_led, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(combust_1, OUTPUT);
  pinMode(combust_2, OUTPUT);
  pinMode(combust_3, OUTPUT);
  pinMode(combust_4, OUTPUT);
  pinMode(combust_5, OUTPUT);
    
  //Buttons and potentiometers
  pinMode(BOTAO, INPUT_PULLUP);
  pinMode(CHAVE, INPUT);
  pinMode(POTENCIOMETRO, INPUT);

  //Tickers
  ticker2Hz.attach(0.5, ticker2HzISR);
  ticker5Hz.attach(0.2, ticker5HzISR);

  //Interruptions
  attachInterrupt(digitalPinToInterrupt(BOTAO), buttonInterruptISR, RISING);
  attachInterrupt(digitalPinToInterrupt(CHAVE), switchInterruptISR, CHANGE);

  return;
}

/* Loop Functions */
void temporizador()
{
  if(boolean1HZ)
  {
    Enduro.segundos++;
    
    if(Enduro.segundos >= 60)
    {

      Enduro.segundos = 0;
      
      Enduro.minutos++;

      if(Enduro.minutos > 60)
      {

        Enduro.minutos = 0;
        
        Enduro.horas++;
            
        if(Enduro.horas > 24)
        {

          Enduro.horas = 0;

        }
      }
    }
    boolean1HZ = false;
  }
}

void recebedor()
{
  uint8_t byteCount = 0;
  
  memset(dado_arr, 0, sizeof(Txtmng));  // the array is cleared with the value 0

  while(byteCount < sizeof(Txtmng))
  {

    dado_arr[byteCount] = Receiver.read(); //then the data in bits that arrives from the serial is placed in the array
    //Serial.printf("Data = %d => c = %d\n", dado_arr[byteCount], byteCount);
    //if(byteCount==0) Serial.printf("velocidade = %d\r\n", Var.velocidade);
    //if(byteCount==1) Serial.printf("rpm = %d\r\n", Var.rpm*6);
    //if(byteCount==2) Serial.printf("SOC = %d\r\n", Var.battery);
    //if(byteCount==3) Serial.printf("fuel level = %d\r\n", Var.combustivel);
    //if(byteCount==4) Serial.printf("temp motor = %d\r\n", Var.temp_motor);
    //if(byteCount==5) Serial.printf("temp CVT = %d\r\n", Var.temp_cvt);
    //if(byteCount==6) Serial.printf("SOT = %d\r\n", Var.telemetry);
    
    byteCount++;
  }
  
  Serial.println("\n");
  memcpy(&Var, (Txtmng *)dado_arr, sizeof(Txtmng)); // then the bit values ​​of the array are placed in the Struct Var to be used
}

void Leds()
{
  if(boolean5HZ)
  {
    LedFuel();
    LedEmergency();
    fourDigits();
    sixDigits();
    boolean5HZ = false;
  }
}

void animacao()
{
  //remember that if you draw many objects on the Oled it will become increasingly slower to execute                                                                                                                                                                        
  bool chaodetras = false;
  static char Speed[6];
  static char T_cvt[6];
  static char T_motor[6];

  debounceSpeed();
  strcpy(Speed, u8x8_u8toa(Var.velocidade, 2));    //speed variable that will be displayed on the Oled with 2 digits
  strcpy(T_cvt, u8x8_u8toa(Var.temp_cvt, 3));      //CVT temperature variable that will be displayed on the OLED with 3 digits
  strcpy(T_motor, u8x8_u8toa(Var.temp_motor, 3));  //Motor temperature variable that will be displayed on the OLED with 3 digits

  u8g2.firstPage();
  do {

    //u8g2.setFontMode(0);
    u8g2.setFontMode(1);                //dark background color I don't recommend changing (this is already the best aesthetic combination)
    u8g2.setDrawColor(chaodetras);              
    u8g2.drawBox(0,0,DisplayWidth+4,DisplayHight);
    u8g2.setDrawColor(2);

    //Display engine temperature and CVT temperature
    u8g2.setFont(u8g2_font_crox2c_tn);          //Font 13 pixels high
    u8g2.drawStr(79,DisplayHight-1,T_motor);    //Motor value
    u8g2.drawStr(0,DisplayHight-1,T_cvt);       //CVT value

    //graus celcius
    u8g2.setFont(u8g2_font_7x13_t_cyrillic);    //Font 9 pixels high
    u8g2.drawStr(110,DisplayHight-1,"'C");      //motor
    u8g2.drawStr(30,DisplayHight-1,"'C");       //cvt
    u8g2.drawStr(86,33,"Km/h");
    
    if(Var.telemetry==1)
      u8g2.drawStr(10,20,"ON");
    else
      u8g2.drawStr(10,20,"   ");    
        
    //Speedometer
    u8g2.setFont(u8g2_font_inb30_mn);           //Font 27 pixels high
    u8g2.drawStr((DisplayWidth-u8g2.getStrWidth(Speed))/2,35,Speed);
  
    Battery_box(!chaodetras);

  } while(u8g2.nextPage());
}

/* General Functions */
// Led functions
void LedFuel()
{
  uint8_t intensity_led_brightness = map(potenciometro, 0, 4095, 1, 255); // Emergency LED brightness control;
  //(*) -> ON
  //( ) -> OFF
  //(*| ) -> emergency light flashes

  switch((int)(Var.combustivel/10)) 
  {
    case (10):
      // (*)(*)(*)(*)(*)
      analogWrite(combust_1, intensity_led_brightness);
      analogWrite(combust_2, intensity_led_brightness);
      analogWrite(combust_3, intensity_led_brightness);
      analogWrite(combust_4, intensity_led_brightness);
      analogWrite(combust_5, intensity_led_brightness);
    break;

    case (5):
      // ( )(*)(*)(*)(*)
      analogWrite(combust_1, LOW);
      analogWrite(combust_2, intensity_led_brightness);
      analogWrite(combust_3, intensity_led_brightness);
      analogWrite(combust_4, intensity_led_brightness);
      analogWrite(combust_5, intensity_led_brightness);
    break;

    case (3):
      // ( )( )(*)(*)(*)
      analogWrite(combust_1, LOW);
      analogWrite(combust_2, LOW);
      analogWrite(combust_3, intensity_led_brightness);
      analogWrite(combust_4, intensity_led_brightness);
      analogWrite(combust_5, intensity_led_brightness);
    break;
    
    case (2):
      // ( )( )( )(*)(*)
      analogWrite(combust_1, LOW);
      analogWrite(combust_2, LOW);
      analogWrite(combust_3, LOW);
      analogWrite(combust_4, intensity_led_brightness);
      analogWrite(combust_5, intensity_led_brightness);
    break;

    case (1):
      // ( )( )( )( )(*|)
      analogWrite(combust_1, LOW);
      analogWrite(combust_2, LOW);
      analogWrite(combust_3, LOW);
      analogWrite(combust_4, LOW);
      analogWrite(combust_5, intensity_led_brightness*emergency_led_state);
    break;

    default:
      //analogWrite(combust_5, intensity_led_brightness*emergency_led_state);
      break;
  }
}

void LedEmergency()
{
  uint8_t intensity_led_brightness = map(potenciometro, 0, 4095, 1, 255); // Emergency LED brightness control;
  //Leds de Emergência
  //O potenciometro foi feito uma gabiarra horrorosa colocando um poteciometro em paralelo com um outro resistor
  //Se possivel alterar a alimentação do potenciometro no deskboar para 3.3V
  
  //pelo resistor em paralelo tem um registro mais linear dos valores menores 
  //enquanto com o aumento do valor é quase que exponencial em um potenciometro do tipo B
  
  //control of the Engine temperature emergency light     
  if(Var.temp_motor < Alerta_TempMOT)
  {
    digitalWrite(mottemp_led, LOW);
  }  else {
    analogWrite(mottemp_led, emergency_led_state*intensity_led_brightness);
  }
  
  //control of the CVT temperature emergency light
  if(Var.temp_cvt < Alerta_TempCVT) 
  {
    digitalWrite(cvttemp_led, LOW);
  }  else {
    analogWrite(cvttemp_led, emergency_led_state*intensity_led_brightness);
  }

  //battery emergency light control
  if(Var.battery > 20)
  {
    digitalWrite(Bat_LED, LOW);
  } else {
    analogWrite(Bat_LED, emergency_led_state*intensity_led_brightness);
  }
}

//Display Segments Functions
void fourDigits() 
{
  uint8_t intensity_led_brightness = map(potenciometro, 0, 4095, 1, 255); // Emergency LED brightness control;
  //control of the 4-digit and 7-segment display to display the RPM
  //setup Four digits display
  Four.setBrightness(intensity_led_brightness);   //display brightness control
  Four.showNumberDecEx(Var.rpm*6, 0, true);       //value displayed on the display true is for if 0s are to be considered
}

void sixDigits()
{
  uint8_t intensity_led_brightness = map(potenciometro, 0, 4095, 1, 255); // Emergency LED brightness control;
  /*
    These are the values ​​of the digit positions and 7 segments, which is the last value placed in the showNumberDec() function

      5   4   3    2   1   0        
      _   _   _    _   _   _
    |_| |_| |_|  |_| |_| |_|
    |_| |_| |_|  |_| |_| |_|

    
  */

  Six.setBrightness(intensity_led_brightness);    //display brightness control

  // the dots variable is to control the dots on the display
  //I don't really know how it works but I know it turned out the way I wanted these combinations I made
  
  uint8_t dots = 0b01010100;      
  switch(six_digits_state) 
  {  
    case TEMPO_DE_ENDURO:
      //Six digits display loop

      Six.showNumberDec(Enduro.horas, dots, true, 2, 0);
      Six.showNumberDec(Enduro.minutos,dots, true, 2, 2);
      Six.showNumberDec(Enduro.segundos, 0, true, 2, 4);
      break;
      
    case CRONOMETRO:
      
      if(Button.mode >= 3 && (millis() - Button.lastDebounceTime)<30000) break; 
      
      transformador_time_current(&Volta);
      
      //Time lap
      dots = 0b01010100;

      Six.showNumber(0, false, 2, 0);
      Six.showNumberDec(Volta.minutos, dots, true , 2, 1);
      Six.showNumberDec(Volta.segundos, dots, true , 2, 3);
      Six.showNumberDec(0, dots, false, 2, 5);
      break;

    case DELTA_CRONOMETRO:
      //Compare between two turns
      dots = 0b01010100;
  
      if(ultima_volta.tempo_volta_ms >= penultima_volta.tempo_volta_ms)
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
}

void transformador_time_current(Tempo* T)
{
  //transforms time into millis and separates it into seconds, minutes and hours
  uint8_t corrector = ((millis() - T->time_current)/100)%10;

  T->tempo_volta_ms = (millis() - T->time_current)/1000;

  if(corrector > 4) T->segundos = T->tempo_volta_ms%60 + 1;
  else T-> segundos = T->tempo_volta_ms%60;
  

  T-> horas = T->tempo_volta_ms/3600;
  T-> minutos = (T->tempo_volta_ms/60)%60;
  T-> segundos = T->tempo_volta_ms%60;
}

//Animation Functions (Display)
void debounceSpeed()
{
  //speed filter for accelerations
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

void Battery_box(int cor)
{  
  uint8_t x = 55;
  uint8_t y = 3;

  //battery icon
  u8g2.setDrawColor(cor);
  u8g2.drawBox(x+7, DisplayHight-y-4*(y+1)-4, DisplayWidth - 2*(x+5),2);
  u8g2.drawBox(x, DisplayHight-y-4*(y+1)-2, DisplayWidth - 2*x+4, 5*(y+1)+2);
    
  u8g2.setDrawColor(!cor);
  u8g2.drawBox(x+1, DisplayHight-y-4*(y+1)-1, DisplayWidth - 2*x+2, 5*(y+1));
  
  //dashed battery icon
  if(Var.battery >= 0)
  {      
    u8g2.setDrawColor(cor);
    
    if(Var.battery >= 20)
    {
      if(Var.battery >= 40)
      {
        if(Var.battery >= 60)
        {
          if(Var.battery >= 80)
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
}

/* Interrupts routine */
void buttonInterruptISR()
{
  if(Switch.buttonState)
  {
     if((millis() - Button.lastDebounceTime) > debounceDelay) 
     {
      Button.mode++;

      penultima_volta = ultima_volta;
      ultima_volta = Volta;
      Volta.segundos = 0;
      Volta.minutos = 0;
      Volta.horas = 0;
      Volta.time_current = millis();
      
      if(Button.mode >= 2)
      {

      //six_digits_state = DELTA_CRONOMETRO;
      
      } else {
        six_digits_state = CRONOMETRO;
      };
    }  
    Button.lastDebounceTime = millis();   
    //ESP.restart();
  }
};

void switchInterruptISR()
{
  if((millis() - Switch.lastDebounceTime) > debounceDelay) 
  {
    
    Switch.buttonState = digitalRead(Switch.Pin);

    if(Switch.buttonState==0)
    {
      six_digits_state = TEMPO_DE_ENDURO;
    }

    if(Switch.buttonState==1)
    {
      six_digits_state = CRONOMETRO;
      Button.mode = 0;
    }
    Volta.time_current = millis();
  }
  Switch.lastDebounceTime = millis();
};

void ticker2HzISR() 
{  
  emergency_led_state = !emergency_led_state;

  if(emergency_led_state) boolean1HZ=true;
};

void ticker5HzISR()
{ 
  boolean5HZ = true;
};
            
/* Useless, only for backup */
/*void doublelines(int x1,int y1,int x2,int y2,int quantidade)
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
};*/

/*void all_lines(int cor)
{
  u8g2.setDrawColor(cor);
  u8g2.drawBox(40, DisplayHight-20, 40, 10);

  u8g2.setDrawColor(2);
};*/