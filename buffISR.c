

ISR_NAKED(INSERT_NAME_vect) {
  __asm volatile ("
    push r26
    push r27
    push r0
    in r26,30
    in r27,31
    ld X+ r0
    sts SPIn.DATA,r0
    out 31,r27
    out 30,r26
    pop r0
    pop r27
    pop r26
    reti
    ");
}

1+1+1 = 3 clocks pushing
2 in'ing, 2 ld
2 sts
2 outing, 6 popping.
4 reti.

3 + 4 +2 + 2 + 6 = 21 clocks

3 for interrupt overhead... this is 24 clocks total?
NO pushing SREG!

At 24 MHz system clock, this is 1 uS.

At 1.25 us/bit and 8 bits per byte, means only spending 1/10th of it's time in the ISR. Not bad!

Configuration:

TCD0:

CMPBCLR = 1.25 * (clocks/us)
CMPACLR = CMPBCLR
CMPASET = 0.25 * (clocks/us)
CMPBSET = 0.75 * (clocks/us)

Logic 1:
In0: TCD0WOB
In1: TCD0WOA
In2: MISO

Truth:
000 0
001 1
010 0
011 1

100 0
101 0
110 1
111 1

Out: -----> Neopixels.

SPI:
Slave, buffered. SSD = 0, SS = input

On DRE, push r0 and one of the pointer registers onto stack.
in two GPRs to load the current byte index
ld X+ to r0 - load that byte to r0
sts SPIn.DATA, r0 - write it to the SPI Data register.
out the now incremented index bytes
pop the registers we pushed out of the way and return.

Looks like 24-25 clocks, or ~1 uS for this hyper-efficient ISR.

None of these change SREG, and only 1 register other than the pointer. It doesn't get much smaller than this...

WOA (long/one squarewave) connected to SCK

TCB clocked on event also driven by WOB.

When TCB hits timeout of 8*pixels, it turns this mess off and raises SS.
