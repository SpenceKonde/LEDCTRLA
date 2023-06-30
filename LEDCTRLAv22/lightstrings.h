
#include <TypeUnions.h>

const uint16_t buckets[12] = {   0,   260,   886,  1469,   2190,  2935,  3604,  4368,   5117,   5984,  6986,  8519, 65535};
//                            ....zero |  one |  two | three | four | five |  six | seven | eight | nine |  ten | Open or Active....

const ledsegment_t ledbkcts[12] ={{200, NEO_GRB, 0x80 },  /* 0 */
                                  {65535, 255, 255 },     /* 680 */
                                  { 50, NEO_GRB, 0x00 },  /*1.33k */
                                  { 50, NEO_RGB, 0x00 },  /*1.83k */
                                  {100, NEO_GRB, 0x00 },  /*3.00k */
                                  {100, NEO_RGB, 0x00 },  /*3.60k */
                                  {200, NEO_GRB, 0x00 },  /*4.7k  */
                                  {65535, 255, 255 },     /*5.62k */
                                  {332, NEO_GRB, 0x00 },  /*6.80k */
                                  {65535, 255, 255 },     /*8.20k */
                                  {100, NEO_GRB, 0x20 },  /*10.0k */
                                  {  0, 255, 255}  }Any higher - no string


ledsegment_t Segments[16];
uint16_t LEDCount[16];
/*
Bucket list
Short  | 50-200 GRB         | 0
 680   | TBD                | 1
 1.33k | 50 led string  GRB | 2
 1.82k | 50 led string  RGB | 3
 3.00k | 100 led string GRB | 4
 3.60k | 100 led string RGB | 5
 4.70k | 200 led string GRB | 6
 5.62k | TBD                | 7
 6.80k | 332 led COB string | 8
 8.20k | TBD                | 9
10.0k  | 100 led fixed address 100-200 GRB            | 10
more   | Open -> nothing connected to this pin, no analog measurement even taken because the pin never went low.   | 11
*/

typedef struct ledsegment {
  uint16_t ledcount;
  uint8_t colororder;
  uint8_t config; // 0x??aa tttt; tttt = type of branching;
} ledsegment_t;

/* 0x10 - fixed addresses!
   0x20 - fixed addresses, second set
   0x30 - TBD
   0x40 -
   0x80 - End of string

/* Branching modes:
 * 0 - sequential, no reversal.
 * 1 - sequential, but this string is backwards Reverse direction,
 * 2 - sequential, string not backwards, but pattern direction reverses here. Once per configuration only.
 * 3 - Sequential, string is backwards, and we're reversing the pattern direction.
 * 4 - Split; - store state we wrote to last pixel before this,
 * 5-7  Split, but with the same special conditions as above.
 * 8-F - As before, but using alternate color pallete, if available.
 */
/*

RGB  0b00000110 = 06
RBG  0b00001001 = 09
GRB  0b01010010 = 52
BRG  0b01011000 = 58
GBR  0b10100010 = A2
BGR  0b10100100 = A4
WRGB 0b00011011 = 1B X
WRBG 0b00011110 = 1E X
WGRB 0b00100111 = 27 X
WBRG 0b00101101 = 2D X
WGBR 0b00110110 = 36 X
WBGR 0b00111001 = 39 X
RWGB 0b01001011 = 4B X
RWBG 0b01001110 = 4E X
GWRB 0b01100011 = 63 36
BWRG 0b01101100 = 6C X
GWBR 0b01110010 = 72 27
BWGR 0b01111000 = 78 X
RGWB 0b10000111 = 87 78
RBWG 0b10001101 = 8D X
GRWB 0b10010011 = 93 39
BRWG 0b10011100 = 9C X
GBWR 0b10110001 = B1 1B
BGWR 0b10110100 = B4 4B
RGBW 0b11000110 = C6 6C
RBGW 0b11001001 = C9 9C
GRBW 0b11010010 = D2 2D
BRGW 0b11011000 = D8 8D
GBRW 0b11100001 = E1 1E
BGRW 0b11100100 = E4 4E
*/

uint8_t findBucket(uint8_t pin) {
  int32_t reading analogReadEnh(pin, 15);
  if (reading < 0) {
    return 255;
  } else {
    uint8_t i = 0;
    while (i < 11) {
      if (reading > buckets[i] && reading < buckets[i + 1]) {
        break;
      } else {
        i++;
      }
    }
    if (i >= 11) {
      return 255;
    }
    return i;
  }
}
