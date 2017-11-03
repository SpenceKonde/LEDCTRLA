#include <Adafruit_NeoPixel_Static.h>
#include <avr/pgmspace.h>

#define LENGTH 50
#define LEDPIN 12
#define REDPIN A1
#define GREENPIN A2
#define BLUEPIN A3
#define RANDOMSEEDPIN A0

#define BUTTON1 3
#define BUTTON2 2
#define BUTTON3 5
#define BUTTON4 4



//DRIFTCHANCE must be a number from 1 to 127 - this is the chance out of 255 for an increase and for a decrease in brightness, eg, 127 will make it get brighter or dimmer every pass through loop();
#define DRIFTCHANCE 64

#define RANDINC 255-DRIFTCHANCE
#define RANDDEC DRIFTCHANCE

unsigned long lastAnimationTick = 0;

byte frameCount = 0;
byte animationState = 0;

uint8_t pixels[LENGTH * 3]; //buffer - 3 bytes per LED
Adafruit_NeoPixel leds = Adafruit_NeoPixel(LENGTH, LEDPIN, NEO_GRB + NEO_KHZ800, pixels);

byte MaxChannel[] = {255, 255, 255};
byte MinChannel[] = {0, 0, 0};
void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(RANDOMSEEDPIN));
  pinMode(LEDPIN, OUTPUT);
  handleAnalogReadings();
  for (byte i = 0; i < (LENGTH * 3); i++) {
    pixels[i] = (MaxChannel[i % 3] + MinChannel[i % 3]) >> 1;
  }
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);
  pinMode(BUTTON4, INPUT_PULLUP);
  leds.show();
}

unsigned int mapChannelColor(int analogVal) {
  byte min;
  byte max;
  if (analogVal > 512 - 128) {
    max = 255;
  } else if (analogVal < 65) {
    max = 0;
  } else {
    max = map(analogVal, 64, (512 - 128), 0, 255);
  }
  if (analogVal > (1023 - 64)) {
    min = 255;
  } else if (analogVal < 512 + 128 ) {
    min = 0;
  } else {
    min = map(analogVal, (512 + 128), (1023 - 64), 0, 255);
  }
  //Serial.print((unsigned int)min+(((unsigned int)max)<<8));
  //Serial.print(":");

  return (unsigned int)min + (((unsigned int)max) << 8);
}

int getAnalogValue(int pin) {
  int temp = 0;
  analogRead(pin); //throw away first reading.
  temp += analogRead(pin);
  temp += analogRead(pin);
  temp += analogRead(pin);
  temp += analogRead(pin);
  return (temp >> 2);
}

void handleAnalogReadings() {
  unsigned int grn = mapChannelColor(getAnalogValue(GREENPIN));
  unsigned int red = mapChannelColor(getAnalogValue(REDPIN));
  unsigned int blu = mapChannelColor(getAnalogValue(BLUEPIN));
  MinChannel[1] = grn & 255;
  MinChannel[0] = red & 255;
  MinChannel[2] = blu & 255;
  MaxChannel[1] = grn >> 8;
  MaxChannel[0] = red >> 8;
  MaxChannel[2] = blu >> 8;
}

void loop() {
  handleAnalogReadings();
  if (doAnimation()) {
    leds.show();
  }
  handleButtons();
}

void handleButtons() {
  byte startstate = animationState;
  if (!digitalRead(BUTTON1)) {
    animationState = 0;
  } else if (!digitalRead(BUTTON2)) {
    animationState = 1;
  } else if (!digitalRead(BUTTON3)) {
    animationState = 2;
  } else if (!digitalRead(BUTTON4)) {
    animationState = 3;
  }
  if (animationState != startstate) {
    Serial.println(animationState);
  }
}

byte doAnimation() {
  byte animationdone = 0;
  if (millis() - lastAnimationTick >= 10) {
    if (animationState == 0) { //Default (drift) state
      animationdone = onDriftState();
    } else if (animationState == 1) { //Default (drift) state
      animationdone = onTwinkleState();
    } else if (animationState == 2) { //Default (drift) state
      animationdone = onDriftState();
    } else {
      animationdone = onDriftState();
    }
  }
  if (animationdone) {
    lastAnimationTick = millis();
    frameCount++;;
    return 1;
  }
  return 0;
}

byte onDriftState() {
  for (byte i = 0; i < (LENGTH * 3); i++) {
    byte rand = random(255);
    if (rand > (pixels[i] > 32 ? RANDINC : (RANDINC + DRIFTCHANCE / 2)) && (pixels[i] < MaxChannel[i % 3])) {
      if (pixels[i] > 128 && pixels[i] < 254) {
        pixels[i] += 2;
      } else {
        pixels[i]++;
      }
    } else if (rand < (pixels[i] > 32 ? RANDDEC : (RANDDEC - DRIFTCHANCE / 2)) && (pixels[i] > MinChannel[i % 3])) {
      if (pixels[i] > 128) {
        pixels[i] -= 2;
      } else {
        pixels[i]--;
      }
    }
  }
  return 1;
}
byte onTwinkleState() {
  static byte twinklemode[LENGTH * 3];
  for (byte i = 0; i < LENGTH * 3; i++) {
    if (pixels[i] <= MinChannel[i % 3]) {
      twinklemode[i] = random(7);
      if (i == 1) {
        Serial.print("Twinkle Random:");
        Serial.println(twinklemode[i]);
      }
    } else {
      if (pixels[i] >= MaxChannel[i % 3]) { //in this case we need to reverse the direction
        twinklemode[i] |= 128;
      }
    }


    if (!(frameCount % ((pixels[i] > 32 ? 1 : 2) * (1 + (twinklemode[i] & 7))))) { //if it's this LED's turn to be incremented/decremented

      if (twinklemode[i] & 128) { //this means it's reached 255;
        pixels[i]--; //so it's getting dimmer;

      } else {
        if (pixels[i]!=255){
          pixels[i]++;
        }
      }
    }


  }
  return 1;
}

byte onPulseState() {
  static byte twinklemode[LENGTH];
  static byte brightness[LENGTH];
  static byte maxval[LENGTH*3];
  for (byte i = 0; i < LENGTH; i++) {
    if (brightness[i] == 0) {
      twinklemode[i] = random(7);
      maxval[(3*i)]=random(MinChannel[0],MaxChannel[0]);
      maxval[(3*i)+1]=random(MinChannel[1],MaxChannel[1]);
      maxval[(3*i)+2]=random(MinChannel[2],MaxChannel[2]);
      
    } else {
      if (brightness[i] == 255) { //in this case we need to reverse the direction
        twinklemode[i] |= 128;
      }
      if (!frameCount % ((brightness[i] > 32 ? 1 : 2) * (1 + (twinklemode[i] & 7)))) { //if it's this LED's turn to be incremented/decremented
        if (twinklemode[i] & 128) { //this means it's reached 255;
          brightness[i]--; //so it's getting dimmer;
        } else {
          brightness[i]++;
        }
        pixels[(3*i)]=map(brightness[i],0,255,0,maxval[3*i]);
        pixels[(3*i)+1]=map(brightness[i],0,255,0,maxval[1+(3*i)]);
        pixels[(3*i)+2]=map(brightness[i],0,255,0,maxval[2+(3*i)]);
      }

    }
  }
}

