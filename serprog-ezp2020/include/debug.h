/*
 * debug.h - System clock configuration and delay functions for CH552/CH554
 *
 * Sourced from ieiao/ch554_sdcc (serprog branch), include/debug.h
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdint.h>

/*
 * CfgFsys() - Configure system clock.
 * Sets CLOCK_CFG based on FREQ_SYS macro (defined at compile time).
 * Call once at startup before any peripheral initialization.
 */
void CfgFsys(void);

/*
 * mDelayuS(n) - Busy-wait delay for n microseconds.
 * Accuracy depends on FREQ_SYS.
 */
void mDelayuS(uint16_t n);

/*
 * mDelaymS(n) - Busy-wait delay for n milliseconds.
 */
void mDelaymS(uint16_t n);

#endif /* __DEBUG_H__ */
