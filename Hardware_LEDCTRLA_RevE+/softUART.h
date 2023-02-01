#if ((F_CPU % SOFT_UART_BAUD) * 50 > SOFT_UART_BAUD)
  #error "The soft UART baud error exceeds the tolerance of the system"
#elif (F_CPU % SOFT_UART_BAUD)
  #warning "There is a clock error in the soft USART baud rate. The splitter is expected to run at a round F_CPU with BAUD = 500k,"
#endif

#define SOFT_UART_DELAY  (F_CPU / SOFT_UART_BAUD - ??? )
#define SOFT_UART_LOOP_CNT (SOFT_UART_DELAY/3)
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
