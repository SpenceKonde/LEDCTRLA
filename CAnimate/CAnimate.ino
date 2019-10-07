// NeoPixel simple sketch (c) 2013 Shae Erisson, adapted to tinyNeoPixel library by Spence Konde 2019.
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library

#include <tinyNeoPixel_Static.h>
#include "CanLights.h"


void setup() {
  
  pinMode(LED_PIN, OUTPUT);
}


void loop() {
  
  setMidAll(255,255,255,0);
  setOuterAll(255,64,0,255);
  setInnerAll(255,64,0,255);
  leds.show();
  delay(20000);
}
