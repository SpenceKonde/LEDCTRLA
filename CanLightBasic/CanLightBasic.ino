
#include <util/crc16.h>
#include <tinyNeoPixel_Static.h>


#define NEOPIXELPIN            0
#define RGBWW_WWA_RGBW
#if defined(RGBWW_WWA_RGBW)
#define OUTERLEDS 18
#define OUTERCHAN 4
#define MIDLEDS 12
#define MIDCHAN 3
#define INNERLEDS 7
#define INNERCHAN 4
#elif defined(RGBW_RGBW_RGBWW)
#define OUTERLEDS 18
#define OUTERCHAN 4
#define MIDLEDS 12
#define MIDCHAN 4
#define INNERLEDS 7
#define INNERCHAN 4
#endif




#define BUFFUSED ((OUTERLEDS*OUTERCHAN)+(INNERLEDS*INNERCHAN)+(MIDLEDS*MIDCHAN)) //Bytes of buffer used for ONE can light
#define NUMPIXELS  (((BUFFUSED*2)/3)+(((BUFFUSED*2)%3)?1:0)+1) //This ensures that the pixel buffer will fit all the LEDs


//These are the bytes in the buffer that correspond to the three rings of the two can lights.
#define OUTERSTARTA 0
#define OUTERENDA ((OUTERLEDS*OUTERCHAN)-1)
#define MIDSTARTA OUTERENDA+1
#define MIDENDA (MIDSTARTA+(MIDLEDS*MIDCHAN)-1)
#define INNERSTARTA MIDENDA+1
#define INNERENDA BUFFUSED-1
#define OUTERSTARTB BUFFUSED
#define OUTERENDB BUFFUSED+((OUTERLEDS*OUTERCHAN)-1)
#define MIDSTARTB OUTERENDB+1
#define MIDENDB (MIDSTARTB+(MIDLEDS*MIDCHAN)-1)
#define INNERSTARTB MIDENDB+1
#define INNERENDB (2*BUFFUSED)-1

// AzzyRF globals

#define RX_PIN_STATE (VPORTA.IN&2) //RX on pin A1 for input capture.  pin 8
#define RX_ASYNC0 0x0B

const byte MyAddress = 0;
volatile byte receiving = 0;
volatile byte bitnum = 0; //current bit received

volatile byte gotMessage = 0;
volatile byte dataIn = 0;
volatile byte pktLength = 31;
volatile unsigned long lastRFMsgAt = 0;
volatile byte rxBuffer[32];
byte recvMessage[32];

unsigned long lastPacketTime = 0;
unsigned long lastPacketSig = 0;

// Version 2.2/2.3
#if defined(TCB1) || defined(TCB0) //means it's a megaavr
#if(F_CPU==8000000)
#define TIME_MULT * 4
#elif(F_CPU==10000000)
#define TIME_MULT * 5
#elif(F_CPU==16000000)
#define TIME_MULT * 8
#elif(F_CPU==20000000)
#define TIME_MULT * 10
#else 
#error "Unsupported clock speed"
#endif
#else
#if(F_CPU==8000000)
#define TIME_MULT * 1
#elif(F_CPU==16000000)
#define TIME_MULT * 2
#elif(F_CPU==12000000)
#define TIME_MULT * 3/2
#else 
#error "Unsupported clock speed"
#endif
#endif

const unsigned int rxSyncMin  = 1750 TIME_MULT;
const unsigned int rxSyncMax  = 2250 TIME_MULT;
const unsigned int rxZeroMin  = 100 TIME_MULT;
const unsigned int rxZeroMax  = 390 TIME_MULT;
const unsigned int rxOneMin  = 410 TIME_MULT;
const unsigned int rxOneMax  = 700 TIME_MULT;
const unsigned int rxLowMax  = 600 TIME_MULT;
const int commandForgetTime = 5000;

//End AzzyRF globals


// Since this is for the static version of the library, we need to supply the pixel array
// This saves space by eliminating use of malloc() and free(), and makes the RAM used for
// the frame buffer show up when the sketch is compiled.

byte pixels[NUMPIXELS * 3];

tinyNeoPixel leds = tinyNeoPixel(NUMPIXELS, 0, NEO_GRB, pixels);

//IMPORTANT NOTE: Since we have a mixture of 3 and 4 color LEDs, we cannot use any of the abstractions for writing to LEDs - we must directly write to the buffer!

//WWA leds have Amber in place of Red, Cool White in place of Green, Warm White in place of Blue

void setup() {
  pinMode(0, OUTPUT);
  Serial.begin(9600);
  delay(500);
}

void loop() {
  static byte pattern=0;
  selfTest();
}

//##################
// Pattern handling
//##################



//####################
//LED control functions
//####################




void setOuterA(byte r, byte g, byte b, byte w = 0) {
  int i = OUTERSTARTA;
  while (i < OUTERENDA) {
    pixels[i++] = g;
    pixels[i++] = r;
    pixels[i++] = b;
    if (OUTERCHAN == 4) {
      pixels[i++] = w;
    }
  }
}
void setInnerA(byte r, byte g, byte b, byte w = 0) {
  int i = INNERSTARTA;
  while (i < INNERENDA) {
    pixels[i++] = g;
    pixels[i++] = r;
    pixels[i++] = b;
    if (INNERCHAN == 4) {
      pixels[i++] = w;
    }
  }
}
void setMidA(byte r, byte g, byte b, byte w = 0) {
  int i = MIDSTARTA;
  while (i < MIDENDA) {
    pixels[i++] = g;
    pixels[i++] = r;
    pixels[i++] = b;
    if (MIDCHAN == 4) {
      pixels[i++] = w;
    }
  }
}

void setOuterB(byte r, byte g, byte b, byte w = 0) {
  int i = OUTERSTARTB;
  while (i < OUTERENDB) {
    pixels[i++] = g;
    pixels[i++] = r;
    pixels[i++] = b;
    if (OUTERCHAN == 4) {
      pixels[i++] = w;
    }
  }
}
void setInnerB(byte r, byte g, byte b, byte w = 0) {
  int i = INNERSTARTB;
  while (i < INNERENDB) {
    pixels[i++] = g;
    pixels[i++] = r;
    pixels[i++] = b;
    if (INNERCHAN == 4) {
      pixels[i++] = w;
    }
  }
}
void setMidB(byte r, byte g, byte b, byte w = 0) {
  int i = MIDSTARTB;
  while (i < MIDENDB) {
    pixels[i++] = g;
    pixels[i++] = r;
    pixels[i++] = b;
    if (MIDCHAN == 4) {
      pixels[i++] = w;
    }
  }
}

void setOuterAll(byte r, byte g, byte b, byte w = 0) {
  setOuterA(r,g,b,w);
  setOuterB(r,g,b,w);
}
void setMidAll(byte r, byte g, byte b, byte w = 0) {
  setMidA(r,g,b,w);
  setMidB(r,g,b,w);
}
void setInnerAll(byte r, byte g, byte b, byte w = 0) {
  setInnerA(r,g,b,w);
  setInnerB(r,g,b,w);
}

//##########
//Self Test
//##########

void selfTest() { //long selftest showing all the max-brightness possibilities for color
  
  setOuterAll(255,0,0,0);
  setMidAll(0,0,0,0);
  setInnerAll(0,0,0,0);
  leds.show();
  delay(1000);
  setOuterAll(0,255,0,0);
  setMidAll(0,0,0,0);
  setInnerAll(0,0,0,0);
  leds.show();
  delay(1000);
  setOuterAll(0,0,255,0);
  setMidAll(0,0,0,0);
  setInnerAll(0,0,0,0);
  leds.show();
  delay(1000);
  setOuterAll(0,0,0,255);
  setMidAll(0,0,0,0);
  setInnerAll(0,0,0,0);
  leds.show();
  delay(1000);
  setOuterAll(0,0,0,0);
  setMidAll(255,0,0,0);
  setInnerAll(0,0,0,0);
  leds.show();
  delay(1000);
  setOuterAll(0,0,0,0);
  setMidAll(0,255,0,0);
  setInnerAll(0,0,0,0);
  leds.show();
  delay(1000);
  setOuterAll(0,0,0,0);
  setMidAll(0,0,255,0);
  setInnerAll(0,0,0,0);
  leds.show();
  delay(1000);
  #if defined(RGBW_RGBW_RGBWW)
  setOuterAll(0,0,0,0);
  setMidAll(0,0,0,255);
  setInnerAll(0,0,0,0);
  leds.show();
  delay(1000);
  #endif
  setOuterAll(0,0,0,0);
  setMidAll(0,0,0,0);
  setInnerAll(255,0,0,0);
  leds.show();
  delay(1000);
  setOuterAll(0,0,0,0);
  setMidAll(0,0,0,0);
  setInnerAll(0,255,0,0);
  leds.show();
  delay(1000);
  setOuterAll(0,0,0,0);
  setMidAll(0,0,0,0);
  setInnerAll(0,0,255,0);
  leds.show();
  delay(1000);
  setOuterAll(0,0,0,0);
  setMidAll(0,0,0,0);
  setInnerAll(0,0,0,255);
  leds.show();
  delay(1000);
  #if defined(RGBW_RGBW_RGBWW)
  setOuterAll(255,0,0,0);
  setMidAll(255,0,0,0);
  setInnerAll(255,0,0,0);
  leds.show();
  delay(1000);
  setOuterAll(0,255,0,0);
  setMidAll(0,255,0,0);
  setInnerAll(0,255,0,0);
  leds.show();
  delay(1000);
  setOuterAll(0,0,255,0);
  setMidAll(0,0,255,0);
  setInnerAll(0,0,255,0);
  leds.show();
  delay(1000);
  setOuterAll(0,0,0,255);
  setMidAll(0,0,0,255);
  setInnerAll(0,0,0,0);
  leds.show();
  delay(1000);
  setOuterAll(0,0,0,255);
  setMidAll(0,0,0,255);
  setInnerAll(0,0,0,255);
  leds.show();
  delay(1000);
  setOuterAll(255,255,0,0);
  setMidAll(255,255,0,0);
  setInnerAll(255,255,0,0);
  leds.show();
  delay(1000);
  setOuterAll(0,255,255,0);
  setMidAll(0,255,255,0);
  setInnerAll(0,255,255,0);
  leds.show();
  delay(1000);
  setOuterAll(255,0,255,0);
  setMidAll(255,0,255,0);
  setInnerAll(255,0,255,0);
  leds.show();
  delay(1000);
  setOuterAll(255,255,255,0);
  setMidAll(255,255,255,0);
  setInnerAll(255,255,255,0);
  leds.show();
  delay(1000);
  setOuterAll(255,0,0,255);
  setMidAll(255,0,0,255);
  setInnerAll(255,0,0,255);
  leds.show();
  delay(1000);
  setOuterAll(0,255,0,255);
  setMidAll(0,255,0,255);
  setInnerAll(0,255,0,255);
  leds.show();
  delay(1000);
  setOuterAll(0,0,255,255);
  setMidAll(0,0,255,255);
  setInnerAll(0,0,255,255);
  leds.show();
  #elif defined(RGBWW_WWA_RGBW)
  setOuterAll(255,0,0,0);
  setMidAll(0,0,0,0);
  setInnerAll(255,0,0,0);
  leds.show();
  delay(1000);
  setOuterAll(0,255,0,0);
  setMidAll(0,0,0,0);
  setInnerAll(0,255,0,0);
  leds.show();
  delay(1000);
  setOuterAll(0,0,255,0);
  setMidAll(0,0,0,0);
  setInnerAll(0,0,255,0);
  leds.show();
  delay(1000);
  setOuterAll(0,0,0,255);
  setMidAll(0,255,0,0);
  setInnerAll(0,0,0,0);
  leds.show();
  delay(1000);
  setOuterAll(0,0,0,255);
  setMidAll(255,255,0,0);
  setInnerAll(0,0,0,0);
  leds.show();
  delay(1000);
  setOuterAll(0,0,0,0);
  setMidAll(0,0,255,0);
  setInnerAll(0,0,0,255);
  leds.show();
  delay(1000);
  setOuterAll(0,0,0,255);
  setMidAll(255,255,0,0);
  setInnerAll(0,0,0,255);
  leds.show();
  delay(1000);
  setOuterAll(255,255,0,0);
  setMidAll(0,0,0,0);
  setInnerAll(255,255,0,0);
  leds.show();
  delay(1000);
  setOuterAll(0,255,255,0);
  setMidAll(0,0,0,0);
  setInnerAll(0,255,255,0);
  leds.show();
  delay(1000);
  setOuterAll(255,0,255,0);
  setMidAll(0,0,0,0);
  setInnerAll(255,0,255,0);
  leds.show();
  delay(1000);
  setOuterAll(255,255,255,0);
  setMidAll(0,0,0,0);
  setInnerAll(255,255,255,0);
  leds.show();
  delay(1000);
  setOuterAll(255,0,0,255);
  setMidAll(255,255,255,0);
  setInnerAll(255,0,0,255);
  leds.show();
  delay(1000);
  setOuterAll(0,255,0,255);
  setMidAll(255,255,255,0);
  setInnerAll(0,255,0,255);
  leds.show();
  delay(1000);
  setOuterAll(0,0,255,255);
  setMidAll(255,255,255,0);
  setInnerAll(0,0,255,255);
  leds.show();
  #endif
  delay(1000);
  setOuterAll(0,0,0,0);
  setMidAll(0,0,0,0);
  setInnerAll(0,0,0,0);
  leds.show();
  delay(1000);
}

//###########################
//AzzyRF for remote control 
//###########################

byte handleReceive() {
  if (gotMessage) {
    byte vers = checkCSC(); //checkCSC() gives 0 on failed CSC, 1 on v1 structure (ACD...), 2 on v2 structure (DSCD...)
    if (!vers) { //if vers=0, unknown format ot bad CSC
      resetReceive();
      return 0;
    }
    if (rxBuffer[0]==0 &&  rxBuffer[1]==0 && rxBuffer[2]==0 && (rxBuffer[3]&0xF0)==0) {
      resetReceive();
      return 0;
    }
    if (!isForMe()) { //matches on MyAddress==0, destination address==0, destination address==MyAddress.
      resetReceive();
      return 0;
    }
    if (lastPacketSig == getPacketSig() && lastPacketTime) {
      
    lastPacketTime = millis();
      resetReceive();
      return 0;
    }
    lastPacketSig = getPacketSig();
    lastPacketTime = millis();
    byte rlen = ((pktLength >> 3) + 1) | ((vers - 1) << 6);
    memcpy(recvMessage, rxBuffer, 32);
    if (rlen==4){
      recvMessage[3]=recvMessage[3]&0xF0;
    } else {
      recvMessage[rlen-1]=0;
    }
    resetReceive();
    return rlen;
  } else {
    unsigned long t=(millis()-lastPacketTime);
    if (lastPacketTime && (t > commandForgetTime)) {
      lastPacketTime = 0;
      lastPacketSig = 0;
    }
    return 0;
  }
}

void resetReceive() {

  bitnum = 0;
  memset(rxBuffer, 0, 32);
  gotMessage = 0;
  #ifdef TCB1
  TCB1.INTCTRL=0x01;
  #elif defined(TCB0)
  TCB0.INTCTRL=0x01;
  #else
  TIMSK1 = 1 << ICIE1;
  #endif
  return;
}

byte checkCSC() {
  byte rxchecksum = 0;
  byte rxchecksum2 = 0;
  for (byte i = 0; i < pktLength >> 3; i++) {
    rxchecksum = rxchecksum ^ rxBuffer[i];
    rxchecksum2 = _crc8_ccitt_update(rxchecksum2,rxBuffer[i]);
  }
  if (pktLength >> 3 == 3) {
    rxchecksum = (rxchecksum & 0x0F) ^ (rxchecksum >> 4) ^ ((rxBuffer[3] & 0xF0) >> 4);
    rxchecksum2 = (rxchecksum2 & 0x0F) ^ (rxchecksum2 >> 4) ^ ((rxBuffer[3] & 0xF0) >> 4);
    if (rxchecksum == rxchecksum2)rxchecksum2++;
    return (rxBuffer[3] & 0x0F) == rxchecksum ? 1 : ((rxBuffer[3] & 0x0F) == rxchecksum2 ) ? 2 : 0;
  } else {
    if (rxchecksum == rxchecksum2)rxchecksum2++;
    return ((rxBuffer[pktLength >> 3] == rxchecksum) ? 1 : ((rxBuffer[pktLength >> 3] == rxchecksum2 ) ? 2 : 0));
  }
}

byte isForMe() {
  if ((rxBuffer[0] & 0x3F) == MyAddress || MyAddress == 0 || (rxBuffer[0] & 0x3F) == 0) {
    return 1;
  }
  return 0;
}

unsigned long getPacketSig() {
  byte len = pktLength >> 3;
  unsigned long lastpacketsig=0;
  for (byte i = (len == 3 ? 0 : 1); i < (len == 3 ? 3 : 4); i++) {
    lastpacketsig += rxBuffer[i];
    lastpacketsig = lastpacketsig << 8;
  }
  lastpacketsig += rxBuffer[len];
  return lastpacketsig;
}

void setupTimer() {
  #if defined(TCCR1A) && defined(TIMSK1) //In this case, it's a classic AVR with a normal timer1
  TCCR1A = 0;
  TCCR1B = 0;
  TIFR1 = bit (ICF1) | bit (TOV1);  // clear flags so we don't get a bogus interrupt
  TCNT1 = 0;          // Counter to zero
  TIMSK1 = 1 << ICIE1; // interrupt on Timer 1 input capture
  // start Timer 1, prescalar of 8, edge select on falling edge
  TCCR1B =  ((F_CPU == 1000000L) ? (1 << CS10) : (1 << CS11)) | 1 << ICNC1; //prescalar 8 except at 1mhz, where we use prescalar of 1, noise cancler active
  //ready to rock and roll
  #elif defined(TCB1) // it's a megaavr
  TCB1.CTRLA=0x02; //disable, CKPER/2 clock source.
  TCB1.CTRLB=0x03; //Input Capture Frequency Measurement mode
  TCB1.INTFLAGS=1; //clear flag
  TCB1.CNT=0; //count to 0
  TCB1.INTCTRL=0x01;
  EVSYS.ASYNCCH0=RX_ASYNC0; //PA1 Set event channel for PA1 pin
  EVSYS.ASYNCUSER11=0x03;
  TCB1.EVCTRL=0x51; //filter, falling edge, ICIE=1
  TCB1.CTRLA=0x03; //enable
  #elif defined(TCB0) // it's a megaavr
  TCB0.CTRLA=0x02; //disable, CKPER/2 clock source.
  TCB0.CTRLB=0x03; //Input Capture Frequency Measurement mode
  TCB0.INTFLAGS=1; //clear flag
  TCB0.CNT=0; //count to 0
  TCB0.INTCTRL=0x01;
  EVSYS.ASYNCCH0=RX_ASYNC0; //PA1 Set event channel for PA1 pin
  EVSYS.ASYNCUSER0=0x03;
  TCB0.EVCTRL=0x51; //filter, falling edge, ICIE=1
  TCB0.CTRLA=0x03; //enable
  #else
  #error "architecture not supported"
  #endif
}

#ifdef TCB1
ISR(TCB1_INT_vect)
#elif defined(TCB0)
ISR(TCB0_INT_vect)
#else
ISR (TIMER1_CAPT_vect)
#endif
{
  #if defined(TCB1)
  static unsigned long lasttime = 0;
  unsigned int newTime = TCB1.CCMP; //immediately get the ICR value
  #elif defined(TCB0)
  static unsigned long lasttime = 0;
  unsigned int newTime = TCB0.CCMP; //immediately get the ICR value
  #else
  unsigned int newTime = ICR1; //immediately get the ICR value
  #endif
  byte state = (RX_PIN_STATE);
  #ifdef TCB1
  TCB1.EVCTRL=state?0x51:0x41; //trigger on falling edge if pin is high, otherwise rising edge
  unsigned int duration = newTime;
  #elif defined(TCB0)
  TCB0.EVCTRL=state?0x51:0x41; //trigger on falling edge if pin is high, otherwise rising edge
  unsigned int duration = newTime;
  #else
  TCCR1B = state ? (1 << CS11 | 1 << ICNC1) : (1 << CS11 | 1 << ICNC1 | 1 << ICES1); //and set edge
  unsigned int duration = newTime - lasttime;
  #endif
  lasttime = newTime;
  if (state) {
    if (receiving) {
      if (duration > rxLowMax) {
        
        receiving = 0;
        bitnum = 0; // reset to bit zero
        memset(rxBuffer, 0, 32); //clear buffer
      }
    } else {
      if (duration > rxSyncMin && duration < rxSyncMax) {
        receiving = 1;
      }
    }
  } else {
    if (receiving) {
      if (duration > rxZeroMin && duration < rxZeroMax) {
        dataIn = dataIn << 1;
      } else if (duration > rxOneMin && duration < rxOneMax) {
        dataIn = (dataIn << 1) + 1;
      } else {
        receiving = 0;
        bitnum = 0; // reset to bit zero
        memset(rxBuffer, 0, 32); //clear buffer
        return;
      }
      if ((bitnum & 7) == 7) {
        rxBuffer[bitnum >> 3] = dataIn;
        if (bitnum == 7) {
          byte t = dataIn >> 6;
          pktLength = t ? (t == 1 ? 63 : (t == 2 ? 127 : 255)) : 31;
        }
        dataIn = 0;
      }
      if (bitnum >= pktLength) {
        bitnum = 0;
        receiving = 0;
        gotMessage = 1;
        #ifdef TCB1
        TCB1.INTCTRL=0x00;
        #elif defined(TCB0)
        TCB0.INTCTRL=0x00;
        #else
        TIMSK1 = 0; //turn off input capture;
        #endif
      } else {
        bitnum++;
      }
    }
  }
}
