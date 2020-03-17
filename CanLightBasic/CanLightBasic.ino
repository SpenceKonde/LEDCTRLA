
#include <util/crc16.h>
#include <tinyNeoPixel_Static.h>

//Runs with ATtiny1614 or 1604; could also fit in 814 or 804.


#define NEOPIXELPIN            0 //happens to make the wiring ot the LEDs easier, by keeping all the relevant pins close together, and is as good a pin as any.
//AzzyRF receive pin is pin 8 (PA1)
//#define DESK_LEFT
//#define DESK_RIGHT
#define CORNER
//#define PLANT

#define CANVERSION "v1.01"


//####################################
// Properties based on light location
//####################################

// Sanity Check - long line that just makes sure two locations were not both defined
#if ((defined(DESK_LEFT)&&(defined(DESK_RIGHT)||defined(PLANT)||defined(CORNER))) ||(defined(DESK_RIGHT)&&(defined(DESK_LEFT)||defined(PLANT)||defined(CORNER))) ||(defined(PLANT)&&(defined( DESK_RIGHT)||defined(DESK_LEFT)||defined(CORNER))) ||(defined(CORNER)&&(defined(DESK_RIGHT)||defined(PLANT)||defined(DESK_LEFT))))
#error "Mutltiple options for which set of can lights specified, specify only one"
#endif



#if (defined(DESK_LEFT)||defined(DESK_RIGHT)||defined(CORNER))
#define RGBWW_WWA_RGBW
#elif defined(PLANT)
#define RGBWW_RGBW_RGBW
#else
#error "unknown can light location"
#endif

// Number of LEDs in each ring
#define OUTERLEDS 18
#define MIDLEDS 12
#define INNERLEDS 7

// LEDs per channel
#if defined(RGBWW_WWA_RGBW)
#define OUTERCHAN 4
#define MIDCHAN 3
#define INNERCHAN 4
#define REDCOUNT (OUTERLEDS+INNERLEDS)
#define GREENCOUNT (OUTERLEDS+INNERLEDS)
#define BLUECOUNT (OUTERLEDS+INNERLEDS)
#define WARMCOUNT (OUTERLEDS+MIDLEDS)
#define COOLCOUNT (MIDLEDS+INNERLEDS)
#define AMBERCOUNT (MIDLEDS)
#elif defined(RGBWW_RGBW_RGBW)
#define OUTERCHAN 4
#define MIDCHAN 4
#define INNERCHAN 4
#define REDCOUNT (OUTERLEDS+MIDLEDS+INNERLEDS)
#define GREENCOUNT (OUTERLEDS+MIDLEDS+INNERLEDS)
#define BLUECOUNT (OUTERLEDS+MIDLEDS+INNERLEDS)
#define WARMCOUNT (OUTERLEDS)
#define COOLCOUNT (MIDLEDS+INNERLEDS)
#define AMBERCOUNT (0)
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

#ifdef DESK_LEFT
const byte MyAddress[] = {0x28, 0x29, 0x2A};
#define DEVICE_NAME "Desk Left"
#endif
#ifdef DESK_RIGHT
const byte MyAddress[] = {0x28, 0x29, 0x2B};
#define DEVICE_NAME "Desk Right"
#endif
#ifdef CORNER
const byte MyAddress[] = {0x28, 0x2C};
#define DEVICE_NAME "Corner"
#endif
#ifdef PLANT
const byte MyAddress[] = {0x2D};
#define DEVICE_NAME "Plant"
#endif




//####################
// AzzyRF globals
//####################

#define RX_PIN_STATE (VPORTA.IN&2) //RX on pin A1 for input capture.  pin 8 - Unfortunatley can't think of a good *compiletime* way to look this up, which we need in order to to bring in this special little snippet; This will, however, be possible when I finally librarify AzzyRF, by way of a set of #if...#elif statements for all the possible I/O pins for a mega-series tiny (and a bunch more if we want it to work on a 4809, which I might as well do. 
#define RX_ASYNC0 0x0B //Similarly when librarified, this will be calculated at compiletime with a bunch of #defines!

//MyAddress set above

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


//####################
// tinyNeoPixel setup
//####################



// Since this is for the static version of the library, we need to supply the pixel array
// This saves space by eliminating use of malloc() and free(), and makes the RAM used for
// the frame buffer show up when the sketch is compiled.

byte pixels[NUMPIXELS * 3];

tinyNeoPixel leds = tinyNeoPixel(NUMPIXELS, 0, NEO_GRB, pixels);

//IMPORTANT NOTE: Since we have a mixture of 3 and 4 color LEDs, we cannot use any of the abstractions for writing to LEDs - we must directly write to the buffer!

//WWA leds have Amber in place of Red, Cool White in place of Green, Warm White in place of Blue

//####################
// Begin Can Lights
//####################

//Function prototypes for these appear to be needed because Arduino Builder wasn't correctly generating them
void setInnerAll(byte, byte, byte, byte);
void setOuterAll(byte, byte, byte, byte);
void setMidAll(byte, byte, byte, byte);
void setAllColor(byte, byte, byte, byte, byte, byte);


void setup() {
  pinMode(0, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.print(DEVICE_NAME);
  Serial.print(" Can Light ");
  Serial.println(CANVERSION);
  Serial.print("Listening on:");
  for (byte i = 0; i < sizeof(MyAddress); i++) {
    Serial.print(" 0x");
    showHex(MyAddress[i]);
  }
  Serial.println();
  setupTimer();
  selfTest();
  setAllColor(0, 0, 0, 255, 255, 255); //start up om white mode for installation purposes
  leds.show();

}

void loop() {
  byte rlen = handleReceive();
  if (rlen) {
    byte rxstatus = processRFPacket(rlen);
    if (!rxstatus) {
      leds.show();
      Serial.print("RX: ");
      for(byte i=0;i<rlen;i++){
        if(i==3&&rlen==4){
          showHex(recvMessage[3] >>4);
        } else {
          showHex(recvMessage[i]);
        }
      }
      Serial.println();
    } else {
      handleBadRX(rlen, rxstatus);
    }
  }
}

void handleBadRX(byte rlen, byte st) {
  switch (st) {
    case 255:
      Serial.print("Unknown command: ");
      Serial.println(recvMessage[1], HEX);
      break;
    case 254:
      Serial.println("Not yet implemented");
    case 3:
      Serial.print("Unknown Target: ");
      Serial.println(recvMessage[2]);
    case 2:
      Serial.print("Unknown Preset: ");
      Serial.println(recvMessage[3] >> 4);
    case 1:
      Serial.println("Invalid Length");
      break;
  }
  Serial.print("Addressed to: ");
  Serial.println(recvMessage[0] & 0x3F, HEX);
  Serial.print("Length: ");
  Serial.println(rlen & 0x3F, HEX);
  Serial.print("Version: ");
  Serial.println(rlen >> 6);
}

byte processRFPacket(byte rlen) { //returns 1 on fail 0 on success

  byte vers = (rlen & 196) >> 6;
  rlen &= 0x3F;
  byte rxstatus = 255;
  switch (recvMessage[1]) {
    case 0x58: //set multicolor
      if (rlen == 4) {
        rxstatus = setPreset(recvMessage[2], recvMessage[3] >> 4);
      } else if (rlen == 8) { //short multicolor set - last channel used for both amber and warm white
        setAllColor(recvMessage[2], recvMessage[3], recvMessage[4], recvMessage[5], recvMessage[6], recvMessage[6]);
        rxstatus = 0;
      } else if (rlen == 16) {
        setAllColor(recvMessage[2], recvMessage[3], recvMessage[4], recvMessage[5], recvMessage[6], recvMessage[7]);
        rxstatus = 0;
      } else {
        rxstatus = 1;
      }
      break;
    case 0x59:
      if (rlen == 16) {
        setOuterAll(recvMessage[2], recvMessage[3], recvMessage[4], recvMessage[5]);
        setMidAll(recvMessage[6], recvMessage[7], recvMessage[8], recvMessage[9]);
        setInnerAll(recvMessage[10], recvMessage[11], recvMessage[12], recvMessage[13]);
        rxstatus = 0;
      } else {
        rxstatus = 1;
      }
      break;
  }
  return rxstatus;
  //}

}


//##################
// Pattern handling
//##################

byte setPreset(byte pattern, byte target) { //Not yet implemented - stub.
  return 254; //NYI
}

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
  setOuterA(r, g, b, w);
  setOuterB(r, g, b, w);
}
void setMidAll(byte r, byte g, byte b, byte w = 0) {
  setMidA(r, g, b, w);
  setMidB(r, g, b, w);
}
void setInnerAll(byte r, byte g, byte b, byte w = 0) {
  setInnerA(r, g, b, w);
  setInnerB(r, g, b, w);
}

void setAllColor(byte r, byte g, byte b, byte w, byte ww, byte a) {
#if defined(RGBWW_WWA_RGBW)
  setOuterAll(r, g, b, ww);
  setMidAll(a, w, ww);
  setInnerAll(r, g, b, w);
#elif defined(RGBWW_RGBW_RGBW)
  setOuterAll(r, g, b, ww);
  setMidAll(r, g, b, w);
  setInnerAll(r, g, b, w);
#else
#error "Color combination not defined"
#endif
}

//##########
//Self Test
//##########

void selfTest() { //long selftest showing all the max-brightness possibilities for color

  setOuterAll(255, 0, 0, 0);
  setMidAll(0, 0, 0, 0);
  setInnerAll(0, 0, 0, 0);
  leds.show();
  delay(1000);
  setOuterAll(0, 255, 0, 0);
  setMidAll(0, 0, 0, 0);
  setInnerAll(0, 0, 0, 0);
  leds.show();
  delay(1000);
  setOuterAll(0, 0, 255, 0);
  setMidAll(0, 0, 0, 0);
  setInnerAll(0, 0, 0, 0);
  leds.show();
  delay(1000);
  setOuterAll(0, 0, 0, 255);
  setMidAll(0, 0, 0, 0);
  setInnerAll(0, 0, 0, 0);
  leds.show();
  delay(1000);
  setOuterAll(0, 0, 0, 0);
  setMidAll(255, 0, 0, 0);
  setInnerAll(0, 0, 0, 0);
  leds.show();
  delay(1000);
  setOuterAll(0, 0, 0, 0);
  setMidAll(0, 255, 0, 0);
  setInnerAll(0, 0, 0, 0);
  leds.show();
  delay(1000);
  setOuterAll(0, 0, 0, 0);
  setMidAll(0, 0, 255, 0);
  setInnerAll(0, 0, 0, 0);
  leds.show();
  delay(1000);
#if defined(RGBW_RGBW_RGBWW)
  setOuterAll(0, 0, 0, 0);
  setMidAll(0, 0, 0, 255);
  setInnerAll(0, 0, 0, 0);
  leds.show();
  delay(1000);
#endif
  setOuterAll(0, 0, 0, 0);
  setMidAll(0, 0, 0, 0);
  setInnerAll(255, 0, 0, 0);
  leds.show();
  delay(1000);
  setOuterAll(0, 0, 0, 0);
  setMidAll(0, 0, 0, 0);
  setInnerAll(0, 255, 0, 0);
  leds.show();
  delay(1000);
  setOuterAll(0, 0, 0, 0);
  setMidAll(0, 0, 0, 0);
  setInnerAll(0, 0, 255, 0);
  leds.show();
  delay(1000);
  setOuterAll(0, 0, 0, 0);
  setMidAll(0, 0, 0, 0);
  setInnerAll(0, 0, 0, 255);
  leds.show();
  delay(1000);
#if defined(RGBW_RGBW_RGBWW)
  setOuterAll(255, 0, 0, 0);
  setMidAll(255, 0, 0, 0);
  setInnerAll(255, 0, 0, 0);
  leds.show();
  delay(1000);
  setOuterAll(0, 255, 0, 0);
  setMidAll(0, 255, 0, 0);
  setInnerAll(0, 255, 0, 0);
  leds.show();
  delay(1000);
  setOuterAll(0, 0, 255, 0);
  setMidAll(0, 0, 255, 0);
  setInnerAll(0, 0, 255, 0);
  leds.show();
  delay(1000);
  setOuterAll(0, 0, 0, 255);
  setMidAll(0, 0, 0, 255);
  setInnerAll(0, 0, 0, 0);
  leds.show();
  delay(1000);
  setOuterAll(0, 0, 0, 255);
  setMidAll(0, 0, 0, 255);
  setInnerAll(0, 0, 0, 255);
  leds.show();
  delay(1000);
  setOuterAll(255, 255, 0, 0);
  setMidAll(255, 255, 0, 0);
  setInnerAll(255, 255, 0, 0);
  leds.show();
  delay(1000);
  setOuterAll(0, 255, 255, 0);
  setMidAll(0, 255, 255, 0);
  setInnerAll(0, 255, 255, 0);
  leds.show();
  delay(1000);
  setOuterAll(255, 0, 255, 0);
  setMidAll(255, 0, 255, 0);
  setInnerAll(255, 0, 255, 0);
  leds.show();
  delay(1000);
  setOuterAll(255, 255, 255, 0);
  setMidAll(255, 255, 255, 0);
  setInnerAll(255, 255, 255, 0);
  leds.show();
  delay(1000);
  setOuterAll(255, 0, 0, 255);
  setMidAll(255, 0, 0, 255);
  setInnerAll(255, 0, 0, 255);
  leds.show();
  delay(1000);
  setOuterAll(0, 255, 0, 255);
  setMidAll(0, 255, 0, 255);
  setInnerAll(0, 255, 0, 255);
  leds.show();
  delay(1000);
  setOuterAll(0, 0, 255, 255);
  setMidAll(0, 0, 255, 255);
  setInnerAll(0, 0, 255, 255);
  leds.show();
#elif defined(RGBWW_WWA_RGBW)
  setOuterAll(255, 0, 0, 0);
  setMidAll(0, 0, 0, 0);
  setInnerAll(255, 0, 0, 0);
  leds.show();
  delay(1000);
  setOuterAll(0, 255, 0, 0);
  setMidAll(0, 0, 0, 0);
  setInnerAll(0, 255, 0, 0);
  leds.show();
  delay(1000);
  setOuterAll(0, 0, 255, 0);
  setMidAll(0, 0, 0, 0);
  setInnerAll(0, 0, 255, 0);
  leds.show();
  delay(1000);
  setOuterAll(0, 0, 0, 255);
  setMidAll(0, 255, 0, 0);
  setInnerAll(0, 0, 0, 0);
  leds.show();
  delay(1000);
  setOuterAll(0, 0, 0, 255);
  setMidAll(255, 255, 0, 0);
  setInnerAll(0, 0, 0, 0);
  leds.show();
  delay(1000);
  setOuterAll(0, 0, 0, 0);
  setMidAll(0, 0, 255, 0);
  setInnerAll(0, 0, 0, 255);
  leds.show();
  delay(1000);
  setOuterAll(0, 0, 0, 255);
  setMidAll(255, 255, 0, 0);
  setInnerAll(0, 0, 0, 255);
  leds.show();
  delay(1000);
  setOuterAll(255, 255, 0, 0);
  setMidAll(0, 0, 0, 0);
  setInnerAll(255, 255, 0, 0);
  leds.show();
  delay(1000);
  setOuterAll(0, 255, 255, 0);
  setMidAll(0, 0, 0, 0);
  setInnerAll(0, 255, 255, 0);
  leds.show();
  delay(1000);
  setOuterAll(255, 0, 255, 0);
  setMidAll(0, 0, 0, 0);
  setInnerAll(255, 0, 255, 0);
  leds.show();
  delay(1000);
  setOuterAll(255, 255, 255, 0);
  setMidAll(0, 0, 0, 0);
  setInnerAll(255, 255, 255, 0);
  leds.show();
  delay(1000);
  setOuterAll(255, 0, 0, 255);
  setMidAll(255, 255, 255, 0);
  setInnerAll(255, 0, 0, 255);
  leds.show();
  delay(1000);
  setOuterAll(0, 255, 0, 255);
  setMidAll(255, 255, 255, 0);
  setInnerAll(0, 255, 0, 255);
  leds.show();
  delay(1000);
  setOuterAll(0, 0, 255, 255);
  setMidAll(255, 255, 255, 0);
  setInnerAll(0, 0, 255, 255);
  leds.show();
#endif
  delay(1000);
  setOuterAll(0, 0, 0, 0);
  setMidAll(0, 0, 0, 0);
  setInnerAll(0, 0, 0, 0);
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
    if (rxBuffer[0] == 0 &&  rxBuffer[1] == 0 && rxBuffer[2] == 0 && (rxBuffer[3] & 0xF0) == 0) {
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
    memcpy(recvMessage, (const void*)rxBuffer, 32);
    if (rlen == 4) {
      recvMessage[3] = recvMessage[3] & 0xF0;
    } else {
      recvMessage[rlen - 1] = 0;
    }
    resetReceive();
    return rlen;
  } else {
    unsigned long t = (millis() - lastPacketTime);
    if (lastPacketTime && (t > commandForgetTime)) {
      lastPacketTime = 0;
      lastPacketSig = 0;
    }
    return 0;
  }
}



void resetReceive() {

  bitnum = 0;
  memset((void*)rxBuffer, 0, 32);
  gotMessage = 0;
#ifdef TCB1
  TCB1.INTCTRL = 0x01;
#elif defined(TCB0)
  TCB0.INTCTRL = 0x01;
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
    rxchecksum2 = _crc8_ccitt_update(rxchecksum2, rxBuffer[i]);
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
  for (byte i = 0; i < sizeof(MyAddress); i++) {
    byte addr = MyAddress[i];
    if ((rxBuffer[0] & 0x3F) == addr || !rxBuffer[0] || !addr) {
      return 1;
    }
  }
  return 0;
}

unsigned long getPacketSig() {
  byte len = pktLength >> 3;
  unsigned long lastpacketsig = 0;
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
  TCB1.CTRLA = 0x02; //disable, CKPER/2 clock source.
  TCB1.CTRLB = 0x03; //Input Capture Frequency Measurement mode
  TCB1.INTFLAGS = 1; //clear flag
  TCB1.CNT = 0; //count to 0
  TCB1.INTCTRL = 0x01;
  EVSYS.ASYNCCH0 = RX_ASYNC0; //PA1 Set event channel for PA1 pin
  EVSYS.ASYNCUSER11 = 0x03;
  TCB1.EVCTRL = 0x51; //filter, falling edge, ICIE=1
  TCB1.CTRLA = 0x03; //enable
#elif defined(TCB0) // it's a megaavr
  TCB0.CTRLA = 0x02; //disable, CKPER/2 clock source.
  TCB0.CTRLB = 0x03; //Input Capture Frequency Measurement mode
  TCB0.INTFLAGS = 1; //clear flag
  TCB0.CNT = 0; //count to 0
  TCB0.INTCTRL = 0x01;
  EVSYS.ASYNCCH0 = RX_ASYNC0; //PA1 Set event channel for PA1 pin
  EVSYS.ASYNCUSER0 = 0x03;
  TCB0.EVCTRL = 0x51; //filter, falling edge, ICIE=1
  TCB0.CTRLA = 0x03; //enable
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
  TCB1.EVCTRL = state ? 0x51 : 0x41; //trigger on falling edge if pin is high, otherwise rising edge
  unsigned int duration = newTime;
#elif defined(TCB0)
  TCB0.EVCTRL = state ? 0x51 : 0x41; //trigger on falling edge if pin is high, otherwise rising edge
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
        memset((void*)rxBuffer, 0, 32); //clear buffer
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
        memset((void*)rxBuffer, 0, 32); //clear buffer
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
        TCB1.INTCTRL = 0x00;
#elif defined(TCB0)
        TCB0.INTCTRL = 0x00;
#else
        TIMSK1 = 0; //turn off input capture;
#endif
      } else {
        bitnum++;
      }
    }
  }
}

void showHex (const byte b) {
  // try to avoid using sprintf
  char buf [3] = { ((b >> 4) & 0x0F) | '0', (b & 0x0F) | '0', 0};
  if (buf [0] > '9') buf [0] += 7;
  if (buf [1] > '9') buf [1] += 7;
  Serial.print(buf);
}
