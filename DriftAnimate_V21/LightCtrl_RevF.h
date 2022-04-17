#ifndef __LightCtrl_h
#define __LightCtrl_h
#include "Arduino.h"
#include "Modes.h"
#include "Colors.h"
#include "HWSpecs.h"



#define SYS_STATUS            (GPR.GPR3)
#define RX_bp                 (0)
#define NEW_MSG_bp            (1)


#define COLOR_CHANGE_bp       (4)
#define ADJUST_CHANGE_bp      (5)
#define MODE_CHANGE_bp        (6)
#define UI_CHANGE_bp          (7)
const uint8_t RXING           = (1 << RX_bp)
const uint8_t NEW_MSG         = (1 << NEW_MSG_bp)


const uint8_t COLOR_CHANGE    = (1 << COLOR_CHANGE_bp)
const uint8_t ADJUST_CHANGE   = (1 << ADJUST_CHANGE_bp)
const uint8_t MODE_CHANGE     = (1 << MODE_CHANGE_bp)
const uint8_t UI_CHANGE       = (1 << UI_CHANGE_bp)
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
#define IsSet(bitmask) = (!!(SYS_STATUS & bitmask))
inline void __attribute__((always_inline)) SysSet(bitmask) {
  SYS_STATUS |= bitmask;
}

inline void __attribute__((always_inline)) SysClr(bitmask) {
  SYS_STATUS &= ~bitmask;
}

/* Das Pinmap

   | PIN 7 | PIN 6 | PIN 5 | PIN 4 | PIN 3 | PIN 2 | PIN 1 | PIN 0 |
---|-------|-------|-------|-------|-------|-------|-------|-------|
PA | LED 2 |LED_PIN| U0 RX | U0 TX |__SCL__|__SDA__| XTAL2 | XTAL1 |
PB                 | MODE  | ENC 1 | ENC 2 | BL B  | BL G  | BL R  |
PC **EVOUT***CCLO1***U1 RX***U1 TX*| ENC2B | ENC2A | ENC1B | ENC1A |
   ***SS******SCK****MISO*****MOSI*| ^^^^^ | ^^^^^ | ^^^^^ | ^^^^^ |
PD |  EN   |CONTRAS|  RW   |  RS   |  LCD  |  LCD  |  LCD  |  LCD  |
PE                                 |*EVOUT****EXT****U4*RX***U4*TX**
PE                                 |**SS******SCK****MISO*****MOSI**
PF | RESET | UPDI  |_U2_RX_| U2 TX | RF IN |_ANA_FB| LED 1 | LED 0 |

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
#define ENC1_BTN    PIN_PB4
#define ENC2_PINA   PIN_PC2
#define ENC2_PINB   PIN_PC3
#define ENC2_BTN    PIN_PB3
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

#define ENC_PORT_READ() (VPORTC.IN & 0x0F)
//0b00002211 <---- Encoder 1
//       ^-------- Encoder 2


//Pins to talk to outside world
#define LEDPIN        PIN_PA6
#define RF_PIN        PIN_PF3
#define INDICATE2     PIN_PA7 // Non-red LED that isn't power.
#define INDICATE1     PIN_PF1 // Red led that isn't power
#define INDICATE0     PIN_PF0 // Green led next to power
#define LED_SER       Serial2 // connected to the LED strip head end adapter.
#define LED_SER_TX    PIN_PF4 // probably used in ODME mode?
#define LED_SER_RX    PIN_PF5

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
* Debug related macros       *
*                            *
*****************************/
#define DBG_UPD_START ({digitalWriteFast(INDICATE1,HIGH);})
#define DBG_UPD_END   ({digitalWriteFast(INDICATE1, LOW);})
#define DBG_OUT_START ({digitalWriteFast(INDICATE2,HIGH);})
#define DBG_UPD_END   ({digitalWriteFast(INDICATE2, LOW);})
#define DBG_RF_PROC   ({digitalWriteFast(INDICATE2,HIGH);})
#define DBG_RF_PROC   ({digitalWriteFast(INDICATE2, LOW);})


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
#endif
