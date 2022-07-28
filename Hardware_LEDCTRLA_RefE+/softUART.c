/* This code (C) Spence Konde 2022. This code is not complete, and has not yet been released under any license, open source or otherwise.
 * All rights reserved
 * The question of whether the release under a license that allows reuse will be revisited when the code actually works.
 * it's purpose is to provide a bidirectional soft USART to allow chaining of USARTs of modern AVRs in one-wire mode like so:
 * (master Tx) <------> (Tx   GPIO) <------->  (Tx   GPIO) <------->  (Tx   GPIO) <------->
 */

#if ((F_CPU % SOFT_UART_BAUD) * 50 > SOFT_UART_BAUD)
  #error "The soft UART baud error exceeds the tolerance of the system"
#elif (F_CPU % SOFT_UART_BAUD)
  #warning "There is a clock error in the soft USART baud rate. The splitter is expected to run at a round F_CPU with BAUD = 500k,"
#endif

#define SOFT_UART_DELAY  (F_CPU / SOFT_UART_BAUD - ??? )
#define SOFT_UART_LOOP_CNT (SOFT_UART_DELAY / 3)
#if (SOFT_UART_DELAY % 3 == 1)
  #define ADD_NOP
#elif (SOFT_UART_DELAY % 3 == 2)
  #define ADD_2NOP
#endif;

const uint8_t SOFT_UART_DELAY = (F_CPU / SOFT_UART_BAUD)

void putch(int8_t ch) {
  uint8_t bit = 10;              // 1 start 8 data 1 stop
  __asm__ __volatile__(
      "sbi 0x00, 2"       "\n\t" // 1 clk, total 1 // set PA2 output - it's already set low. Begin start bit.
      /* now to match bit length below, we need 8 clocks */
      "rcall .-4"         "\n\t" // 2+4 clk, total 7 //
     "txdlystart:"        "\n\t"
      "mov r0, %2"        "\n\t" // 1 clk         // copy delay count to r0;
      "dec r0"            "\n\t" // 1 clk         // decrement delay clock
      "breq .4"           "\n\t" // 2 clk         // return to start of delay
      /* Loop total: 1 + (1+2)*loopcount - 1 = 3 * loopcount */
#if defined(SOFT_UART_ADD_NOP)
      "nop"               "\n\t" // 1 clk, total dly + 1 // 1 extra cycle
#elif defined(SOFT_UART_ADD_2NOP)
      "rjmp .+0"          "\n\t" // 1 clk, total dly + 2 // 2 extra cycles
#endif
      /* Thus the delay must be the bit length minus the below overhead */
      "sbrs %0, 0"        "\n\t" // 1 clk, total 0 // is low bit 0?
      "cbi 0x00, 2"       "\n\t" // 1 clk, total 1 // If so skip clearing of DIR register: line would not be pulled high
      "sbrc %0, 0"        "\n\t" // 1 clk, total 2 // is low bit 1?
      "sbi 0x00, 2"       "\n\t" // 1 clk, total 3 // If so skip setting of DIR register: Line would not be driven low. Net effect is to set pin direction to output if bit is 0, and input (allowing pullups to take it high if 1)
      "lsr %0"            "\n\t" // 1 clk, total 4 // Shift character being transmitted right by one place and shift a 0 in from the left.
      "dec %1"            "\n\t" // 1 clk, total 5 // decrement the bit counter. will be 9 after transmitting stop bit, 1 after transmitting data, giving us one stop bit.
      "brneq txdlystart"  "\n\t" // 2 clk, total 7 // if we aren't down to 0, jump delay again
      "rjmp .+2"          "\n\t" // skip the dummy ret
    "txdummyret:"
      "ret"               "\n\t" // dummmy ret used for a 2 word 6 clock delay.
     :"+r"((int8_t)ch)
      "+r"((uint8_t)bit)
     :"r" ((uint8_t)SOFT_UART_LOOP_CNT)

  );
}
void loop() {
  // do everything else
  if (incoming_expect) {
    receive_datapacket();
    incoming_expect = 0;
    process_datapacket();
    if (!digitalReadFast(PIN_PA2)) {
      /* houston, we have a problem. */
    } else {
      PORTA.PIN2CTRL = 0x0B; // 0b00001011 = int on falling edge, pullup enable;
    }
  }
}
uint8_t rxbuffer[16]
uint8_t* rxbptr;
void receive_datapacket() {
  uint8_t bytestorx = 1;
  uint8_t bytenum = 0;
  rxbptr = &rxbuffer;
  uint8_t cmd_or_len = getch(); // last stopbit middle + 8
  __asm__ __volatile__ (
      "mov r0, %0"      "\n\t" // 1 clk, first stopbit middle + 12 //
      "cpi %0, 0x30"    "\n\t" // 1 clk, first stopbit middle + 13 //
      "brcs .+8"        "\n\t" // 1 clk, first stopbit middle + 14 //
      "cpi %0, 0x40"    "\n\t" // 1 clk, first stopbit middle + 15 //
      "brcc .+2"        "\n\t" // 1 clk, first stopbit middle + 16 //
      "subi %0, 0x30"   "\n\t" // 1 clk, first stopbit middle + 17 //
      "mov %1, r0"      "\n\t" // 1 clk, first stopbit middle + 18 //
     :"+d" ((uint8_t) cmd_or_len),
      "=r" ((uint8_t) bytestorx)
     :"r" ((uint8_t)SOFT_UART_LOOP_CNT)
    );
  if (bytestorx) { // cp. breq = 2 clocks if not equal, first stopbit middle + 20
    getchs();      // call or rcall, 2 or 3 clocks, first stopbit middle +23
  }
}
uint8_t getch() {
  uint8_t ch = 0;
  __asm__ __volatile__(
    "sbis 0x02,2"     "\n\t" // 1 clk, total 0 //
    "rjmp .-4"        "\n\t" // 1 clk, total 0 //
    /* todo - 1.5bit delay delay -2 */
    "eor r0,r0"       "\n\t" // 1 clk, total 0 //
    "ldi %0, 8"       "\n\t" // 1 clk, total 0 //
     "readbit:"       "\n\t" // 1 clk, total 0 //
      "sbic"          "\n\t" // 1 clk, total 0 //
      "sec"           "\n\t" // 1 clk, total 0 //
      "ror r0"        "\n\t" // 1 clk, total 0 //
      /* todo 1 bit delay -3 */
      "dec %0"        "\n\t" // 1 clk, total 0 //
    "brneq readbit"   "\n\t" // 1 clk, total 0 //
    "mov %1, r0"      "\n\t" // 1 clk, total 0 //
    "eor r1, r1"      "\n\t" // clear r1 which we used as a loop counter for delay
   :"+d"((uint8_t) bitsleft)
    "+r"((uint16_t) ch)
   :"r" ((uint8_t)SOFT_UART_LOOP_CNT)
  );
  return ch;                 // 4 clk - last stopbit middle + 8
}
void getchs() {              // first stopbit middle +23
  uint8_t bitsleft;
  __asm__ __volatile__(      // implicit: ldix4 first stopbit middle +27
   "startbyte:"
    "dec r1"          "\n\t" // 1 clk, total 0 // timeout counter - r1 starts at 0
    "breq timeout"    "\n\t" // 1 clk, total 0 // handle case of expected byte not arriving
    "sbis 0x02,2"     "\n\t" // 1 clk, total 0 // Start looking for expected start bit
    "rjmp .-8"        "\n\t" // 1 clk, total 0 //
    "eor r1, r1"      "\n\t" // clear r1 which we used as a loop counter fortimeout
    /* todo - 1.5bit delay delay -3 */
    "eor r0,r0"       "\n\t" // 1 clk, total 0 // clear currenc char register
    "ldi %0, 8"       "\n\t" // 1 clk, total 0 // load bit counter
     "readbit:"       "\n\t"
      "sbic 0x02, 2"  "\n\t" // 1 clk, total 0 // read the bit, if 0, don't set carry
      "sec"           "\n\t" // 1 clk, total 0 // if bit 1 set carry
      "ror r0"        "\n\t" // 1 clk, total 0 // shift r0 right 1 place, shifting a 0 into carry bit.
      "eor r1, r1"    "\n\t" // 1 clk, total 0 // clear r1 used for loopcount.
      /* todo 1 bit delay -4 */
      "dec %0"        "\n\t" // 1 clk, total 0 // decrement bitcount
    "brneq readbit"   "\n\t" // 1 clk, total 0 // if not last bit, we're now at middle of next bit. jump to reading it.
    "st %a1+, r0"     "\n\t" // 1 clk, total 0 // we now have 1.5 bit times to store the byte to the buffer with postincrement
    "dec %2"          "\n\t" // 1 clk, total 0 // decrement bytecount
    "eor r1, r1"      "\n\t" // clear r1 which we used as a loop counter for delay
    "brneq startbyte" "\n\t" // 2 clk, total 0 // unless last byte, jump back to receive next byte
   "timeout:"
    "eor r1, r1"      "\n\t" // clear r1 which we used as a loop counter for delay and timeout
   :"+d"((uint8_t) bitsleft)
    "+e"((uint16_t) rxbptr)
    "+d"((uint8_t) bytestorx)
   :"r" ((uint8_t) SOFT_UART_LOOP_CNT)
  );
}

// downstream transmitting upstream must start with a dummy 0x00 character, which should only be issued between LED data transfers
ISR(PORTA_PORT_vect) {
  // we have 11 bit times to do this in only.
  /* TODO: Stop incomming led data processing */
  PORTA.PIN2CTRL = 0x08; // pullup on but no interruopt enable
  VPORTA.INTFLAGS |= 1; //clear the intflag
}
