# DriftAnimatePlus is henceforth known as LEDCTRLA (from the convention used for naming AVR registers).

## What is LEDCTRLA
This is a WS2812-alike controller with several key features that are absent in most controllers:
  1. The patterns don't look like garbage most of the time.
  2. From Version 2.1.0 it has taken advantage of the capabilities of the AVR-DA to displaty a far larger number of pixels.
  3. It incorporates a simple UI based on a 16x2 character LED allowing operaion with minimal training.
  4. (planneed - must be ported to modern AVRs) wireless scheme described below allowing arbitrary control from a computer on the local network.
  5. It has a solution for dealing with the fairy lights which cannot be daisy chained: A three way splitter based on a an ATtiny424 allows splitting the signal up
  6. (planned) It will have modes to take advantaqge of this feature
  7. Randomness is key to making patterns look goood. The arduino Random function is a steaming pile of crap - we have one that generates the amount of random data we need via an xorshift algorithm more than an order of magnitude faster
  8. (planned) it will take advantage of the event system and CCL to output the data for one from while calculaing the data for the next one, allowing much higher frame rates

## Why another LED controller?
Not invented here syntromee of course! That and I was profoundly disappointed in the commercially available units.

Not only were the patterns disappointing and uninspired, (too uniform), they frequently required a phone app or something to control. I wanted a standalone thing, because I wanted to be able to give the "training" on their use to people in a minute or two to trainees who were at a an event (sometimes called a party) where sobriety could not be assumed. Having remote control take prioritiy allows the hosts of such an event to remothely override the current lighting to evoke a particular mood to fit the theme.

## Scope of the system
This system comprises three components - power supply, the light strings themselves, and the controller, as well as interconnection hardware, firmware, and the

### Power supply
Power is supplied to the strings using standard laptop power bricks which supply 18-20V at around 3.5A.

These are modified for use with LEDCTRLA by replacing whatever dumbass connector the laptop manufacturer thought would make it the hardest for third parties to make replacement power supplies, and replacing it with a female 2-pin TE SuperSeal 1.5 connector. Ground is on pin 1 of the SuperSeal connector.

Particularly beefy laptop power supplies, typically made for so-called gaming laptops (which would look comically large in a giant's lap, are so heavy you need a wheelbarrow to move them around, emit more heat than a fire-breating dragon and are more expensive than a new car), typically supply around twice as much current. These should install two connectors in a Y configuration. The SuperSeal connector is theoretically rated for 7A, but I think the chance that the ones I'm using were actually made by TE are about 0% and the liklihood that they're as good as the real TE ones not much higher. But half the spec at most doesn't seem too demanding. For powering a 400-LED string of 2811 christmas light style lights, the power ought to come in the middle anyway....

This all, however, is matters of basic electrical engineering, not anything fundamentally unusual or different



## Hardware

### Light Strings

WS2811 light strings - from Aliexpress. Connectors at both ends (JST-SM) are unfit for purpose, and are removed, and replaced 5-pin Molex MX3.0 connectors (well, JMCCON's clone of them, which fits Molex's pins better than Molex's housings). 2 pins are dedicated to ground and connected to eachother on both ends of each string. The outermost pinsare connected to a color-coordinated 2-conductor wire wrapped around each section of 50 LEDs about 12-15 times and ziptied in place. This serves as a heavier gauge power wire to the downstream sections, and the positive side carries the 18v. At the head end, each string has a Mini560 buck converter with synchronous rectification outputting 5v from the 19V supply.

Fairy light strings - several styles, all the lower-current lower-brightness version of the LED. Usually hardwired to a controller that generates hideous patterns. That controller is removed and discarded. These get attached to JST-PH-4 connectors, as they don't support daisy-chaining, and will instead plug into LEDROUTEA. The fourth pin of the connector is connected to ground with a resistor, and this is used to indicate to the router what length and color order that section is.

COB strings - details TBD. I think the strip can be stuck to one side of a piece of "wire tape" (2 conductor copper tape with plastic on either side, and that the required power injectors could be powered from that. )

### LEDCTRLA
The "Production" version of LEDCTRLA follows a >5 year development cycle including changes to nearly every significant component.

LEDCTRLA is built around an AVR128DB48-EPT. Maximizing the memory was seen as key, so a 128k part was the obvious choice (the flash matters much less, but the ram is our frame buffer and matters a great deal). The extended temperature version was selected to improve overclocking performance should that become necessary to meet system requirements. Because a crystal is not needed for speeds of 32 MHz and below, the crystal frequency chosen was 40 MHz, an aggressive overclock but typically within the capabilities of E-spec parts. . It uses the Optiboot Dx bootloader. A 1602 display with an RGB backlight from Surenoo Display is used as the display (monochrome ones, more abundant, can also be used). This is used in 4-bit parallel mode, eliminating requirements for a port expander and reducing latency.

The board mounts a 3.3v regulator used to power the VDDIO2 rail (not currently used, but envisioned as a requirement for L.E.D. Zepplin or similar devices), and an expansion header supplying PORTE (PE0-PE3 - it is a half port on these parts) and the high-half of PORTC as well as ground, +5V and +3V3 power, this faces towards the back and is made from dual row round 0.1" pin header. Pads are provided for one of a number of OOK/ASK RF receiver modules - the SRX-887 is belived to be the most advanced and most effective. The RXB-12 is very low cost, and performs very well. No transmitter is provided. This is intended to provide the simplest lowest cost method of one-to-many remote control (which can simulate targeted commands through an addressing system).

There are three controls on the forward side below the screen. Two rotary encoders allow configuration of each mode, one handling color configuration and the other other parameters. In both cases, to scroll through the options, press the center of the rotary encoder shaft. The mode switch between them scrolls between modes.

Colors will be specified in a manner dependent on the mode, generally corresponding to one of three mechanisms: Most modes have a selectable, named, pallete. Others may have either a separate adjustment for the three channels, min and max settings per channel or other schemes as detailed in the mode documentation.

Parameters depend entirely on the mode. Some modes have none, others have a large number with different minimum and maximum values.

A means will be provided to enter a diagnostic mode without the aid of a computer, though a computer will aid the process.

#### Revisions
* Rev. -: Initial version used a I2C backpack on the display and mounted a pro mini.
* Rev. A:
* Rev. B: 238pb/1284p on board, parallel control of display.
* Rev. C: Incorrect placement of controls, was not used.
* Rev. D: Used 128DB32. This version was totally non-functional, as the AVDD and AGND pins were reversed. As these are connected internally to Vdd and Gnd, applying power would either cause the power supply to register an overload, or result in an overcurrent condition sufficient to blast away a bit of the package between those two pins.
* Rev. E: Finally switch to 128DB48. Add expansion header, place to mount SRX887, 4 status LEDs as 2 dual leds. Likely the first one with the ZH connector. Had a bug regarding display - power was not correctly connected to the display and a jumper was needed.
* **ALL VERSIONS REV E OR LOWER ARE NOT FIT FOR THE INTENDED USE CASE** - Rev C cannot be installed in the enclosure at all, and Rev. D immediately destroys the chip when power is applied. Rev. -, Rev. A, and Rev. B use an inappropriate control chip with insufficient memory, and Rev - through Rev. E were designed before understanding of the problem which caused extremely frequent failures of the controller and the LCD connected to it.
* Rev. F: Add debug header. In recognition of the stratospheric failure rate in the field, investigation of causes was undertaken, and it was found that damaging spikes could be created on the 5v line during startup as well as in the case where the +19V input remains, but ground is not connected to all parts. Regardless of the mechanism, when 19V is applied to the 5V rail, it causes, unsurprisingly, rapid and total failure of all components, explaining the issues which had been seen. This revision was directed towards correcting this issue:
  * A 100 ohm series resistor (as a resistor network) was added to the 4 data lines that connect to each controller to the light strings.
  * A <500mA PTC fuse is added to the +5V in pin.
  * An NCP349 OVP protection chip (now on last-time buy - I use these all over the place and have not found any replacements, and thus panic-bought 100 of them).
  * A 5 Watt, 5.6V zener diode was added across the power rails to clamp any momentary excess (located after the NCP349).
  * 4 dual schottky diodes were added just after the 100 ohm resistor network, 2 common anode and 2 common cathode, with the common terminals connected to the appropriate supply rail. Each data line was connected to 1 of each type of these.
* Rev. G: The 4 dual diodes inflated part count without need. They were replaced with a single NUP4302, which has the same functionality.  Switch the quartet of dual diodes for a NUP4302 to clamp the data lines. The debug header is moved again.
* Rev. H: Move PWR BAD LED and label it. Add reset button, add "microJST" (Molex Picoblade) connector for programming. Fix the package design for the NUP4302 and NCP349, which were missing the orientation mark. The debug header was ONCE MORE moved. This version onwards will not use the bulky dupont connector for programming.

### LEDROUTEA
Based around a tinyAVR 424, and driven almost entirely using the custom logic functionality (CPU intervention is involved only at the start and end of each frame via interrupt). These check the resistors to see what the lengths of the strings are, reports this via half-duplex single wire serial upstream to the controller (and receives data from any downstream routers), and then sends the first however-many LEDs of data in each frame down the three strings in orders, and lets rest pass through to the downstream devices, if any.

#### Revisions:
* Rev. -: Used a tiny412 to switch pixels down one of two routes. Frequent failures.
* Rev. A: Even more frequent failures, despite the fact that development was aimed towards the opposite.
* At this point the initial version was abandoned.
* Rev. -: This represents a new version using a Mini560 power supply, and an ATtiny 424. The greater pincount and additional CCL modules allowed for up to three branches to be added.
* Rev. A: Insufficient circuit protection.
* Rev. B: Viable proof of concept after a large number of ECO's.
* Rev. C: Electronics checked out 100%. Unfortunately it was found to not fit in the enclosures that were prepared for it. In response to concern that Rev. B could be vulnerable to similar issues as LEDCTRLA, A suite of OVP mitigation mechanisms was implemented around the buck regulator:
  * 330 uF tantalum cap on the output.
  * 23V bidirectional TVS diode on the input.
  * NCP349 OVP IC on the output.
* Rev. D: Fits in the enclosure. Routing and layout improvements.

### Interconnection hardware
Throughout this repo I refer to the end of a string of lights where the data comes in as the "head" end and the end that the data comes out of as the "tail" end.

#### Recap
This has been mentioned in passing elsewhere, but let's make these things clear at this point
* The controller will have either a dupont or "microJST" (aka picoblade clones) connector. If these are present they are wired in identical (FTDI) pin order, and if both 0.1" header and microJST is available, the pins are literally tied to eachother. Only one should be used
*

#### Head End Adapter
This wiring harness consists of three connectors with wire between them:
* 5-pin MX 3.0 connector (male housing female pins). Pinout is +19V, +5V, Data, Comms (between LEDROUTEA and , and Ground. This connects to the downstream device (led string or LEDROUTEA) providing 19v to it's buck adapter. It may *take* 5v from the 5v rail. It shall under no circumstances *supply* any power on that pin.
* 2-pin TE/Tyco SuperSeal 1.5 connector (male pins, female housing)to connect to the power supply. These two wires are passed to the two outer pins of the MX 3.0 connector.
* Finally, it has a JST-PH-5 plug. The pin order is +5V, Data, Gnd, N/C, Return data. It is important to not let it be a JST-PH-4 header, as that could be plugged into a splitter accidentally.

#### Tail end diagnostics adapter
Dual voltage meter on a female MX 3.0 housing to permit monitoring of the voltage drop over both wires.

#### Head end diagnostics adapter
Voltage and current meter in series with a pair of TE superseal connectors.

#### Development adapter
Short JST PH-5 extension with onlty pins 2, 3, and 5 connected (no +5V) for use during development

#### Controller adapter
The connection to the controller is made with a JST-PH-6 connector, of which 4 wires are used to talk to the LED strings and LEDROUTEA modules.


## Possible addons

I've considered a small ESP8266-based board that could connect to the expansion header on LEDCTRLA. These devices would of course have to have another networked device to shepherd them, but the implementation could probably be pretty bare-bones.

Anyway, I know what the devices would be called:

The board that attaches to LEDCTRLA would be the L.E.D. ZEPPLIN - and it would interface over wifi with CEMENTCLOUD (from the expressions, one of which the band took it's name from, phrases that mean "That's not gonna fly" or less ideomatically, "Nobody will stand for that"/"no way will that idea work": "That'll go over like a lead [the metal] balloon"  or "that'll go over like a cement cloud"). (Trivia: the band had originally wanted to spell it Lead Zepplin, but people thought Lead was meant to be pronounced and parsed like "leader" rather than the metal - a totally different meaning than they were going for - so they changed the spelling so it could only be pronounced to sound like the metal, as was their intent.)

The other thing that the expansion header might be used for is a device based on the MGSEQ7 to for audio-reactive lighting.
