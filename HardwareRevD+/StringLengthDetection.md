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





## FB protocol

1. LEDCTRLA enables pullup on FB. 
2. Measure with ADC repeatedly. Analog values indicate dumb fairy light string, 50, 100 or 200 leds in length, or 332 LED COB string. 
  a. If it is grounded, there are no splitters. Splitters, if any, must preceed all other modules for detection.
  b. Otherwise, splitter shall allow pin to go high for 1 second, and then drive it low for 100ms, release, and pull up.
  c. If intermediate analog value seen, and hence it is a dumb-string, determine length from the resistance
3. LECTRLA, as soon as the line is brought high, will then drive it low for 100ms, release, and pull up, enabling USART in single wire mode
4. Splitter will then indicate busy state by driving line low, and pull up it's downstream FB line, repeating the above protocol. 
  a. If it sees another splitter, it shall wait for the line to be released. During this time it may measure the resistances on strings to find length
  b. In either event the splitter will measure the connected pins of outputs and deduce string lengths.   
5. Splitter which does see another splitter downstream will instead hold hold it's upstream line low and enable pullup on downstream fbline.
6. A splitter with the length of it's string and all downstream ones known shall release the upstream feedback line.
  a. Downstream device has 100ms to enable it's USART to 2-wire mode to receive the data. At that point, it will know the length of it's strings and all downstream ones. Repeat step 6 until the first splitter has informd controller of the strings connected. 
7. Normal operation commences. All splitters will have USART in one-wire mode, 

Adapters:
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
