#include "typedefs.h"
#include "HWSpecs.h"



#define SYS_STATUS                    (GPR.GPR3)
#define SYS_ERROR                     (GPR.GPR2)

/* Das Pinmap

   | PIN 7 | PIN 6 | PIN 5 | PIN 4 | PIN 3 | PIN 2 | PIN 1 | PIN 0 |
---|-------|-------|-------|-------|-------|-------|-------|-------|
PA | LED 2 |LED_PIN| U0 RX | U0 TX |__SCL__|__SDA__| XTAL2 | XTAL1 |
PB |xxxxxxx|xxxxxxx| MODE  | ENC 1 | ENC 2 | BL B  | BL G  | BL R  |
PC | EVOUT | CCLO1 | U1 RX | U1 TX | ENC2B | ENC2A | ENC1B | ENC1A |
   |  SS   |  SCK  | MISO  |  MOSI |   -   |   -   |   -   |   -   |
PD |  EN   |CONTRAS|  RW   |  RS   |  LCD  |  LCD  |  LCD  |  LCD  |
PE |xxxxxxx|xxxxxxx|xxxxxxx|xxxxxxx| EVOUT |  EXT  | U4 RX | U4 TX |
PE |xxxxxxx|xxxxxxx|xxxxxxx|xxxxxxx|  SS   |  SCK  | MISO  |  MOSI |
PF | UPDI  | RESET |_U2_RX_| U2 TX | RF IN | LED 1 | LED 0 |  AUX  |

Serial Ports:
USART0 (Serial) - Programming + Debug
USART1 (Serial1) - Unused, on EXT pins
USART2 (LED SERIAL) - Likely will be used in half-duplex.
USART3 (Unavailable)
USART4 (Serial4 = Unused on EXT pins)
*/





// UI + encoder involved globals
#define ENC1_PINA   PIN_PC0
#define ENC1_PINB   PIN_PC1
#define ENC2_PINA   PIN_PC2
#define ENC2_PINB   PIN_PC3

#define ENC2_BTN    PIN_PB3
#define ENC1_BTN    PIN_PB4
#define MODE_BTN    PIN_PB5

// Pin Related Macros
// Used to provide prefab port reads.
#define BTN_PORT_READ() ((VPORTB.IN >> 3) & 0x07)
/* 0b00000M21 <---- Encoder 2
 *         \ \----- Encoder 1
 *          \------ MODE
 */

#define ENC1_BTN_bm  (1<<1)
#define ENC2_BTN_bm  (1<<0)
#define MODE_BTN_bm  (1<<2)

#define ENC_PORT_READ (ENC_VPORT_IN & ENC_MASK )
//0b00002211 <---- Encoder 1
//       ^-------- Encoder 2


//Pins to talk to outside world
#define RF_PIN        PIN_PF3
#define LEDPIN        PIN_PA6
// Indicator LEDs
#define IND2          PIN_PA7 // Non-red LED that isn't power.
#define IND1          PIN_PF2 // Red led that isn't power
#define IND0          PIN_PF1 // Green led next to power


//LCD Pins
#define LCD_DATA4     PIN_PD0
#define LCD_DATA5     PIN_PD1
#define LCD_DATA6     PIN_PD2
#define LCD_DATA7     PIN_PD3
#define LCD_RS        PIN_PD4
#define LCD_RW        PIN_PD5
#define LCD_Contrast  PIN_PD6
#define LCD_EN        PIN_PD7
#define LCD_BL_R      PIN_PB0
#define LCD_BL_G      PIN_PB1
#define LCD_BL_B      PIN_PB2

/*****************************
* Debug LED macros           *
*                            *
*****************************/
#define LED0_ON()     digitalWriteFast(IND0, HIGH)
#define LED0_OFF()    digitalWriteFast(IND0,  LOW)
#define LED0(a)       digitalWriteFast(IND0, a)
#define LED1_ON()     digitalWriteFast(IND1, HIGH)
#define LED1_OFF()    digitalWriteFast(IND1,  LOW)
#define LED2(a)       digitalWriteFast(IND1, a)
#define LED2_ON()     digitalWriteFast(IND2, HIGH)
#define LED2_OFF()    digitalWriteFast(IND2,  LOW)
#define LED2(a)       digitalWriteFast(IND2, a)

/************************
 * Serial Configuration *
 *                      *
 ************************/
#define FB_SER        Serial2 // connected to the LED strip head end adapter.
#define FB_SER_TX     PIN_PF4
#define FB_SER_RX     PIN_PF5

#define CONSOLE       Serial
#define CONSOLE_TX    PIN_PA4
#define CONSOLE_RX    PIN_PA5

#define FB_SER_MODE   (HALF_DUPLEX)
#define FB_SER_BAUD   (115200)
#define FB_SER_SWAP   (1)

#define CONSOLE_SWAP  (1)
#define CONSOLE_BAUD  (115200)

// #define EXTEND_SERIAL
#if defined(EXTEND_SERIAL)
  #if EXTEND_SERIAL == 1
    #define EXTEND_SER    Serial1
    #define EXTEND_SER_SWAP 1
  #else
    #define EXTEND_SER    Serial4
    #define EXTEND_SER_SWAP 0
  #endif
#endif
//#define EXTEND_SER_MODE
//#define EXTEND_SER_BAUD
  /* LightCtrl.c */
  void init_Console();
  void init_BL_PWM();
  void set_BL_PWM(uint16_t red, uint16_t green, uint16_t blue);
  bool init_UI_Pins();
  bool init_LCD();
  bool init_POST();
  int16_t enumerate_leds();
  void init_FB();
  /* Modes.c */
  uint8_t getFrameDelay();
  uint8_t getDirection();
  uint8_t getColorMenuNumber();
  uint8_t getColorMenuSelection();
  char* getColorMenuName();
  uint8_t getAdjustMenuNumber();
  uint8_t getAdjustMenuSelection();
  char* getAdjustOptionName();
  bool adjustValue(int8_t change);
  void scrollColorMenu();
  bool scrollAdjustMenu();
  void scrollMode();
  bool setMode(uint8_t num);
  extern controlstate_t Mode;
  extern globalstate_t LedState;
  const modeset_t modes[];



#endif
