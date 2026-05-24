/*
 * spi.h - Hardware SPI0 master driver for CH552/CH554
 *
 * Sourced from ieiao/ch554_sdcc (serprog branch), include/spi.h
 *
 * Pin mapping (CH552 hardware SPI0):
 *   P1.4 = SCS  (hardware chip-select, also used as GPIO CS in serprog)
 *   P1.5 = MOSI
 *   P1.6 = MISO
 *   P1.7 = SCK
 */

#ifndef __SPI_H__
#define __SPI_H__

#include <stdint.h>

/*
 * SPIMasterModeSet(mode) - Initialize SPI0 as master.
 *   mode = 0: CPOL=0, CPHA=0 (most flash chips)
 *   mode = 3: CPOL=1, CPHA=1
 * Sets clock to Fsys/4 (6 MHz at 24 MHz system clock).
 * Configures P1.4/P1.5/P1.7 as push-pull outputs, P1.6 as quasi-bidir input.
 */
void SPIMasterModeSet(uint8_t mode);

/*
 * CH554SPIMasterWrite(dat) - Transmit one byte over SPI0 (full-duplex, discards RX).
 * Blocks until the byte is fully clocked out.
 */
void CH554SPIMasterWrite(uint8_t dat);

/*
 * CH554SPIMasterRead() - Receive one byte over SPI0.
 * Sends 0xFF as TX byte, returns received byte.
 */
uint8_t CH554SPIMasterRead(void);

#endif /* __SPI_H__ */
