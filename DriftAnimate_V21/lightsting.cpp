//we detect length with a passive resistor between ground and ID pin. These are the key values that serve as divisions between them

typedef enum color_enum {
  GRB = 0x01
  GBR = 0x02
  RGB = 0x03
  RBG = 0x04
  BGR = 0x05
  BRG = 0x06
  RGBW = 0x07
  RGBWW = 0x08
}

const MAPPED_PROGMEM leds_per_string={}

/* Resistance:                                    ~0,    6800,     10k,     15k,  22k,  33k,  39k,  47k,  56k,  68k  100k  150k  220k  470k  More]
const MAPPED_PROGMEM uint16_t length_buckets =  {  0,     344,     983,    1471, 1877, 2173, 2328, 2503, 2675, 2808, 2988, 3272, 3495, 3716, 3971, 4096];
const MAPPED_PROGMEM uint8_t colororders =      {GRB, INVALID, INVALID, INVALID, 1877, 2173, 2328, 2503, 2675, 2808, 2988, 3272, 3495, 3716, 3971, 4096};
const MAPPED_PROGMEM uint16_t lengths =         {200,      -1,      -1,      -1, 1877, 2173, 2328, 2503, 2675, 2808, 2988, 3272, 3495, 3716, 3971, 4096};
const length_buckets =  {0, 344, 983, 1471, 1877, 2173, 2328, 2503, 2675, 2808, 2988, 3272, 3495, 3716, 3971, 4096];

typedef struct Light_struct {
  int16_t length;
  colororder_t order;
} Light_t
