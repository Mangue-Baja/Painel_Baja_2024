#ifndef HARD_DEF
#define HARD_DEF

#include <Arduino.h>

// Four digits display
#define Fourdigits_DIO      15     // 4-digit display DATA
#define Fourdigits_CLK      4      // 4-digit display CLOCK

// Six digits display
#define Sixdigits_DIO       18     // 6-digit display DATA
#define Sixdigits_CLK       19     // 6-digit display CLOCK

// OLED
#define SCLK_OLED           21     // OLED CLOCK
#define SDA_OLED            22     // OLED DATA

#define DisplayHight        64 
#define DisplayWidth        124    

// Leds
#define combust_1           14    // 1st fuel LED
#define combust_2           27    // 2st fuel LED
#define combust_3           25    // 3st fuel LED 
#define combust_4           33    // 4st fuel LED
#define combust_5           32    // fuel emergency LED 

#define MotorTemp_led       13    // Motor temperature warning LED
#define CVTtemp_led         12    // CVT temperature warning LED
#define Battery_led         26    // Battery percentage warning LED

// Buttons and keys for interactions with the user in the car
#define BUTTON              23    // Green
#define SWITCH              35    // Purple
#define POTENTIOMETER       34    // Gray

#define Alert_TempCVT       90    // Estimated temperature of when the cvt belt may have a problem
#define Alert_TempMOT       105   // Estimated temperature for engine temperature problems

// Serial Communication with the Front ECU (MMI)
#define Receiver_Rxd_pin    17    // Blue
#define Receiver_Txd_pin    16    // Yellow

#endif
