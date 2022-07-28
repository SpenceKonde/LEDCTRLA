#include "LightCtrl_RevE.h"
#include "Colors.h"


volatile byte bitnum = 0; //current bit received

volatile byte gotMessage = 0;

volatile byte dataIn = 0;
volatile byte pktLength = 31;
volatile unsigned long lastRFMsgAt = 0;
volatile byte rxBuffer[32];
byte recvMessage[32];

unsigned long lastPacketTime = 0;
unsigned long lastPacketSig = 0;

void printSpecs();
void printSpecs(UartClass t);
