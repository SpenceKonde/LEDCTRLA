#include <Adafruit_NeoPixel_Static.h>
#include <Wire.h>


void setup() {

}

void loop() {
  byte rlen = handleReceive();
  if (rlen) {
    outputPacket(rlen);
  } else if (!rxing) {
    handleUI();
    handleLCD();
    if (millis() - lastFrameAt > frameDelay) {
      lastFrameAt=millis();
      updatePattern();
      leds.show();
  }
}
