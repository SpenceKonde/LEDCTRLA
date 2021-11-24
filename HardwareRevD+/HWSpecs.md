# Hardware Revision D
~Rev. D is a MAJOR revision. It is expected to be incompatible with software for Rev. C hardware and earlier~
Rev. D suffered from a fatal design defect. Rev. E is the next generation, and have major improvements over the D as well.

## Basic Specs
* ~AVR128DA32~ AVR128DB48 E/PT
* ~Internal Oscillator~ External crystal oscillator
* will be installed., tentatively planned for 40 MHz, unless 48 is found to be reliable on the E-spec parts under typical codnitions; the more processing power we nave, the bettter eqiopped we will be to meet peak perfor,ance requiirements. When performance is less critical, or on chip temperature sensor reaches exceeds thresholds, speed could be backed off to 32 MHz internal or evem 24.
* 1602 LCD controlled in 4-bit mode with RGB backlight
  * requiring all of PORTD for the non-backlight functionality
* RF remote control using either RXB12 or RXB14 with loosly coiled wire wnip (low cost solution) or a STXii7 with SMT ceramic antenna for more robust reception requirements.
  * All devices will have a unique ID stored in USERROW that allows it to reocognize RF packets forit. No mechanism of response is planned.
* Contrast voltage will be controlled using the DAC, replacing phtysical pot
  * To our surprise, it has been found that the contrast seems to be correct out of the box, and that the voltage on this pin has no impact on the contrast, at least with the LCDs used so far. Okay, works for me. If I end up going back to the old style ones, I'll still need it.
*
* Single Channel Output

## Pin connections:

### PORTA
* PA0, PA1 - Crystal or osc.
* PA2 - Unused. SDA
* PA3 - Unused, SCL
* PA4 - TX
* PA5 - RX
* PA6 - Unused (PWM)
* PA7 - LED (PWM)

### PORTB
* PB0, PB1, PB2 = PWM control for red, greemn and blue backlight leds. TCA1 will be used to provide 16-bit buffered PW<, as only three channels are needed.
* PB3, PB4, PB5 - sense for the three standard buttomns, Color, Parameter, and Mode.

### PORTC
* PC0 - Color Emcoder A
* PC1 - Color Emcoder B
* PC2 - Param Emcoder A
* PC3 - Param Emcoder B
 * Placed in one group like this to make the encoder reading trick maximally efficient.
* PC4-7 - to Extend header. Recall that this is a DB, so MVIO could be used here...

### PORTD
* PD0 - Data4
* PD1 - Data5
* PD2 - Data6
* PD3 - Data7
* PD4 - RS
* PD5 - RW
* PD6 - VEE/Contrast (DAC)
* PD7 - Enable

### PORTE
PE0 - PE3 Extend header

### PORTF
* PF0 - optionally xtal or unused
* PF1 - optionally xtal or unused
* PF2 - LEDs
* PF3 - RF Data
* PF4 - unused, has analog - TX2
* PF5 - unused, has analog - RX2
* PF6 - Reset

Serial2 used to communicate with head end adapter, potentially.
