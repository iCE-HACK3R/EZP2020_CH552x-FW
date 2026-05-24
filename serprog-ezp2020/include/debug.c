/*
 * debug.c - System clock configuration and delay functions for CH552/CH554
 *
 * Sourced from ieiao/ch554_sdcc (serprog branch), include/debug.c
 *
 * FREQ_SYS must be defined at compile time (e.g. -DFREQ_SYS=24000000).
 * Supported values: 32000000, 24000000, 16000000, 12000000, 6000000, 3000000
 */

#include <stdint.h>
#include "ch554.h"
#include "debug.h"

void CfgFsys(void)
{
#if FREQ_SYS == 32000000
    CLOCK_CFG = CLOCK_CFG & ~MASK_SYS_CK_SEL | 0x07;
#elif FREQ_SYS == 24000000
    CLOCK_CFG = CLOCK_CFG & ~MASK_SYS_CK_SEL | 0x06;
#elif FREQ_SYS == 16000000
    CLOCK_CFG = CLOCK_CFG & ~MASK_SYS_CK_SEL | 0x05;
#elif FREQ_SYS == 12000000
    CLOCK_CFG = CLOCK_CFG & ~MASK_SYS_CK_SEL | 0x04;
#elif FREQ_SYS == 6000000
    CLOCK_CFG = CLOCK_CFG & ~MASK_SYS_CK_SEL | 0x03;
#elif FREQ_SYS == 3000000
    CLOCK_CFG = CLOCK_CFG & ~MASK_SYS_CK_SEL | 0x02;
#else
#error "Unsupported FREQ_SYS value. Use 3/6/12/16/24/32 MHz."
#endif
}

void mDelayuS(uint16_t n)
{
    while (n) {
        /* Calibrated for 24 MHz; adjust nop count for other frequencies */
#if FREQ_SYS >= 24000000
        __asm__("nop"); __asm__("nop"); __asm__("nop");
        __asm__("nop"); __asm__("nop"); __asm__("nop");
        __asm__("nop"); __asm__("nop"); __asm__("nop");
        __asm__("nop"); __asm__("nop"); __asm__("nop");
        __asm__("nop"); __asm__("nop"); __asm__("nop");
        __asm__("nop"); __asm__("nop"); __asm__("nop");
        __asm__("nop"); __asm__("nop"); __asm__("nop");
        __asm__("nop"); __asm__("nop"); __asm__("nop");
#elif FREQ_SYS >= 16000000
        __asm__("nop"); __asm__("nop"); __asm__("nop");
        __asm__("nop"); __asm__("nop"); __asm__("nop");
        __asm__("nop"); __asm__("nop"); __asm__("nop");
        __asm__("nop"); __asm__("nop"); __asm__("nop");
        __asm__("nop"); __asm__("nop"); __asm__("nop");
        __asm__("nop");
#elif FREQ_SYS >= 12000000
        __asm__("nop"); __asm__("nop"); __asm__("nop");
        __asm__("nop"); __asm__("nop"); __asm__("nop");
        __asm__("nop"); __asm__("nop"); __asm__("nop");
        __asm__("nop"); __asm__("nop"); __asm__("nop");
#else
        __asm__("nop"); __asm__("nop"); __asm__("nop");
        __asm__("nop"); __asm__("nop"); __asm__("nop");
#endif
        --n;
    }
}

void mDelaymS(uint16_t n)
{
    while (n) {
        mDelayuS(1000);
        --n;
    }
}
