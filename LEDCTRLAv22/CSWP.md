# CSWP - Chainable Single Wire Protocol
These parts make possible a unique chaining method using both USARTs and the ADC to pass information

Each light string has a resistance from this pins to ground of either 0 ohms (jumper) or a resistor of of not more than 10k ohms - or it is connected to the TX pin of the USART


1) The head end will first apply power to the slaves. At this point no configuration of this pin is done. The master then enables the pullup, and the following sequence of events shall occur.
  a) The master measures the voltage on the pin:
    i. If it is in the zero bucket, we conclude we are talking to a series of 50 LED strings. Done.
    ii. If it goes all the way to Vdd, go to section b.
    iii. Otherwise, this is a resistor tagged string, take 5 measurements, if 4+ of them agree, that is the result, if only 3 agree, or if the measured resistance is invalid, display a warning or error.
  b) So the feedback line went all the way to Vdd or was at Vdd already?
    i. That means a slave/router is present. Head end will wait at least 10ms, but probably longer, and then generate a LOW pulse. This should be done such that the LOW pulse is longer than the time it takes to update the frame. The slave, if present, will respond by sending first a single 0xA9.
    ii. Master will then ask slave for information on it and downstream routers, via 0x05, 0x30, 0x3F (**ENQ**0?)
    iii. This is to be acknowledged by the slave with an ACK.
    iV. The head end will discontinue frame transmission after receiving the copyright sign (if the ACK is not received in a timely manner, it will conclude that the device is malfunctioning)
    v. The slave, once it has tested it's downstream port, and knows the lengths of all of it's strings and has any downstream responses, it will output something that shown below.


  0x02


After the fourth of these ID-readings has been taken, which should be done in a fraction of a second. If M_HOLD is used to confirm, the voltage should stay stable for the next several seconds, after which master sends "SYN?" to which the immediate downstream port should reply "ACK!" while both parts

|          | READ1 | READ2 | READ3 | READ4 | CONFIRM |  Meaning   |
|----------|-------|-------|-------|-------|---------|------------|
|          |  GND  |  GND  |  GND  |  GND  | No need | Daisy_50   |
|          |  GND  |  MID  |  GND  |  MID  | M HOLD  | Fixed len. |
|          |  GND  |  VCC  |   *   |  VCC  | SYN-ACK | Splitter   |
|          |  MID  |  GND  |  MID  |  GND  | M HOLD  | Fixed len. |
|          |  MID  |  GND  |   *   |  GND  | M_HOLD  | Fixed len. |
|          |  MID  |  VCC  |  MID  |  VCC  | M_TESTA | Damaged    |
|          |  MID  |  VCC  |   *   |  VCC  | SYN-ACK | Splitter   |
|          |  MID  |  VCC  |   *   |  VCC  | SYN-ACK | Splitter   |
|          |  MID  |  VCC  |   *   |  VCC  | Poll    | Nothing    |
|          |  VCC  |  VCC  |   *   |  VCC  | SYN-ACK | Splitter   |
|          |  VCC  |  VCC  |  VCC  |  VCC  | M_TESTB | Damaged    |

M_TEST: Connect pullup, briefly release

In the case of an M_TEST confirmation, the master will likely become aware of the slave when the slave has seen nothing suggesting other than noise that tends towards the positive supply rail here
In both M_HOLD and M_TEST, the master leaves their pullups on. But to pass M_HOLD, the slave should wait 100ms from when it released the pin before driving the pin LOW for 25ms, and then



|Resistance|  Min | Center |  Max | Length              |     | Type ID |
|----------|------|--------|------|---------------------|-----|---------|
|   Active |  N/A |  N/A   |  N/A | N/A                 | N/A |       0 |
|        0 |    0 |  216.5 |  433 | 50-200 or 400 leds  | GRB |       1 |
|     6800 |  434 |  613.5 |  793 | 50 fairy lights     | RGB |       2 |
|    10000 |  794 |  940   | 1086 | 100 fairy lights    | RGB |       3 |
|    15000 | 1087 | 1262.5 | 1438 | 200 fairy lights    | RGB |       4 |
|    22000 | 1439 | 1637   | 1835 | 322 COB lights      | RGB |       5 |
|    33000 | 1836 | 1988   | 2140 | TBD                 |     |       6 |
|    39000 | 2141 | 2237.5 | 2334 | TBD                 |     |       7 |
|    47000 | 2335 | 2428.5 | 2522 | TBD                 |     |       8 |
|    56000 | 2523 | 2633   | 2743 | TBD                 |     |       9 |
|    75000 | 2744 | 2869.5 | 2995 | TBD                 |     |      10 |
|   100000 | 2996 | 3121.5 | 3247 | TBD                 |     |      11 |
|   150000 | 3248 | 3365.5 | 3483 | TBD                 |     |      12 |
|   220000 | 3484 | 3597.5 | 3711 | TBD                 |     |      13 |
|   470000 | 3712 | 3839.5 | 3967 | TBD                 |     |      14 |
| No Conn. | 3968 | 4031.5 | 4095 | No string.          | N/A |      15 |




2) If the head end sees a passive string of a known length, it now knows the length of  the string and thus how many LEDs of data it needs to send, and we're done. The pullup is left on to detect if the string is disconnected, which is a necessary step in connecting something different. A change from 0 resistance to something that registers as a digital high is a disconnection event.

2a) A length is assumed if a daisychainable string is seen.

3) If power went all the way to Vcc, either there is nothing there, or there is an active splitter. The head end leaves the pullup on in this case.


S1) At power on, the slave will see either a floating pin, or a pin pulled up to Vcc [this pin should probably have had a very high value pulldown, say 5 meg or something, just to keep it from floating BUT we can also simply turn on the pullup on the LED data line if we don't see a controller. The controller sets the pin output. So if the pullup is on, and the value isn't consistently high, there's something connected to it. Testing will be needed to make sure this works]. It will then pull it's downstream pin high for 50ms and take an analog reading, reaching the same conclusion as the controller about what is downstream of it. To signal that it is something, rather than nothing, the slave will pulse the ID line low, and then release it.

4) At that point, the master should switch the pin from GPIO to UART with ODME and LBME set, putting it into half duplex mode (though the slave will only have ODME and it's pullup enabled )


This will proceed down a chain of splitters. As the controllers might at most support 1500 LEDs, and likely 600, 800, or 1000, and LED strings have at least 50 LEDs in them, with splitters going 3 ways, we could assign each device a number from 0 to 15 as it's ID - however this does not need to be done explicitly (the splitters are never aware of their ID, nor do they need to be). But the limit is 16 splitters, as we statically allocate ram to store this.

S2) Once a slave has done this, it will then turn on the USART, (unless it's the end of the string). If it is, it will instead leave on the pullup and wait for a connection. Each slave will then measure the string ID pin on each of the connected strings, which use the same resistance-length table, generating a 12-bit description of the connected LEDs, and configuring it's timer appropriately.

S3a) The USART configuration will be ODME (since connections between active units do not have a resistor, this works), but not one-wiremode, except for the controller.

S4) The end of line slave will then transmit a packet consisting of a unique hardware ID (derived from the serial number), the software version,

Something like:

0xVVVVVVVV 0xSSSS 0xABCD 0xABCD 0xABCD 0xRRRR RR 0xKK
Where
V = ID
S = Software version
A, B, C, D = the type ID of the 4 things connected to the splitter (D will generally be 0).
R = Reserved for future use
K = a 1-byte checksum value

4 + 2 + 2 + 2 + 2 + 3 + 1 = 12 bytes with triple redundancy on the string types, allowing for the head end slave to buffer the entire accumulated message in RAM. Since the splitting is done with virtually no CPU intervention, they don't need RAM - 256b is plenty of ram.


S5) When a slave receives a packet, it knows that if the low nybble of the 8th, 10th and 12th bytes are 0 or 1, it knows no further bytes are coming, and that is what tells it to send it's data followed by the contents of the buffer.

This repeats until it reaches the master

5) Master will receive up to 15 packets, and store them in a buffer. With 16k of SRAM, there is plenty of space for a 256b buffer, even with 6 or 9k dedicated to pixel data (3 bytes per pixel for the actual data, and 3 bytes per pixel to store information that may be used by certain animation modes. )
