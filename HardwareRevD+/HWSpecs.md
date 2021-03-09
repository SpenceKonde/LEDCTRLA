# Hardware Revision D
Rev. D is a MAJOR revision. It is expected to be incompatible with software for Rev. C hardware and earlier

## Basic Specs
* AVR128DA32
* Internal Oscillator
* 1602 LCD controlled in 4-bit mode
* LCD/XOSC32K
  * Option 1: 32K xtal for autotune, single-color LCD backlight
  * (selected) Option 2: no autotune, RGB LCD backlight.

* Single Channel Output
* DAC used for contrast control

## Pin connections: 

### PORTA
* PA0 - ???
* PA1 - ???
* PA2 - Button1
* PA3 - ModeButton
* PA4 - TX
* PA5 - RX
* PA6 - Button2
* PA7 - LED
### PORTC
* PA0 - RotaryEnc1A
* PA1 - RotaryEnc1A
* PA2 - RotaryEnc2B
* PA3 - RotaryEnc2B

### PORTD
* PD0 - RS
* PD1 - RW
* PD2 - Data4
* PD3 - Data5
* PD4 - Data6
* PD5 - Data7
* PD6 - VEE/Contrast (DAC)
* PD7 - Enable

### PORTF
* PF0 - Backlight Blue FET
* PF1 - Backlight Green FET
* PF2 - Backlight Red FET
* PF3 - RF Data
* PF4 - LEDs
* PF5 - Analog Input


# Rev. E plans
* Switch to AVR128DB48 - this will give us an ample sufficiency pf pins and peripherals to use for even advanced techniques which are currently only theoretical, like the SPI + CCL concept to capture, play back and more, HF crystal, etc
* multiple bits of digital sense input. 
* Multiple analog in channels, on potential opmp input pins. which could be used to for amplification or other tasks. Worth considering at least...

DA32 has 26 pins that we can use. 48 addds 16 raw pins, 2 of which are not I/O - while 64 does the same. Added pins in these cases are:

32 adds: PF2-5

48 adds: PB 0 - 5. PC 4,5,6,7, PE 0-3

64 adds: PB 6, 7. PE 4,5,6,7, PG0-7 (all of them)
