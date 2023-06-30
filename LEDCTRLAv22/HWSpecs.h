#include "LightCtrl_RevF.h"
#ifndef SKETCH_MAIN_H


#define SKETCH_MAIN_H
typedef struct hardwarespecs {
  uint8_t HW_Version;     //  starts at 0x1402 in the unified address space.
  uint8_t Seq_Number;     //  Running tally of Dx-based boards produced.
  uint8_t LCD_flags;      //
  uint8_t RF_Flags;       //  (Identifies the type and type )
  uint8_t Basic_Features; //  (tentative - circut protection and connections present)
  uint8_t UI_connections; //  (tentative - invert encoders and such)
  uint8_t data_config;    //  Allows disabling serial, and configuring extend ports
  uint8_t RF_config;      //  Address and related options
  uint8_t extensionconfig[8];
} hardware_t; // total 16 bytes; plus 2 before and after, leaving 12 bytes at end of user row for other requirements.

extern hardware_t Hardware;


  /* Stored form is bitwise inverted: Empty cells contain 0xFF, and that is inverted to 0x00;
   * Must be preceeded by the CRC of the serial number, and followed by the CRC of the magic number + hw version + seq_number to proove that the data there was config data.


  /* LCD flags:
   *  0x01: Has color backlight.
   *  0x02: Has negative image
   *  0x04: Contrast can be configured by DAC.
   *  0x08: No contrast needs to be set.
   *  if (Hardware.LCDFlags & 0x0C == 0, you need an external pot. But you know that because we rely on you telling us this stuff. )
   *  0x10: Reserved
   *  0x20: Reserved
   *  0x40: Reserved
   *  0x80: Reserved
   *
   */

  #if defined(SKETCH_MAIN)
    PROGMEM_MAPPED char names_bl_mode[2][6] = {"Color", "Mono."};
    PROGMEM_MAPPED char names_contrast[4][7] = {"Manual", "DAC", "Fixed"};
  #endif

  /*  RF Info:
   *  Turn pullup on data pin on. Check if any transitions are observed. If none seen conclude we don't have one.
   *  0x80: Reserved
   *  0x40: Reserved
   *  0x10-0x20: Antenna configuration
   *      0b00 = no antenna
   *      0b01 = wire whip
   *      0b10 = Ceramic antenna
   *      0b11 = PCB antenna
   *  0x00-0x0F: Receiver hardware:
   *      0b000 No receiver hardware
   *      0b001 Synoxo SYN470/480
   *      0b010 STX882
   *      0b011 Other super het
   *      0b100 superregenerative
   *      0b101 H5V4D
   *      0b110 STX887
   *      0b111 Reserved
   *     0b1xxx Reserved
   * 0bxxaa0000 (antenna reported but no receiver hardware) is given to show certain errors. aa=
   * 0bxx000xxx (Everything is normal)
   * 0bxxxx0hhh (hhh != 000 - everyhing is normal.
   * 0bxx010000 We have receiving hardware, but we are unable to use it. Possibly the frequencty is unsupported, or a key #define is missing?
   * 0bxx100000 We don't have receiving hardware, but couldn't use it anyway because unsupported F_CPU.
   * 0bxx110000 Reserved for another state in which RF is non-functional.
   * 0bxxxx1xxx Reserved - all code points with a 1 here are reserved for future use, which may include either new harware, or new errors.

   */
  #define RECEIVER_RESERVED1_bm           (0x40)
  #define RECEIVER_RESERVED2_bm           (0x80)
  #define RECEIVER_RESERVED3_bm           (0x04)
  #define RECEIVER_RESERVED1_bp           (6)
  #define RECEIVER_RESERVED2_bp           (7)
  #define RECEIVER_RESERVED3_bp           (3)
  #define RECEIVER_RESERVED_gm            (0xC4)
  #define MASK_RECEIVER(rflags)           (rflags & ~RECEIVER_RESERVED_gm) /* filters out the reserved bits; all below constants assume this was done, as these bits may be used in teh future */
  #define RECEIVER_ERROR_FREQ_gc          (0x10)
  #define RECEIVER_ERROR_NOHW_OR_FREQ_gc  (0x20)
  #define RECEIVER_ERROR_OTHER_gc         (0x30)
  #define RECEIVER_ANTTYPE_gm             (0x30)
  #define RECEIVER_NOANT_gc               (0x00)
  #define RECEIVER_WHIPANT_gc             (0x10)
  #define RECEIVER_CHIPANT_gc             (0x20)
  #define RECEIVER_PCBANT_gc              (0x30)
  #define RECEIVER_NoRFRX_gc              (0x00)
  #define RECEIVER_TYPE_gm                (0x07)
  #define RECEIVER_SYN4x0_gc              (0x01)
  #define RECEIVER_SRX882_gc              (0x02)
  #define RECEIVER_SuperHet_gc            (0x03) // generic superhet receiver
  #define RECEIVER_SuoerReg_gc            (0x04) // generic supetregenerative receiver
  #define RECEIVER_H5V4D_gc               (0x05)
  #define RECEIVER_SRX887_gc              (0x06)
  #define RECEIVER_TBD_gc                 (0x07)
  #if defined(SKETCH_MAIN)
    const PROGMEM_MAPPED char  names_antennae[4][5] = {"None", "Whip", "Chip", "PCB"};
    const PROGMEM_MAPPED char  names_errors[4][9]={"","Bad FCPU", "All bad!", "Unknown"}
    const PROGMEM_MAPPED char  names_receivers[8][9] = {"None", "SYN470", "SRX882", "SuperHet", "SuperReg", "H5V4D", "STX887", "Other"};
  #else
    extern char names_antennae[4][5];
    extern char names_errors[4][5];
    extern char names_receivers[8][9];
  #endif
  /*  Basic_Features:
   *  Only 0x44 should be seen.
   *      0x01: 3 wire output present
   *      0x02  4 wire output present
   *      0x04  6 wire output present
   *      0x08  reserved
   *      0x10  3 wire has internal protection
   *      0x20  4 wire has internal protection
   *      0x40  6 wire has internal protection
   *      0x80  reserved
   *
   *  UI OPTs: Directions can optionally be reversed and the functions of the two encoders swapped.
   *  0x01 Reverse left encoder
   *  0x02 reverse right encoder
   *  0x04 swap encoders
   *
   *   Normally in order
   *  0bxx000xxx menu highest
   *  0bxx001xxx
   *  0bxx010xxx swap menu and left buttons
   *  0bxx011xxx swap menu and left buttons and reverse encoder button functions
   *  0bxx100xxx swap menu and right button.
   *  0bxx101xxx swap menu and right buttons and swap button functions.
   *  0b01xxxxxx: C7 debug present
   *  0b100xxxxx:
   *
   *  Configuration: Data Control
   *  First two bits: Comms Control:
   *  1. Disable  on Serial (and hence no serial control).
   *  2. enable RX duplex two wire downstream serial.
   *
   *  Configuration: RF
   *  0bOSaaaaaa
   *  a: 6-bit address
   *  S: Spy - output all messages to serial, regardless of what is done with them.
   *  O: Obedient mode - obey all commands, regardless of what address they targeted;
   *
   *  Extension configuration
   *
   *
   *
   *
   *
   */
#endif
