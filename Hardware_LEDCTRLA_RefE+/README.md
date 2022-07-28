# Hardware in planned final versions of the DriftAnimate LED controller, now being called LEDCTRLA. 
Rev. E was found to have crititcal flaws that made it entirely unusablke without bulky external components - namely, the damned things burned out f

## Rev. E issues:
* AVR128DB48 is used for the controller. The 32 did not have sufficient pins. 
* There was no circuit protection. Unfortunately, while FAR more efficient, when using the Mini560 converters and some +19v power bricks, there wwas an unacceptabl y high rate if faiures, particularly in combination with unreliable connections 
  * They were made usable for testing with a protection board consisting od a 5.1V zener across the power rails, a PTC fuse, and a pair of protection diodes (schottky) on each data line. These were bulky and considered too fragile for largescale deployment. 
* Additionally, the possibility of background LED output via slave SPI and nearly every peripheral had not been consdered; It requires several jumpers between pins to implement, and specific pins must be used. 
* 

## Rev. E & F Pin connections:
Rev. F simply added a robust protection circuit. 
* 250mA PTC fuse that would trip if the zener kicked in or some other fault was occurring.
* NCP349 OVP and UVLO IC - other parts were considered, but only these were available with a reasonable lead time. The package is challenging to work with, but cest la vie. 
* A 5.6V 3W zener diode was added between power and ground. 
* There are potentially 4 data lines connected to the LED strings, 2 of which are currently integral to operation:
  * PF2 (LEDs)
  * PF5 (unused currently)
  * Feedback (PF4, wired in half-duplex)
  * PF0 (Aux, unused currently

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
| PA6 | Led Data Out)   |                |
| PA7 | LED (PWM)      | LED0           |

### PORTB
| Pin | Function Rev F | Function Rev G |
|-----|----------------|----------------|
| PB0 | Backlight RED  | Backlight RED  |
| PB1 | Backlight GRN  | Backlight GRN  |
| PB2 | Backlight BLU  | Backlight BLU  |
| PB3 | Button - Color | Button - Color |
| PB4 | Button - Param | Button - Param |
| PB5 | Button - Mode  | Button - Mode  |

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


## Splitters
  Single wire protocol is used for communication between light controller and downstream splitters on a separate line from the pixel data. The splitters themselves will, for example, take 500 pixels of data in, forward the first 100 to one string, the next 100 to another string, the next 100 to a third string, and then forward whatever's left to the downstream splitter or dumb 2811 string. 
  
 ``` text
 LEDCTRLA ---------[SPLIT 1]---------[SPLIT 1]---------[SPLIT n]
                   |   |   |         |   |   |         |   |   |  
                  1A  1B   1C       2A  2B  2C        nA   nB  nC
                  

 LEDCTRLA ---------[SPLIT 1]---------[SPLIT 1]---------[SPLIT n]---------[50xWS2811 1]---------[50xWS2811 N]
                   |   |   |         |   |   |         |   |   |  
                  1A  1B   1C       2A  2B  2C        nA   nB  nC
```

1A is 50, 100, or 322 LED long string. 

As current is limited to 3.42A @ 19v plus conversion loss and wire loss, max of 4 strings in sequence supported, or 8 with center-powered configuration.

Higher current supported with power injected in multiple places. 

### FB protocol to request and receive specs on all connected strings. 

LEDCTRLA enables pullup on FB. 
Measure with ADC repeatedly. Analog values indicate dumb fairy light string, 50, 100 or 200 leds in length, or 332 LED COB string. 
If it is grounded, there are no splitters. Splitters, if any, must preceed all other modules for detection.
Otherwise, splitter shall allow pin to go high for 1 second, and then drive it low for 100ms, release, and pull up.
LECTRLA, as soon as the line is brought high, will then drive it low for 100ms, during which time the splitter, if it hasn't already finished it's interrogation of downstream units must drive the pin low. The controller will then release the pin and pull it up, and when the line goes high, enable USART in single wire mode
Splitter will then indicate busy state by driving line low, and pull up it's downstream FB line, repeating the above protocol. 

The RX lines of splitters, connected to the bidirectional hardware serial of downstream splitters, would use a custom software serial implementation to allow RX and TX of short messages to occur. 

  Single wire protocol is used for communication between light controller and downstream splitters. 
  
 ``` text
 LEDCTRLA ---------[SPLIT 1]---------[SPLIT 1]---------[SPLIT n]
                   |   |   |         |   |   |         |   |   |  
                  1A  1B   1C       2A  2B  2C        nA   nB  nC
                  

 LEDCTRLA ---------[SPLIT 1]---------[SPLIT 1]---------[SPLIT n]---------[50xWS2811 1]---------[50xWS2811 N]
                   |   |   |         |   |   |         |   |   |  
                  1A  1B   1C       2A  2B  2C        nA   nB  nC
```

A/B/C are 50, 100, 200, or 322 LED long strings. 

As current is limited to 3.42A @ 19v plus conversion loss and wire loss, max of 4 strings in sequence supported, or 8 with center-powered configuration.

Higher current supported with power injected in multiple places. 

:
### Basic
+19v and Gnd lines to superseal connector should be the heaviest gauge wire that the connectors will fit, and may be either Orange and Black, Orange and Grey, or Unmarked and Marked black, white, or transparent zipcord. 

Lines to PH4 F to controller should be the listed colors, or this should be a 4 conductor cable chosen for appropriate aesthetics. 

```
 PH4 F                                      ___________
 ____                         /------+19V--| MX 3.0|___]
|    |--R-----+5V------------t-------------| 5 pin |___]
|    |--U-----Data-----------t-------------| Female|___]
|    |--B-----Gnd--------..--t-------------| (male)|___]
|____|--Y-----FB---------``--t----.-Gnd----|_______|___]
                             |    |
 _______________             /    |
| Male          |-O---+19V--`     |
| SuperSeal 2p  |                 /
|_______________|-B---Gnd--------`        
```

### LEDCTRLA to String

Lines to PH4 F to controller should be the listed colors, or this should be a 4 conductor cable chosen for appropriate aesthetics.

``` text
 PH4 Male                         ZH6 
 ________                         ___
|      1 |--R-----+5V------------|6  |   +5V
|      2 |--U-----Data-----------|5  |   Data
|      3 |--B-----Gnd------------|4  |   Gnd
|______4_|--Y-----FB-----.       |3  |   RX2/etc
                          `------|2  |   TX2/FB
                                 |1__|   Aux
```

### Power injection 

```text                    
___________                                       ___________
:-- MX 3.0 |                       /--+19V-------| MX 3.0|___]
:-- 5 pin  |                       |             | 5 pin |___]
:-- female |--U-----Data-----------t-------------| Male  |   ]
:--        |--Y-----Gnd------------t-------------| (male)|___]
:--________|--B-----FB-------------t----.-Gnd----|_______|___]
                                   |    |
       _______________             /    |
      | Male          |-O---+19V--`     |
      | SuperSeal 2p  |                 /
      |_______________|-B---Gnd--------`
```

Allows power to be injected in the middle of a string to supply up to 4 more downstream sections of 50 LEDs. Note that +19 and +5v from other string are not connected!

### Double Power injection 

```text                    
___________                                       ___________
:-- MX 3.0 |----------------\           /--+19V--| MX 3.0|___]
:-- 5 pin  |                |           |        | 5 pin |___]
:-- female |--U-----Data----t-----------t--------| Male  |   ]
:--        |--Y-----Gnd-----t-----------t--------| (male)|___]
:--________|--B-----FB------T----,--,---t-Gnd----|_______|___]
                            |    |  |   |
 _______________            /    |  |   |       _______________
| Male          |-O--+19V--`     |  |    +19-O-| Male          |
| SuperSeal 2p  |                /  \          | SuperSeal 2p  |
|_______________|-B---Gnd-------`    `---Gnd-B-|_______________|
```
