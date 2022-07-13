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
  * Vcc
  * PF2 (LEDs)
  * Gnd
  * PF5 (unused currently)
  * Feedback (PF4, wired in half-duplex

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
