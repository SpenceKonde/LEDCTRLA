# Design for magic output

## TCD0
  The core of the magic happens here.
  Set in 2-ramp mode wth the compare channel override enabled.
  This divides the time into 4 periods:
    G of gated D type latch sets the output one or zero decision here
  DTA: Both channels high, length of 350ns
  OTA: Zero channel off, One channel high. Length of 350ns
  DTB: Both channel off, very short 50ns
0    Set SCK latch  - so during the OTB when both channels are off, SCK is low and next bt is being set up
  OTB: Both channel off, the low perios od both SCK and data.


## CCL allocation
### LUT0
  Configured so input 0 (LINK from LUT5) chooses between INPUT1 or INPUT2
  0: LINK
  1: TCD WOA
  2: TCD WOB
  OUT: PA6: LED

### LUT TBD
  Configured as SR latch to generate SCK signal
  0: R - CPBSET
  1: S - PROGEV?? CMPBCLR


### LUT5
  Configured as a D type Latch without using the sequencer?
  0: EVSYS PIN INPUT from MISO
  1: EVSYS CMPBset
  2: FB
