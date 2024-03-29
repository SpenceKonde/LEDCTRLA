

/* Constants and basics  */
#define SYS_STATUS            (GPR.GPR3)
/* Don't use these */
#define RX_bp                 (0)
#define NEW_MSG_bp            (1)
#define GPR_RESVD_bp          (2)
#define GPR_RESVD2_bp         (3)
#define COLOR_CHANGE_bp       (4)
#define ADJUST_CHANGE_bp      (5)
#define MODE_CHANGE_bp        (6)
#define UI_CHANGE_bp          (7)
/* Unless you can't use these for some weird reason */
const uint8_t RXING           = (1 << RX_bp)
const uint8_t NEW_MSG         = (1 << NEW_MSG_bp)
//TBD
//TBD
const uint8_t  COLOR_CHANGE   = (1 <<  COLOR_CHANGE_bp)
const uint8_t ADJUST_CHANGE   = (1 << ADJUST_CHANGE_bp)
const uint8_t   MODE_CHANGE   = (1 <<   MODE_CHANGE_bp)
const uint8_t     UI_CHANGE   = (1 <<     UI_CHANGE_bp)
/* used for system status flags
 *  bit 0 = receiving
 *  bit 1 = new message receive comepleted
 *  bit 2 =
 *  bit 3 =
 *  bit 4 = color menu changed. We need to process this before any additional input is handled/
 *  bit 5 = Adjust menu changed. We need to process this before any additional input is handled.
 *  bit 6 = Mode has been changed. We need to process this before any input is handled.
 *  bit 7 = UI changed - redraw it.
 */
/* Helper fucntions to set it */
#define IsSet(bitmask) = (!!(SYS_STATUS & bitmask))
inline void __attribute__((always_inline)) SysSet(bitmask) {
  SYS_STATUS |= bitmask;
}

inline void __attribute__((always_inline)) SysClr(bitmask) {
  SYS_STATUS &= ~bitmask;
}
