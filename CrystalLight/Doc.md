/* Modes:
   0 constant color
   1 spin
   2 fade
   3 wave
*/

0 - constant color
All pixels are set to the specified color. 

1 - spin

Each ring will spin as specified by spinMode
High 4 bits are for outer ring, low 4 bits are for inner ring:
spinMode:
0 - Don't spin this ring
1 - Spin forwards
2 - Spin backwards
3 - make this match the closest pixels on the other ring


2. fade

Fades through the specified colors (see below). 
dwellFrames - pause this many frames when on a color
transitionFrames - transition between colors within this many frames. 

3. wave

Wave through the specified colors, rotating them into the outer ring and smoothing inner ring to match.
dwellFrames - pause this many frames when on a color
transitionFrames - transition between colors within this many frames. 

4. drift 

Drift (like my other LED projects). 
driftMode: 
0. 
1. drift inner ring only. 
2. drift outer ring only. 
3. drift both rings independently. 
4. drift each inner pixel indepentently, smooth outward

4th bit - 0 = each channel drifts randomly, 

5. pan
Specified colors enter from one side along outer ring, and move laterally across the ring, inner ring smoothed to match. 



Color pallet: 
waveColorMode is an 8x3 array, up to 8 colors can be specified. Used for many modes. 
waveColorCount is the number of these colors that are specified at present

