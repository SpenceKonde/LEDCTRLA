#ifndef __Modes_h
#define __Modes_h

typedef struct modeset {          // Size //
  char    modename[9];            //    9 //  name of mode
  uint8_t frame_delay_base;       //    1 //
  uint8_t frame_delay_incr;       //    1 //
  uint8_t color_select_mode;      //    1 //  used by getNextColor() - 0 = special or not used 1 = output the first three setting values verbatim. 2 = Output color with channels between first and last of settings 1-6. 63 = use Colorset. High bit = random, else sequential
  uint8_t animate_mode;           //    1 //  Which function do we call 0 = special or not used 1 = fill; 2 = pushPixel
  uint8_t direction_opt;          //    1 //  Which of the option adjustments is the direction? 255 = not used
  int8_t  speed_opt;              //    1 //  Which of the option adjustments is the speed? 255 = not used
  uint8_t color_mode;             //    1 //  128 = uses the colorsets, 0 indicaes no options. Otherwise, this is the number if color settings this mode has.
  char    (*color_names[8]);         //    2 //  Pointer to the list of length = 8 strings containing mode names.
  uint8_t color_max[10];          //   10 //  at most 6 used currently. The minimum for a color setting is always zero.  If this is = ColorAdjustMax, settings will display those numbers.
  uint8_t color_defaults[10];     //   10 //  255 = random
  uint8_t adjust_count;           //    1 //  0-9 - the number of adjustments on the non-color-setting knob.
  char    adjust_names[10][8];    //   80 //  Array of strings naming the modes.
  uint8_t adjust_min[10];         //   10 //  For each settingm that is used, minimum and maximum are provided.
  uint8_t adjust_max[10];         //   10 //
  uint8_t adjust_default[10];     //   10 // The initial state is also specified. 255 = random
} modeset_t;

#define modeset_size 169// 39 + 80 + 30 = 169

typedef struct controllstate {
  uint8_t modenum;
  mode_t *mode;
  volatile uint8_t menuleft;          // Index of current color menu
  volatile uint8_t menuright;         // Index of curent setting menu
  volatile uint8_t values_left[10];   // the current values that the colors menus are set to. For color_mode == 128, only values_left[0] is used.
  volatile uint8_t values_right[10];  // the curret values that the options menus are set to
} controlstate_t;                     /* This is what gets saved and restored */


typedef struct ledzone {
  uint16_t length;
  uint8_t flags;
  uint8_t node;
} zonespec_t;

typedef struct ledsstate {
  volatile uint8_t frame_delay; // frame delay currently being used.
  zonespec_t zones[32]; // Size of each zone in pixels
  colorset_t *colors; // Pointer to color set, if used.
  uint16_t position;
} globalstate_t; // changing or calculated values that don't get saved.

extern globalstate_t LedState;
extern controlstate_t Mode;
extern modeset_t modes[];

#endif
