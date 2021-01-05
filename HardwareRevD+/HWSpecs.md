# Hardware Revision D
Rev. D is a MAJOR revision. It is expected to be incompatible with software for Rev. C hardware and earlier

## Basic Specs
* AVR128DA32
* Internal Oscillator
* 1602 LCD controlled in 4-bit mode
* LCD/XOSC32K
  * Option 1: 32K xtal for autotune, single-color LCD backlight
  * Option 2: no autotune, RGB LCD backlight.
* Single Channel Output
* DAC used for contrast control

## Pin connections: 

### PORTA
* PA0 - 
* PA1 - 
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
