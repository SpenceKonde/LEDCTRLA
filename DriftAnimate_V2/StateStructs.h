typedef hardwarespecs { 
  uint8_t HW_Version;     //  starts at 0x1402 in the unified address space. PCB rev.
  uint8_t Seq_Number;     //  Running tally of Dx-based boards produced.
  uint8_t LCD_flags;      //  
  uint8_t RF_Flags;       //
  uint8_t Basic_Features; //  circit protection and connections present
  uint8_t UI_connections; //  (invert encoders and such)
  uint8_t data_connfig;   // 
  uint8_t RF_config;      // 
  uint8_t extensionconfig[8];
} hardware_t; /total 16 bytes;



/* LCD flags:
 *  0x01: Has color backlight.
 *  0x02: Has negative image
 *  0x04: Contrast can be configured by DAC. 
 *  0x08: No contrast needs to be set.
 *  0x10: Reserved 
 *  0x20: Reserved
 *  0x40: Reserved
 *  080 : Reserved
 *  
 *  
 *  RF Info:
 *  0x01: Set if RF receiver is preset and available.. 
 *  0x04-02: Antenna configuration
 *      0b00 = no antenna
 *      0b01 = wire whip
 *      0b10 = Ceranuc antenna
 *      0b11 = PCB antenna
 *  0x08: Reserved
 *  0x20-0xE0: Receiver  hardware:
 *      0b000 No receiver hardware
 *      0b001 Synoxo SYN470/480
 *      0b010 STX882
 *      0b011 Other super het
 *      0b100 superregenerative
 *      0b101 H5V4D
 *      0b110 STX887
 *      0b111 Reserved
 *  
 *  Basic_Features:
 *      0x01: 3 wure output present
 *      0x02  4 wire output present
 *      0x04  6 wire output present
 *      0x08  reserved
 *      0x10  3 wire has internal protection
 *      0x20  4 wire has internal protection
 *      0x40  6 wire has itnernal protection
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
 *  0s40: Has 32k crystal
 * 
 *  Configuration: Data Control
 *  First three bits: Comms Control:
 *  1. Disable RS on Serail (and hence no serial control. 
 *  2. enable full duplex two wire downstream serial. 
 *  
 *  
 *  Configuration: RF
 *  0baaaaaaSO 
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
