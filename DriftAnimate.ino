#include <Adafruit_NeoPixel_Static.h>
#include <avr/pgmspace.h>

#define LENGTH 50
#define LEDPIN 14
#define REDPIN 11
#define GREENPIN 12
#define BLUEPIN 13
#define RANDOMSEEDPIN 0

//DRIFTCHANCE must be a number from 1 to 127 - this is the chance out of 255 for an increase and for a decrease in brightness, eg, 127 will make it get brighter or dimmer every pass through loop();
#define DRIFTCHANCE 64 

#define RANDINC 255-DRIFTCHANCE
#define RANDDEC DRIFTCHANCE

uint8_t pixels[LENGTH*3]; //buffer - 3 bytes per LED 
Adafruit_NeoPixel leds = Adafruit_NeoPixel(LENGTH, LEDPIN, NEO_GRB + NEO_KHZ800, pixels);

byte UseChannel[]={1,1,1};

void setup() {
   randomSeed(analogRead(RANDOMSEEDPIN));
   pinMode(GREENPIN,INPUT_PULLUP);
   pinMode(REDPIN,INPUT_PULLUP);
   pinMode(BLUEPIN,INPUT_PULLUP);
   pinMode(LEDPIN,OUTPUT);
   for (byte i=0;i<(LENGTH*3);i++) {
      
      pixels[i]=128;
   }
   leds.show();
}

void loop() {
   if (digitalRead(GREENPIN) {UseChannel[0]=1;} else {UseChannel[0]=0;}
   if (digitalRead(REDPIN) {UseChannel[1]=1;} else {UseChannel[1]=0;}
   if (digitalRead(BLUEPIN) {UseChannel[2]=1;} else {UseChannel[2]=0;}
   for (byte i=0;i<(LENGTH*3);i++) {
      if (UseChannel[i%3]) {
         byte rand=random(255);
         if (rand > RANDINC && pixels[i] < 255) {
            pixels[i]++;
         } else if (rand < RANDDEC && pixels[i]) {
            pixels[i]--;
         }
      } else {
         pixels[i]=0;
      }
   }
   leds.show(); 
}
