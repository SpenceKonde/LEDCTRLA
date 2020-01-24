
#include <tinyNeoPixel_Static.h>


#define NEOPIXELPIN            0

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
#define NUMPIXELS  (((BUFFUSED/3)*2)+(((BUFFUSED*2)%3)?1:0)+1) //This ensures that the pixel buffer will fit all the LEDs


//These are the bytes in the buffer that correspond to the three rings of the two can lights.
#define OUTERSTARTA 0
#define OUTERENDA ((OUTERLEDS*OUTERCHANA)-1)
#define MIDSTARTA OUTERENDA+1
#define MIDENDA (MIDSTARTB+(MIDLEDS*MIDCHANB)-1)
#define INNERSTARTA MIDENDA+1
#define INNERENDA BUFFUSED-1
#define OUTERSTARTB BUFFUSED
#define OUTERENDB BUFFUSED+((OUTERLEDS*OUTERCHAN)-1)
#define MIDSTARTB OUTERENDB+1
#define MIDENDB (MIDSTARTB+(MIDLEDS*MIDCHAN)-1)
#define INNERSTARTB MIDENDB+1
#define INNERENDB (2*BUFFUSED)-1



// Since this is for the static version of the library, we need to supply the pixel array
// This saves space by eliminating use of malloc() and free(), and makes the RAM used for
// the frame buffer show up when the sketch is compiled.

byte pixels[NUMPIXELS * 3];

tinyNeoPixel leds = tinyNeoPixel(NUMPIXELS, PIN, NEO_GRB, pixels);

//IMPORTANT NOTE: Since we have a mixture of 3 and 4 color LEDs, we cannot use any of the abstractions for writing to LEDs - we must directly write to the buffer!

//WWA leds have Amber in place of Red, Cool White in place of Green, Warm White in place of Blue

void setup() {
  pinMode(PIN, OUTPUT);
  Serial.begin(9600);
  delay(100);
}

void loop() {
  selfTest();
}

//##################
// Pattern handling
//##################



//####################
//LED control functions
//####################

void setOuterAll(byte r, byte g, byte b, byte w = 0) {
  setOuterA(r,g,b,w);
  setOuterB(r,g,b,e);
}
void setMidAll(byte r, byte g, byte b, byte w = 0) {
  setMidA(r,g,b,w);
  setMidB(r,g,b,e);
}
void setInnerAll(byte r, byte g, byte b, byte w = 0) {
  setInnerA(r,g,b,w);
  setInnerB(r,g,b,e);
}


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
  byte i = INNERSTARTA
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
  byte i = MIDSTARTA;
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
void setInnerB(byte r, byte g, byte b, byte w = 0) {
  byte i = INNERSTARTA
  while (i < INNERENDA) {
    pixels[i++] = g;
    pixels[i++] = r;
    pixels[i++] = b;
    if (INNERCHAN == 4) {
      pixels[i++] = w;
    }
  }
}
void setMidB(byte r, byte g, byte b, byte w = 0) {
  byte i = MIDSTARTB;
  while (i < MIDENDB) {
    pixels[i++] = g;
    pixels[i++] = r;
    pixels[i++] = b;
    if (MIDCHAN == 4) {
      pixels[i++] = w;
    }
  }
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

// TO DO

