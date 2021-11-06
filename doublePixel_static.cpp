/*-------------------------------------------------------------------------
  Arduino library to control a wide variety of WS2811- and WS2812-based RGB
  LED devices such as Adafruit FLORA RGB Smart Pixels and NeoPixel strips.
  Currently handles 800 KHz bitstreams on 8, 10, 12, 16, and 20 MHz ATtiny
  MCUs used with megaTinyCore 1.0.3+ and ATTinyCore 1.30+ with LEDs wired
  for various color orders.

  Written by Phil Burgess / Paint Your Dragon for Adafruit Industries,
  contributions by PJRC, Michael Miller and other members of the open
  source community.

  Modified for megaAVR and 20MHz and 10MHz support by Spence Konde

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  -------------------------------------------------------------------------
  This file is part of the Adafruit NeoPixel library.

  NeoPixel is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.

  NeoPixel is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with NeoPixel.  If not, see
  <http://www.gnu.org/licenses/>.
  -------------------------------------------------------------------------*/

#include "tinyNeoPixel_Static.h"

// Constructor when length, pin and type are known at compile-time:
tinyNeoPixel::tinyNeoPixel(uint16_t n, uint8_t p1, uint8_t p2, neoPixelType t, uint8_t *pxl) :
  brightness(0), pixels(pxl), endTime(0)
{
  //boolean oldThreeBytesPerPixel = (wOffset == rOffset); // false if RGBW

  wOffset = (t >> 6) & 0b11; // See notes in header file
  rOffset = (t >> 4) & 0b11; // regarding R/G/B/W offsets
  gOffset = (t >> 2) & 0b11;
  bOffset =  t       & 0b11;
  numBytes = n * ((wOffset == rOffset) ? 3 : 4);
  numLEDs=n;
  pin1 = p1;
  pin2 = p2;
#ifdef __AVR__
    port    = portOutputRegister(digitalPinToPort(p1));
    pinMask1 = digitalPinToBitMask(p1);
    pinMask2 = digitalPinToBitMask(p2);
#endif
  }
}



tinyNeoPixel::~tinyNeoPixel() {
  //if(pixels)   free(pixels);
  //if(pin >= 0) pinMode(pin, INPUT);
}


void tinyNeoPixel::show(void) {

  if(!pixels) return;

  // Data latch = 50+ microsecond pause in the output stream.  Rather than
  // put a delay at the end of the function, the ending time is noted and
  // the function will simply hold off (if needed) on issuing the
  // subsequent round of data until the latch time has elapsed.  This
  // allows the mainline code to start generating the next frame of data
  // rather than stalling for the latch.
  while(!canShow());
  // endTime is a private member (rather than global var) so that multiple
  // instances on different pins can be quickly issued in succession (each
  // instance doesn't delay the next).

  // In order to make this code runtime-configurable to work with any pin,
  // SBI/CBI instructions are eschewed in favor of full PORT writes via the
  // OUT or ST instructions.  It relies on two facts: that peripheral
  // functions (such as PWM) take precedence on output pins, so our PORT-
  // wide writes won't interfere, and that interrupts are globally disabled
  // while data is being issued to the LEDs, so no other code will be
  // accessing the PORT.  The code takes an initial 'snapshot' of the PORT
  // state, computes 'pin high' and 'pin low' values, and writes these back
  // to the PORT register as needed.

  noInterrupts(); // Need 100% focus on instruction timing


// AVR MCUs --  ATtiny and megaAVR ATtiny -------------------------------

  volatile uint16_t
    i   = numBytes>>1; // Loop counter
  volatile uint8_t
   *ptr1 = (pixels+i),          // Pointer to next byte, ascending half
    b1   = *ptr1++,             // Current byte, ascending half
   *ptr2 = (pixels+i),          // Pointer to next byte, descending half
    b2   = *--ptr2,             // Current byte, descending half
    hi,                         // PORT w/both output bits set high
    lo;                         // PORT w/both output bits set low

  // Hand-tuned assembly code issues data to the LED drivers at a specific
  // rate.  There's separate code for different CPU speeds (8, 12, 16 MHz)
  // for both the WS2811 (400 KHz) and WS2812 (800 KHz) drivers.  The
  // datastream timing for the LED drivers allows a little wiggle room each
  // way (listed in the datasheets), so the conditions for compiling each
  // case are set up for a range of frequencies rather than just the exact
  // 8, 12 or 16 MHz values, permitting use with some close-but-not-spot-on
  // devices (e.g. 16.5 MHz DigiSpark).  The ranges were arrived at based
  // on the datasheet figures and have not been extensively tested outside
  // the canonical 8/12/16 MHz speeds; there's no guarantee these will work
  // close to the extremes (or possibly they could be pushed further).
  // Keep in mind only one CPU speed case actually gets compiled; the
  // resulting program isn't as massive as it might look from source here.


// 20 MHz(ish) AVRxt ------------------------------------------------------
#if (F_CPU >= 19000000UL) && (F_CPU <= 22000000L)


  // 25 inst. clocks per bit: HHHHHHHxxxxxxxxLLLLLLLLLL
  // ST instructions:         ^      ^       ^       (T=0,7,15)

  volatile uint8_t next, bit;

  hi   = *port |  pinMask;
  lo   = *port & ~pinMask;
  next = lo;
  bit  = 8;

  asm volatile(
   "head20:"                   "\n\t" // Clk  Pseudocode    (T =  0)
    "st   %a[port],  %[hi]"    "\n\t" // 1    PORT = hi     (T =  1)
    "sbrc %[byte],  7"         "\n\t" // 1-2  if(b & 128)
     "mov  %[next], %[hi]"     "\n\t" // 0-1   next = hi    (T =  3)
    "dec  %[bit]"              "\n\t" // 1    bit--         (T =  4)
    "nop"                      "\n\t" // 1    nop           (T =  5)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T =  7)
    "st   %a[port],  %[next]"  "\n\t" // 1    PORT = next   (T =  8)
    "mov  %[next] ,  %[lo]"    "\n\t" // 1    next = lo     (T =  9)
    "breq nextbyte20"          "\n\t" // 1-2  if(bit == 0) (from dec above)
    "rol  %[byte]"             "\n\t" // 1    b <<= 1       (T = 11)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 13)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 15)
    "st   %a[port],  %[lo]"    "\n\t" // 1    PORT = lo     (T = 16)
    "nop"                      "\n\t" // 1    nop           (T = 17)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 19)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 21)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 23)
    "rjmp head20"              "\n\t" // 2    -> head20 (next bit out)
   "nextbyte20:"               "\n\t" //                    (T = 11)
    "ldi  %[bit]  ,  8"        "\n\t" // 1    bit = 8       (T = 12)
    "ld   %[byte] ,  %a[ptr]+" "\n\t" // 2    b = *ptr++    (T = 14)
    "nop"                      "\n\t" // 1    nop           (T = 15)
    "st   %a[port], %[lo]"     "\n\t" // 1    PORT = lo     (T = 16)
    "nop"                      "\n\t" // 1    nop           (T = 17)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 19)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 21)
    "sbiw %[count], 1"         "\n\t" // 2    i--           (T = 23)
     "brne head20"             "\n"   // 2    if(i != 0) -> (next byte)  ()
    : [port]  "+e" (port),
      [byte]  "+r" (b),
      [bit]   "+r" (bit),
      [next]  "+r" (next),
      [count] "+w" (i)
    : [ptr]    "e" (ptr),
      [hi]     "r" (hi),
      [lo]     "r" (lo));

// 24 (22~26) MHz AVRxt  ------------------------------------------------------
#elif (F_CPU >= 22000000UL) && (F_CPU <= 26000000L)


  // 30 inst. clocks per bit: HHHHHHHxxxxxxxxLLLLLLLLLL
  // ST instructions:         ^      ^       ^       (T=0,9,18)

  volatile uint8_t next, bit;

  hi   = *port |  (pinMask1 | pinMask2);
  lo   = *port & ~(pinMask1 | pinMask2);
  next = lo;
  bit  = 8;


  asm volatile(
   "head24:"                   "\n\t" // Clk  Pseudocode    (T =  0)
    "st   %a[port],  %[hi]"    "\n\t" // 1    PORT = hi     (T =  1)
    "sbrc %[byt1],  7"         "\n\t" // 1-2  if(b1 & 128)
    "or   %[next], %[pm1]"     "\n\t" // 0-1  next |= pm1   (T =  3)
    "sbrc %[byt2],  7"         "\n\t" // 1-2  if(b2 & 128)
    "or   %[next], %[pm2]"     "\n\t" // 0-1  next |= pm2   (T =  5)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T =  7)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T =  9)
    "st   %a[port],  %[next]"  "\n\t" // 1    PORT = next   (T = 10)
    "mov  %[next] ,  %[lo]"    "\n\t" // 1    next = lo     (T = 11)
    "dec  %[bit]"              "\n\t" // 1    bit--         (T = 12)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 14)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 16)
    "breq nextbyte24"          "\n\t" // 1-2  if(bit == 0) (from dec above)
    "rol  %[byt1]"             "\n\t" // 1    b1 <<= 1      (T = 18)
    "st   %a[port],  %[lo]"    "\n\t" // 1    PORT = lo     (T = 19)
    "rol  %[byt2]"             "\n\t" // 1    b2 <<= 1      (T = 20)
    "rcall seconddelay24"      "\n\t" // 2+4+2=8            (T = 28)
    "rjmp head24"              "\n\t" // 2    -> head20 (next bit out)
   "seconddelay24:"            "\n\t" //
    "rjmp .+0"                 "\n\t" // 2
    "ret"                      "\n\t" // 4
   "nextbyte24:"               "\n\t" // last bit of a byte (T = 18)
    "st   %a[port], %[lo]"     "\n\t" // 1    PORT = lo     (T = 19)
    "ldi  %[bit]  ,  8"        "\n\t" // 1    bit = 8       (T = 20)
    "ld   %[byt1] ,  %a[pt1]+" "\n\t" // 2    b1 = *ptr++   (T = 22)
    "ld   %[byt2] ,  -%a[pt2]" "\n\t" // 2    b2 = *--ptr   (T = 24)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 26)
    "sbiw %[count], 1"         "\n\t" // 2    i--           (T = 28)
    "brne head24"              "\n"   // 2    if(i != 0) -> (next byte)  ()
    : [port]  "+e" (port),
      [byt1]  "+r" (b1),
      [byt2]  "+r" (b2),
      [bit]   "+r" (bit),
      [next]  "+d" (next),
      [count] "+w" (i)
    : [pt1]    "e" (ptr1),
      [pt2]    "e" (ptr2),
      [hi]     "r" (hi),
      [lo]     "r" (lo)),
      [pm1]    "r" (pinMask1),
      [pm2]    "r" (pinMask2);




// 28 (26~30) MHz AVRxt  ------------------------------------------------------
#elif (F_CPU >= 26000000UL) && (F_CPU <= 30000000L)


  // 35 inst. clocks per bit: HHHHHHHxxxxxxxxLLLLLLLLLL
  // ST instructions:         ^      ^       ^       (T=0,10,21)

  volatile uint8_t next, bit;

  hi   = *port |  pinMask;
  lo   = *port & ~pinMask;
  next = lo;
  bit  = 8;

  asm volatile(
   "head28:"                   "\n\t" // Clk  Pseudocode    (T =  0)
    "st   %a[port],  %[hi]"    "\n\t" // 1    PORT = hi     (T =  1)
    "sbrc %[byte],  7"         "\n\t" // 1-2  if(b & 128)
    "mov  %[next], %[hi]"      "\n\t" // 0-1   next = hi    (T =  3)
    "dec  %[bit]"              "\n\t" // 1    bit--         (T =  4)
    "rcall zerothdelay32"      "\n\t" // 2+4=6
    "st   %a[port],  %[next]"  "\n\t" // 1    PORT = next   (T = 11)
    "mov  %[next] ,  %[lo]"    "\n\t" // 1    next = lo     (T = 12)
    "rcall firstdelay28"       "\n\t" // 2+4 = 7            (T = 19)
    "breq nextbyte28"          "\n\t" // 1-2  if(bit == 0) (from dec above)
    "rol  %[byte]"             "\n\t" // 1    b <<= 1       (T = 21)
    "st   %a[port],  %[lo]"    "\n\t" // 1    PORT = lo     (T = 22)
    "rcall seconddelay28"      "\n\t" // 2+4+1+4=11         (T = 33)
    "rjmp head28"              "\n\t" // 2    -> head20 (next bit out)
   "seconddelay28:"            "\n\t" //
   "rjmp .+0"                  "\n\t" // 2
   "rjmp .+0"                  "\n\t" // 2
   "firstdelay28:"             "\n\t" // first delay
    "nop"                      "\n\t" // 1    nop
   "thirddelay28:"             "\n\t" // third delay
   "zerothdelay28:"            "\n\t"
    "ret"                      "\n\t" // 4
   "nextbyte28:"               "\n\t" // last bit of a byte (T = 21)
    "st   %a[port], %[lo]"     "\n\t" // 1    PORT = lo     (T = 22)
    "ldi  %[bit]  ,  8"        "\n\t" // 1    bit = 8       (T = 23)
    "ld   %[byte] ,  %a[ptr]+" "\n\t" // 2    b = *ptr++    (T = 25)
    "rcall thirddelay28"       "\n\t" // 2+4 = 6            (T = 31)
    "sbiw %[count], 1"         "\n\t" // 2    i--           (T = 33)
     "brne head28"             "\n"   // 2    if(i != 0) -> (next byte)  ()
    : [port]  "+e" (port),
      [byte]  "+r" (b),
      [bit]   "+r" (bit),
      [next]  "+r" (next),
      [count] "+w" (i)
    : [ptr]    "e" (ptr),
      [hi]     "r" (hi),
      [lo]     "r" (lo));


// 32 (30~34) MHz AVRxt  ------------------------------------------------------
#elif (F_CPU > 30000000UL) && (F_CPU <= 34000000L)


  // 40 inst. clocks per bit: HHHHHHHxxxxxxxxLLLLLLLLLL
  // ST instructions:         ^      ^       ^       (T=0,11,24)

  volatile uint8_t next, bit;

  hi   = *port |  pinMask;
  lo   = *port & ~pinMask;
  next = lo;
  bit  = 8;

  asm volatile(
   "head32:"                   "\n\t" // Clk  Pseudocode    (T =  0)
    "st   %a[port],  %[hi]"    "\n\t" // 1    PORT = hi     (T =  1)
    "sbrc %[byte],  7"         "\n\t" // 1-2  if(b & 128)
    "mov  %[next], %[hi]"      "\n\t" // 0-1   next = hi    (T =  3)
    "dec  %[bit]"              "\n\t" // 1    bit--         (T =  4)
    "rcall zerothdelay32"      "\n\t" // 2+4+1=7
    "st   %a[port],  %[next]"  "\n\t" // 1    PORT = next   (T = 12)
    "mov  %[next] ,  %[lo]"    "\n\t" // 1    next = lo     (T = 13)
    "rcall firstdelay32"       "\n\t" // 2+4+1+2 = 9        (T = 22)
    "breq nextbyte32"          "\n\t" // 1-2  if(bit == 0) (from dec above)
    "rol  %[byte]"             "\n\t" // 1    b <<= 1       (T = 24)
    "st   %a[port],  %[lo]"    "\n\t" // 1    PORT = lo     (T = 25)
    "rcall seconddelay32"      "\n\t" // 2+4+3+2+3=13       (T = 38)
    "rjmp head32"              "\n\t" // 2    -> head20 (next bit out)
   "seconddelay32:"            "\n\t" // second delay 13 cycles
    "rjmp .+0"                 "\n\t" // 2
    "rjmp .+0"                 "\n\t" // 2
   "firstdelay32:"             "\n\t" // first delay 9 cycles
    "nop"                      "\n\t" // 1    nop
   "thirddelay32:"             "\n\t" // third delay 8 cycles
    "nop"                      "\n\t" // 1    nop
   "zerothdelay32:"            "\n\t" // zeroth delay 7 cycles
    "nop"                      "\n\t" // 1    nop
    "ret"                      "\n\t" // 4
   "nextbyte32:"               "\n\t" // last bit of a byte (T = 24)
    "st   %a[port], %[lo]"     "\n\t" // 1    PORT = lo     (T = 25)
    "ldi  %[bit]  ,  8"        "\n\t" // 1    bit = 8       (T = 26)
    "ld   %[byte] ,  %a[ptr]+" "\n\t" // 2    b = *ptr++    (T = 28)
    "rcall thirddelay32"       "\n\t" // 2+4+1+1 = 8        (T = 36)
    "sbiw %[count], 1"         "\n\t" // 2    i--           (T = 38)
     "brne head32"             "\n"   // 2    if(i != 0) -> (next byte)  ()
    : [port]  "+e" (port),
      [byte]  "+r" (b),
      [bit]   "+r" (bit),
      [next]  "+r" (next),
      [count] "+w" (i)
    : [ptr]    "e" (ptr),
      [hi]     "r" (hi),
      [lo]     "r" (lo));

// 36 (34~38) MHz AVRxt  ------------------------------------------------------
#elif (F_CPU > 3400000UL) && (F_CPU <= 38000000L)


  // 45 inst. clocks per bit: HHHHHHHxxxxxxxxLLLLLLLLLL
  // ST instructions:         ^      ^       ^       (T=0,12,27)

  volatile uint8_t next, bit;

  hi   = *port |  pinMask;
  lo   = *port & ~pinMask;
  next = lo;
  bit  = 8;

  asm volatile(
   "head36:"                   "\n\t" // Clk  Pseudocode    (T =  0)
    "st   %a[port],  %[hi]"    "\n\t" // 1    PORT = hi     (T =  1)
    "sbrc %[byte],  7"         "\n\t" // 1-2  if(b & 128)
    "mov  %[next], %[hi]"      "\n\t" // 0-1   next = hi    (T =  3)
    "dec  %[bit]"              "\n\t" // 1    bit--         (T =  4)
    "rcall zerothdelay36"      "\n\t" // 2+4+2=8
    "st   %a[port],  %[next]"  "\n\t" // 1    PORT = next   (T = 13)
    "mov  %[next] ,  %[lo]"    "\n\t" // 1    next = lo     (T = 14)
    "rcall firstdelay36"       "\n\t" // 2+4+3 = 11         (T = 25)
    "breq nextbyte36"          "\n\t" // 1-2  if(bit == 0) (from dec above)
    "rol  %[byte]"             "\n\t" // 1    b <<= 1       (T = 27)
    "st   %a[port],  %[lo]"    "\n\t" // 1    PORT = lo     (T = 28)
    "rcall seconddelay36"      "\n\t" // 2+4+3+2+2=15       (T = 43)
    "rjmp head36"              "\n\t" // 2    -> head20 (next bit out)
   "seconddelay36:"            "\n\t" // second delay 15 cycles
    "rjmp .+0"                 "\n\t" // 2
    "rjmp .+0"                 "\n\t" // 2
   "firstdelay36:"             "\n\t" // first delay 11 cycles
    "nop"                      "\n\t" // 1    nop
   "thirddelay36:"             "\n\t" // third delay 10 cycles
    "rjmp .+0"                 "\n\t" // 2    nop nop
   "zerothdelay36:"            "\n\t" // zeroth delay 8 cycles
    "rjmp .+0"                 "\n\t" // 2    nop nop
    "ret"                      "\n\t" // 4
   "nextbyte36:"               "\n\t" // last bit of a byte (T = 27)
    "st   %a[port], %[lo]"     "\n\t" // 1    PORT = lo     (T = 28)
    "ldi  %[bit]  ,  8"        "\n\t" // 1    bit = 8       (T = 29)
    "ld   %[byte] ,  %a[ptr]+" "\n\t" // 2    b = *ptr++    (T = 31)
    "rcall thirddelay36"       "\n\t" // 2+4 = 10           (T = 41)
    "sbiw %[count], 1"         "\n\t" // 2    i--           (T = 43)
     "brne head36"             "\n"   // 2    if(i != 0) -> (next byte)  ()
    : [port]  "+e" (port),
      [byte]  "+r" (b),
      [bit]   "+r" (bit),
      [next]  "+r" (next),
      [count] "+w" (i)
    : [ptr]    "e" (ptr),
      [hi]     "r" (hi),
      [lo]     "r" (lo));               "\n\t" // 2    nop nop       (T = 19)


// 40 (38-44) MHz AVRxt  ------------------------------------------------------
#elif (F_CPU > 3800000UL) && (F_CPU <= 44000000L)


  // 50 inst. clocks per bit: HHHHHHHxxxxxxxxLLLLLLLLLL
  // ST instructions:         ^      ^       ^       (T=0,14,30)

  volatile uint8_t next, bit;

  hi   = *port |  pinMask;
  lo   = *port & ~pinMask;
  next = lo;
  bit  = 8;

  asm volatile(
   "head40:"                   "\n\t" // Clk  Pseudocode    (T =  0)
    "st   %a[port],  %[hi]"    "\n\t" // 1    PORT = hi     (T =  1)
    "sbrc %[byte],  7"         "\n\t" // 1-2  if(b & 128)
    "mov  %[next], %[hi]"      "\n\t" // 0-1   next = hi    (T =  3)
    "dec  %[bit]"              "\n\t" // 1    bit--         (T =  4)
    "rcall zerothdelay40"      "\n\t" // 2+4+4=10
    "st   %a[port],  %[next]"  "\n\t" // 1    PORT = next   (T = 15)
    "mov  %[next] ,  %[lo]"    "\n\t" // 1    next = lo     (T = 16)
    "rcall firstdelay40"       "\n\t" // 2+4+4+2 = 12         (T = 28)
    "breq nextbyte40"          "\n\t" // 1-2  if(bit == 0) (from dec above)
    "rol  %[byte]"             "\n\t" // 1    b <<= 1       (T = 30)
    "st   %a[port],  %[lo]"    "\n\t" // 1    PORT = lo     (T = 31)
    "rcall seconddelay40"      "\n\t" // 2+4+3+2+3=17       (T = 48)
    "rjmp head40"              "\n\t" // 2    -> head20 (next bit out)
   "seconddelay40:"            "\n\t" // second delay 17 cycles
    "nop"                      "\n\t" // 1    nop
    "rjmp .+0"                 "\n\t" // 2
    "rjmp .+0"                 "\n\t" // 2
   "thirddelay40:"             "\n\t" // third delay 12 cycles
   "firstdelay40:"             "\n\t" // first delay 12 cycles
    "rjmp .+0"                 "\n\t" // 2    nop nop
   "zerothdelay40:"            "\n\t" // zeroth delay 10 cycles
    "rjmp .+0"                 "\n\t" // 2    nop nop
    "rjmp .+0"                 "\n\t" // 2    nop nop
    "ret"                      "\n\t" // 4
   "nextbyte40:"               "\n\t" // last bit of a byte (T = 30)
    "st   %a[port], %[lo]"     "\n\t" // 1    PORT = lo     (T = 31)
    "ldi  %[bit]  ,  8"        "\n\t" // 1    bit = 8       (T = 32)
    "ld   %[byte] ,  %a[ptr]+" "\n\t" // 2    b = *ptr++    (T = 34)
    "rcall thirddelay40"       "\n\t" // 2+4+4+2 = 12       (T = 46)
    "sbiw %[count], 1"         "\n\t" // 2    i--           (T = 48)
     "brne head40"             "\n"   // 2    if(i != 0) -> (next byte)  ()
    : [port]  "+e" (port),
      [byte]  "+r" (b),
      [bit]   "+r" (bit),
      [next]  "+r" (next),
      [count] "+w" (i)
    : [ptr]    "e" (ptr),
      [hi]     "r" (hi),
      [lo]     "r" (lo));

// 48 (44-50) MHz AVRxt  ------------------------------------------------------
#elif (F_CPU > 4400000UL) && (F_CPU <= 50000000L)


  // 60 inst. clocks per bit: HHHHHHHxxxxxxxxLLLLLLLLLL
  // ST instructions:         ^      ^       ^       (T=0,16,35)

  volatile uint8_t next, bit;

  hi   = *port |  pinMask;
  lo   = *port & ~pinMask;
  next = lo;
  bit  = 8;
  asm volatile(
   "head48:"                   "\n\t" // Clk  Pseudocode    (T =  0)
    "st   %a[port],  %[hi]"    "\n\t" // 1    PORT = hi     (T =  1)
    "sbrc %[byte],  7"         "\n\t" // 1-2  if(b & 128)
    "mov  %[next], %[hi]"      "\n\t" // 0-1   next = hi    (T =  3)
    "dec  %[bit]"              "\n\t" // 1    bit--         (T =  4)
    "rcall zerothdelay48"      "\n\t" // 2+4=13
    "st   %a[port],  %[next]"  "\n\t" // 1    PORT = next   (T = 17)
    "mov  %[next] ,  %[lo]"    "\n\t" // 1    next = lo     (T = 18)
    "rcall firstdelay48"       "\n\t" // 2+4+3 = 15         (T = 33)
    "breq nextbyte48"          "\n\t" // 1-2  if(bit == 0) (from dec above)
    "rol  %[byte]"             "\n\t" // 1    b <<= 1       (T = 35)
    "st   %a[port],  %[lo]"    "\n\t" // 1    PORT = lo     (T = 36)
    "rcall seconddelay48"      "\n\t" // 2+4+3+2+3=22       (T = 58)
    "rjmp head48"              "\n\t" // 2    -> head20 (next bit out)
   "seconddelay48:"            "\n\t" // second delay 22 cycles
    "rjmp .+0"                 "\n\t" // 2
    "rjmp .+0"                 "\n\t" // 2
    "nop"                      "\n\t" // 1    nop
   "thirddelay48:"             "\n\t" // third delay 17 cycles
    "rjmp .+0"                 "\n\t" // 2
   "firstdelay48:"             "\n\t" // first delay 15 cycles
    "rjmp .+0"                 "\n\t" // 2    nop nop
   "zerothdelay48:"            "\n\t" // zeroth delay 13 cycles
    "nop"                      "\n\t" // 1    nop
    "rcall emptydelay48"       "\n\t" // 2+4
    "ret"                      "\n\t" // 4
   "emptydelay48:"
    "ret"                      "\n\t" // 4
   "nextbyte48:"               "\n\t" // last bit of a byte (T = 35)
    "st   %a[port], %[lo]"     "\n\t" // 1    PORT = lo     (T = 36)
    "ldi  %[bit]  ,  8"        "\n\t" // 1    bit = 8       (T = 37)
    "ld   %[byte] ,  %a[ptr]+" "\n\t" // 2    b = *ptr++    (T = 39)
    "rcall thirddelay48"       "\n\t" // 2+4 = 17           (T = 56)
    "sbiw %[count], 1"         "\n\t" // 2    i--           (T = 58)
     "brne head48"             "\n"   // 2    if(i != 0) -> (next byte)  ()
    : [port]  "+e" (port),
      [byte]  "+r" (b),
      [bit]   "+r" (bit),
      [next]  "+r" (next),
      [count] "+w" (i)
    : [ptr]    "e" (ptr),
      [hi]     "r" (hi),
      [lo]     "r" (lo));

#else
 #error "CPU SPEED NOT SUPPORTED"
#endif

// END AVR ----------------------------------------------------------------


  interrupts();
#ifndef DISABLEMILLIS
  endTime = micros(); // Save EOD time for latch on next call
#endif
}

// Set the output pin number
void tinyNeoPixel::setPin(uint8_t p) {
    pin = p;
    port    = portOutputRegister(digitalPinToPort(p));
    pinMask = digitalPinToBitMask(p);
}

// Set pixel color from separate R,G,B components:
void tinyNeoPixel::setPixelColor(
 uint16_t n, uint8_t r, uint8_t g, uint8_t b) {

  if(n < numLEDs) {
    if(brightness) { // See notes in setBrightness()
      r = (r * brightness) >> 8;
      g = (g * brightness) >> 8;
      b = (b * brightness) >> 8;
    }
    uint8_t *p;
    if(wOffset == rOffset) { // Is an RGB-type strip
      p = &pixels[n * 3];    // 3 bytes per pixel
    } else {                 // Is a WRGB-type strip
      p = &pixels[n * 4];    // 4 bytes per pixel
      p[wOffset] = 0;        // But only R,G,B passed -- set W to 0
    }
    p[rOffset] = r;          // R,G,B always stored
    p[gOffset] = g;
    p[bOffset] = b;
  }
}

void tinyNeoPixel::setPixelColor(
 uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {

  if(n < numLEDs) {
    if(brightness) { // See notes in setBrightness()
      r = (r * brightness) >> 8;
      g = (g * brightness) >> 8;
      b = (b * brightness) >> 8;
      w = (w * brightness) >> 8;
    }
    uint8_t *p;
    if(wOffset == rOffset) { // Is an RGB-type strip
      p = &pixels[n * 3];    // 3 bytes per pixel (ignore W)
    } else {                 // Is a WRGB-type strip
      p = &pixels[n * 4];    // 4 bytes per pixel
      p[wOffset] = w;        // Store W
    }
    p[rOffset] = r;          // Store R,G,B
    p[gOffset] = g;
    p[bOffset] = b;
  }
}

// Set pixel color from 'packed' 32-bit RGB color:
void tinyNeoPixel::setPixelColor(uint16_t n, uint32_t c) {
  if(n < numLEDs) {
    uint8_t *p,
      r = (uint8_t)(c >> 16),
      g = (uint8_t)(c >>  8),
      b = (uint8_t)c;
    if(brightness) { // See notes in setBrightness()
      r = (r * brightness) >> 8;
      g = (g * brightness) >> 8;
      b = (b * brightness) >> 8;
    }
    if(wOffset == rOffset) {
      p = &pixels[n * 3];
    } else {
      p = &pixels[n * 4];
      uint8_t w = (uint8_t)(c >> 24);
      p[wOffset] = brightness ? ((w * brightness) >> 8) : w;
    }
    p[rOffset] = r;
    p[gOffset] = g;
    p[bOffset] = b;
  }
}


// Query color from previously-set pixel (returns packed 32-bit RGB value)
uint32_t tinyNeoPixel::getPixelColor(uint16_t n) const {
  if(n >= numLEDs) return 0; // Out of bounds, return no color.

  uint8_t *p;

  if(wOffset == rOffset) { // Is RGB-type device
    p = &pixels[n * 3];
    if(brightness) {
      // Stored color was decimated by setBrightness().  Returned value
      // attempts to scale back to an approximation of the original 24-bit
      // value used when setting the pixel color, but there will always be
      // some error -- those bits are simply gone.  Issue is most
      // pronounced at low brightness levels.
      return (((uint32_t)(p[rOffset] << 8) / brightness) << 16) |
             (((uint32_t)(p[gOffset] << 8) / brightness) <<  8) |
             ( (uint32_t)(p[bOffset] << 8) / brightness       );
    } else {
      // No brightness adjustment has been made -- return 'raw' color
      return ((uint32_t)p[rOffset] << 16) |
             ((uint32_t)p[gOffset] <<  8) |
              (uint32_t)p[bOffset];
    }
  } else {                 // Is RGBW-type device
    p = &pixels[n * 4];
    if(brightness) { // Return scaled color
      return (((uint32_t)(p[wOffset] << 8) / brightness) << 24) |
             (((uint32_t)(p[rOffset] << 8) / brightness) << 16) |
             (((uint32_t)(p[gOffset] << 8) / brightness) <<  8) |
             ( (uint32_t)(p[bOffset] << 8) / brightness       );
    } else { // Return raw color
      return ((uint32_t)p[wOffset] << 24) |
             ((uint32_t)p[rOffset] << 16) |
             ((uint32_t)p[gOffset] <<  8) |
              (uint32_t)p[bOffset];
    }
  }
}

// Returns pointer to pixels[] array.  Pixel data is stored in device-
// native format and is not translated here.  Application will need to be
// aware of specific pixel data format and handle colors appropriately.
uint8_t *tinyNeoPixel::getPixels(void) const {
  return pixels;
}

uint16_t tinyNeoPixel::numPixels(void) const {
  return numLEDs;
}

// Adjust output brightness; 0=darkest (off), 255=brightest.  This does
// NOT immediately affect what's currently displayed on the LEDs.  The
// next call to show() will refresh the LEDs at this level.  However,
// this process is potentially "lossy," especially when increasing
// brightness.  The tight timing in the WS2811/WS2812 code means there
// aren't enough free cycles to perform this scaling on the fly as data
// is issued.  So we make a pass through the existing color data in RAM
// and scale it (subsequent graphics commands also work at this
// brightness level).  If there's a significant step up in brightness,
// the limited number of steps (quantization) in the old data will be
// quite visible in the re-scaled version.  For a non-destructive
// change, you'll need to re-render the full strip data.
void tinyNeoPixel::setBrightness(uint8_t b) {
  // Stored brightness value is different than what's passed.
  // This simplifies the actual scaling math later, allowing a fast
  // 8x8-bit multiply and taking the MSB.  'brightness' is a uint8_t,
  // adding 1 here may (intentionally) roll over...so 0 = max brightness
  // (color values are interpreted literally; no scaling), 1 = min
  // brightness (off), 255 = just below max brightness.
  uint8_t newBrightness = b + 1;
  if(newBrightness != brightness) { // Compare against prior value
    // Brightness has changed -- re-scale existing data in RAM
    uint8_t  c,
            *ptr           = pixels,
             oldBrightness = brightness - 1; // De-wrap old brightness value
    uint16_t scale;
    if(oldBrightness == 0) scale = 0; // Avoid /0
    else if(b == 255) scale = 65535 / oldBrightness;
    else scale = (((uint16_t)newBrightness << 8) - 1) / oldBrightness;
    for(uint16_t i=0; i<numBytes; i++) {
      c      = *ptr;
      *ptr++ = (c * scale) >> 8;
    }
    brightness = newBrightness;
  }
}

//Return the brightness value
uint8_t tinyNeoPixel::getBrightness(void) const {
  return brightness - 1;
}

void tinyNeoPixel::clear() {
  memset(pixels, 0, numBytes);
}
