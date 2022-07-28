# Hardware in planned final versions of the DriftAnimate LED controller, now being called LEDCTRLA. 
Rev. E was found to have crititcal flaws that made it entirely unusablke without bulky external components - namely, the damned things burned out f

Rev. E issues:
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

### PORTA
* PA0, PA1 - Crystal or osc.
* PA2 - Unused. SDA
* PA3 - Unused, SCL
* PA4 - TX
* PA5 - RX
* PA6 - Unused (PWM)
* PA7 - Status LED 2.1 (PWM)

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
* PF0 - Rev. F: status LED 1.1
* PF1 - Ref. F: status LED 2.0
* PF2 - LEDs
* PF3 - RF Data
* PF4 - TX2 (Feedback)
* PF5 - unused, has analog - RX2
* PF6 - Reset

## Rev. G pin connections
Rev. F simply added a robust protection circuit. 
* 250mA PTC fuse that would trip if the zener kicked in or some other fault was occurring.
* NCP349 OVP and UVLO IC - other parts were considered, but only these were available with a reasonable lead time. The package is challenging to work with, but cest la vie. 
* A 5.6V 3W zener diode was added between power and ground. 
* There are potentially 4 data lines connected to the LED strings. The pairs of protection diodes in SOT-23 were replaced with a single NUP3402 clamp array. The 68 ohm resistor rmaines, 2 of which are currently integral to operation; notice that the pins have CHANGED to ensure that it is possible to send LEDs in the background. 
  * Vcc
  * PA6(LEDs)
  * Gnd
  * TBD PF5
  * Feedback (PF6 - TX of Serial 2, configured in half duplex mode)
  * Aux (PF0)

### PORTA
* PA0, PA1 - Crystal, 40 MHz
* PA2 - Unused. SDA
* PA3 - Unused, SCL
* PA4 - TX used for upload
* PA5 - RX used for upload
* PA6 - WS2812B data output (can be CCL0 out by alternate utput. 
* PA7 - Status LED 2.1 (PWM)

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
* PF0 - AUX to/from LEDs? 
* PF1 - Rev. F: status LED 1.1
* PF2 - Ref. F: status LED 2.0
* PF3 - RF Data
* PF4 - TX2 (Feedback)
* PF5 - unused, has analog - RX2
* PF6 - Reset


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


## Adapters:
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
| Male          |-O---+19B--`     |
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

