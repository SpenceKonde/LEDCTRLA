# Rev. D+ String Specs

## Glossary:
* Head and tail- Data and power goes in at `head`, and data out comes from the `tail` end.
  * power-capable end is female, Power receiving end is male.
  * End that can supply power is always female, so it's hard to short out.
  * Head End Adapter - Male JST-SM2 for +19V, female JST-SM4 connector for controller, 2x JST-SM female connectors to go to head end of light string (10 ft? 12 ft?).
  * Yes, this means the connectors on commercial line are bacwards in this scheme. 

## Ctrl-Line cable
* Male JST-SM3 on head, female JST-SM5 or other on tail.

## Type 1:,
WS2811 sections, 50 lights with black or white wire. Zip cord to supply +19v & Gnd from power brick. Head Ends, tail end female. JST-SM5.

Pinout:
From Pin1: +19v, +5v, Data, GND, GND.

Mini560 buck converter to generate 5V at high efficienct, still power-limited at 4 strings. Could power mid-string for 400. 

### Assemnbly
**Phase 1** String with cut wires and female (tail) connector

1. cut output JST-SM3 connector (male) connector and +terminals off, right at the connector. Crimp new JST-SM3 terminals on using a suitable crimp tool. The obnoxiously thick insulation oprevents really satisfying crimps though. 
2. Crimp male JST-SM pins onto both conductors in a piece of color coordinated zipcord. Install both into a JST-SM5 housing taking care that the pinout is correct. 
3. With an assistant, stretch out the string. Measuring the wires against string ensure that zipcord is at least as long as the light string and not much longer.Cut it abd set this assembly aside. 
4. Repeat steps 1-3 for each light string in this production run.  the data, ground, and +5v wires, as well as on two members of a piece of color coordinated zipcord. 

**Phase 2** zipties, lots of them
1. Set up a place where you can suspend as long a portion as possible between furnature poles, etc. You should be able to comfortbly handle the leds at their current height. loosly coil the zipcord
2. Begin wrapping this around the light string. You will have to keep passing the wire over or under. You want 1 full revolution per 2-2.5 LEDs, ie, 20-25 wraps, and it should be tight.
3. Secure in place with tight, color coordinated zipties (black or white). You should use 20-35 zipties per section, Complete each string set aside each completed section
4. repeat 1-3 for all sections.

**intermission** 
1. In your spare time, cut off the ends of the zipties on the Type string subassenmblies. They cannot be further processed successfully until this is done. 
2. Measure a 2-inch length of color-coordinated heat shrink tube "with thick glue" for each string, 

**Phase 3** install mini560 on 5V feed
1. On the head-ward end clip off the original connectoron male JST-SM connectors in their place. Install them in a JST-SM5 housing taking extra care of pinout! It is extremnely easy to screw up the pinout!
2. locate the two power feeds closer to the head, strip away enough insulation to wrap around output terminal of the Mini560 sync. buck converter. Solder each one on oriented with the output towards the tail (so the wires will cross over the PCB which will give strain relief). Carefully measure the tail-wire to line up withe the input end of regulator, it will likewise need to cross alongside the converter. DO NOT CONNECT THEM.
3. From the cut-off wire, yoou should be able to reach the buck converter from the head connector easily. 
Crimp connectors on and install in the head end connector housing.
4. Set aside and move on to the next. 

**Phase 4** Completion
1. Thread the two input HV pairs through the piece of shrink tube. Confirm pinouts and connections a final time and solder down in position. 
2. Test, and if successful, shrink on the t hgeatshrinbk tube and ziptie it in position.use zipties to make the connector stay sealed against wire. 
3. Locate the power feeds at the tail end. Cut the wire shorter - maybe 1-2 inches, then separate the two strandsand fold one dback against itself. Cut a piece of 2" long glue lined shrink tubing, meltm, and fold over end of shrink tube before it hardens. If adjusted correcty, the glue will not burn you. If it iisn'... watch out!!
4. Do a final verification and wind onto a reel for storage, 
5. Repeat for all strings. You can fit 8 strings of black or white LEDs onto a 72mm  wide 22" diameter reel from [Mid America Taping and Reeling]. Check inventory of reels fort storing incoming cut tape, strips of terminal ends, and simple wire.>  Narrow *8-16mm*( rolls are good for strips of JST connectors. *24mm* is good for general purpose wire that you have lots of (like the zipcord this project uses, or the double-silver wire many other projects use. Finally, the 12 or 16mm wide rolls are excellent for storing type 2 (fairly light)) strings when not in use. One string of 200 fits on a 7.5" reel.

## Type 2:

6. You *must* have a way to roll up the fairly lights when handling them! Their tangles cannot be undone by mere mortals, and you must rotate reel along it's axis wheb rolling them up. Wrapping around a stationary reel or objevt wwill introduce a twist for each revolution, and the momentyou let go ofitm it will almost exoplosively uncoil itseklf, Rolling up correctlky prevents this. 

Unit of importance is 200- strings made from fairy lights. ToDo: - how sloppy can we be about getting power into these? If they do 100-strings, can we power the middle of a 200 string? we can right? head end might need it's own power, but it will be a ways away... plus we can deliver data to the middle using pixelrouter. Could we power and data into middle of 2x200? Pinout:
+19v, empty or +5, data, ground, 

## type 3
COTS 100-string fairly lights, standard 3p in WITH THE WRONG FUCKING gender - side that supplies power must always be female, never male! will make adapters to reverse it, and heat-shrink them on or swap connectors. 
