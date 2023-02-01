# DriftAnimatePlus is henceforth known as LEDCTRLA (from the convention used for naming AVR registers).

## What is LEDCTRLA
This is a WS2812-alike controller with several key features that are absent in most controllers:
  1. The patterns don't look like garbage most of the time.
  2. From Version 2.1.0 it has taken advantage of the capabilities of the AVR-DA to displaty a far larger number of pixels.
  3. It incorporates a simple UI based on a 16x2 character LED allowing operaion with minimal training.
  4. (planneed - must be ported to modern AVRorts teh wireless scheme described below allowing arbitrary control from a computer on the local network.
  5. It has a solution for dealing with the fairy lights which cannot be daisty chained: A three way splitter based on a simple ATtiny424 allows splitting the signal up
  6. (planned) It will have modes to take advantaqge of this feature
  7. Randomness is key to making patterns look goood. The arduino Random function is a steaming pile of crap - we have one that generates the amount of random data we need via an xorshift algorithm more than an order of magnitude faster
  8. (planned) it will take advantage of the event system and CCL to output the data for one from while calculaing the data for the next one, allowing much higher frame rates

## Why another LED controller?
Not invented here syntromee of course! That and I was profoundly disappointed in the commercially available units.

Not only were the patterns disappointing and uninspired, (too uniform), they frequently required a phone app or something to control. I wanted a standalone thing, because I wanted to be able to give the "training" on their use to people in a minute or two to trainees who were at a an event (sometimes called a party) where sobriety could not be assumed. Having remote control take prioritiy allows the hosts of such an event to remothely override the current lighting to evoke a particular mood to fit the theme.

## Scope of the system
This system comprises three components - power supply, the light strings themselves, and the controller, and the three of them join at the Head End Adapter

### Power supply
Power is supplied to the strings using standard laptop power bricks which supply 18-20V at around 3.5A.

These are modified for use with LEDCTRLA by replacing whatever dumbass connector the laptop manufacturer though would make it the hardest for third parties to make replacement power supplies, and replacing it with a female 2-pin TE SuperSeal 1.5 connector. Ground is on pin 1 of the SuperSeal connector.

Particularly beefy laptop power supplies, typically made for so-called gaming laptops (which would look comically large in a giant's lap, are so heavy you need a wheelbarrow to move them around, emit more heat than a fire-breating dragon and are more expensive than a new car), typically supply around twice as much current. These should install two connectors in a Y configuration. The SuperSeal connector is theoretically rated for 7A, but I think the chance tha

### Light Strings



### LEDCTRLA
