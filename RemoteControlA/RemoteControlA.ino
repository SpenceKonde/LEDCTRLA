#include <avr/sleep.h>
#include <avr/power.h>

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

#define PCMSK0_SLEEP 0x79 //0b01111001

unsigned char txrxbuffer[RX_MAX_LEN >> 3];
byte btnst = 0;
byte myid;
byte mytarget=0x33;
byte sleeping = 0;
byte TXLength = 0;


unsigned int txOneLength  = 525;
unsigned int txZeroLength  = 300;
unsigned int txLowTime  = 300;
unsigned int txSyncTime  = 2000;
unsigned int txTrainLen  = 200;
byte txTrainRep  = 20;

const byte commands[][13] PROGMEM={ //mode,first six left settings,first six right settings
  {1,0,15,0,15,0,15,10,0,0,0,0,0}, //Basement 1
  {7,31,0,31,0,31,15,9,4,0,0,0,0}, //Basement 2
  {8,0,31,0,31,0,31,8,4,0,0,0,0}, //Basement 3
  {4,0,31,0,31,0,31,8,4,0,0,0,0}, //Basement 4
  {0,0,0,0,0,0,0,0,0,0,0,0,0} //Off
};
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
  pinMode(txpin,OUTPUT);
  //pinMode(LED5,OUTPUT);
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
  byte retval=0;
  retval+=digitalRead(BUTTON_1); //pin1
  retval+=digitalRead(BUTTON_2)<<1; //pin2
  retval+=digitalRead(BUTTON_3)<<2; //pin3
  retval+=digitalRead(BUTTON_4)<<3; //pin4
  retval+=digitalRead(BUTTON_5)<<4; //pin5
  return (~retval) & 0x1F;
}



void loop() {
  //btnst=(~PINB)&0x0F;
  byte btnst = getBtnst();
  if (btnst) {
    Serial.println(btnst);
    if (btnst == 1 ) {
      preparePayload16(0,0);
    } else if (btnst == 2) {
      preparePayload16(1,0);
    } else if (btnst == 4) {
      preparePayload16(2,0);
    } else if (btnst == 8) {
      preparePayload16(3,0);
    } else if (btnst ==16){
      preparePayload16(4,0);
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

void preparePayload(byte btn,byte rover) {
  byte plen = txrxbuffer[0] >> 6;
  plen = 4 << plen;
  txrxbuffer[0] = mytarget;
  txrxbuffer[1] = 0x55;
  txrxbuffer[2] = ((myid & 0x0F) << 4) + btn;
  txrxbuffer[3] = (rover?0x50:0x20);
  TXLength = 4;
}
void preparePayload8(byte btn,byte rover) {
  byte plen = txrxbuffer[0] >> 6;
  plen = 4 << plen;
  txrxbuffer[0] = 64|mytarget;
  txrxbuffer[1] = 0x55;
  txrxbuffer[2] = ((myid & 0x0F) << 4) + btn;
  txrxbuffer[3] = (rover?0x50:0x20);
  txrxbuffer[4] = 0x54;
  txrxbuffer[5] = 0x55;
  txrxbuffer[6] = 0x56;
  
  TXLength = 8;
}
void preparePayload16(byte btn,byte notused) {
  if (btn > 4) btn=4;
  txrxbuffer[0] = 128|(btn&2?0x2A,0x29);
  txrxbuffer[1] = 0x54;
  txrxbuffer[2] = pgm_read_byte_near(&commands[btn][0]);
  txrxbuffer[3] = pgm_read_byte_near(&commands[btn][1]);
  txrxbuffer[4] = pgm_read_byte_near(&commands[btn][2]);
  txrxbuffer[5] = pgm_read_byte_near(&commands[btn][3]);
  txrxbuffer[6] = pgm_read_byte_near(&commands[btn][4]);
  txrxbuffer[7] = pgm_read_byte_near(&commands[btn][5]);
  txrxbuffer[8] = pgm_read_byte_near(&commands[btn][6]);
  txrxbuffer[9] = pgm_read_byte_near(&commands[btn][7]);
  txrxbuffer[10] = pgm_read_byte_near(&commands[btn][8]);
  txrxbuffer[11] = pgm_read_byte_near(&commands[btn][9]);
  txrxbuffer[12] = pgm_read_byte_near(&commands[btn][10]);
  txrxbuffer[13] = pgm_read_byte_near(&commands[btn][11]);
  txrxbuffer[14] = pgm_read_byte_near(&commands[btn][12]);
  TXLength = 16;
}
void preparePayload32(byte btn,byte rover) {
  byte plen = txrxbuffer[0] >> 6;
  plen = 4 << plen;
  txrxbuffer[0] = 128|64|mytarget;
  txrxbuffer[1] = 0x55;
  txrxbuffer[2] = ((myid & 0x0F) << 4) + btn;
  txrxbuffer[3] = (rover?0x50:0x20);
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
  
  byte txchecksum = 0;
  for (byte i = 0; i < TXLength - 1; i++) {
    txchecksum = txchecksum ^ txrxbuffer[i];
  }
  if (TXLength == 4) {
    txchecksum = (txchecksum & 0x0F) ^ (txchecksum >> 4) ^ ((txrxbuffer[3] & 0xF0) >> 4);
    txrxbuffer[3] = (txrxbuffer[3] & 0xF0) + (txchecksum & 0x0F);
  } else {
    txrxbuffer[TXLength - 1] = txchecksum;
  }
  for (byte r = 0; r < rep; r++) {
    
    for (byte j = 0; j < 2 * txTrainRep; j++) {
      delayMicroseconds(txTrainLen);
      digitalWrite(txpin, j & 1);
      txPIN = txBV;
    }
    txPORT|=txBV;
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
    delayMicroseconds(rep>5?2500:5000); //wait 2.5ms before doing the next round.
    digitalWrite(txpin,0);
  }
  TXLength = 0;
}
