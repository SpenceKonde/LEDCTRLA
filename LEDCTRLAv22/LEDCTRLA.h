/* LEDCTRLA - Universal */

#include "Arduino.h"
#include "Modes.h"
#include "Colors.h"
#ifndef __LightCtrl_h
#define __LightCtrl_h

#define FLASH(flashptr) (reinterpret_cast<const __FlashStringHelper *>(pgm_read_word_near(&flashptr)))

/* Bit Positions */
#define STATUS_RX_bp                  (0)
#define STATUS_NEW_MSG_bp             (1)
#define STATUS_TEST_MODE_bp           (2)
// TBD                                (3)
#define STATUS_COLOR_CHANGE_bp        (4)
#define STATUS_ADJUST_CHANGE_bp       (5)
#define STATUS_MODE_CHANGE_bp         (6)
#define STATUS_UI_CHANGE_bp           (7)
/* Bitmasks */
#define STATUS_RX_bm                  (1 << STATUS_RX_bp)
#define STATUS_NEW_MSG_bm             (1 << STATUS_NEW_MSG_bp)
#define STATUS_TEST_MODE_bm           (1 << STATUS_TEST_MODE_bp)
// TBD                                (1 << STATUS_TBD_bp)
#define STATUS_COLOR_CHANGE_bm        (1 << STATUS_COLOR_CHANGE_bp)
#define STATUS_ADJUST_CHANGE_bm       (1 << STATUS_ADJUST_CHANGE_bp)
#define STATUS_MODE_CHANGE_bm         (1 << STATUS_MODE_CHANGE_bp)
#define STATUS_UI_CHANGE_bm           (1 << STATUS_UI_CHANGE_bp)
/* used for system status flags
 *  bit 0 = receiving
 *  bit 1 = new message
 *  bit 2 =
 *  bit 3 =
 *  bit 4 = color menu changed. We need to process this before any additional input is handled/
 *  bit 5 = Adjust menu changed. We need to process this before any additional input is handled.
 *  bit 6 = Mode has been changed. We need to process this before any input is handled.
 *  bit 7 = UI changed - redraw it.
 */

/* Serious errors */
/* Bit Positions */
#define ERROR_NO_CFG_bp               (0)
#define ERROR_BAD_CFG_bp              (1)
// TBD                                (2)
// TBD                                (3)
// TBD                                (4)
#define ERROR_NO_LCD_bp               (5)
#define ERROR_NO_ENC_bp               (6)
#define ERROR_FATAL_CLR_bp            (7)

/* Bitmasks */
#define ERROR_NO_CFG_bm               (1 << ERROR_NO_CFG_bp)
#define ERROR_BAD_CFG_bm              (1 << ERROR_BAD_CFG_bp)
// TBD                                (1 << ERROR_TBD_bp)
// TBD                                (1 << ERROR_TBD_bp)
// TBD                                (1 << ERROR_TBD_bp)
#define ERROR_NO_LCD_bm               (1 << ERROR_NO_LCD_bp)
#define ERROR_NO_ENC_bm               (1 << ERROR_NO_ENC_bp)
#define ERROR_FATAL_CLR_bm            (1 << ERROR_FATAL_CLR_bp)
#define FATAL_ERRORS_gm               (FATAL_CLR | NO_ENCODERS | NO_LCD)
/* used for errors that preclude normal operation.
 *  bit 0 = There is no configuration data stored
 *  bit 1 = Configuration data was found, but invalid
 *  bit 2 =
 *  bit 3 =
 *  bit 4 =
 *  bit 5 = If we are unable to communicate with the LCD, that constitutes a fatal error, as we cannot display output to the user
 *  bit 6 = The encoders, on PORTC, are powered by VDDIO2. This bit is set if we find the MVIO status to be 0 even after waiting 2 seconds.
 *          We cannot read the pins if there is no power on VDDIO2, so we cannot receive most user input.
 *  bit 7 =
 *          We cannot read the pins if there is no power on VDDIO2, so we cannot receive most user input.
 */

/* Tools for working with the system status flags stored in the GPRs */

// Testing and Clearing statii

#define StatusGet(bitmask) = (!!(SYS_STATUS & bitmask))
#define ErrorGet(bitmask)  = (!!(SYS_ERROR & bitmask))
#define Fatal_Error_Check  = (!!(SYS_ERROR & FATAL_ERRORS_gm))

/* Status Manipulation Inlines
 *
 * always-inline functions that do the simple stuff.
 */

/* These should *not* be getting called with variable arguments.
 *
 * Any calls to these functions made with non-compile-time-known arguments should be considered to be unsupported. Author reserves the right to
 * replace the body of the 'else' blocks with badArg("This function is designed for use with constant compile time known arguments only in order
 * ensure that the binary is efficient."); // Your code is inefficient to an unacceptable degree. Code better.
 *
 * Everything related to this device must be designed with performance formost in the mind.
 *
 * However if they do, an ill-timed ISR could cause malfunction
 * when setting or clearing, so we need to test that. If we can constant-fold it, and multiple bits aren't set (also a no-no)
 * we just set or clear it and the compiler will generate a single cbi or sbi instruction
 * Ottherwise, we need to turn off interrupts while we do it, but it's still not painful to inline:
 * in, cli, in, and, out, out
 */

// Status set bit
inline void __attribute__((always_inline)) StatusSet(bitmask) {
  if (__builtin_constant_p(bitmask) && (!((bitmask - 1) & (bitmask)))) {
    SYS_STATUS |= bitmask;
  } else {
    uint8_t old_sreg = SREG;
    cli();
    SYS_STATUS |= bitmask;
    SREG=old_sreg;
  }
}
// Status clear bit
inline void __attribute__((always_inline)) StatusClr(bitmask) {
  if (__builtin_constant_p(bitmask) && (!((bitmask - 1) & (bitmask)))) {
    SYS_STATUS &= ~bitmask;
  } else {
    uint8_t old_sreg = SREG;
    cli();
    SYS_STATUS &= ~bitmask;
    SREG = old_sreg;
  }
}
// Status load and clear bit.
inline bool __attribute__((always_inline)) StatusLAC(bitmask) {
  if (__builtin_constant_p(bitmask) && (!((bitmask - 1) & (bitmask)))) {
    uint8_t old_sreg = SREG;
    cli();
    uint8_t r = !!(SYS_STATUS & bitmask);
    SYS_STATUS &= ~bitmask;
    SREG = old_sreg;
    return r;
  } else {
    badArg("StatusLAC only accepts constant bitmask");
    return 0;
  }
}

inline void __attribute__((always_inline)) ErrorSet(bitmask) {
    if (__builtin_constant_p(bitmask) && (!((bitmask - 1) & (bitmask)))) {
    SYS_ERROR |= bitmask;
  } else {
    uint8_t old_sreg = SREG;
    cli();
    SYS_ERROR |= bitmask;
    SREG = old_sreg;
  }
}

// Fatal errors can't be cleared; they indicate a hardware fault and
// the only remedy is to fix the device.
inline void __attribute__((always_inline)) ErrorClr(bitmask) {
  if ((bitmask & ~FATAL_ERROR_gm)) {
    if (__builtin_constant_p(bitmask) && (!((bitmask - 1) & (bitmask)))) {
      SYS_ERROR &= ~bitmask;
    } else {
      uint8_t old_sreg = SREG;
      cli();
      SYS_ERROR &= ~bitmask;
      SREG = old_sreg;
    }
  } else {
    ErrorSet(FATAL_CLR);
  }
}


/*****************************
 *     RF related globals    *
 *  Macros, buffers, etc     *
 *                           *
 ****************************/
#define MyAddress     (Hardware->RF_config & 0x3F);
#define RFRX_NOW()    (SYS_STATUS  &    (1 << RX_BITPOS))
#define START_RX()    (SYS_STATUS  |=   (1 << RX_BITPOS); digitalWriteFast(INDICATE0, HIGH);)
#define DONE_RX()     (SYS_STATUS  &= (~(1 << RX_BITPOS)); digitalWriteFast(INCICATE0, LOW;))
#define SET_MESSAGE() (SYS_STATUS  |=   (1 << NEW_MSG_BITPOS);)
#define GOT_MESSAGE() (SYS_STATUS  &    (1 << NEW_MSG_BITPOS);)
#define CLR_MESSAGE() (SYS_STATUS  &= (~(1 << NEW_MSG_BITPOS);))
#define RX_PIN_STATE  (digitalReadFast(RF_PIN))

// Version 2.2/2.3
  #if defined(SKETCH_MAIN)

    volatile byte bitnum                = 0; //current bit received

    volatile byte dataIn                = 0;
    volatile byte pktLength             = 31;
    volatile unsigned long lastRFMsgAt  = 0;

    volatile byte rxBuffer[32] = {
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0
    };

    byte     recvMessage[32] = {
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0
    };

    unsigned long lastPacketTime = 0;
    unsigned long lastPacketSig = 0;

    #if(F_CPU == 8000000)
      #define TIME_MULT(t) (t     )
    #elif(F_CPU == 16000000)
      #define TIME_MULT(t) (t *  2)
    #elif(F_CPU == 20000000)
      #define TIME_MULT(t) ((t *  5) >> (1))
    #elif(F_CPU == 24000000)
      #define TIME_MULT(t) (t *  3)
    #elif(F_CPU == 30000000)
      #define TIME_MULT(t) ( (t * 15) >> (2))
    #elif(F_CPU == 32000000)
      #define TIME_MULT(t) (t *  4)
    #elif(F_CPU == 36000000)
      #define TIME_MULT(t) ((t *  9) >> (1))
    #elif(F_CPU == 40000000)
      #define TIME_MULT(t) (t *  5)
    #elif(F_CPU == 48000000)
      #define TIME_MULT(t) (t *  6)
    #endif
    #if defined(TIME_MULT)  //include guard doesn't catch this.
      const PROGMEM_MAPPED unsigned int rxSyncMin  = TIME_MULT(1750);
      const PROGMEM_MAPPED unsigned int rxSyncMax  = TIME_MULT(2250);
      const PROGMEM_MAPPED unsigned int rxZeroMin  = TIME_MULT(100);
      const PROGMEM_MAPPED unsigned int rxZeroMax  = TIME_MULT(390);
      const PROGMEM_MAPPED unsigned int rxOneMin   = TIME_MULT(410);
      const PROGMEM_MAPPED unsigned int rxOneMax   = TIME_MULT(700);
      const PROGMEM_MAPPED unsigned int rxLowMax   = TIME_MULT(600);
      const PROGMEM_MAPPED int commandForgetTime = 5000;
    #else
      #warning "no RF decoding available, unknown speed"
    #endif
  #endif
#endif

#endif
