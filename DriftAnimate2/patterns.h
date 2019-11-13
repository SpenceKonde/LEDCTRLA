#define MODE_DRIFT2 9

#ifndef MegaTinyCore
#define pgmem PROGMEM
#else
#define pgmem
#endif

const byte colorPallete[][8][3] PROGMEM = {
  {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, //Rainbow
  {{255, 96, 32}, {255, 100, 0}, {240, 128, 40}, {255, 32, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, //Warm
  {{255, 0, 64}, {0, 64, 255}, {0, 210, 160}, {160, 0, 160}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, //Cool
  {{255, 80, 20}, {255, 100, 0}, {196, 16, 64}, {160, 0, 64}, {255, 32, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, //sunset
  {{0, 255, 0}, {0, 128, 16}, {0, 32, 0}, {64, 160, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},  //Lizard
  {{255, 0, 16}, {196, 0, 64}, {255, 16, 100}, {255, 0, 64}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, //sextime
  {{255, 64, 0}, {196, 32, 0}, {220, 64, 0}, {255, 16, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, //fire
  //{{255, 0, 0}, {128, 128, 128}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, //USA
  {{0, 255, 0}, {32, 255, 0}, {0, 255, 32}, {32, 255, 32}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, //Jungle
  //{{255, 0, 0}, {0, 0, 0}, {0, 255, 0}, {0, 0, 0}, {255, 200, 64}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}} //xmas
  {{255, 64, 0}, {196, 32, 0}, {220, 64, 0}, {32, 255, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, //Pumpkin
  {{192, 0, 96}, {196, 0, 64}, {255, 16, 100}, {255, 0, 64}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, //Purples
  {{160,160,140}, {228,228,200}, {128,128, 110}, {60, 60, 50}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}} //static
};
const byte colorCount[] PROGMEM = {3, 4, 4, 5, 4, 4, 4, 4, 4,4,4};

const char pallete0[] PROGMEM = "RAINBOW";
const char pallete1[] PROGMEM = "  WARM ";
const char pallete2[] PROGMEM = "  COOL ";
const char pallete3[] PROGMEM = "SUNSET ";
const char pallete4[] PROGMEM = "LIZARD ";
const char pallete5[] PROGMEM = "SEXTIME";
const char pallete6[] PROGMEM = "  FIRE ";
//const char pallete7[] PROGMEM = "  USA  ";
const char pallete7[] PROGMEM = "JUNGLE ";
//const char pallete8[] PROGMEM = "  XMAS ";
const char pallete8[] PROGMEM = "PUMPKIN";
const char pallete9[] PROGMEM = "PURPLE ";
const char pallete10[] PROGMEM ="STATIC ";

const char * const palleteNames[] PROGMEM = {pallete0, pallete1, pallete2, pallete3, pallete4, pallete5, pallete6, pallete7,pallete8,pallete9,pallete10};

#define PALLETEMAX 10

// Names of mode settings - these get stuffed into modesL and modesR below.
const char mode0L0[] PROGMEM = "  RED  ";
const char mode0L1[] PROGMEM = " GREEN ";
const char mode0L2[] PROGMEM = "  BLUE ";
const char mode1L0[] PROGMEM = "MIN RED";
const char mode1L1[] PROGMEM = "MAX RED";
const char mode1L2[] PROGMEM = "MIN GRN";
const char mode1L3[] PROGMEM = "MAX GRN";
const char mode1L4[] PROGMEM = "MIN BLU";
const char mode1L5[] PROGMEM = "MAX BLU";
const char mode0R0[] PROGMEM = "       ";
const char mode1R0[] PROGMEM = " SPEED ";
const char mode1R1[] PROGMEM = " NUMBER";
const char mode2R2[] PROGMEM = " LENGTH";
const char mode4R2[] PROGMEM = " DIRECT";
const char mode5R1[] PROGMEM = "DENSITY";
const char mode7R2[] PROGMEM = " DWELL ";
const char mode0Name[] PROGMEM = " SOLID  ";
const char mode1Name[] PROGMEM = "DRIFTING";
const char mode2Name[] PROGMEM = " COMETS ";
const char mode3Name[] PROGMEM = " PULSE  ";
const char mode4Name[] PROGMEM = " RAINBOW";
const char mode5Name[] PROGMEM = "  DOTS  ";
const char mode6Name[] PROGMEM = "  FADE  ";
const char mode7Name[] PROGMEM = "  WAVE  ";
const char mode8Name[] PROGMEM = " DOTS2  ";
const char mode9Name[] PROGMEM = " FADE2  ";
const char mode10Name[] PROGMEM = " DRIFT2 ";


//Names of settings by mode
const char * const modesL[][8] PROGMEM = {
  {mode0L0, mode0L1, mode0L2},
  {mode1L0, mode1L1, mode1L2, mode1L3, mode1L4, mode1L5},
  {mode1L0, mode1L1, mode1L2, mode1L3, mode1L4, mode1L5},
  {mode1L0, mode1L1, mode1L2, mode1L3, mode1L4, mode1L5},
  {mode0R0}, //rainbow
  {mode1L0, mode1L1, mode1L2, mode1L3, mode1L4, mode1L5},
  {mode1L0, mode1L2, mode1L4, mode1L1, mode1L3, mode1L5}, //different order!
  {mode0R0}, //wave
  {mode0R0}, //dots with pallete
  {mode0R0}, //fade with pallete
  {mode0R0} //drift with pallete

};

const char * const modesR[][8] PROGMEM = {
  {mode0R0}, //solid
  {mode1R0, mode1R1}, //drift
  {mode1R0, mode1R1, mode2R2}, //comet
  {mode1R0, mode5R1}, //pulse
  {mode1R0, mode2R2, mode4R2}, //rainbow
  {mode1R0, mode5R1, mode4R2}, //dots
  {mode1R0}, //fade
  {mode1R0, mode2R2, mode7R2, mode4R2}, //wave
  {mode1R0, mode5R1, mode4R2}, //dots with pallete
  {mode1R0, mode2R2, mode7R2, mode4R2}, //fade with pallete
  {mode1R0, mode2R2, mode7R2} //drift with pallete

};

// names of modes
const char * const modeNames[] PROGMEM = {mode0Name, mode1Name, mode2Name, mode3Name, mode4Name, mode5Name, mode6Name, mode7Name, mode8Name, mode9Name, mode10Name};

#define COLORTABLEMAX 31

//max and default settings controlled by left knob. 26 is special, it indicates to use the leftValues array
const byte maxValueLeft[][8] PROGMEM = {
  {COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX},
  {COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX},
  {COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX},
  {COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX},
  {0},
  {COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX},
  {COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX, COLORTABLEMAX},
  {PALLETEMAX},
  {PALLETEMAX},
  {PALLETEMAX},
  {PALLETEMAX}
};
const byte defaultValueLeft[][8] PROGMEM = { //255 is special - indicates to pick random value.
  {255, 255, 255},
  {0, COLORTABLEMAX, 0, COLORTABLEMAX, 0, COLORTABLEMAX},
  {0, COLORTABLEMAX, 0, COLORTABLEMAX, 0, COLORTABLEMAX},
  {0, COLORTABLEMAX, 0, COLORTABLEMAX, 0, COLORTABLEMAX},
  {0},
  {0, COLORTABLEMAX, 0, COLORTABLEMAX, 0, COLORTABLEMAX},
  {255, 255, 255, 255, 255, 255},
  {0},
  {0},
  {0},
  {0}
};

//if above max is COLORTABLEMAX, use this value - otherwise use raw value.
const byte leftValues[COLORTABLEMAX + 1] PROGMEM = {0, 1, 2, 3, 4, 6, 8, 11, 14, 18, 22, 27, 33, 39, 46, 54, 63, 73, 84, 95, 106, 117, 128, 139, 151, 163, 176, 189, 204, 220, 237, 255};

const byte maxValueRight[][8] PROGMEM = {
  {0},
  {10},
  {10, 10, 10},
  {10, 20},
  {10, 10, 1},
  {10, 12, 1},
  {10},
  {10, 40, 20, 1},
  {10, 12, 1},
  {10, 20, 20, 1},
  {10, 20, 20}
};
const byte defaultValueRight[][8] PROGMEM = {
  {0},
  {5},
  {10, 10, 5},
  {5, 5},
  {5, 10, 0},
  {5, 10, 0},
  {5},
  {5, 20, 2, 0},
  {5, 10, 0},
  {5, 10, 2, 0},
  {5, 10, 2}
};
const byte maxSetting[][2] PROGMEM = {
  {2, 0}, //solid
  {5, 0}, //drift
  {5, 2}, //comets
  {5, 1}, //pulse
  {255, 2}, //rainbow
  {5, 2}, //dots
  {5, 0}, //fade
  {0, 3}, //wave
  {0, 2}, //dots2
  {0, 3}, //fade2
  {0, 2} //drift2
};

const byte maxMode = 10;

const byte pulseBrightnessTable[] PROGMEM = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 31, 34, 37, 40, 43, 46, 49, 52, 55, 59, 63, 67, 71, 75, 79, 83, 87, 92, 97, 102, 107, 112, 117, 122, 127, 133, 139, 145, 151, 157, 163, 169, 175, 182, 189, 196, 203, 210, 217, 224, 231, 239, 247, 255};
