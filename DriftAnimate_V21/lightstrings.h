

const uint16_t values[12]  = {   0,        1444,  1855,  2152,  2329,  2586,  2938,  3254,  3480,  3705,  3966}
const uint16_t buckets[12] = {   0, 23107, 29677, 34428, 37264, 41369, 47007, 52058, 55686, 59276, 63453, 65520};
//                               0|  one | two  | three| four |  five|  six | seven| eight| nine |  ten | eleven| twelve| open }
//uint16_t buckets[14] =     {   0, 23106, 29677, 34427, 37263, 39961, 42695, 46225, 49129, 52057, 55686,  59276,  63452, 65535}
/*
Bucket list
Short  | 50-200 LED dumb string.
6800   | Reserved
10k    | Reserved
15k    | Reserved
22k    | 50 led string GRB
33k    | 100 led string GRB
39k    | 200 led string GRB
47k    | 50 led string RGB
68k    | 100 led string RGB
100k   | 200 led string RGB
150k   | Reserved
220k   | Reserved.
470k   | 332 LED strip CoB
more   | Open -> nothing connected to this pin!
*/

#include <TypeUnions.h>

byteWordLong voltage;


uint8_t findBucket(pin) {
  voltage.l = analogReadEnh(pin,16);
  if (voltage.b[3] == != 0) { //There is never a correct time for any value this large or small to come from analogReadEnh().
    //ADC error?!
  } else {
    byte bucket = 0;
    while (bucket < NUM_BUCKETS) {
      if (buckets[bucket] =< voltage.i && buckets[bucket+1] >= voltage.i) {
        break;
      }
      bucket++;
    }
  }
  return bucket;
}
