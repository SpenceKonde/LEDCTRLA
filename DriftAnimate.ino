#include <Adafruit_NeoPixel_Static.h>

#define LENGTH 50
#define LEDPIN 14

uint8_t pixels[LENGTH*3]; //buffer - 3 bytes per LED 
Adafruit_NeoPixel leds = Adafruit_NeoPixel(LENGTH, LEDPIN, NEO_GRB + NEO_KHZ800, pixels);

void setup() {
   pinMode(LEDPIN,OUTPUT);
   for (byte i=0;i<(LENGTH*3);i++) {
      pixels[i]=128;
   }
}

void loop() {
   leds.show(); 
}
