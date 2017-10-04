#include <Adafruit_NeoPixel_Static.h>
#include <avr/pgmspace.h>

#define LENGTH 50
#define LEDPIN 12
#define REDPIN A1
#define GREENPIN A2
#define BLUEPIN A3
#define RANDOMSEEDPIN 0

//DRIFTCHANCE must be a number from 1 to 127 - this is the chance out of 255 for an increase and for a decrease in brightness, eg, 127 will make it get brighter or dimmer every pass through loop();
#define DRIFTCHANCE 64

#define RANDINC 255-DRIFTCHANCE
#define RANDDEC DRIFTCHANCE

uint8_t pixels[LENGTH * 3]; //buffer - 3 bytes per LED
Adafruit_NeoPixel leds = Adafruit_NeoPixel(LENGTH, LEDPIN, NEO_GRB + NEO_KHZ800, pixels);

byte MaxChannel[] = {255, 255, 255};
byte MinChannel[] = {0,0,0};
void setup() {
  randomSeed(analogRead(RANDOMSEEDPIN));
  pinMode(LEDPIN, OUTPUT);
  handleAnalogReadings();
  for (byte i = 0; i < (LENGTH * 3); i++) {
    pixels[i] = (MaxChannel[i%3]+MinChannel[i%3])>>1;
  }
  leds.show();
}

unsigned int mapChannelColor(int analogVal) {
  byte min;
  byte max;
  if (analogVal > 512-128) {
    max=255;
  } else if (analogVal < 65) {
    max=0;
  } else {
    max=map(analogVal,64,(512-128),0,255);
  }
  if (analogVal > (1023-64)) {
    min=255;
  } else if (analogVal < 512+128 ) {
    min=0;
  } else {
    min=map(analogVal,(512+128),(1023-64),0,255);
  }
  return min+(((int)max)<<8);
}

int getAnalogValue(pin) {
  int temp=0;
  temp+=analogRead(pin);
  temp+=analogRead(pin);
  temp+=analogRead(pin);
  temp+=analogRead(pin);
  return (temp>>2);
}

void handleAnalogReadings() {
  unsigned int grn = mapChannelColor(analogRead(GREENPIN));
  unsigned int red = mapChannelColor(analogRead(REDPIN));
  unsigned int blu = mapChannelColor(analogRead(BLUEPIN));
  MinChannel[0]=grn&&255;
  MinChannel[1]=red&&255;
  MinChannel[2]=blu&&255;
  MaxChannel[0]=grn>>8;
  MaxChannel[1]=red>>8;
  MaxChannel[2]=blu>>8;
}

void loop() {
  handleAnalogReadings();
  for (byte i = 0; i < (LENGTH * 3); i++) {
    //if (UseChannel[i % 3]) {
      byte rand = random(255);
      if (rand > RANDINC && (pixels[i] < MaxChannel[i%3])) {
        if (pixels[i] > 128 && pixels[i] < 254) {
          pixels[i] += 2;
        } else {
          pixels[i]++;
        }
      } else if (rand < RANDDEC && (pixels[i]>MinChannel[i%3])) {
        if (pixels[i] > 128) {
          pixels[i] -= 2;
        } else {
          pixels[i]--;
        }
      }
    //} else {
    //  pixels[i] = 0;
    //}
  }
  leds.show();
}
