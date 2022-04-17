# Rev. D+ String Specs

## Glossary:
* Head and tail- Data and power goes in at `head`, and data out comes from the `tail` end.
  * Power and Data receiving end has male connectors.
  * End that can supply power is always female, so it's hard to short out.
  * Head End Adapter - Takes power input 
  * Yes, this means the connectors on commercial line are bacwards in this scheme.

## Head End Adapter
Male SuperSeal connector for power. 
Female MicroFit 3.0 5P 

JST-SM was found to be unfit for purpose at least for the case of the WS2811 strings,

## Type 1:,
WS2811 sections, 50 lights with black or white wire. Zip cord to supply +19v & Gnd from power brick. Head Ends male, tail end female. ~JST-SM5~ Molex MicroFit 3.0 5p single row.

Pinout:
From Pin1: +19v, +5v, Data, GND, GND.

Mini560 buck converter to generate 5V at high efficienct, still power-limited at 4 strings. Could power mid-string for 400.

### Assemnbly
**Phase 1** String with cut wires and female (tail) connector

1. cut output JST-SM3 connector (male) terminals off, right at the connector. Crimp MicroFit 3.0 (Female) terminals using a suitable crimp tool to the positive side and data pins. Strip the ground wire as if you were going to do the same. 
2. Crimp a  Microfit 3.0 (Female) terminals to the positive wire of a piece of color coordinated zipcord. It is recommended to have this zipcord rolled onto a reel of some kind.
3. Strip the ground wiew of the zipcord, as well as a 1.5-2 inch length of 24 AWG FEP insulated wire, stripped at both ends slightly longer than would normally be appropriate for MicroFit. Twist it around one of the ground wires, and crimp on MicroFit (F) terminal. Repeat with the other end and the other ground wire. This is a failsave to ensure that ground is always ground at both points. This not being the case was implicated in a seris of failures. 
4. Triple check wiring order and insert into a 1x5 Microfit housing. 
5. With an assistant, stretch out the string. Measuring the wires against string, ensure that zipcord is at least as long as the light string and not much longer. Cut it and set this assembly aside.
6. Repeat steps 1-3 for each light string in this production run.

**Phase 2** zipties, lots of them
1. Set up a place where you can suspend as long a portion as possible between furnature poles, etc. You should be able to comfortbly handle the leds at their current height. loosly coil the zipcord
2. Begin wrapping this around the light string. You will have to keep passing the wire over or under. You want 1 full revolution per 2-2.5 LEDs, ie, 20-25 wraps, and it should be tight.
3. Secure in place with tight, color coordinated zipties (black or white). You should use 20-35 zipties per section, Complete each string set aside each completed section
4. repeat 1-3 for all sections.

**intermission**
1. In your spare time, cut off the ends of the zipties on the Type string subassenmblies. They cannot be further processed successfully until this is done.
2. Measure a 2-inch length of color-coordinated heat shrink tube "with thick glue" for each string, of a diameter sufficient to slide over the wires and buck converter without excessive force; in order to provide a seal agaist water though, it must also not be too large in diameter.

**Phase 3** install mini560 on 5V feed
1. On the head-ward end clip off the original connector. Crimp on Male MicroFit 3.0 terminals in their place, again exceopt for the ground. Install them in a JST-SM5 housing taking extra care of pinout! It is extremnely easy to screw up the pinout!
2. locate the two power feeds closer to the head, strip away enough insulation to wrap around output terminal of the Mini560 sync. buck converter. Solder each one on oriented with the output towards the tail (so the wires will cross over the PCB which will give strain relief). Carefully measure the tail-wire to line up withe the input end of regulator, it will likewise need to cross alongside the converter. DO NOT CONNECT THEM.
3. From the cut-off wire, yoou should be able to reach the buck converter from the head connector easily.
Crimp connectors on and install in the head end connector housing.
4. Set aside and move on to the next.

**Phase 4** Completion
1. Thread the two input HV pairs through the piece of shrink tube. Confirm pinouts and connections a final time and solder down in position.
2. Test, and if successful, shrink on the t hgeatshrinbk tube and ziptie it in position.use zipties to make the connector stay sealed against wire.
3. Locate the power feeds at the tail end. Cut the wire shorter - maybe 1-2 inches, then separate the two strands and fold one dback against itself. Cut a piece of 2" long glue lined shrink tubing, meltm, and fold over end of shrink tube before it hardens. If adjusted correcty, the glue will not burn you. If it iisn'... watch out!!
4. Do a final verification and wind onto a reel for storage,
5. Repeat for all strings. You can fit 8 strings of black or white LEDs onto a 72mm  wide 22" diameter reel from [Mid America Taping and Reeling]. Check inventory of reels fort storing incoming cut tape, strips of terminal ends, and simple wire.>  Narrow *8-16mm*( rolls are good for strips of JST connectors. *24mm* is good for general purpose wire that you have lots of (like the zipcord this project uses, or the double-silver wire many other projects use. Finally, the 12 or 16mm wide rolls are excellent for storing type 2 (fairly light)) strings when not in use. One string of 200 fits on a 7.5" reel.

## Type 2:

1. You *must* have a way to roll up the fairly lights when handling them! Their tangles cannot be undone by mere mortals, and you must rotate reel along it's axis wheb rolling them up. Wrapping around a stationary reel or object wwill introduce a twist for each revolution, eventually tangling itself up and  it will almost exoplosively uncoil itself, Rolling up correctly prevents this.

Unit of importance is 50, 100 LED stringsts.
The strings with the opaque blobs are brighter and draw >2x the current. even with 5.5v input, whites brighter than 144/channel will show discoloration at the end. Powered from one end strings of 100 of these will reach apx. 1.7A/string at full brightness, but with it limited such that no discoloration is seen, current tops out at around 1.5A. The 200 LED strings are made from a pair of 100-led strings; these can be desoldered from eachother in the middle, and this is required for a decent brightness

The strings with the clearer plastic draw about half the current/LED, though you can't quite get 100 at full brightness.

They will be driven by splitter boxes, which will have an XH header for the string (4 pins, Gnd, Data, +5v, typesense.)


COTS 100-string fairly lights, standard 3p in WITH THE WRONG FUCKING gender - side that supplies power must always be female, never male! will make adapters to reverse it, and heat-shrink them on or swap connectors.
