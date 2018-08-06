#include <avr/sleep.h>
#include <avr/power.h>
#include <util/crc16.h>

#define txpin 2
#define txPIN PINB
#define txPORT PORTB
#define txBV 4
#define RX_MAX_LEN 256
#include <EEPROM.h>
//#define LED5 4
//#define LED_ON 1
//#define LED_OFF 0
#define BUTTON_1 10
#define BUTTON_2 7
#define BUTTON_3 6
#define BUTTON_4 5
#define BUTTON_5 4
#define BUTTON_6 3

#define PCMSK0_SLEEP 0xF9 //0b11111001

unsigned char txrxbuffer[RX_MAX_LEN >> 3];
byte btnst = 0;
byte myid;
byte mytarget = 40;
byte sleeping = 0;
byte TXLength = 0;
#define TXVERSION 2

unsigned int txOneLength  = 525;
unsigned int txZeroLength  = 300;
unsigned int txLowTime  = 300;
unsigned int txSyncTime  = 2000;
unsigned int txTrainLen  = 200;
byte txTrainRep  = 20;


/*
  unsigned int rxSyncMin  = 1750;
  unsigned int rxSyncMax  = 2250;
  unsigned int rxZeroMin  = 100;
  unsigned int rxZeroMax  = 490;
  unsigned int rxOneMin  = 510;
  unsigned int rxOneMax  = 900;
  unsigned int rxLowMax  = 900;
  unsigned int txOneLength  = 700;
  unsigned int txZeroLength  = 300;
  unsigned int txLowTime  = 500;
  unsigned int txSyncTime  = 2000;
  unsigned int txTrainLen  = 200;
  byte txTrainRep  = 30;
*/
unsigned int txRepDelay = 2000; //delay between consecutive transmissions
byte txRepCount = 5; //number of times to repeat each transmission

void setup() {
  if (EEPROM.read(3) < 255) {
#ifdef OSCCAL
    OSCCAL = EEPROM.read(3);
#else
    OSCCAL0 = EEPROM.read(3);
#endif
    delay(50); //let's be cautious;
  }
  byte tval = EEPROM.read(8);
  mytarget = (tval == 255) ? mytarget : tval;
  tval = EEPROM.read(9);
  myid = (tval == 255) ? myid : tval;
  pinMode(txpin, OUTPUT);
  //pinMode(LED5,OUTPUT);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  GIMSK |= 1 << PCIE0; //enable PCINT on port A
  PCMSK0 = 0;
  ADCSRA &= 127; //turn off ADC, we don't need it and it's just gonna waste power.
  Serial.begin(9600);
  delay(500);
  Serial.println("ROAR");

}

ISR (PCINT0_vect) // handle pin change interrupt for D0 to D7 here
{
  PCMSK0 = 0; //disable the interrupts by masking it off.
  sleeping = 0;
}

byte getBtnst() {
  byte retval = 0;
  retval += digitalRead(BUTTON_1); //pin1
  retval += digitalRead(BUTTON_2) << 1; //pin2
  retval += digitalRead(BUTTON_3) << 2; //pin3
  retval += digitalRead(BUTTON_4) << 3; //pin4
  retval += digitalRead(BUTTON_5) << 4; //pin5
  retval += digitalRead(BUTTON_6) << 5; //pin6
  return (~retval) & 0x3F;
}



void loop() {
  //btnst=(~PINB)&0x0F;
  byte btnst = getBtnst();
  if (btnst) {
    Serial.println(btnst);
    if (btnst == 1 ) {
      preparePayload_1();
    } else if (btnst == 2) {
      preparePayload_2();
    } else if (btnst == 4) {
      preparePayload_3();
    } else if (btnst == 8) {
      preparePayload_4();
    } else if (btnst == 16) {
      preparePayload_5();
    } else if (btnst == 32) {
      preparePayload_6();
    }
    doTransmit(10);
  }
  delay(50);
  if (!btnst) { //make sure all the buttons are not pressed, otherwise skip sleep and send the signal again
    PCMSK0 = PCMSK0_SLEEP;
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleeping = 1;
    sleep_enable();
    //Serial.println("sleep enabled");
    //delay(200);
    sleep_mode();
    //now we're sleeping....

    sleep_disable(); //execution will continue from here.
    delay(60);
  }
}

void preparePayload_1() {
  txrxbuffer[0] = 128 | 64 | mytarget;
  txrxbuffer[1] = 0x58;
  txrxbuffer[2] = ((myid & 0x3F));
  txrxbuffer[3] = (0x12); //mode 1 (fade), direction
  txrxbuffer[4] = 0xFF;
  txrxbuffer[5] = 0x0;
  txrxbuffer[6] = 0x0;
  txrxbuffer[7] = 0x0;
  txrxbuffer[8] = 0xFF;
  txrxbuffer[9] = 0x0;
  txrxbuffer[10] = 0x0;
  txrxbuffer[11] = 0x0;
  txrxbuffer[12] = 0xFF;
  txrxbuffer[13] = 0x55;
  txrxbuffer[14] = 0x55;
  txrxbuffer[15] = 0x55;
  txrxbuffer[16] = 0x55;
  txrxbuffer[17] = 0x55;
  txrxbuffer[18] = 0x55;
  txrxbuffer[19] = 0x55;
  txrxbuffer[20] = 0x55;
  txrxbuffer[21] = 0x55;
  txrxbuffer[22] = 0x55;
  txrxbuffer[23] = 0x57;
  txrxbuffer[24] = 0x55;
  txrxbuffer[25] = 0x55;
  txrxbuffer[26] = 0x55;
  txrxbuffer[27] = 0x55;
  txrxbuffer[28] = 0x0; //high bits
  txrxbuffer[29] = 0x20; //dwell
  txrxbuffer[30] = 0xFF; //transition
  txrxbuffer[31] = 0; //doesn't matter, used for CSC

  TXLength = 32;
}

void preparePayload_2() {
  txrxbuffer[0] = 128 | 64 | mytarget;
  txrxbuffer[1] = 0x58;
  txrxbuffer[2] = ((myid & 0x3F));
  txrxbuffer[3] = (0x02); //mode 0 (wave), direction, 3 colors
  txrxbuffer[4] = 0xFF;
  txrxbuffer[5] = 0x0;
  txrxbuffer[6] = 0x0;
  txrxbuffer[7] = 0x0;
  txrxbuffer[8] = 0xFF;
  txrxbuffer[9] = 0x0;
  txrxbuffer[10] = 0x0;
  txrxbuffer[11] = 0x0;
  txrxbuffer[12] = 0xFF;
  txrxbuffer[13] = 0x55;
  txrxbuffer[14] = 0x55;
  txrxbuffer[15] = 0x55;
  txrxbuffer[16] = 0x55;
  txrxbuffer[17] = 0x55;
  txrxbuffer[18] = 0x55;
  txrxbuffer[19] = 0x55;
  txrxbuffer[20] = 0x55;
  txrxbuffer[21] = 0x55;
  txrxbuffer[22] = 0x55;
  txrxbuffer[23] = 0x57;
  txrxbuffer[24] = 0x55;
  txrxbuffer[25] = 0x55;
  txrxbuffer[26] = 0x55;
  txrxbuffer[27] = 0x55;
  txrxbuffer[28] = 0x0; //high bits
  txrxbuffer[29] = 0x20; //dwell
  txrxbuffer[30] = 0xFF; //transition
  txrxbuffer[31] = 0; //doesn't matter, used for CSC

  TXLength = 32;
}

void preparePayload_3() {
  txrxbuffer[0] = 128 | 64 | mytarget;
  txrxbuffer[1] = 0x59;
  txrxbuffer[2] = ((myid & 0x3F));
  txrxbuffer[3] = 0x10; //Outer rotates, inner is blank
  txrxbuffer[4] = 0x0;
  txrxbuffer[5] = 0xFF;
  txrxbuffer[6] = 0;
  txrxbuffer[7] = 0;
  txrxbuffer[8] = 0;
  txrxbuffer[9] = 0;
  txrxbuffer[10] = 0;
  txrxbuffer[11] = 0;
  txrxbuffer[12] = 0;
  txrxbuffer[13] = 0;
  txrxbuffer[14] = 0;
  txrxbuffer[15] = 0;
  txrxbuffer[16] = 0xFF;
  txrxbuffer[17] = 0;
  txrxbuffer[18] = 0;
  txrxbuffer[19] = 0;
  txrxbuffer[20] = 0;
  txrxbuffer[21] = 0;
  txrxbuffer[22] = 0;
  txrxbuffer[23] = 0;
  txrxbuffer[24] = 0;
  txrxbuffer[25] = 0;
  txrxbuffer[26] = 0;
  txrxbuffer[27] = 0;
  txrxbuffer[28] = 0x0; //not used
  txrxbuffer[29] = 0x05; //dwell
  txrxbuffer[30] = 0x0; //not used
  txrxbuffer[31] = 0; //doesn't matter, used for CSC

  TXLength = 32;
}

void preparePayload_4() {
  txrxbuffer[0] = 128|mytarget;
  txrxbuffer[1] = 0x59; //spin
  txrxbuffer[2] = myid & 0x3F;
  txrxbuffer[3] = 0x32; //inside rotates backwards, outside follows.
  txrxbuffer[4] = 0;
  txrxbuffer[5] = 0;
  txrxbuffer[6] = 255;
  txrxbuffer[7] = 0;
  txrxbuffer[8] = 0;
  txrxbuffer[9] =0;
  txrxbuffer[10] = 0;
  txrxbuffer[11] = 0;
  txrxbuffer[12] = 0;
  txrxbuffer[13] = 0;
  txrxbuffer[14] = 0;
  TXLength = 16;
}

void preparePayload_5() {
  txrxbuffer[0] = mytarget;
  txrxbuffer[1] = 0x5F;
  txrxbuffer[2] = myid & 0x3F;
  txrxbuffer[3] = 0;
  TXLength = 4;
}


void preparePayload_6() {
  txrxbuffer[0] = mytarget;
  txrxbuffer[1] = 0x5F;
  txrxbuffer[2] = myid & 0x3F;
  txrxbuffer[3] = 0;
  TXLength = 4;
}


void preparePayload(byte btn, byte rover) {
  byte plen = txrxbuffer[0] >> 6;
  plen = 4 << plen;
  txrxbuffer[0] = mytarget;
  txrxbuffer[1] = 0x55;
  txrxbuffer[2] = ((myid & 0x0F) << 4) + btn;
  txrxbuffer[3] = (rover ? 0x50 : 0x20);
  TXLength = 4;
}
void preparePayload8(byte btn, byte rover) {
  byte plen = txrxbuffer[0] >> 6;
  plen = 4 << plen;
  txrxbuffer[0] = 64 | mytarget;
  txrxbuffer[1] = 0x55;
  txrxbuffer[2] = ((myid & 0x0F) << 4) + btn;
  txrxbuffer[3] = (rover ? 0x50 : 0x20);
  txrxbuffer[4] = 0x54;
  txrxbuffer[5] = 0x55;
  txrxbuffer[6] = 0x56;

  TXLength = 8;
}
void preparePayload16(byte btn, byte notused) {
  if (btn > 5) btn = 5;
  txrxbuffer[0] = 128 | mytarget;
  txrxbuffer[1] = 226;
  txrxbuffer[2] = 0x31;
  txrxbuffer[3] = 0;
  txrxbuffer[4] = 0;
  txrxbuffer[5] = 0;
  txrxbuffer[6] = 0;
  txrxbuffer[7] = 0;
  txrxbuffer[8] = 0;
  txrxbuffer[9] =0;
  txrxbuffer[10] = 0;
  txrxbuffer[11] = 0;
  txrxbuffer[12] = 0;
  txrxbuffer[13] = 0;
  txrxbuffer[14] = 0;
  TXLength = 16;
}
void preparePayload32(byte btn, byte rover) {
  byte plen = txrxbuffer[0] >> 6;
  plen = 4 << plen;
  txrxbuffer[0] = 128 | 64 | mytarget;
  txrxbuffer[1] = 0x55;
  txrxbuffer[2] = ((myid & 0x0F) << 4) + btn;
  txrxbuffer[3] = (rover ? 0x50 : 0x20);
  txrxbuffer[4] = 0x54;
  txrxbuffer[5] = 0x55;
  txrxbuffer[6] = 0x56;
  txrxbuffer[7] = 0x54;
  txrxbuffer[8] = 0x55;
  txrxbuffer[9] = 0x56;
  txrxbuffer[10] = 0x54;
  txrxbuffer[11] = 0x55;
  txrxbuffer[12] = 0x56;
  txrxbuffer[13] = 0x54;
  txrxbuffer[14] = 0x55;
  txrxbuffer[15] = 0x55;
  txrxbuffer[16] = 0x55;
  txrxbuffer[17] = 0x55;
  txrxbuffer[18] = 0x55;
  txrxbuffer[19] = 0x55;
  txrxbuffer[20] = 0x10;
  txrxbuffer[21] = 0x25;
  txrxbuffer[22] = 0xFF;
  txrxbuffer[23] = 0x57;
  txrxbuffer[24] = 0x55;
  txrxbuffer[25] = 0x0F;
  txrxbuffer[26] = 0xF0;
  txrxbuffer[27] = 0x55;
  txrxbuffer[28] = 0x55;
  txrxbuffer[29] = 0x55;
  txrxbuffer[30] = 0x55;
  txrxbuffer[31] = 0x55;

  TXLength = 32;
}



void doTransmit(int rep) { //rep is the number of repetitions
  Serial.println("Starting transmit");
  byte vers=TXVERSION;
  byte txchecksum = 0;
  byte txchecksum2 = 0;
  for (byte i = 0; i < TXLength - 1; i++) {
    txchecksum = txchecksum ^ txrxbuffer[i];
    txchecksum2 = _crc8_ccitt_update(txchecksum2, txrxbuffer[i]);
  }
  if (TXLength == 4) {
    txchecksum = (txchecksum & 0x0F) ^ (txchecksum >> 4) ^ ((txrxbuffer[3] & 0xF0) >> 4);
    txchecksum2 = (txchecksum2 & 0x0F) ^ (txchecksum2 >> 4) ^ ((txrxbuffer[3] & 0xF0) >> 4);
    if (txchecksum == txchecksum2)txchecksum2++;
      txrxbuffer[3] = (txrxbuffer[3] & 0xF0) | 0x0F & (vers == 1 ? txchecksum : txchecksum2);
  } else {
    if (txchecksum == txchecksum2)txchecksum2++;
    txrxbuffer[TXLength - 1] = (vers == 1 ? txchecksum : txchecksum2);
  }
  for (byte r = 0; r < rep; r++) {

    for (byte j = 0; j < 2 * txTrainRep; j++) {
      delayMicroseconds(txTrainLen);
      digitalWrite(txpin, j & 1);
      txPIN = txBV;
    }
    txPORT |= txBV;
    delayMicroseconds(txSyncTime);
    txPIN = txBV;
    delayMicroseconds(txSyncTime);
    for (byte k = 0; k < TXLength; k++) {
      //send a byte
      for (int m = 7; m >= 0; m--) {
        txPIN = txBV;
        if ((txrxbuffer[k] >> m) & 1) {
          delayMicroseconds(txOneLength);
        } else {
          delayMicroseconds(txZeroLength);
        }
        txPIN = txBV;
        /*
          if ((txrxbuffer[k] >> m) & 1) {
          delayMicroseconds(txOneLength);
          } else {
          delayMicroseconds(txZeroLength);
          }
        */
        delayMicroseconds(txLowTime);
      }
      //done with that byte
    }
    //done with sending this packet;
    //digitalWrite(txpin, 1); //make sure it's off;
    //interrupts();
    delayMicroseconds(rep > 5 ? 2500 : 5000); //wait 2.5ms before doing the next round.
    digitalWrite(txpin, 0);
  }
  TXLength = 0;
}
