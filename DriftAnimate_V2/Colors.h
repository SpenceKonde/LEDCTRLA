
typedef struct colorset_t {
  char colorname[8];    //8
  uint8_t colorcount;   //1
  uint8_t colors[8][3]; //24
} colorset_t;

#define size_colorset (33)

#define colors_RAINBOW  "RAINBOW", 3, {{255,   0,   0},   {  0, 255,   0},   {  0,   0, 255},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0}}
#define colors_WARM     "  WARM ", 4, {{255,  96,  32},   {255, 100,   0},   {240, 128,  40},   {255,  32,   0},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0}}
#define colors_COOL     "  COOL ", 4, {{255,   0,  64},   {  0,  64, 255},   {  0, 210, 160},   {160,   0, 160},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0}}
#define colors_SUNSET   "SUNSET ", 5, {{255,  80,  20},   {255, 100,   0},   {196,  16,  64},   {160,   0,  64},   {255,  32,   0},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0}}
#define colors_LIZARD   "LIZARD ", 4, {{  0, 255,   0},   {  0, 128,  16},   {  0,  32,   0},   { 64, 160,   0},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0}}
#define colors_SEXTIME  "SEXTIME", 4, {{255,   0,  16},   {196,   0,  64},   {255,  16, 100},   {255,   0,  64},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0}}
#define colors_FIRE     "  FIRE ", 4, {{255,  64,   0},   {196,  32,   0},   {220,  64,   0},   {255,  16,   0},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0}}
#define colors_USA      "  USA  ", 3, {{255,   0,   0},   {128, 128, 128},   {  0,   0, 255},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0}}
#define colors_JUNGLE   "JUNGLE ", 4, {{  0, 255,   0},   { 32, 255,   0},   {  0, 255,  32},   { 32, 255,  32},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0}}
#define colors_XMAS     "  XMAS ", 5, {{255,   0,   0},   {  0,   0,   0},   {  0, 255,   0},   {  0,   0,   0},   {255, 200,  64},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0}}
#define colors_PUMPKIN  "PUMPKIN", 4, {{255,  64,   0},   {196,  32,   0},   {220,  64,   0},   { 32, 255,   0},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0}}
#define colors_PURPLE   "PURPLE ", 4, {{192,   0,  96},   {196,   0,  64},   {255,  16, 100},   {255,   0,  64},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0}}
#define colors_STATIC   "STATIC ", 4, {{160, 160, 140},   {228, 228, 200},   {128, 128, 110},   { 60,  60,  50},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0},   {  0,   0,   0}}
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

uint8_t getColorSetCount() {
  return sizeof(ColorTable)/size_colorset;
}
