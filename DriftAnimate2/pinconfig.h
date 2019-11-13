// UI + encoder involved globals
// General functions that vary based on what hardware is in use

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
#ifdef RED_DOT
LiquidCrystal_I2C lcd(0x3F, 16, 2); //red dot
#else
LiquidCrystal_I2C lcd(0x27, 16, 2); //green dot
#endif
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

void setupLCD() {
  #ifdef __AVR_ATmega328P__
  Wire.begin();
  lcd.begin();
  #else 
  lcd.begin(16,2);
  #endif
}

void backlightOn() {
  #ifdef __AVR_ATmega328P__
  lcd.backlight();
  #else
  #ifndef __AVR_ATmega1284P__
  pinMode(18,OUTPUT);
  digitalWrite(18,1);
  #else
  pinMode(20,OUTPUT);
  digitalWrite(20,1);
  #endif
  #endif
}

void backlightOff() {
    lcd.noBacklight();
  #else
  #ifndef __AVR_ATmega1284P__
  digitalWrite(18,0);
  #else
  digitalWrite(20,0);
  #endif
  #endif
}

void setupPins() {
  pinMode(LEDPIN, OUTPUT);
  pinMode(ENC1_PINA, INPUT_PULLUP);
  pinMode(ENC1_PINB, INPUT_PULLUP);
  pinMode(ENC1_BTN, INPUT_PULLUP);
  pinMode(ENC2_PINA, INPUT_PULLUP);
  pinMode(ENC2_PINB, INPUT_PULLUP);
  pinMode(ENC2_BTN, INPUT_PULLUP);
  pinMode(MODE_BTN, INPUT_PULLUP);
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
}

void setupPCINT() {
#ifndef __AVR_ATmega1284P__ //encoders are PCINT8~15 range on 328p
  PCMSK1 = 0x0F;
  PCICR = 2;
#else //encoders are PCINT16~24 range on 1284p
  PCMSK2 = 0x0F;
  PCICR = 4;
#endif
}



