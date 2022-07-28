#include <Arduino.h>

#define SYS_STATUS GPR.GPR3 
/* used for system status flags
 *  bit 0 = receiving
 *  bit 1 = new message
 *  
 */



// UI + encoder involved globals
#define RX_PIN_STATE (PINB&1) //RX on pin 8 for input capture. 
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
#define LEDPIN      PIN_PA6
#define RF_PIN      PIN_PF3
#define INDICATE2   PIN_PA7 // Non-red LED that isn't power.
#define INDICATE1   PIN_PF1 // Red led that isn't power
#define INDICATE0   PIN_PF0 // Green led next to power
#define LED_SER     Serial2 // connected to the LED strip head end adapter. 
#define LED_SER_TX  PIN_PF4 // probably used in ODME mode? 
#define LED_SER_RX  PIN_PF5 

//LCD Pins
#define LCD_DATA4   PIN_PD0
#define LCD_DATA5   PIN_PD1
#define LCD_DATA6   PIN_PD2
#define LCD_DATA7   PIN_PD3
#define LCD_RS      PIN_PD4
#define LCD_RW      PIN_PD5
#define LCD_EN      PIN_PD7
#define LCD_BL_R    PIN_PB0
#define LCD_BL_G    PIN_PB1
#define LCD_BL_B    PIN_PB2
#define LCD_BL      LCD_BL_R

/*****************************    
 *     RF related globals    *
 *  Macros, buffers, etc     *
 *                           * 
 ****************************/
extern byte MyAddress;
#define RX_BITPOS 0
#define NEW_MSG_BITPOS 1
#define RFRX_NOW      (SYS_STATUS &    (1 << RX_BITPOS))
#define START_RX       SYS_STATUS |=   (1 << RX_BITPOS)
#define DONE_RX        SYS_STATUS &= (~(1 << RX_BITPOS))
#define SET_MESSAGE    SYS_STATUS |=   (1 << NEW_MSG_BITPOS)
#define GOT_MESSAGE   (SYS_STATUS &    (1 << NEW_MSG_BITPOS))
#define CLR_MESSAGE    SYS_STATUS &= (~(1 << NEW_MSG_BITPOS))

extern volatile byte bitnum;

extern volatile byte gotMessage;

extern volatile byte dataIn;
extern volatile byte pktLength;
extern volatile unsigned long lastRFMsgAt;
extern volatile byte rxBuffer[32];
extern byte recvMessage[32];

extern unsigned long lastPacketTime;
extern unsigned long lastPacketSig;

void printSpecs();
void printSpecs(UartClass t);



// Version 2.2/2.3
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

const unsigned int rxSyncMin  = TIME_MULT(1750);
const unsigned int rxSyncMax  = TIME_MULT(2250);
const unsigned int rxZeroMin  = TIME_MULT(100);
const unsigned int rxZeroMax  = TIME_MULT(390);
const unsigned int rxOneMin   = TIME_MULT(410);
const unsigned int rxOneMax   = TIME_MULT(700);
const unsigned int rxLowMax   = TIME_MULT(600);
const int commandForgetTime = 5000;
