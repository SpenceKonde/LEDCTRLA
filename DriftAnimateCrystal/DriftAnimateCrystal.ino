#include <Adafruit_NeoPixel_Static.h>
#include <avr/pgmspace.h>


#define COLORTABLEMAX 31
const byte normalizedBrightnessTable[COLORTABLEMAX + 1] PROGMEM = {0, 1, 2, 3, 4, 6, 8, 11, 14, 18, 22, 27, 33, 39, 46, 54, 63, 73, 84, 95, 106, 117, 128, 139, 151, 163, 176, 189, 204, 220, 237, 255};

const byte pulseBrightnessTable[] PROGMEM = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 31, 34, 37, 40, 43, 46, 49, 52, 55, 59, 63, 67, 71, 75, 79, 83, 87, 92, 97, 102, 107, 112, 117, 122, 127, 133, 139, 145, 151, 157, 163, 169, 175, 182, 189, 196, 203, 210, 217, 224, 231, 239, 247, 255};

volatile unsigned long lastRFUpdateAt=0;

byte currentMode = 0;

//animation related globals
#define LENGTH 11
#define OUTERLENGTH 8
#define INNERLENGTH 3
#define LEDPIN 10
unsigned long lastFrameAt;
byte pixels[LENGTH * 3];
unsigned long frameNumber = 0;
Adafruit_NeoPixel leds = Adafruit_NeoPixel(LENGTH, LEDPIN, NEO_GRB + NEO_KHZ800, pixels);

//RF related globals
const byte MyAddress = 0;
volatile byte receiving = 0;
volatile byte bitnum = 0; //current bit received
volatile byte gotMessage = 0;
volatile byte dataIn = 0;
volatile byte pktLength = 31;
volatile unsigned long lastRFMsgAt = 0;
volatile byte rxBuffer[32];
byte recvMessage[32];
#define RX_PIN_STATE (PINB&1) //RX on pin 8 for input capture. 

unsigned long lastPacketTime = 0;
unsigned long lastPacketSig = 0;

// Version 2.2/2.3
#if(F_CPU==8000000)
#define TIME_MULT * 1
#elif(F_CPU==16000000)
#define TIME_MULT * 2
#endif

const unsigned int rxSyncMin  = 1750 TIME_MULT;
const unsigned int rxSyncMax  = 2250 TIME_MULT;
const unsigned int rxZeroMin  = 100 TIME_MULT;
const unsigned int rxZeroMax  = 390 TIME_MULT;
const unsigned int rxOneMin  = 410 TIME_MULT;
const unsigned int rxOneMax  = 700 TIME_MULT;
const unsigned int rxLowMax  = 600 TIME_MULT;
const int commandForgetTime = 5000;

void setup() {
  setupPins();
  setupRF();
  Serial.begin(115200);
  delay(2000);
  pixels[0]=255;
  pixels[4]=255;
  pixels[8]=255;
}

void loop() {
  byte rlen = handleReceive();
  if (rlen) {
    processRFPacket(rlen);
  } else if ((!receiving)  && (millis() - lastRFMsgAt > 100)) { //will be if not receiving, but we don't know where we're saving the RXing status.
    if (millis() - lastFrameAt > getFrameDelay()) {
      lastFrameAt = millis();
      updatePattern();
      leds.show();
    }
  }
}

byte getFrameDelay() {
  return 400; 
}

void processRFPacket(byte rlen) {

  byte vers = (rlen & 196) >> 6;
  rlen &= 0x3F;
  //TODO: Everything. 
  
}

void setMode(byte mode) {
  currentMode = mode; 
  memset(pixels, 0, LENGTH * 3);
  if (!initializeMode(mode)) mode=0;
  // start with the first setting selected, in case we had a setting now out of index.
  frameNumber = 0;
}

byte initializeMode(byte mode) {
  for (byte i=0;i<8;i++) {
    updatePatternRainbow();
  }
}


void updatePattern() {
  //updatePatternRainbow();
  updatePatternSpinner();
  frameNumber++;
}


void rotateOuter(byte dir) {
  byte r;
  byte g;
  byte b;
  if (dir) { //reverse 
    g=pixels[0];
    r=pixels[1];
    b=pixels[2];
  } else {
    g=pixels[OUTERLENGTH*3-3];
    r=pixels[OUTERLENGTH*3-2];
    b=pixels[OUTERLENGTH*3-1];
  }
  pushOuter(r,g,b,dir);
}

void pushOuter(byte r,byte g,byte b,byte dir) {
  if (dir) { //reverse 
    for (byte i=0;i<(OUTERLENGTH-1)*3;i++) {
      pixels[i]=pixels[i+3];
    }
    pixels[OUTERLENGTH*3-3]=g;
    pixels[OUTERLENGTH*3-2]=r;
    pixels[OUTERLENGTH*3-1]=b;
  } else {
    for (byte i=(OUTERLENGTH*3)-1;i>2;i--) {
      pixels[i]=pixels[i-3];
    }
    pixels[0]=g;
    pixels[1]=r;
    pixels[2]=b;
  }
}

void rotateInner(byte dir) {
  byte r;
  byte g;
  byte b;
  if (dir) { //reverse 
    g=pixels[(OUTERLENGTH*3)];
    r=pixels[(OUTERLENGTH*3)+1];
    b=pixels[(OUTERLENGTH*3)+2];
  } else {
    g=pixels[((OUTERLENGTH+INNERLENGTH)*3)-3];
    r=pixels[((OUTERLENGTH+INNERLENGTH)*3)-2];
    b=pixels[((OUTERLENGTH+INNERLENGTH)*3)-1];
  }
  pushInner(r,g,b,dir);
}

void pushInner(byte r,byte g,byte b,byte dir) {
  if (dir) { //reverse 
    for (byte i=(OUTERLENGTH*3);i<((OUTERLENGTH+INNERLENGTH-1)*3);i++) {
      pixels[i]=pixels[i+3];
    }
    pixels[((OUTERLENGTH+INNERLENGTH)*3)-3]=g;
    pixels[((OUTERLENGTH+INNERLENGTH)*3)-2]=r;
    pixels[((OUTERLENGTH+INNERLENGTH)*3)-1]=b;
  } else {
    for (byte i=((OUTERLENGTH+INNERLENGTH)*3);i>((OUTERLENGTH)*3+2);i--) {
      pixels[i]=pixels[i-3];
    }
    pixels[(OUTERLENGTH*3)]=g;
    pixels[(OUTERLENGTH*3)+1]=r;
    pixels[(OUTERLENGTH*3)+2]=b;
  }
}

void smoothInner() { //set the inner pixels to averages of the outer pixels. 
  //Aligned = (outer + outer + right + left) >>2  //between (right + left)>>1)
  pixels[24]=(pixels[0]+pixels[3]*2+pixels[6])>>2; //G 
  pixels[25]=(pixels[1]+pixels[4]*2+pixels[7])>>2; //R
  pixels[26]=(pixels[2]+pixels[5]*2+pixels[8])>>2; //B
  pixels[30]=(pixels[9]+pixels[12])>>1; //G
  pixels[31]=(pixels[10]+pixels[13])>>1; //R
  pixels[32]=(pixels[11]+pixels[14])>>1; //B
  pixels[27]=(pixels[18]+pixels[21])>>1; //G
  pixels[28]=(pixels[19]+pixels[22])>>1; //R
  pixels[29]=(pixels[20]+pixels[23])>>1; //B
}



//length
//direction

void updatePatternSpinner() {
  rotateOuter(1);
  smoothInner();
}

void updatePatternRainbow() {
  static byte dir=0;
  static byte l=66;
  byte f = ((dir?0:8)+frameNumber) % (3*l); 
  byte r=0;
  byte g=0;
  byte b=0;
  byte fal = 0.0;
  byte rise = 0.0;
  if (f % l) {
    float tem = f % l;
    tem /= l;
    float temr = (tem * 255);
    float temg = (tem * 255);
    float temb = (tem * 255);
    if (f < l) { // sector 1 - green rising red falling
      g = temg + 0.5;
      temr = 255 - temr;
      r = temr + 0.5;
      b = 0;
    } else if (f < 2 * l) { // sector 2 - blue rising green falling
      temg = 255 - temg;
      g = temg + 0.5;
      b = temb + 0.5;
      r = 0;
    } else { // sector 3 - red rising blue falling
      temb = 255 - temb;
      b = temb + 0.5;
      r = temr + 0.5;
      g = 0;
    }
  } else {
    if (f == 0) {
      r = 255;
      g = 0;
      b = 0;
    } else if (f == l) {
      r = 0;
      g = 255;
      b = 0;
    } else {
      r = 0;
      g = 0;
      b = 255;
    }
  }
  pushOuter(r,g,b,dir); 
  smoothInner();
}



void setupPins() {
  pinMode(LEDPIN, OUTPUT);
}



void setupRF() {
  TCCR1A = 0;
  TCCR1B = 0;
  TIFR1 = bit (ICF1) | bit (TOV1);  // clear flags so we don't get a bogus interrupt
  TCNT1 = 0;          // Counter to zero
  TIMSK1 = 1 << ICIE1; // interrupt on Timer 1 input capture
  // start Timer 1, prescalar of 8, edge select on falling edge
  TCCR1B =  ((F_CPU == 1000000L) ? (1 << CS10) : (1 << CS11)) | 1 << ICNC1; //prescalar 8 except at 1mhz, where we use prescalar of 1, noise cancler active
  //ready to rock and roll
}

byte handleReceive() {
  if (gotMessage) {
    byte vers = checkCSC(); //checkCSC() gives 0 on failed CSC, 1 on v1 structure (ACD...), 2 on v2 structure (DSCD...)
    if (!vers) { //if vers=0, unknown format ot bad CSC
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
  if (bitnum > 4) {lastRFMsgAt=millis();}
  bitnum = 0;
  memset(rxBuffer, 0, 32);
  gotMessage = 0;
  TIMSK1 = 1 << ICIE1;
  return;
}

byte checkCSC() {
  byte rxchecksum = 0;
  byte rxchecksum2 = 0;
  byte rxc2;
  for (byte i = 0; i < pktLength >> 3; i++) {
    rxchecksum = rxchecksum ^ rxBuffer[i];
    rxc2 = rxchecksum2 & 128 ? 1 : 0;
    rxchecksum2 = (rxchecksum2 << 1 + rxc2)^rxBuffer[i];
  }
  if (pktLength >> 3 == 3) {
    rxchecksum = (rxchecksum & 0x0F) ^ (rxchecksum >> 4) ^ ((rxBuffer[3] & 0xF0) >> 4);
    rxchecksum2 = (rxchecksum2 & 0x0F) ^ (rxchecksum2 >> 4) ^ ((rxBuffer[3] & 0xF0) >> 4);
    if (rxchecksum == rxchecksum2)rxchecksum2++;
    return (rxBuffer[3] & 0x0F) == rxchecksum ? 1 : ((rxBuffer[3] & 0x0F) == rxchecksum2 ) ? 2 : 0;
  } else {
    if (rxchecksum == rxchecksum2)rxchecksum2++;
    return ((rxBuffer[pktLength >> 3] == rxchecksum) ? 1 : ((rxBuffer[bitnum >> 3] == rxchecksum2 ) ? 2 : 0));
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
  unsigned long lastpacketsig = 0;
  for (byte i = (len == 3 ? 0 : 1); i < (len == 3 ? 3 : 4); i++) {
    lastpacketsig += rxBuffer[i];
    lastpacketsig = lastpacketsig << 8;
  }
  lastpacketsig += rxBuffer[len];
  return lastpacketsig;
}

ISR (TIMER1_CAPT_vect)
{
  static unsigned long lasttime = 0;
  unsigned int newTime = ICR1; //immediately get the ICR value
  byte state = (RX_PIN_STATE);
  TCCR1B = state ? (1 << CS11 | 1 << ICNC1) : (1 << CS11 | 1 << ICNC1 | 1 << ICES1); //and set edge
  unsigned int duration = newTime - lasttime;
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
        TIMSK1 = 0; //turn off input capture;
      } else {
        bitnum++;
      }
    }
  }
}
/* //Old code, for reference. 
void updatePatternRainbow() {
  byte maxVal = COLORTABLEMAX;
  byte l = 9 + (6 * currentValueRight[1]);
  byte f = ((currentValueRight[2] ? 0 : LENGTH) + frameNumber) % (3 * l); //if in forward direction, add 200, otherwise don't - this keeps the color from skipping when reversing the direction.
  byte r = 0;
  byte g = 0;
  byte b = 0;
  byte fal = 0.0;
  byte rise = 0.0;
  if (f % l) {

    float tem = f % l;
    tem /= l;
    float temr = (tem * (getLeftVal(currentValueLeft[1]) - getLeftVal(currentValueLeft[0])) + getLeftVal(currentValueLeft[0]));
    float temg = (tem * (getLeftVal(currentValueLeft[3]) - getLeftVal(currentValueLeft[2])) + getLeftVal(currentValueLeft[2]));
    float temb = (tem * (getLeftVal(currentValueLeft[5]) - getLeftVal(currentValueLeft[4])) + getLeftVal(currentValueLeft[4]));
    if (f < l) { // sector 1 - green rising red falling
      g = temg + 0.5;
      temr = 255 - temr;
      r = temr + 0.5;
      b = getLeftVal(currentValueLeft[4]);
    } else if (f < 2 * l) { // sector 2 - blue rising green falling
      temg = 255 - temg;
      g = temg + 0.5;
      b = temb + 0.5;
      r = getLeftVal(currentValueLeft[0]);
    } else { // sector 3 - red rising blue falling
      temb = 255 - temb;
      b = temb + 0.5;
      r = temr + 0.5;
      g = getLeftVal(currentValueLeft[2]);
    }
  } else {
    if (f == 0) {
      r = getLeftVal(currentValueLeft[1]);
      g = getLeftVal(currentValueLeft[2]);
      b = getLeftVal(currentValueLeft[4]);
    } else if (f == l) {
      r = getLeftVal(currentValueLeft[0]);
      g = getLeftVal(currentValueLeft[3]);
      b = getLeftVal(currentValueLeft[4]);
    } else {
      r = getLeftVal(currentValueLeft[0]);
      g = getLeftVal(currentValueLeft[2]);
      b = getLeftVal(currentValueLeft[5]);
    }
  }
  if (currentValueRight[2]) { //reverse
    for (unsigned int i = 0; i < ((LENGTH - 1) * 3); i++) {
      pixels [i] = pixels[i + 3];
    }
    pixels[(LENGTH * 3) - 3] = r;
    pixels[(LENGTH * 3) - 2] = g;
    pixels[(LENGTH * 3) - 1] = b;
  } else {//forward
    for (unsigned int i = ((LENGTH) * 3)-1; i > 2; i--) {
      pixels [i] = pixels[i - 3];
    }
    pixels[0] = r;
    pixels[1] = g;
    pixels[2] = b;
  }
}
void updatePatternFade() {
  static byte bright=0;
  if (bright&128) {
    if (bright&63) {
      bright--;
    } else {
      bright=0;
    }
  } else {
    if (bright>=63) {
      bright=0xBF;
    } else {
      bright++;
    }
  }
  byte nbright = pgm_read_byte_near(&pulseBrightnessTable[63&bright]);
  byte r = map(nbright, 0, 255, getLeftVal(currentValueLeft[0]), getLeftVal(currentValueLeft[3]));
  byte g = map(nbright, 0, 255, getLeftVal(currentValueLeft[1]), getLeftVal(currentValueLeft[4]));
  byte b = map(nbright, 0, 255, getLeftVal(currentValueLeft[2]), getLeftVal(currentValueLeft[5]));
  for (unsigned int i=0;i<(LENGTH*3);i+=3) {
    pixels[i]=r;
    pixels[i+1]=g;
    pixels[i+2]=b;
  }   
}
void updatePatternWave() {
  static byte bright=0;
    for (byte i=0;i<(1+pgm_read_byte_near(&maxValueRight[currentMode][1])-currentValueRight[1]);i++) { 
  if (bright&128) {
    if (bright&63) {
      bright--;
    } else {
      bright=0;
    }
  } else {
    if (bright>=63) {
      bright=0xBF;
    } else {
      bright++;
    }
  }
  }
  byte nbright = pgm_read_byte_near(&pulseBrightnessTable[63&bright]);
  byte r = map(nbright, 0, 255, getLeftVal(currentValueLeft[0]), getLeftVal(currentValueLeft[3]));
  byte g = map(nbright, 0, 255, getLeftVal(currentValueLeft[1]), getLeftVal(currentValueLeft[4]));
  byte b = map(nbright, 0, 255, getLeftVal(currentValueLeft[2]), getLeftVal(currentValueLeft[5]));
  if (currentValueRight[2]) { //reverse
    for (unsigned int i = 0; i < ((LENGTH - 1) * 3); i++) {
      pixels [i] = pixels[i + 3];
    }
    pixels[(LENGTH * 3) - 3] = r;
    pixels[(LENGTH * 3) - 2] = g;
    pixels[(LENGTH * 3) - 1] = b;
  } else {//forward
    for (unsigned int i = ((LENGTH) * 3)-1; i > 2; i--) {
      pixels [i] = pixels[i - 3];
    }
    pixels[0] = r;
    pixels[1] = g;
    pixels[2] = b;
  }
}
void updatePatternChase() {
  static byte nextColorAt=0;
  static byte r;
  static byte g;
  static byte b;
  if (!nextColorAt){
   r = random(getLeftVal(currentValueLeft[0]), getLeftVal(currentValueLeft[3]));
   g = random(getLeftVal(currentValueLeft[1]), getLeftVal(currentValueLeft[4]));
   b = random(getLeftVal(currentValueLeft[2]), getLeftVal(currentValueLeft[5]));
   nextColorAt=random(5,5+(currentValueRight[1]*2)); 
  } else {
    nextColorAt--;
  }
  if (currentValueRight[2]) { //reverse
    for (unsigned int i = 0; i < ((LENGTH - 1) * 3); i++) {
      pixels [i] = pixels[i + 3];
    }
    pixels[(LENGTH * 3) - 3] = r;
    pixels[(LENGTH * 3) - 2] = g;
    pixels[(LENGTH * 3) - 1] = b;
  } else {//forward
    for (unsigned int i = ((LENGTH) * 3)-1; i > 2; i--) {
      pixels [i] = pixels[i - 3];
    }
    pixels[0] = r;
    pixels[1] = g;
    pixels[2] = b;
  }
}
*/
