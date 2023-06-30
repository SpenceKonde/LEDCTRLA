#include "lightstring.h"
const uint16_t buckets[12] = {   0,   260,   886,  1469,   2190,  2935,  3604,  4368,   5117,   5984,  6986,  8519, 65535};
//                            ....zero |  one |  two | three | four | five |  six | seven | eight | nine |  ten | Open or Active....

ledsegment_t Segments[16];
uint16_t LEDCount[16];

uint8_t findBucket(pin) {
  byteWordLong voltage;
  voltage.l = analogReadEnh(pin,15);
  uint8_t bucket = 0;
  if (voltage.b[3] != 0) { //There is never a correct time for any value this large or small to come from analogReadEnh().
    Serial.print(F("Wtf, serial error? "));
    Serial.println(voltage.l);
  } else {
    while (bucket < NUM_BUCKETS) {
      if (buckets[bucket] =< voltage.i && buckets[bucket + 1] >= voltage.i) {
        break;
      }
      bucket++;
    }
  }
  return bucket;
}
