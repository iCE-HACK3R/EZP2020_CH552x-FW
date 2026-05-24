/*
 * serprog.h - Serial Flasher Protocol (serprog) command / constant definitions
 *
 * Sourced from ieiao/ch554_sdcc (serprog branch), examples/serprog/serprog.h
 * Protocol specification: https://www.flashrom.org/Serprog
 */

#ifndef __SERPROG_H__
#define __SERPROG_H__

/* ---- Wire-level byte values ---------------------------------------------- */
#define S_ACK               0x06    /* command accepted */
#define S_NAK               0x15    /* command rejected */

/* ---- Serprog commands ---------------------------------------------------- */
#define S_CMD_NOP           0x00    /* no-operation (used for sync) */
#define S_CMD_Q_IFACE       0x01    /* query interface version */
#define S_CMD_Q_CMDMAP      0x02    /* query supported command bitmap */
#define S_CMD_Q_PGMNAME     0x03    /* query programmer name (16 bytes, NUL-padded) */
#define S_CMD_Q_SERBUF      0x04    /* query serial buffer size (2 bytes, LE) */
#define S_CMD_Q_BUSTYPE     0x05    /* query supported bus types */
#define S_CMD_Q_CHIPSIZE    0x06    /* query chip size (not used here) */
#define S_CMD_Q_OPBUF       0x07    /* query operation buffer size (not used) */
#define S_CMD_Q_WRNMAXLEN   0x08    /* query max write length (not used) */
#define S_CMD_R_BYTE        0x09    /* read single byte */
#define S_CMD_R_NBYTES      0x0A    /* read N bytes */
#define S_CMD_O_INIT        0x0B    /* initialize operation buffer */
#define S_CMD_O_WRITEB      0x0C    /* write byte to operation buffer */
#define S_CMD_O_WRITEN      0x0D    /* write N bytes to operation buffer */
#define S_CMD_O_DELAY       0x0E    /* delay (microseconds) */
#define S_CMD_O_EXEC        0x0F    /* execute operation buffer */
#define S_CMD_SYNCNOP       0x10    /* sync: host sends, expects NAK then ACK */
#define S_CMD_Q_RDNMAXLEN   0x11    /* query max read length (not used) */
#define S_CMD_S_BUSTYPE     0x12    /* set active bus type */
#define S_CMD_O_SPIOP       0x13    /* SPI operation: write then read */
#define S_CMD_S_SPI_FREQ    0x14    /* set SPI frequency */
#define S_CMD_S_PIN_STATE   0x15    /* set programmer pin state (not used) */

/* ---- Bus type flags ------------------------------------------------------ */
#define S_BUSTYPE_PARALLEL  (1 << 0)
#define S_BUSTYPE_LPC       (1 << 1)
#define S_BUSTYPE_FWH       (1 << 2)
#define S_BUSTYPE_SPI       (1 << 3)

/* ---- Supported bus types for this firmware ------------------------------- */
#define SUPPORTED_BUS       S_BUSTYPE_SPI

/* ---- Capability bitmap (one bit per command 0..31) ----------------------- */
/* We support: NOP(0), Q_IFACE(1), Q_CMDMAP(2), Q_PGMNAME(3), Q_SERBUF(4),
 *             Q_BUSTYPE(5), SYNCNOP(10h), S_BUSTYPE(12h), O_SPIOP(13h)
 * Bit positions in the 32-bit LE bitmap below:
 *   0x00: NOP       bit 0
 *   0x01: Q_IFACE   bit 1
 *   0x02: Q_CMDMAP  bit 2
 *   0x03: Q_PGMNAME bit 3
 *   0x04: Q_SERBUF  bit 4
 *   0x05: Q_BUSTYPE bit 5
 *   0x10: SYNCNOP   bit 16
 *   0x12: S_BUSTYPE bit 18
 *   0x13: O_SPIOP   bit 19
 * 8-byte (64-bit) bitmap used by flashrom:
 *   bytes 0-3: commands 0x00-0x1F
 *   bytes 4-7: commands 0x20-0x3F
 */

#endif /* __SERPROG_H__ */
