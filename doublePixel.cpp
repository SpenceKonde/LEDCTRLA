/*-------------------------------------------------------------------------
  Arduino library to control a wide variety of WS2811- and WS2812-based RGB
  LED devices such as Adafruit FLORA RGB Smart Pixels and NeoPixel strips.
  Currently handles 400 and 800 KHz bitstreams on 8, 12 and 16 MHz ATmega
  MCUs, with LEDs wired for various color orders.  Handles most output pins
  (possible exception with upper PORT registers on the Arduino Mega).

  Written by Phil Burgess / Paint Your Dragon for Adafruit Industries,
  contributions by PJRC, Michael Miller and other members of the open
  source community.

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

#include "tinyNeoPixel.h"

// Constructor when length, pin and type are known at compile-time:
tinyNeoPixel::tinyNeoPixel(uint16_t n, uint8_t p, neoPixelType t) :
  begun(false), brightness(0), pixels(NULL), endTime(0)
{
  updateType(t);
  updateLength(n);
  setPin(p);
}

// via Michael Vogt/neophob: empty constructor is used when strand length
// isn't known at compile-time; situations where program config might be
// read from internal flash memory or an SD card, or arrive via serial
// command.  If using this constructor, MUST follow up with updateType(),
// updateLength(), etc. to establish the strand type, length and pin number!
tinyNeoPixel::tinyNeoPixel() :
  begun(false), numLEDs(0), numBytes(0), pin(-1), brightness(0), pixels(NULL),
  rOffset(1), gOffset(0), bOffset(2), wOffset(1), endTime(0)
{
}

tinyNeoPixel::~tinyNeoPixel() {
  if(pixels)   free(pixels);
  if(pin >= 0) pinMode(pin, INPUT);
}

void tinyNeoPixel::begin(void) {
  if(pin >= 0) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
  begun = true;
}

void tinyNeoPixel::updateLength(uint16_t n) {
  if(pixels) free(pixels); // Free existing data (if any)

  // Allocate new data -- note: ALL PIXELS ARE CLEARED
  numBytes = n * ((wOffset == rOffset) ? 3 : 4);
  if((pixels = (uint8_t *)malloc(numBytes))) {
    memset(pixels, 0, numBytes);
    numLEDs = n;
  } else {
    numLEDs = numBytes = 0;
  }
}

void tinyNeoPixel::updateType(neoPixelType t) {
  boolean oldThreeBytesPerPixel = (wOffset == rOffset); // false if RGBW

  wOffset = (t >> 6) & 0b11; // See notes in header file
  rOffset = (t >> 4) & 0b11; // regarding R/G/B/W offsets
  gOffset = (t >> 2) & 0b11;
  bOffset =  t       & 0b11;
#ifdef NEO_KHZ400
  is800KHz = (t < 256);      // 400 KHz flag is 1<<8
#endif

  // If bytes-per-pixel has changed (and pixel data was previously
  // allocated), re-allocate to new size.  Will clear any data.
  if(pixels) {
    boolean newThreeBytesPerPixel = (wOffset == rOffset);
    if(newThreeBytesPerPixel != oldThreeBytesPerPixel) updateLength(numLEDs);
  }
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
    i   = numBytes; // Loop counter
  volatile uint8_t
   *ptr = pixels,   // Pointer to next byte
    b   = *ptr++,   // Current byte value
    hi,             // PORT w/output bit set high
    lo;             // PORT w/output bit set low

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

// 8 MHz(ish) AVRxt ---------------------------------------------------------
#if (F_CPU >= 7400000UL) && (F_CPU <= 9500000UL)

  volatile uint8_t n1, n2 = 0;  // First, next bits out

  // We need to be able to write to the port register in one clock
  // to meet timing constraints here.

  // 10 instruction clocks per bit: HHxxxxxLLL
  // OUT instructions:              ^ ^    ^   (T=0,2,7)

  hi   = *port |  pinMask;
  lo   = *port & ~pinMask;
  n1 = lo;
  if(b & 0x80) n1 = hi;

  // Dirty trick: RJMPs proceeding to the next instruction are used
  // to delay two clock cycles in one instruction word (rather than
  // using two NOPs).  This was necessary in order to squeeze the
  // loop down to exactly 64 words -- the maximum possible for a
  // relative branch.

  asm volatile(
   "headD:"                   "\n\t" // Clk  Pseudocode
    // Bit 7:
    "st   %a[port], %[hi]"    "\n\t" // 1    PORT = hi
    "mov  %[n2]   , %[lo]"    "\n\t" // 1    n2   = lo
    "st   %a[port], %[n1]"    "\n\t" // 1    PORT = n1
    "rjmp .+0"                "\n\t" // 2    nop nop
    "sbrc %[byte] , 6"        "\n\t" // 1-2  if(b & 0x40)
     "mov %[n2]   , %[hi]"    "\n\t" // 0-1   n2 = hi
    "st   %a[port], %[lo]"    "\n\t" // 1    PORT = lo
    "rjmp .+0"                "\n\t" // 2    nop nop
    // Bit 6:
    "st   %a[port], %[hi]"    "\n\t" // 1    PORT = hi
    "mov  %[n1]   , %[lo]"    "\n\t" // 1    n1   = lo
    "st   %a[port], %[n2]"    "\n\t" // 1    PORT = n2
    "rjmp .+0"                "\n\t" // 2    nop nop
    "sbrc %[byte] , 5"        "\n\t" // 1-2  if(b & 0x20)
     "mov %[n1]   , %[hi]"    "\n\t" // 0-1   n1 = hi
    "st   %a[port], %[lo]"    "\n\t" // 1    PORT = lo
    "rjmp .+0"                "\n\t" // 2    nop nop
    // Bit 5:
    "st   %a[port], %[hi]"    "\n\t" // 1    PORT = hi
    "mov  %[n2]   , %[lo]"    "\n\t" // 1    n2   = lo
    "st   %a[port], %[n1]"    "\n\t" // 1    PORT = n1
    "rjmp .+0"                "\n\t" // 2    nop nop
    "sbrc %[byte] , 4"        "\n\t" // 1-2  if(b & 0x10)
     "mov %[n2]   , %[hi]"    "\n\t" // 0-1   n2 = hi
    "st   %a[port], %[lo]"    "\n\t" // 1    PORT = lo
    "rjmp .+0"                "\n\t" // 2    nop nop
    // Bit 4:
    "st   %a[port], %[hi]"    "\n\t" // 1    PORT = hi
    "mov  %[n1]   , %[lo]"    "\n\t" // 1    n1   = lo
    "st   %a[port], %[n2]"    "\n\t" // 1    PORT = n2
    "rjmp .+0"                "\n\t" // 2    nop nop
    "sbrc %[byte] , 3"        "\n\t" // 1-2  if(b & 0x08)
     "mov %[n1]   , %[hi]"    "\n\t" // 0-1   n1 = hi
    "st   %a[port], %[lo]"    "\n\t" // 1    PORT = lo
    "rjmp .+0"                "\n\t" // 2    nop nop
    // Bit 3:
    "st   %a[port], %[hi]"    "\n\t" // 1    PORT = hi
    "mov  %[n2]   , %[lo]"    "\n\t" // 1    n2   = lo
    "st   %a[port], %[n1]"    "\n\t" // 1    PORT = n1
    "rjmp .+0"                "\n\t" // 2    nop nop
    "sbrc %[byte] , 2"        "\n\t" // 1-2  if(b & 0x04)
     "mov %[n2]   , %[hi]"    "\n\t" // 0-1   n2 = hi
    "st   %a[port], %[lo]"    "\n\t" // 1    PORT = lo
    "rjmp .+0"                "\n\t" // 2    nop nop
    // Bit 2:
    "st   %a[port], %[hi]"    "\n\t" // 1    PORT = hi
    "mov  %[n1]   , %[lo]"    "\n\t" // 1    n1   = lo
    "st   %a[port], %[n2]"    "\n\t" // 1    PORT = n2
    "rjmp .+0"                "\n\t" // 2    nop nop
    "sbrc %[byte] , 1"        "\n\t" // 1-2  if(b & 0x02)
     "mov %[n1]   , %[hi]"    "\n\t" // 0-1   n1 = hi
    "st   %a[port], %[lo]"    "\n\t" // 1    PORT = lo
    "rjmp .+0"                "\n\t" // 2    nop nop
    // Bit 1:
    "st   %a[port], %[hi]"    "\n\t" // 1    PORT = hi
    "mov  %[n2]   , %[lo]"    "\n\t" // 1    n2   = lo
    "st   %a[port], %[n1]"    "\n\t" // 1    PORT = n1
    "rjmp .+0"                "\n\t" // 2    nop nop
    "sbrc %[byte] , 0"        "\n\t" // 1-2  if(b & 0x01)
     "mov %[n2]   , %[hi]"    "\n\t" // 0-1   n2 = hi
    "st   %a[port], %[lo]"    "\n\t" // 1    PORT = lo
    "sbiw %[count], 1"        "\n\t" // 2    i-- (don't act on Z flag yet)
    // Bit 0:
    "st   %a[port], %[hi]"    "\n\t" // 1    PORT = hi
    "mov  %[n1]   , %[lo]"    "\n\t" // 1    n1   = lo
    "st   %a[port], %[n2]"    "\n\t" // 1    PORT = n2
    "ld   %[byte] , %a[ptr]+" "\n\t" // 2    b = *ptr++
    "sbrc %[byte] , 7"        "\n\t" // 1-2  if(b & 0x80)
     "mov %[n1]   , %[hi]"    "\n\t" // 0-1   n1 = hi
    "st   %a[port], %[lo]"    "\n\t" // 1    PORT = lo
    "brne headD"              "\n"   // 2    while(i) (Z flag set above)
  : [port]  "+e" (port),
    [byte]  "+r" (b),
    [n1]    "+r" (n1),
    [n2]    "+r" (n2),
    [count] "+w" (i)
  : [ptr]    "e" (ptr),
    [hi]     "r" (hi),
    [lo]     "r" (lo));

#elif (F_CPU >= 9500000UL) && (F_CPU <= 11100000UL)
/*
  volatile uint8_t n1, n2 = 0;  // First, next bits out

  // Squeezing an 800 KHz stream out of an 8 MHz chip requires code
  // specific to each PORT register.  At present this is only written
  // to work with pins on PORTD or PORTB, the most likely use case --
  // this covers all the pins on the Adafruit Flora and the bulk of
  // digital pins on the Arduino Pro 8 MHz (keep in mind, this code
  // doesn't even get compiled for 16 MHz boards like the Uno, Mega,
  // Leonardo, etc., so don't bother extending this out of hand).
  // Additional PORTs could be added if you really need them, just
  // duplicate the else and loop and change the PORT.  Each add'l
  // PORT will require about 150(ish) bytes of program space.

  // 13 instruction clocks per bit: HHHxxxxxLLLLL
  // OUT instructions:              ^  ^    ^   (T=0,3,7)
  hi = VPORTA.OUT |  pinMask;
  lo = VPORTA.OUT & ~pinMask;
  n1 = lo;
  if(b & 0x80) n1 = hi;

  // Dirty trick: RJMPs proceeding to the next instruction are used
  // to delay two clock cycles in one instruction word (rather than
  // using two NOPs).  This was necessary in order to squeeze the
  // loop down to exactly 64 words -- the maximum possible for a
  // relative branch.
  */

    // 14 instruction clocks per bit: HHHHxxxxLLLLL
    // OUT instructions:              ^   ^   ^   (T=0,4,7)
      volatile uint8_t next;

      hi   = *port |  pinMask;
      lo   = *port & ~pinMask;
      next = lo;
      if(b & 0x80) next = hi;

      // Don't "optimize" the OUT calls into the bitTime subroutine;
      // we're exploiting the RCALL and RET as 3- and 4-cycle NOPs!
      asm volatile(
       "headD:"                   "\n\t" //        (T =  0)
        "st   %a[port], %[hi]"    "\n\t" //        (T =  1)
        "rcall bitTimeD"          "\n\t" // Bit 7  (T = 14)
        "st   %a[port], %[hi]"    "\n\t"
        "rcall bitTimeD"          "\n\t" // Bit 6
        "st   %a[port], %[hi]"    "\n\t"
        "rcall bitTimeD"          "\n\t" // Bit 5
        "st   %a[port], %[hi]"    "\n\t"
        "rcall bitTimeD"          "\n\t" // Bit 4
        "st   %a[port], %[hi]"    "\n\t"
        "rcall bitTimeD"          "\n\t" // Bit 3
        "st   %a[port], %[hi]"    "\n\t"
        "rcall bitTimeD"          "\n\t" // Bit 2
        "st   %a[port], %[hi]"    "\n\t"
        "rcall bitTimeD"          "\n\t" // Bit 1
        // Bit 0:
        "st   %a[port], %[hi]"    "\n\t" // 1    PORT = hi    (T =  1)
        "rjmp .+0"                "\n\t" // 2    nop nop      (T =  3)
        "ld   %[byte] , %a[ptr]+" "\n\t" // 2    b = *ptr++   (T =  5)
        "st   %a[port], %[next]"  "\n\t" // 1    PORT = next  (T =  6)
        "mov  %[next] , %[lo]"    "\n\t" // 1    next = lo    (T =  7)
        "sbrc %[byte] , 7"        "\n\t" // 1-2  if(b & 0x80) (T =  8)
         "mov %[next] , %[hi]"    "\n\t" // 0-1    next = hi  (T =  9)
        "st   %a[port], %[lo]"    "\n\t" // 1    PORT = lo    (T = 10)
        "sbiw %[count], 1"        "\n\t" // 2    i--          (T = 12)
        "brne headD"              "\n\t" // 2    if(i != 0) -> (next byte)
         "rjmp doneD"             "\n\t"
        "bitTimeD:"               "\n\t" //      nop nop nop     (T =  4)
        "st   %a[port], %[next]"  "\n\t" // 1    PORT = next     (T =  5)
         "mov  %[next], %[lo]"    "\n\t" // 1    next = lo       (T =  6)
         "rol  %[byte]"           "\n\t" // 1    b <<= 1         (T =  7)
         "sbrc %[byte], 7"        "\n\t" // 1-2  if(b & 0x80)    (T =  8)
          "mov %[next], %[hi]"    "\n\t" // 0-1   next = hi      (T =  9)
         "st   %a[port], %[lo]"    "\n\t" // 1    PORT = lo       (T = 10)
         "ret"                    "\n\t" // 4    nop nop nop nop (T = 14)
         "doneD:"                 "\n"
        : [port]  "+e" (port),
          [byte]  "+r" (b),
          [next]  "+r" (next),
          [count] "+w" (i)
        : [ptr]    "e" (ptr),
          [hi]     "r" (hi),
          [lo]     "r" (lo));



// 12 MHz(ish) AVRxt --------------------------------------------------------
#elif (F_CPU >= 11100000UL) && (F_CPU <= 14300000UL)

    // In the 12 MHz case, an optimized 800 KHz datastream (no dead time
    // between bytes) requires a PORT-specific loop similar to the 8 MHz
    // code (but a little more relaxed in this case).

    // 15 instruction clocks per bit: HHHHxxxxxxLLLLL
    // OUT instructions:              ^   ^     ^     (T=0,4,10)

    volatile uint8_t next;

      hi   = *port |  pinMask;
      lo   = *port & ~pinMask;
      next = lo;
      if(b & 0x80) next = hi;

      // Don't "optimize" the OUT calls into the bitTime subroutine;
      // we're exploiting the RCALL and RET as 3- and 4-cycle NOPs!
      asm volatile(
       "headD:"                   "\n\t" //        (T =  0)
        "st   %a[port], %[hi]"    "\n\t" //        (T =  1)
        "rcall bitTimeD"          "\n\t" // Bit 7  (T = 15)
        "st   %a[port], %[hi]"    "\n\t"
        "rcall bitTimeD"          "\n\t" // Bit 6
        "st   %a[port], %[hi]"    "\n\t"
        "rcall bitTimeD"          "\n\t" // Bit 5
        "st   %a[port], %[hi]"    "\n\t"
        "rcall bitTimeD"          "\n\t" // Bit 4
        "st   %a[port], %[hi]"    "\n\t"
        "rcall bitTimeD"          "\n\t" // Bit 3
        "st   %a[port], %[hi]"    "\n\t"
        "rcall bitTimeD"          "\n\t" // Bit 2
        "st   %a[port], %[hi]"    "\n\t"
        "rcall bitTimeD"          "\n\t" // Bit 1
        // Bit 0:
        "st   %a[port], %[hi]"    "\n\t" // 1    PORT = hi    (T =  1)
        "rjmp .+0"                "\n\t" // 2    nop nop      (T =  3)
        "ld   %[byte] , %a[ptr]+" "\n\t" // 2    b = *ptr++   (T =  5)
        "st   %a[port], %[next]"  "\n\t" // 1    PORT = next  (T =  6)
        "mov  %[next] , %[lo]"    "\n\t" // 1    next = lo    (T =  7)
        "sbrc %[byte] , 7"        "\n\t" // 1-2  if(b & 0x80) (T =  8)
         "mov %[next] , %[hi]"    "\n\t" // 0-1    next = hi  (T =  9)
        "nop"                     "\n\t" // 1                 (T = 10)
        "st   %a[port], %[lo]"    "\n\t" // 1    PORT = lo    (T = 11)
        "sbiw %[count], 1"        "\n\t" // 2    i--          (T = 13)
        "brne headD"              "\n\t" // 2    if(i != 0) -> (next byte)
         "rjmp doneD"             "\n\t"
        "bitTimeD:"               "\n\t" //      nop nop nop     (T =  4)
         "st   %a[port], %[next]" "\n\t" // 1    PORT = next     (T =  5)
         "mov  %[next], %[lo]"    "\n\t" // 1    next = lo       (T =  6)
         "rol  %[byte]"           "\n\t" // 1    b <<= 1         (T =  7)
         "sbrc %[byte], 7"        "\n\t" // 1-2  if(b & 0x80)    (T =  8)
          "mov %[next], %[hi]"    "\n\t" // 0-1   next = hi      (T =  9)
         "nop"                    "\n\t" // 1                    (T = 10)
         "st   %a[port], %[lo]"   "\n\t" // 1    PORT = lo       (T = 11)
         "ret"                    "\n\t" // 4    nop nop nop nop (T = 15)
         "doneD:"                 "\n"
        : [port]  "+e" (port),
          [byte]  "+r" (b),
          [next]  "+r" (next),
          [count] "+w" (i)
        : [ptr]    "e" (ptr),
          [hi]     "r" (hi),
          [lo]     "r" (lo));


// 16 MHz(ish) AVRxt ------------------------------------------------------
#elif (F_CPU >= 15400000UL) && (F_CPU <= 19000000L)

  // WS2811 and WS2812 have different hi/lo duty cycles; this is
  // similar but NOT an exact copy of the prior 400-on-8 code.

  // 20 inst. clocks per bit: HHHHHxxxxxxxxLLLLLLL
  // ST instructions:         ^    ^       ^       (T=0,5,13)

  volatile uint8_t next, bit;

  hi   = *port |  pinMask;
  lo   = *port & ~pinMask;
  next = lo;
  bit  = 8;

  asm volatile(
   "head20:"                   "\n\t" // Clk  Pseudocode    (T =  0)
    "st   %a[port],  %[hi]"    "\n\t" // 1    PORT = hi     (T =  1)
    "nop"                      "\n\t" // 1    nop           (T =  2)
    "sbrc %[byte],  7"         "\n\t" // 1-2  if(b & 128)
     "mov  %[next], %[hi]"     "\n\t" // 0-1   next = hi    (T =  4)
    "dec  %[bit]"              "\n\t" // 1    bit--         (T =  5)
    "st   %a[port],  %[next]"  "\n\t" // 1    PORT = next   (T =  6)
    "nop"                      "\n\t" // 1    nop           (T =  7)
    "mov  %[next] ,  %[lo]"    "\n\t" // 1    next = lo     (T =  8)
    "breq nextbyte20"          "\n\t" // 1-2  if(bit == 0) (from dec above)
    "rol  %[byte]"             "\n\t" // 1    b <<= 1       (T = 10)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 12)
    "nop"                      "\n\t" // 1    nop           (T = 13)
    "st   %a[port],  %[lo]"    "\n\t" // 1    PORT = lo     (T = 14)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 16)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 18)
    "rjmp head20"              "\n\t" // 2    -> head20 (next bit out) (T=20)
   "nextbyte20:"               "\n\t" //                    (T = 10)
    "ldi  %[bit]  ,  8"        "\n\t" // 1    bit = 8       (T = 11)
    "ld   %[byte] ,  %a[ptr]+" "\n\t" // 2    b = *ptr++    (T = 13)
    "st   %a[port], %[lo]"     "\n\t" // 1    PORT = lo     (T = 14)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 16)
    "sbiw %[count], 1"         "\n\t" // 2    i--           (T = 18)
     "brne head20"             "\n"   // 2    if(i != 0) -> (next byte) (T=20)
    : [port]  "+e" (port),
      [byte]  "+r" (b),
      [bit]   "+r" (bit),
      [next]  "+r" (next),
      [count] "+w" (i)
    : [ptr]    "e" (ptr),
      [hi]     "r" (hi),
      [lo]     "r" (lo));

// 20 MHz(ish) AVRxt ------------------------------------------------------
#elif (F_CPU >= 19000000UL) && (F_CPU <= 22000000L)


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

  hi   = *port |  pinMask;
  lo   = *port & ~pinMask;
  next = lo;
  bit  = 8;


  asm volatile(
   "head24:"                   "\n\t" // Clk  Pseudocode    (T =  0)
    "st   %a[port],  %[hi]"    "\n\t" // 1    PORT = hi     (T =  1)
    "sbrc %[byte],  7"         "\n\t" // 1-2  if(b & 128)
    "mov  %[next], %[hi]"      "\n\t" // 0-1   next = hi    (T =  3)
    "dec  %[bit]"              "\n\t" // 1    bit--         (T =  4)
    "nop"                      "\n\t" // 1    nop           (T =  5)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T =  7)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T =  9)
    "st   %a[port],  %[next]"  "\n\t" // 1    PORT = next   (T = 10)
    "mov  %[next] ,  %[lo]"    "\n\t" // 1    next = lo     (T = 11)
    "nop"                      "\n\t" // 1    nop           (T = 12)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 14)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 16)
    "breq nextbyte24"          "\n\t" // 1-2  if(bit == 0) (from dec above)
    "rol  %[byte]"             "\n\t" // 1    b <<= 1       (T = 18)
    "st   %a[port],  %[lo]"    "\n\t" // 1    PORT = lo     (T = 19)
    "rcall seconddelay24"      "\n\t" // 2+4+3=9            (T = 28)
    "rjmp head24"              "\n\t" // 2    -> head20 (next bit out)
   "seconddelay24:"            "\n\t" //
    "nop"                      "\n\t" // 1
    "rjmp .+0"                 "\n\t" // 2
    "ret"                      "\n\t" // 4
   "nextbyte24:"               "\n\t" // last bit of a byte (T = 18)
    "st   %a[port], %[lo]"     "\n\t" // 1    PORT = lo     (T = 19)
    "ldi  %[bit]  ,  8"        "\n\t" // 1    bit = 8       (T = 20)
    "ld   %[byte] ,  %a[ptr]+" "\n\t" // 2    b = *ptr++    (T = 22)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 24)
    "rjmp .+0"                 "\n\t" // 2    nop nop       (T = 26)
    "sbiw %[count], 1"         "\n\t" // 2    i--           (T = 28)
    "brne head24"              "\n"   // 2    if(i != 0) -> (next byte)  ()
    : [port]  "+e" (port),
      [byte]  "+r" (b),
      [bit]   "+r" (bit),
      [next]  "+r" (next),
      [count] "+w" (i)
    : [ptr1]   "e" (ptr1),
      [ptr2]   "e" (ptr2),
      [hi]     "r" (hi),
      [lo]     "r" (lo));




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
  if(begun && (pin >= 0)) pinMode(pin, INPUT);
    pin = p;
    if(begun) {
      pinMode(p, OUTPUT);
      digitalWrite(p, LOW);
    }
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
