#include <Adafruit_NeoPixel_Static.h>
#include <avr/pgmspace.h>

#define LENGTH 200
#define LEDPIN 12
#define REDPIN A1
#define GREENPIN A2
#define BLUEPIN A3
#define RANDOMSEEDPIN A0

//DRIFTCHANCE must be a number from 1 to 127 - this is the chance out of 255 for an increase and for a decrease in brightness, eg, 127 will make it get brighter or dimmer every pass through loop();
#define DRIFTCHANCE 64

#define RANDINC 255-DRIFTCHANCE
#define RANDDEC DRIFTCHANCE

uint8_t pixels[LENGTH * 3]; //buffer - 3 bytes per LED
Adafruit_NeoPixel leds = Adafruit_NeoPixel(LENGTH, LEDPIN, NEO_GRB + NEO_KHZ800, pixels);

byte MaxChannel[] = {255, 255, 255};
byte MinChannel[] = {0,0,0};
void setup() {
  //Serial.begin(9600);
  //randomSeed(analogRead(RANDOMSEEDPIN));
  pinMode(LEDPIN, OUTPUT);
  //handleAnalogReadings();

  for (unsigned int i = 0; i < (LENGTH * 3); i++) {
    pixels[i] = 255;
  }
}

unsigned int mapChannelColor(int analogVal) {
  //Serial.print(analogVal);
  //Serial.print(" ");
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
  //Serial.print((unsigned int)min+(((unsigned int)max)<<8));
  //Serial.print(":");
  
  return (unsigned int)min+(((unsigned int)max)<<8);
}

int getAnalogValue(int pin) {
  int temp=0;
  temp+=analogRead(pin);
  temp+=analogRead(pin);
  temp+=analogRead(pin);
  temp+=analogRead(pin);
  return (temp>>2);
}

void handleAnalogReadings() {
  unsigned int grn = mapChannelColor(getAnalogValue(GREENPIN));
  unsigned int red = mapChannelColor(getAnalogValue(REDPIN));
  unsigned int blu = mapChannelColor(getAnalogValue(BLUEPIN));
  MinChannel[1]=grn&255;
  MinChannel[0]=red&255;
  MinChannel[2]=blu&255;
  MaxChannel[1]=grn>>8;
  MaxChannel[0]=red>>8;
  MaxChannel[2]=blu>>8;
}

void loop() {
  pixels[0]=~pixels[0];
  pixels[1]=~pixels[1];
  pixels[2]=~pixels[2];
  leds.show();
  /*
  handleAnalogReadings();
  for (byte i = 0; i < (LENGTH * 3); i++) {
    //if (UseChannel[i % 3]) {
      byte rand = random(255);
      if (rand > (pixels[i]>32?RANDINC:(RANDINC+DRIFTCHANCE/2)) && (pixels[i] < MaxChannel[i%3])) {
        if (pixels[i] > 128 && pixels[i] < 254) {
          pixels[i] += 2;
        } else {
          pixels[i]++;
        }
      } else if (rand < (pixels[i]>32?RANDDEC:(RANDDEC-DRIFTCHANCE/2)) && (pixels[i]>MinChannel[i%3])) {
        if (pixels[i] > 128) {
          pixels[i] -= 2;
        } else {
          pixels[i]--;
        }
      }
    //} else {
    //  pixels[i] = 0;
    //}
    */
  //}
  //leds.show();
  delay(250);
}
