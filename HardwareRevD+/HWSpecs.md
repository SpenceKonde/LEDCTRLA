include <stdint.h>

/*
# Hardware Revision D
~Rev. D is a MAJOR revision. It is expected to be incompatible with software for Rev. C hardware and earlier~
Rev. D suffered from a fatal design defect. ~Rev. E is the next generation, and have major improvements over the D as well.~ 
Rev. E had the improved processor, but it was found to be unsuitable because it lacked protection against spikes on the power supply and data lines causing them to rapidly fail when deployed without a bulky external protection circuit. When one unit was burned out due to an intermittant connection, it was determined that a revised design with on-board protection was required. For Rev. F, which is belived to be usable, we addressed 

## Basic Specs
* ~AVR128DA32~ AVR128DB48 E/PT
* ~Internal Oscillator~ External 40 MHz crystal will be installed, and software is designed to function at 16, 20, 24, 32, or 40 MHz. It is expected that most operation will occur at 40 MHz to give adequate processing budget. 
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
| Pin | Function Rev F | Function Rev G |
|-----|----------------|----------------|
| PA0 | Crystal        | Crystal        |
| PA1 | Crystal        | Crystal        |
| PA2 | Unused. SDA    | Unused SDA     |
| PA3 | Unused, SCL    | Unused SCL     |
| PA4 | TX0 console    | TX0 console    |
| PA5 | RX0 console    | RX0 console    |
| PA6 | Unused (PWM)   |                |
| PA7 | LED (PWM)      | LED0           |

### PORTB
| Pin | Function Rev F | Function Rev G |
|-----|----------------|----------------|
| PB0 | Backlight RED  | Backlight RED  |
| PB1 | Backlight GRN  | Backlight GRN  |
| PB2 | Backlight BLU  | Backlight BLU  |
| PB3 | Button - Color | Button - Color |
| PB4 | Button - Color | Button - Color |
| PB5 | Button - Color | Button - Color |

* PB0, PB1, PB2 = PWM control for red, greemn and blue backlight leds. TCA1 will be used to provide 16-bit buffered PW<, as only three channels are needed.
* PB3, PB4, PB5 - sense for the three standard buttomns, Color, Parameter, and Mode.

### PORTC
First half is used for the encoders. 
Second half is either for extensions, or on Rev. G, for the self-connections required for the CCL/EVSYS driven SPI ByteOut functionality, which will increase the cycle budget per frame for generation of pixel data by more than a factor of 3. 

| Pin | Function Rev F | Function Rev G |
|-----|----------------|----------------|
| PC0 | Color Enc. A   | Color Enc. A   |
| PC1 | Color Enc. B   | Color Enc. B   |
| PC2 | Param Enc. A   | Param Enc. A   |
| PC3 | Param Enc. B   | Param Enc. B   |
| PC4 | Extend Header  | Extend Header  |
| PC5 | Extend Header  | Extend Header  |
| PC6 | Extend Header  | Extend Header  |
| PC7 | Extend Header  | Extend Header  |



### PORTD
PORTD is dedicated to interfacing with the LCD. We will likely adapt the library to improve the currently dismal performance 
| Pin | Function Rev F | Function Rev G |
|-----|----------------|----------------|
| PD0 | Data4          | Data4          |
| PD1 | Data4          | Data4          |
| PD2 | Data4          | Data4          |
| PD3 | Data4          | Data4          |
| PD4 | RS             | RS             |
| PD5 | RW             | RW             |
| PD6 | Contrast (DAC) | Contrast (DAC) |
| PD7 | EN             | EN             |

The contrast pin appears to have no effect on the screens we are currently using. 

### PORTE

| Pin | Function Rev F | Function Rev G |
|-----|----------------|----------------|
| PE0 | Extend Header  | Extend Header  |
| PE1 | Extend Header  | Extend Header  |
| PE2 | Extend Header  | Extend Header  |
| PE3 | Extend Header  | Extend Header  |

### PORTF
| Pin | Function Rev F | Function Rev G |
|-----|----------------|----------------|
| PF0 | AUX            | AUX            |
| PF1 | LED1           | LED1           |
| PF2 | LED2           | LED2           |
| PF3 | RF In          | RF In          |
| PF4 | TX2 Feedback   | TX2 Feedback   |
| PF5 | RX2 on header  | RX2 on header  |
| PF6 | Reset          | Reset          |
