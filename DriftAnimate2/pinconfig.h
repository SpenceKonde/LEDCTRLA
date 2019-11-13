// UI + encoder involved globals

#ifndef __AVR_ATmega1284P__
#ifdef __AVR_ATmega328PB__
#include <LiquidCrystal.h> 
LiquidCrystal lcd(5, 6, 7, 23, 24, 25, 26);
//LiquidCrystal lcd(5, 6, 7, 26, 25, 24, 23);
#define LDR_PIN A5
#define LEDPIN 10
#else //ATmega328p with I2C LCD
#define LEDPIN 9
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2); //red dot
//LiquidCrystal_I2C lcd(0x27, 16, 2); //green dot
#define LDR_PIN A6
#endif
#define RX_PIN_STATE (PINB&1) //RX on pin 8 for input capture. 
#define ENC1_PINA 14
#define ENC1_PINB 15
#define ENC2_PINA 16
#define ENC2_PINB 17
#define ENC1_BTN 3
#define ENC2_BTN 4
#define MODE_BTN 2
#else
#include <LiquidCrystal.h> //ATmega1284p with LCD connected in 4-bit mode
LiquidCrystal lcd(2, 3, 4, 24, 25, 26, 27);
#define RX_PIN_STATE (PIND&64) //RX on PD6 for input capture. 
#define LEDPIN 21
#define LDR_PIN A4
#define ENC1_PINA 16
#define ENC1_PINB 17
#define ENC2_PINA 18
#define ENC2_PINB 19
#define ENC1_BTN 12
#define ENC2_BTN 11
#define MODE_BTN 10
#endif
