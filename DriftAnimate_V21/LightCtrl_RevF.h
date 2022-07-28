#ifndef __LightCtrl_h
#define __LightCtrl_h
#include "Arduino.h"
#include "Modes.h"
#include "Colors.h"
#include "HWSpecs.h"



#define SYS_STATUS                    (GPR.GPR3)
#define STATUS_RX_bp                  (0)
#define STATUS_NEW_MSG_bp             (1)
// TBD                                (2)
// TBD                                (3)
#define STATUS_COLOR_CHANGE_bp        (4)
#define STATUS_ADJUST_CHANGE_bp       (5)
#define STATUS_MODE_CHANGE_bp         (6)
#define STATUS_UI_CHANGE_bp           (7)
#define RX_ONGOING                    (1 << STATUS_RX_bp)
#define NEW_MSG                       (1 << STATUS_NEW_MSG_bp)
#define COLOR_CHANGE                  (1 << STATUS_COLOR_CHANGE_bp)
#define ADJUST_CHANGE                 (1 << STATUS_ADJUST_CHANGE_bp)
#define MODE_CHANGE                   (1 << STATUS_MODE_CHANGE_bp)
#define UI_CHANGE                     (1 << STATUS_UI_CHANGE_bp)
/* used for system status flags
 *  bit 0 = receiving
 *  bit 1 = new message
 *  bit 2 =
 *  bit 3 =
 *  bit 4 = color menu changed. We need to process this before any additional input is handled/
 *  bit 5 = Adjust menu changed. We need to process this before any additional input is handled.
 *  bit 6 = Mode has been changed. We need to process this before any input is handled.
 *  bit 7 = UI changed - redraw it.
 */

/* Serious errors */
#define SYS_ERROR                     (GPR.GPR2)
#define ERROR_NO_CFG_bp               (0)
#define ERROR_BAD_CFG_bp              (1)
// TBD                                (2)
// TBD                                (3)
// TBD                                (4)
#define ERROR_NO_LCD_bp               (5)
#define ERROR_NO_ENC_bp               (6)
#define ERROR_FATAL_CLR_bp            (7)
#define NO_CFG                        (1 << ERROR_NO_CFG_bp)
#define BAD_CFG                       (1 << ERROR_BAD_CFG_bp)
#define NO_LCD                        (1 << ERROR_NO_LCD_bp)
#define NO_ENCODERS                   (1 << ERROR_NO_ENC_bp)
#define FATAL_CLR                     (1 << ERROR_FATAL_CLR_bp)
#define FATAL_ERRORS_gm               (FATAL_CLR | NO_ENCODERS | NO_LCD)
/* used for errors that preclude normal operation.
 *  bit 0 = There is no configuration data stored
 *  bit 1 = Configuration data was found, but invalid
 *  bit 2 =
 *  bit 3 =
 *  bit 4 =
 *  bit 5 = If code attempts, for some stupid reason, to clear a hardware error from software.
 *  bit 6 = If we are unable to communicate with the LCD, that constitutes a fatal error, as we cannot display output to the user
 *  bit 7 = The encoders, on PORTC, are powered by VDDIO2. This bit is set if we find the MVIO status to be 0 even after waiting 2 seconds.
 *              We cannot read the pins if there is no power on VDDIO2, so we cannot receive most user input.
 */
#define StatusGet(bitmask) = (!!(SYS_STATUS & bitmask))
#define ErrorGet(bitmask) = (!!(SYS_ERROR & bitmask))
#define Fatal_Error_Check  = (!!(SYS_ERROR & FATAL_ERRORS_gm))
/* These should *not* be getting called with variable arguments. However if they do, an ill-timed ISR could cause malfunction
 * when setting or clearing, so we need to test that. If we can constant-fold it, and multiple bits aren't set (also a no-no)
 * we just set or clear it and the compiler will generate a single cbi or sbi instruction
 * Ottherwise, we need to turn off interrupts while we do it, but it's still not painful to inline:
 * in, cli, in, and, out, out
 */
// Status set bit
inline void __attribute__((always_inline)) StatusSet(bitmask) {
  if (__builtin_constant_p(bitmask) && (!((bitmask - 1) & (bitmask)))) {
    SYS_STATUS |= bitmask;
  } else {
    uint8_t old_sreg = SREG;
    cli();
    SYS_STATUS |= bitmask;
    SREG=old_sreg;
  }
}
// Status clear bit
inline void __attribute__((always_inline)) StatusClr(bitmask) {
  if (__builtin_constant_p(bitmask) && (!((bitmask - 1) & (bitmask)))) {
    SYS_STATUS &= ~bitmask;
  } else {
    uint8_t old_sreg = SREG;
    cli();
    SYS_STATUS &= ~bitmask;
    SREG = old_sreg;
  }
}
// Status load and clear bit.
inline bool __attribute__((always_inline)) StatusLAC(bitmask) {
  if (__builtin_constant_p(bitmask) && (!((bitmask - 1) & (bitmask)))) {
    uint8_t old_sreg = SREG;
    cli();
    uint8_t r = !!(SYS_STATUS & bitmask);
    SYS_STATUS &= ~bitmask;
    SREG = old_sreg;
    return r;
  } else {
    badArg("StatusLAC only accepts constant bitmask");
    return 0;
  }
}

inline void __attribute__((always_inline)) ErrorSet(bitmask) {
    if (__builtin_constant_p(bitmask) && (!((bitmask - 1) & (bitmask)))) {
    SYS_ERROR |= bitmask;
  } else {
    uint8_t old_sreg = SREG;
    cli();
    SYS_ERROR |= bitmask;
    SREG = old_sreg;
  }
}

// Fatal errors can't be cleared; they indicate a hardware fault and
// the only remedy is to fix the device.
inline void __attribute__((always_inline)) ErrorClr(bitmask) {
  if ((bitmask & ~FATAL_ERROR_gm)) {
    if (__builtin_constant_p(bitmask) && (!((bitmask - 1) & (bitmask)))) {
      SYS_ERROR &= ~bitmask;
    } else {
      uint8_t old_sreg = SREG;
      cli();
      SYS_ERROR &= ~bitmask;
      SREG = old_sreg;
    }
  } else {
    ErrorSet(FATAL_CLR);
  }
}

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
#define LEDPIN        PIN_PA6
#define RF_PIN        PIN_PF3
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
#define LED0_ON()     digitalWriteFast(INDICATE0, HIGH)
#define LED0_OFF()    digitalWriteFast(INDICATE0,  LOW)
#define LED1_ON()     digitalWriteFast(INDICATE1, HIGH)
#define LED1_OFF()    digitalWriteFast(INDICATE1,  LOW)
#define LED2_ON()     digitalWriteFast(INDICATE2, HIGH)
#define LED2_OFF()    digitalWriteFast(INDICATE2,  LOW)

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
//#define EXTEND_SER    Serial1
//#define EXTEND_SER    Serial4
#define FB_SER_MODE   (HALF_DUPLEX)
#define FB_SER_BAUD   (115200)
#define FB_SER_SWAP   (1)
#define CONSOLE_SWAP  (1)
#define CONSOLE_BAUD  (115200)
//#define EXTEND_SER_MODE
//#define EXTEND_SER_BAUD
//#define EXTEND_SER_SWAP  (1) // If Serial1 is used, this MUST be 1. If Serial4 is used, this MUST be 0.


/*****************************
 *     RF related globals    *
 *  Macros, buffers, etc     *
 *                           *
 ****************************/
#define MyAddress     (Hardware->RF_config & 0x3F);
#define RFRX_NOW()    (SYS_STATUS  &    (1 << RX_BITPOS))
#define START_RX()    (SYS_STATUS  |=   (1 << RX_BITPOS); digitalWriteFast(INDICATE0, HIGH);)
#define DONE_RX()     (SYS_STATUS  &= (~(1 << RX_BITPOS)); digitalWriteFast(INCICATE0, LOW;))
#define SET_MESSAGE() ({SYS_STATUS |=   (1 << NEW_MSG_BITPOS);})
#define GOT_MESSAGE() (SYS_STATUS  &    (1 << NEW_MSG_BITPOS);)
#define CLR_MESSAGE() ({SYS_STATUS &= (~(1 << NEW_MSG_BITPOS);)})
#define RX_PIN_STATE  (digitalReadFast(RF_PIN))

// Version 2.2/2.3

#ifdef SKETCH_MAIN
  volatile byte bitnum                = 0; //current bit received

  volatile byte dataIn                = 0;
  volatile byte pktLength             = 31;
  volatile unsigned long lastRFMsgAt  = 0;

  volatile byte rxBuffer[32]          = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0;
  }
  byte recvMessage[32];               = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0;
  }

  unsigned long lastPacketTime = 0;
  unsigned long lastPacketSig = 0;

  #if(F_CPU==8000000)
    #define TIME_MULT(t) (t)
  #elif(F_CPU==16000000)
    #define TIME_MULT(t) (t * 2 )
  #elif(F_CPU==20000000)
    #define TIME_MULT(t) (t *  5) >> (1)
  #elif(F_CPU==24000000)
    #define TIME_MULT(t) (t * 3 )
  #elif(F_CPU==30000000)
    #define TIME_MULT(t) (t * 15) >> (2)
  #elif(F_CPU==32000000)
    #define TIME_MULT(t) (t * 4 )
  #elif(F_CPU==36000000)
    #define TIME_MULT(t) (t * 9 ) >> (1)
  #elif(F_CPU==40000000)
    #define TIME_MULT(t) (t * 5 )
  #elif(F_CPU==48000000)
    #define TIME_MULT(t) (t * 6 )
  #endif

  PROGMEM_MAPPED unsigned int rxSyncMin  = TIME_MULT(1750);
  PROGMEM_MAPPED unsigned int rxSyncMax  = TIME_MULT(2250);
  PROGMEM_MAPPED unsigned int rxZeroMin  = TIME_MULT(100);
  PROGMEM_MAPPED unsigned int rxZeroMax  = TIME_MULT(390);
  PROGMEM_MAPPED unsigned int rxOneMin   = TIME_MULT(410);
  PROGMEM_MAPPED unsigned int rxOneMax   = TIME_MULT(700);
  PROGMEM_MAPPED unsigned int rxLowMax   = TIME_MULT(600);
  PROGMEM_MAPPED int commandForgetTime = 5000;

#endif
  void init_BL_PWM();
  void set_BL_PWM(uint16_t red, uint16_t green, uint16_t blue);
  bool init_UI_Pins();
  bool init_LCD();
  bool init_POST();
  void init_Console();
  int16_t enumerate_leds();
  void init_FB();

#endif
