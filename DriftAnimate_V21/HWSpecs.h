#include "LightCtrl_RevF.h"
#ifndef SKETCH_MAIN_H

#define SKETCH_MAIN_H
typedef struct hardwarespecs {
  uint8_t HW_Version;     //  starts at 0x1402 in the unified address space. PCB rev.
  uint8_t Seq_Number;     //  Running tally of Dx-based boards produced.
  uint8_t LCD_flags;      //
  uint8_t RF_Flags;       //
  uint8_t Basic_Features; //  (tentative - circut protection and connections present)
  uint8_t UI_connections; //  (tentative - invert encoders and such)
  uint8_t data_config;    //
  uint8_t RF_config;      //
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
   *  0x10: Reserved
   *  0x20: Reserved
   *  0x40: Reserved
   *  0x80: Reserved
   *
   */

  PROGMEM_MAPPED char  names_bl_mode[2][6] = {"Color", "Mono."};
  PROGMEM_MAPPED char  names_contrast[4][7] = {"Manual", "DAC", "Fixed"};

  /*  RF Info:
   *
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
   */

  #define RECEIVER_NoRFRX = 0,
  #define RECEIVER_SYN4x0 = 1,
  #define RECEIVER_SRX882 = 2,
  #define RECEIVER_SRX887 = 3,
  #define RECEIVER_SuperHet = 4, // generic superhet receiver
  #define RECEIVER_SuoerReg = 5, // generic supetregenerative receiver
  #define RECEIVER_H5V4D  = 6
  PROGMEM_MAPPED char  names_antennae[4][5] = {"None", "Whip", "Chip", "PCB"};
  PROGMEM_MAPPED char  names_receivers[8][9] = {"None", "SYN470", "SRX882", "SuperHet", "SuperReg", "H5V4D", "STX887", "Other"};

  /*  Basic_Features:
   *      0x01: 3 wire output present
   *      0x02  4 wire output present
   *      0x04  6 wire output present
   *      0x08  reserved
   *      0x10  3 wire has internal protection
   *      0x20  4 wire has internal protection
   *      0x40  6 wire has internal protection
   *      0x80  reserved
   *
   *  Low level configuration:
   *  0x01 Reverse left encoder
   *  0x02 reverse right encoder
   *  0x04 swap encoders
   *  0bxx000xxx default button order
   *  0bxx001xxx swap the two encoder buttons
   *  0bxx010xxx swap menu and left buttons
   *  0bxx011xxx swap menu and left buttons and reverse encoder button functions
   *  0bxx100xxx swap menu and right buttons
   *  0bxx101xxx swap menu and right buttons and swap button functions.
   *  0s40:
   *  0x80:
   *
   *  Configuration: Data Control
   *  First two bits: Comms Control:
   *  1. Disable RS on Serial (and hence no serial control).
   *  2. enable full duplex two wire downstream serial.
   *
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
