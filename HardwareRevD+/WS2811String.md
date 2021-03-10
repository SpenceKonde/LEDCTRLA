# Rev. D+ String Specs

## Glossary:
* Head and tail- Data and power goes in at `head`, and data out comes from the `tail` end.`
* No JST-SM connector is to be wired such that either end could be carrying the power:
  * End that can supply power is always female, so it's hard to short out.
  * Head End Adapter - Male JST-SM2 for +19V, female JST-SM4 connector for controller, 2x JST-SM female connectors to go to head end of light string (10 ft? 12 ft?).

## Ctrl-Line cable
* Male JST-SM3 on head, female JST-SM5 or other on tail.

## Type 1:,
WS2811 sections, 50 lights with black or white wire. Zip cord to supply +19v & Gnd from power brick. Head Ends, tail end female. JST-SM5.

Pinout:
(Gnd from +19) (direct ground) Data +5V, +19V

Mini560 buck converter to generate 5V at high efficienct, still power-limited at 4 strings. Could power mid-string for 400. 

## Type 2:
Unit of importance is200- strings made from fairy lights. ToDo: - how sloppy can we be about getting power into these? If they do 100-strings, can we power the middle of a 200 string? we can right? head end might need it's own power, but it will be a ways away... plus we can deliver data to the middle using pixelrouter. Could we power and data into middle of 2x200? Pinout:
+19v, empty or +5, data, ground, 

## type 3
COTS 100-string fairly lights, standard 3p in WITH THE WRONG FUCKING gender - side that supplies power must always be female, never male! will make adapters to reverse it, and heat-shrink them on or swap connectors. 
