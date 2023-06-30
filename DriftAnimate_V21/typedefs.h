/* Basic functions and datatypes */


/*****************************\
| TYPEDEFs that we need later |
\____________________________*/

typedef enum {
  HWClassOLS=0x00000,
  HWClassE = 0x01000,
  HWClassF = 0x02000,
  HWClassG = 0x02001, //Changes are trivial.
  HWClassH = 0x02002
} hwclass_t;

typedef struct _az_rgb_t{
  uint8_t red,
  uint8_t green,
  uint8_t blue
} az_rgb_t;


typedef struct _az_rgb_t{
  uint8_t red,
  uint8_t green,
  uint8_t blue,
  uint8_t white
} az_rgbw_t;


typedef struct _az_wwa_t{
  uint8_t neutral,
  uint8_t warm,
  uint8_t amber
} az_wwa_t;


typedef struct _az_wcw_t{
  uint8_t warm,
  uint8_t cool
} az_wcw_t;

typedef union _min_section {
  uint8_t raw[12],
  _az_rgb_t rgb[8],
  _az_rgb_t wwa[8],
  _az_rgb_t rgbw[8],
  _ag_wcw_t rgbw
}

/*****************************
 Smallest common denominator:
4 x WCW or 2x RGBW
12x WCW or 6x RGBW or 8xRGB

6x WCW or 3xRGBW or 4x RGB



******************************/
/* A major breakthrough in negotiations
 * with the local Variables Union
 * has led to a deal that will bring
 * the combined services of integral datatypes
 * and ponters to them to the literally several
 * users of this code.
 */


typedef union _flex_ptr {
  uint8_t[2] b,
  uint16_t w,
  uint16_t * wptr,
  uint8_t * bptr,
  void * vptr;
} flex_ptr_t;

typedef union _vol_flex_ptr {
  uint8_t[2] ab,
  uint16_t aw,
  volatile uint8_t * bptr,
  volatile uint16_t * wptr
} vol_flex_ptr_t;

/* this makes compiler do a markedly better job at this common task than usual */
typedef union _bw16{
  uint8_t b[2],
  uint16_t w,
} bw_t;
/* Simpler way to write words bytewise. */

typedef union _bwlf {
  uint32_t l,
  uint16_t w[2],
  uint8_t b[4],
  float f
} bwlf_t;
