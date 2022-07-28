#include "Colors.h"
#include <Arduino.h>


typedef struct colorset_t {
  char colorname[8];    //8
  uint8_t colorcount;   //1
  uint8_t colors[8][3]; //24
} colorset_t;

PROGMEM_MAPPED colorset_t ColorTable[] = {
  {colors_RAINBOW},
  {colors_WARM},
  {colors_COOL},
  {colors_SUNSET},
  {colors_LIZARD},
  {colors_SEXTIME},
  {colors_FIRE},
  {colors_USA},
  {colors_JUNGLE},
  {colors_XMAS},
  {colors_PUMPKIN},
  {colors_PURPLE},
  {colors_STATIC}
};
