#include "LightCtrl_RevF.h"
#include <stdint.h>
const PROGMEM_MAPPED char  basiccolor[9][8]      = {" RED 1 ", "GREEN 1", " BLUE 1"," RED 2 ", "GREEN 2", " BLUE 2", " RED 3 ", "GREEN 3", " BLUE 3"};
const PROGMEM_MAPPED char simplecolor[3][8]      = {" RED   ", "GREEN  ", " BLUE  "};
const PROGMEM_MAPPED char minmaxcolor[6][8]      = {"MIN RED", "MAX RED", "MIN GRN", "MAX GRN", "MIN BLU", "MAX BLU"};
const PROGMEM_MAPPED char    mincolor[6][8]      = {"MIN RED", "MAX RED", "MIN GRN"};
const PROGMEM_MAPPED char    maxcolor[6][8]      = {"MAX GRN", "MIN BLU", "MAX BLU"};
const PROGMEM_MAPPED char nullsetting[1][8]      = {"       "};

#define RAND        255
/*

frame_delay_base,
At minimum speed, this is the delay between frames, minimum.

frame_delay_incr,
At minimum speed, this is the delay added for each notch on the speed setting.

color_select_mode,
// Color options */
#define COL_NONE       0
#define COL_SPECIAL    0 //
#define COL_DIRECT  0x01 // First three values are R, G and B
#define COL_MINMAX  0x02 // first two colors are min and max red, next two are green, etc
#define COL_SETS    0x80 // Use the color palletes
#define COL_RANDOM  0x40 // new colors are picked randomly instead of sequentially
#define COL_RA_MM   0x42 // COL_RANDOM | COL_MINMAX - when we choose a color, pick a random color between the bounds set by the min/max controls
#define COL_RA_SET  0xC0 // COL_RANDOM | COL_SETS - when we choose a color, pick a random color within the set.
#define COL_RA_ABS  0x41 // COL_RANDOM | COL_DIRECT - when we choose a color, let R, G and B = random numbers
/*
animate_mode,
// Animate Mode Options */
#define ANI_NONE      0
#define ANI_SPECIAL   0
#define ANI_FLOOD     1
#define ANI_PUSH      2

/*
direction_opt;
// This is the number of the option that controls the direction of reversable */
#define DIR_NONE    255
#define DIR_ZERO      0
#define DIR_ONE       1
#define DIR_TWO       2
#define DIR_THREE     3
#define DIR_FOUR      4
#define DIR_FIVE      5
#define DIR_SIX       6
#define DIR_SEVEN     7
#define DIR_EIGHT     8
#define DIR_NINE      9

/*
speed_opt;
// Speed adjustment option number */
#define SPD_NONE    255
#define SPD_ZERO      0
#define SPD_ONE       1
#define SPD_TWO       2
#define SPD_THREE     3
#define SPD_FOUR      4
#define SPD_FIVE      5
#define SPD_SIX       6
#define SPD_SEVEN     7
#define SPD_EIGHT     8
#define SPD_NINE      9

/* Color Option Options*/
// Number of color option. Not more than 10.
#define COLOP_NOOPTS    0
#define COLOP_SETS    128
#define COLOP_MINMAX    6
#define COLOP_MAX       3
/*{typedef struct modeset {          // Size //
 {modename[9], frame_delay_base, frame_delay_incr, color_select_mode, animate_mode, direction_opt; speed_opt; uint8_t color_mode; *color_names
  color_max[10];         =
  color_defaults[10];     //   10 //  255 = random
  adjust_count;           //    1 //  0-9 - the number of adjustments on the non-color-setting knob.
  adjust_names[10][8];    //   80 //  Array of strings naming the custom adjustment options.
  adjust_min[10];         //   10 //  For each settingm that is used, minimum and maximum are provided.
  adjust_max[10];         //   10 //
  adjust_default[10];     //   10 //  The initial state is also specified. 255 = random
} modeset_t;
*/

const PROGMEM_MAPPED modeset_t modes[] ={
  {" SOLID  ", 100,  0,  COL_DIRECT,   ANI_FLOOD, DIR_NONE, SPD_NONE,    COLOP_MAX, &basiccolor,
                    {   31,   31,   31,    0,    0,    0,    0,    0,    0,   0},
                    {  255,  255,  255,    0,    0,    0,    0,    0,    0,   0},
                0, {{"        "},{"       "},{"       "},{"       "},{"       "},
                    {"        "},{"       "},{"       "},{"       "},{"       "}},
                    {    0,    0,    0,    0,    0,    0,    0,    0,    0,   0},
                    {    0,    0,    0,    0,    0,    0,    0,    0,    0,   0},
                    {    0,    0,    0,    0,    0,    0,    0,    0,    0,   0},
  },
  {"DRIFTING",  20,  3, COL_SPECIAL, ANI_SPECIAL, DIR_NONE, SPD_ZERO, COLOP_MINMAX, &minmaxcolor,
                    {   31,   31,   31,   31,   31,   31,    0,    0,    0,   0},
                    {  255,  255,  255,  255,  255,  255,    0,    0,    0,   0},
               1,  {{"SPEED   "},{"       "},{"       "},{"       "},{"       "},
                    {"        "},{"       "},{"       "},{"       "},{"       "}},
                    {    1,    0,    0,    0,    0,    0,    0,    0,    0,   0},
                    {   10,    0,    0,    0,    0,    0,    0,    0,    0,   0},
                    {    5,    0,    0,    0,    0,    0,    0,    0,    0,   0}
  },
  {" PULSE  ",  20,  3,    COL_SETS, ANI_SPECIAL, DIR_NONE, SPD_ZERO, COLOP_MINMAX, &minmaxcolor,
                    {   31,   31,   31,   31,   31,   31,    0,    0,    0,   0},
                    {  255,  255,  255,  255,  255,  255,    0,    0,    0,   0},
              1,   {{"SPEED   "},{"       "},{"       "},{"       "},{"       "},
                    {"        "},{"       "},{"       "},{"       "},{"       "}},
                    {    1,    0,    0,    0,    0,    0,    0,    0,    0,   0},
                    {   10,    0,    0,    0,    0,    0,    0,    0,    0,   0},
                    {    5,    0,    0,    0,    0,    0,    0,    0,    0,   0}
  },
  {" RAINBOW",  20,  3,    COL_SETS,    ANI_PUSH,  DIR_ONE, SPD_ZERO, COLOP_NOOPTS, &nullsetting,
                    {    0,    0,    0,    0,    0,    0,    0,    0,    0,   0},
                    {    0,    0,    0,    0,    0,    0,    0,    0,    0,   0},
                1, {{"SPEED   "},{"<----->"},{"       "},{"       "},{"       "},
                    {"        "},{"       "},{"       "},{"       "},{"       "}},
                    {    1,    0,    0,    0,    0,    0,    0,    0,    0,   0},
                    {   10,    1,    0,    0,    0,    0,    0,    0,    0,   0},
                    {    5,    0,    0,    0,    0,    0,    0,    0,    0,   0}
  },
  {" DOTS2  ",  20,  3,    COL_SETS,    ANI_PUSH,  DIR_TWO, SPD_ZERO,   COLOP_SETS, &nullsetting,
                    {    0,    0,    0,    0,    0,    0,    0,    0,    0,   0},
                    {    0,    0,    0,    0,    0,    0,    0,    0,    0,   0},
                3, {{"SPEED   "},{"DENSITY"},{"<----->"},{"       "},{"       "},
                    {"        "},{"       "},{"       "},{"       "},{"       "}},
                    {    1,    1,    0,    0,    0,    0,    0,    0,    0,   0},
                    {   12,   15,    1,    0,    0,    0,    0,    0,    0,   0},
                    {    5,    4,    0,    0,    0,    0,    0,    0,    0,   0}
  },
  {" FADE2  ",  20,  3,    COL_SETS,   ANI_FLOOD, DIR_NONE, SPD_ZERO,   COLOP_SETS, &nullsetting,
                    {    0,    0,    0,    0,    0,    0,    0,    0,    0,   0},
                    {    0,    0,    0,    0,    0,    0,    0,    0,    0,   0},
                1, {{"SPEED   "},{"       "},{"       "},{"       "},{"       "},
                    {"        "},{"       "},{"       "},{"       "},{"       "}},
                    {    1,    0,    0,    0,    0,    0,    0,    0,    0,   0},
                    {   10,    0,    0,    0,    0,    0,    0,    0,    0,   0},
                    {    5,    0,    0,    0,    0,    0,    0,    0,    0,   0}
  },
  {" DRIFT2 ",  20,  0,    COL_SETS, ANI_SPECIAL, DIR_NONE, SPD_ZERO,   COLOP_SETS, &nullsetting,
                    {    0,    0,    0,    0,    0,    0,    0,    0,    0,   0},
                    {    0,    0,    0,    0,    0,    0,    0,    0,    0,   0},
                2, {{"SPEED   "},{"DENSITY"},{"       "},{"       "},{"       "},
                    {"        "},{"       "},{"       "},{"       "},{"       "}},
                    {    1,    0,    0,    0,    0,    0,    0,    0,    0,   0},
                    {   10,    0,    0,    0,    0,    0,    0,    0,    0,   0},
                    {    5,    0,    0,    0,    0,    0,    0,    0,    0,   0}
  },
  {"  WAVE  ",  20,  3,    COL_SETS,    ANI_PUSH, DIR_FOUR, SPD_ZERO,   COLOP_SETS, &nullsetting,
                    {    0,    0,    0,    0,    0,    0,    0,    0,    0,   0},
                    {    0,    0,    0,    0,    0,    0,    0,    0,    0,   0},
                4, {{"SPEED   "},{"DWELL  "},{"LENGTH "},{"<----->"},{"       "},
                    {"        "},{"       "},{"       "},{"       "},{"       "}},
                    {    1,    0,    1,    0,    0,    0,    0,    0,    0,   0},
                    {   10,   30,   30,    1,    0,    0,    0,    0,    0,   0},
                    {    5,    1,   10,    0,    0,    0,    0,    0,    0,   0}
  },
  {"  FADE  ",  20,  3,  COL_DIRECT,   ANI_FLOOD, DIR_NONE, SPD_ZERO,  COLOP_MINMAX, &basiccolor,
                    {   31,   31,   31,   31,   31,   31,    0,    0,    0,   0},
                    {  255,  255,  255,  255,  255,  255,    0,    0,    0,   0},
                2, {{"SPEED   "},{"DWELL  "},{"       "},{"       "},{"       "},
                    {"        "},{"       "},{"       "},{"       "},{"       "}},
                    {    1,    0,    0,    0,    0,    0,    0,    0,    0,   0},
                    {   10,   30,    0,    0,    0,    0,    0,    0,    0,   0},
                    {    5,    1,    0,    0,    0,    0,    0,    0,    0,   0}
  }
};
const byte maxMode = 10;
/*


typedef struct modeset {          // Size //
  char    modename[9];            //    9 //  name of mode
  uint8_t frame_delay_base;       //    1 //
  uint8_t frame_delay_incr;       //    1 //
  uint8_t color_select_mode;      //    1 //  used by getNextColor() - 0 = special or not used 1 = output the first three setting values verbatim. 2 = Output color with channels between first and last of settings 1-6. 63 = use Colorset. High bit = random, else sequential
  uint8_t animate_mode;           //    1 //  Which function do we call 0 = none - static color, 1 = fill with a color; 2 = push/pull pixels.
  uint8_t direction_opt;          //    1 //  Which of the option adjustments is the direction? 255 = not used
  uint8_t speed_opt;              //    1 //  Which of the option adjustments is the speed? 255 = not used
  uint8_t color_mode;             //    1 //  128 = uses the colorsets, 0 indicaes no options. Otherwise, this is the number if color settings this mode has.
  char    (*color_names);         //    2 //  Pointer to the list of length = 8 strings containing mode names.
  uint8_t color_max[10];          //   10 //  at most 6 used currently. The minimum for a color setting is always zero.  If this is = ColorAdjustMax, settings will display those numbers.
  uint8_t color_defaults[10];     //   10 //  255 = random
  uint8_t adjust_count;           //    1 //  0-9 - the number of adjustments on the non-color-setting knob.
  char    adjust_names[10][8];    //   80 //  Array of strings naming the modes.
  uint8_t adjust_min[10];         //   10 //  For each settingm that is used, minimum and maximum are provided.
  uint8_t adjust_max[10];         //   10 //
  uint8_t adjust_default[10];     //   10 // The initial state is also specified. 255 = random
} modeset_t;

#define modeset_size 169      // 39 + 80 + 30 = 169

typedef struct controllstate {
  uint8_t modenum;            // Index of current mode
  mode_t *mode;               // pointer to current mode
  uint8_t menuleft;           // Index of current color menu
  uint8_t menuright;          // Index of curent setting menu
  uint8_t values_left[10];    // the current values that the colors menus are set to. For color_mode == 128, only values_left[0] is used.
  uint8_t values_right[10];   // the curret values that the options menus are set to.
} controlstate_t;

typedef struct ledsstate {
  uint8_t frame_delay; // frame delay currently being used.
  zonespec_t zones[32]; // Size of each zone in pixels
  colorset_t *colors; // Pointer to color set, if used.
  uint16_t position;
} globalstate_t; // changing or calculated values that don't get saved.

*/
globalstate_t LedState ={
  100,
  {

  },
  NULL,
  0;

};

controlstate_t Mode = {
  0,
  &modes[0],
  0,
  0,
  {0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0}
};

uint8_t getFrameDelay() = {
  uint8_t retval=Mode.mode -> frame_delay_base;
  if (Mode.mode -> speed_opt != 255) {
    retval -= Mode.mode -> frame_delay_incr * values_right[Mode.mode -> speed_opt]
  }
  return retval;
}

uint8_t getDirection() = {
  uint8_t dir = Mode.mode -> direction_opt;
  if (dir != 255) {
    return Mode.values_right[dir]
  }
  return 0;
}

uint8_t getColorMenuNumber() {
  return Mode.menu_left;
}
uint8_t getColorMenuSelection() {
  return Mode.values_left;
}

char* getColorMenuName() {
  if (Mode.mode -> color_select_mode == 128) {
    return getColorName(Mode.values_left[0]);
  } else {
    char[8]* temp = Mode.mode -> color_names;
    return temp + Mode.menu_left;
  }
}

uint8_t getAdjustMenuNumber() {
  return Mode.menu_right;
}

uint8_t getAdjustMenuSelection() {
  return Mode.values_right[Mode.menu_right];
}

char* getAdjustOptionName() {
  return &(Mode.mode -> adjustNames[Mode.menu_right])
}

bool adjustValue(int8_t change) {
  // change = 0, -1 or 1
  if (IsSet(ADJUST_CHANGE) || IsSet(MODE_CHANGE) || !change) {
    return 0;
  } else if ((Mode.mode -> adjust_max[Mode.menuright] == 1) && (Mode.mode -> adjust_min[Mode.menuright] == 1)) {
    // This means the max is 1; and thus min is 1 and this is a toggle.
    uint8_t val = Mode.values_right[Mode.menuRight];
    if (val) {
      Mode.values_right[Mode.menuRight] = 0;
    } else {
      Mode.values_right[Mode.menuRight] = 1;
    }
    return 1;
  } else {
    if (change > 0) {
      if ((Mode.mode))
    } else { //already returned if change == 0 so change < 0
      if ((Mode.mode -> adjust_max[Mode.menuright])) {

      }
    }
  }
}

void scrollColorMenu() {
  if ((Mode.mode -> color_mode & 0x7F) == 0) {
    SysClr(COLOR_CHANGE);
    return 0;
  }
  Mode.menu_left++;
  if (Mode.menu_left >= (Mode.mode -> color_mode & 0x7F)) {
    Mode.menu_left = 0;
  }
  SysClr(COLOR_CHANGE);
  SysSet(UI_CHANGE);
  return 1;
}

bool scrollAdjustMenu() {
  if (Mode.mode -> adjust_count == 0) {
    SysClr(ADJUST_CHANGE);
    return 0;
  }
  Mode.menu_right++;
  if (Mode.menu_right >= Mode.mode -> adjust_count) {
    Mode.menu_right = 0;
  }
  SysClr(ADJUST_CHANGE);
  SysSet(UI_CHANGE);
  return 1;
}

void scrollMode() {
  newmode = Mode.modenum + 1;
  if (newmode >= maxMode) {
    newmode = 0;
  }
  setMode(newmode);
}

bool setMode(uint8_t num) {
  if (num >= maxMode) {
    SysClr(MODE_CHANGE);
    return 0;
  }
  Mode.modenum = num;
  Mode.mode = &modes[num];
  Mode.modeleft = 0;
  Mode.moderight = 0;
  uint8_t left = modes[num].color_mode & 0x7F;
  uint8_t right = modes[num].adjust_count;
  for (byte i = 0; i < 10; i++) {
    if (i < left) {
      if (modes[num].color_default[i] == 255) {
        Mode.values_left[i] = math.random(0, modes[num].color_max[i]);
      } else {
        Mode.values_left[i] = modes[num].colorDefault[i];
      }
    }
    if (i < right) {
      if (modes[num].adjust_default[i] == 255) {
        Mode.values_right[i] = math.random(modes[num].adjust_min[i], modes[num].adjust_max[i]);
      } else {
        Mode.values_right[i] = modes[num].color_default[i];
      }
    }
  }
  SysClr(MODE_CHANGE);
  SysSet(UI_CHANGE);
}
