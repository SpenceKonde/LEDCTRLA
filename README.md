# DriftAnimate
Simple Neopixel animation sketch for private event. Suitable as demonstration of Neopixel_Static library use. Runs on a '328(p) based board (like uno/nano/pro mini) and generates simple patterns. 

# DriftAnimatePlus
Example firmware for use with the Azzy's Electronics Light Controller module (coming to store 1H 2019) - though it is expected that users will create their own firmware to suit their specific needs and preferences. This module integrates a standard 1602 LCD, OOK RF receiver (based on RXB-12 or RXB-14) for remote control, and a button plus two rotary encoders for manual control. An analog pin is exposed for use with an LDR or external analog signal (though this is not used in the example code at this time). We use a large number of these in-house for parties and events. 

### Modes


### Remote Control

This sketch uses AzzyRF for remote control. It is controlled by sending a Large (16-byte) AzzyRF 2.2 packet with the following data: 

0 Destination ID and length
1 Command ID: 0x54
2 Mode number
3~8 Settings for the (up to) 6 settings controlled by left knob
9~14 Settings for the (up to) 6 settings controlled by right knob