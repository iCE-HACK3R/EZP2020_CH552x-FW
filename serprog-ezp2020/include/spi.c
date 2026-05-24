/*
 * spi.c - Hardware SPI0 master driver for CH552/CH554
 *
 * Sourced from ieiao/ch554_sdcc (serprog branch), include/spi.c
 */

#include <stdint.h>
#include "ch554.h"
#include "spi.h"

void SPIMasterModeSet(uint8_t mode)
{
    SPI0_SETUP = 0;        /* master mode, MSB first */

    if (mode == 0) {
        /* CPOL=0, CPHA=0: SCK idle low, sample on rising edge */
        SPI0_CTRL = bS0_MOSI_OE | bS0_SCK_OE;
    } else {
        /* CPOL=1, CPHA=1: SCK idle high, sample on falling edge */
        SPI0_CTRL = bS0_MOSI_OE | bS0_SCK_OE | bS0_MST_CLK;
    }

    /* Configure P1 pins:
     *   P1.4 (SCS), P1.5 (MOSI), P1.7 (SCK): push-pull output (MOC=0, DIR_PU=1)
     *   P1.6 (MISO): quasi-bidirectional input (MOC=1, DIR_PU=1) */
    P1_MOD_OC &= 0x0F;     /* clear P1.4-P1.7 open-drain bits */
    P1_MOD_OC |= 0x40;     /* P1.6 (MISO) = open-drain (gives quasi-bidir with pull-up below) */
    P1_DIR_PU |= 0xF0;     /* P1.4/P1.5/P1.6/P1.7: enable output/pull-up */

    /* Set SPI clock: Fspi = Fsys / (SPI0_CK_SE * 2)
     * With SPI0_CK_SE=2 and Fsys=24MHz: Fspi = 6 MHz */
    SPI0_CK_SE = 0x02;
}

void CH554SPIMasterWrite(uint8_t dat)
{
    SPI0_DATA = dat;
    while (!S0_FREE);      /* wait for SPI0 to finish */
}

uint8_t CH554SPIMasterRead(void)
{
    SPI0_DATA = 0xFF;      /* clock out 0xFF, receive data */
    while (!S0_FREE);
    return SPI0_DATA;
}
