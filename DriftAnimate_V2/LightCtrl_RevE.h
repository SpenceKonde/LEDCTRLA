// UI + encoder involved globals
#define RX_PIN_STATE (PINB&1) //RX on pin 8 for input capture. 
#define ENC1_PINA   PIN_PC0
#define ENC1_PINB   PIN_PC1
#define ENC1_BTN    PIN_PB3
#define ENC2_PINA   PIN_PC2
#define ENC2_PINB   PIN_PC3
#define ENC2_BTN    PIN_PB4
#define MODE_BTN    PIN_PB5

// Pin Related Macros
// Used to provide prefab port reads. 
#define BTN_PORT_READ() ((VPORTB.IN >> 3) & 0x07)
/* 0b00000M21 <---- Encoder 1
 *         \ \----- Encoder 2
 *          \------ MODE  
 */
#define ENC1_BTN_bm  (1<<0)
#define ENC2_BTN_bm  (1<<1)
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

/*****************************    
 *     RF related globals    *
 *  Macros, buffers, etc     *
 *                           * 
 ****************************/
const byte MyAddress = 0;
#define SYS_STATUS GPR.GPR3
#define RX_BITPOS 0
#define NEW_MSG_BITPOS 1
#define RFRX_NOW      (SYS_STATUS &    (1 << RX_BITPOS))
#define START_RX       SYS_STATUS |=   (1 << RX_BITPOS)
#define DONE_RX        SYS_STATUS &= (~(1 << RX_BITPOS))
#define SET_MESSAGE    SYS_STATUS |=   (1 << NEW_MSG_BITPOS)
#define GOT_MESSAGE   (SYS_STATUS &    (1 << NEW_MSG_BITPOS))
#define CLR_MESSAGE    SYS_STATUS &= (~(1 << NEW_MSG_BITPOS))

volatile byte bitnum = 0; //current bit received

volatile byte gotMessage = 0;
#define check_NewMessage (GPR.GRP2 & 0x01

volatile byte dataIn = 0;
volatile byte pktLength = 31;
volatile unsigned long lastRFMsgAt = 0;
volatile byte rxBuffer[32];
byte recvMessage[32];

unsigned long lastPacketTime = 0;
unsigned long lastPacketSig = 0;

// Version 2.2/2.3
#if(F_CPU==8000000)
  #define TIME_MULT * 1
#elif(F_CPU==16000000)
  #define TIME_MULT * 2
#elif(F_CPU==20000000)
  #define TIME_MULT *  5) >> (1
#elif(F_CPU==24000000)
  #define TIME_MULT * 3
#elif(F_CPU==30000000)
  #define TIME_MULT * 15) >> (2
#elif(F_CPU==32000000)
  #define TIME_MULT * 4
#elif(F_CPU==36000000)
  #define TIME_MULT * 9) >> (1
#elif(F_CPU==40000000)
  #define TIME_MULT * 5
#elif(F_CPU==48000000)
  #define TIME_MULT * 6
#endif

const unsigned int rxSyncMin  = (1750 TIME_MULT);
const unsigned int rxSyncMax  = (2250 TIME_MULT);
const unsigned int rxZeroMin  = (100  TIME_MULT);
const unsigned int rxZeroMax  = (390  TIME_MULT);
const unsigned int rxOneMin   = (410  TIME_MULT);
const unsigned int rxOneMax   = (700  TIME_MULT);
const unsigned int rxLowMax   = (600  TIME_MULT);
const int commandForgetTime = 5000;
