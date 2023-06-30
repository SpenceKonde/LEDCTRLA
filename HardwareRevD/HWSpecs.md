
# Hardware Revision ~D~ ~E~ ~F~ G
Rev. E had the improved processor, but it was found to be unsuitable because it lacked protection against spikes on the power supply and data lines causing them to rapidly fail when deployed without a bulky external protection circuit. When one unit was burned out due to an intermittant connection, it was determined that a revised design with on-board protection was required. For Rev. F, which is belived to be usable, we addressed

## Basic Specs
* ~AVR128DA32~ AVR128DB48 E/PT. Extended temperature range to allow us more breathing room if we are CPU bound. Improves the chance that it will run at 40 MHz at room temperature
* ~Internal Oscillator~ External 40 MHz crystal will be installed, and software is designed to function at 16, 20, 24, 32, or 40 MHz. It is expected that most operation will occur at 40 MHz to give adequate processing budget.
* 1602 LCD controlled in 4-bit mode with RGB backlight
  * requiring all of PORTD for the non-backlight functionality
* RF remote control using either RXB12 or RXB14 with loosly coiled wire wnip (low cost solution) or a STX887 with SMT ceramic antenna for more robust reception requirements.
  * All devices will have a unique ID stored in USERROW that allows it to reocognize RF packets forit. No mechanism of response is planned.
* Contrast voltage will be controlled using the DAC, replacing phtysical pot
  * To our surprise, it has been found that the contrast seems to be correct out of the box, and that the voltage on this pin has no impact on the contrast, at least with the LCDs used so far. Okay, works for me. If I end up going back to the old style ones, I'll still need it.
* Rev. F and G have rigorous protection circuits. that's why most of the Rev. E and earlier controllers burned out.
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
| PA6 | LED Data Out   | LED Data Out   |
| PA7 | LED (PWM)      | LED0           |

### PORTB
| Pin | Function Rev F | Function Rev G |
|-----|----------------|----------------|
| PB0 | Backlight RED  | Backlight RED  |
| PB1 | Backlight GRN  | Backlight GRN  |
| PB2 | Backlight BLU  | Backlight BLU  |
| PB3 | Button - Color | Button - Color |
| PB4 | Button - Param | Button - Param |
| PB5 | Button - Mode  | Button -  Mode |

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
| PF0 | AUX IN         | AUX IN         |
| PF1 | LED1           | LED1           |
| PF2 | LED2           | LED2           |
| PF3 | RF In          | RF In          |
| PF4 | TX2 Feedback   | TX2 Feedback   |
| PF5 | RX2 on header  | RX2 on header  |
| PF6 | Reset          | Reset          |


### CPU core: The big kahuna
Assume that we specify that the frame rate will be 30fps, and that the number of LEDs will be a multiple of 100, plus some margin. That means that if the frame generation were instant, . A TCB in periodic interrupt mode could be used to kick off the send (by disabling interrupt during pattern generation, so if we miss the target, the send fires as soon as the generation is done and it's not an utter catastrophe. It is worse to display artifacts or ghosts than miss the frame rate target).

Calculation   | Clocks          | Description
--------------|-----------------|----------------------------------------------------------------------------
Simplest push | 27/LED          | Push each pixel 1 pixel up or down the string without order correction
Correct push  | 13/LED  16/Zone | Push eacg pixel 1 pixel up or down the string with order correction - much faster, but larger code size and more register use.
Buffer stuff  | 120/LED         | Used only if self-driving SPI scheme works, using the DREI to stuff the data into tghe buffer
polled stuff  | 42/LED minimum  | Used only if self-driving SPI scheme works, but polling, which is more awkward but may be faster.
random()      | 700/number      | uint32_t - Arduino random() is AWFUL AWFUL AWFUL
xorshift32_x  | 57-100/nbr est  | uint32_t. Not canonical xorshift32, but a simplified version with a 2^31-1 period. Canon xorshift uses a 128-bitstate, which is impractical on an AVR if performance matters.
xorshift16    | ~37/number      | uint16_t



**Will my scheme to drive my own SPI slave work?**

If it does, it saves 30us per LED that we would otherwise need to spend sending data. We can send


Assuming the target frame rate is 30 fps, this makes the difference between a budget of 864 clocks per LED to generate a pattern for only 500 LEDs, and 720 clocks per LED sending, and 792 clocks per LED for ONE THOUSAND LEDs!

Cycle count     | 1 bit send | 1 byte send | 1 LED send |  1 LED gen | 1 LED sendgen | All leds send | All leds gen  |         total |
----------------|------------|-------------|------------|------------|----------------|---------------|---------------|---------------|
500 LED Time  N |    1.25 us |       10 us |      30 us |      36 us |          66 us |         15 ms |         18 ms |         33 ms |
500 LED 24MHz N |   30 clock |   240 clock |  720 clock |  864 clock |     1584 clock | 360,000 clock | 432,000 clock | 792,000 clock |
500 LED 32MHz N |   40 clock |   320 clock |  960 clock | 1052 clock |     2112 clock | 480,000 clock | 576,000 clock | 1.056m  clock |
500 LED 40MHz N |   50 clock |   400 clock | 1200 clock | 1440 clock |     2640 clock | 600,000 clock | 720,000 clock | 1.320m  clock |
500 LED 48MHz N |   60 clock |   480 clock | 1440 clock | 1728 clock |     3168 clock | 720,000 clock | 864,000 clock | 1.584m  clock |
1k LED Time  Ev |    1.25 us |       10 us |      30 us |      33 us |          33 us |         30 ms |         33 ms |         33 ms |
1k LED 24MHz Ev |   30 clock |   240 clock |  720 clock |  792 clock |      792 clock | 720,000 clock | 792,000 clock | 792,000 clock |
1k LED 32MHz Ev |   40 clock |   320 clock |  960 clock | 1056 clock |     1056 clock | 960,000 clock | 1.056m  clock | 1.056m  clock |
1k LED 40MHz Ev |   50 clock |   400 clock | 1200 clock | 1440 clock |     1320 clock | 1.200m  clock | 1.320m  clock | 1.320m  clock |
1k LED 48MHz Ev |   60 clock |   480 clock | 1440 clock | 1728 clock |     3168 clock | 1.440m  clock | 1.584m  clock | 1.584m  clock |

Available Peripherals:
* TCA0
* ~TCA1~ used for backlight
* TCB0
* TCB1
* ~TCB2~ used for millis
* ~TCB3~ used for RF in
* TCD0
* 6 CCL LUTS
* 9 event channels
* SPI1 on PC4-7
* SPI0 on PE0-3
* USART4 on PE0-3
* USART1 P4-7
* TWI0 PA2, PA3
* PF0
* PF5 (note TX2 used in one-wire mode)

### Memory usage
* 1500 or 3000 bytes for the main pixel buffer;
* 1500 or 3000 bytes for the scratch buffer.
* a 3n-byte pallete buffer,
