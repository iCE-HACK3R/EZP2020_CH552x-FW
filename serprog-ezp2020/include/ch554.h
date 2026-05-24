/*
 * ch554.h - CH552/CH554 SFR register definitions
 *
 * Based on ieiao/ch554_sdcc (serprog branch), include/ch554.h
 * Original copyright (C) W.ch 1999-2014  http://wch.cn
 *
 * Addresses verified against CH554 datasheet.
 */

#ifndef __CH554_H__
#define __CH554_H__

#include "compiler.h"
#include <stdint.h>

/* ---- System --------------------------------------------------------------- */
SFR(PSW,         0xD0);
  SBIT(CY,       0xD0, 7);
  SBIT(AC,       0xD0, 6);
  SBIT(F0,       0xD0, 5);
  SBIT(RS1,      0xD0, 4);
  SBIT(RS0,      0xD0, 3);
  SBIT(OV,       0xD0, 2);
  SBIT(F1,       0xD0, 1);
  SBIT(P,        0xD0, 0);
SFR(ACC,         0xE0);
SFR(B,           0xF0);
SFR(SP,          0x81);
SFR(DPL,         0x82);
SFR(DPH,         0x83);

SFR(SAFE_MOD,    0xA1);   /* WriteOnly: safe-mode; ReadOnly: CHIP_ID */
#define CHIP_ID  SAFE_MOD
SFR(GLOBAL_CFG,  0xB1);   /* global config, Write@SafeMode */
#define bBOOT_LOAD    0x20
#define bSW_RESET     0x10
#define bCODE_WE      0x08
#define bDATA_WE      0x04
#define bLDO3V3_OFF   0x02
#define bWDOG_EN      0x01

SFR(XBUS_AUX,    0xA2);
#define bUART0_TX     0x80
#define bSAFE_MOD_ACT 0x20
#define bDPTR_AUTO_INC 0x04
#define DPS           0x01

SFR(PCON,        0x87);
#define SMOD        0x80
#define GF1         0x08
#define GF0         0x04
#define PD          0x02

SFR(CLOCK_CFG,   0xB9);   /* Write@SafeMode */
#define bOSC_EN_INT     0x80
#define bOSC_EN_XT      0x40
#define MASK_SYS_CK_SEL 0x07

SFR(WAKE_CTRL,   0xA9);   /* Write@SafeMode */
#define bWAK_BY_USB   0x80
#define bWAK_RXD1_LO  0x40
#define bWAK_RXD0_LO  0x01

SFR(RESET_KEEP,  0xFE);
SFR(WDOG_COUNT,  0xFF);

/* ---- Interrupts ----------------------------------------------------------- */
SFR(IE,          0xA8);
  SBIT(EA,       0xA8, 7);
  SBIT(E_DIS,    0xA8, 6);
  SBIT(ET2,      0xA8, 5);
  SBIT(ES,       0xA8, 4);
  SBIT(ET1,      0xA8, 3);
  SBIT(EX1,      0xA8, 2);
  SBIT(ET0,      0xA8, 1);
  SBIT(EX0,      0xA8, 0);

SFR(IP,          0xB8);
  SBIT(PH_FLAG,  0xB8, 7);
  SBIT(PL_FLAG,  0xB8, 6);
  SBIT(PT2,      0xB8, 5);
  SBIT(PS,       0xB8, 4);
  SBIT(PT1,      0xB8, 3);
  SBIT(PX1,      0xB8, 2);
  SBIT(PT0,      0xB8, 1);
  SBIT(PX0,      0xB8, 0);

SFR(IE_EX,       0xE8);   /* extended interrupt enable */
  SBIT(IE_WDOG,  0xE8, 7);
  SBIT(IE_GPIO,  0xE8, 6);
  SBIT(IE_PWMX,  0xE8, 5);
  SBIT(IE_UART1, 0xE8, 4);
  SBIT(IE_ADC,   0xE8, 3);
  SBIT(IE_USB,   0xE8, 2);
  SBIT(IE_TKEY,  0xE8, 1);
  SBIT(IE_SPI0,  0xE8, 0);

SFR(IP_EX,       0xE9);
#define bIP_USB  0x04

/* ---- Timers --------------------------------------------------------------- */
SFR(TCON,        0x88);
  SBIT(TF1,      0x88, 7);
  SBIT(TR1,      0x88, 6);
  SBIT(TF0,      0x88, 5);
  SBIT(TR0,      0x88, 4);
  SBIT(IE1,      0x88, 3);
  SBIT(IT1,      0x88, 2);
  SBIT(IE0,      0x88, 1);
  SBIT(IT0,      0x88, 0);
SFR(TMOD,        0x89);
SFR(TL0,         0x8A);
SFR(TL1,         0x8B);
SFR(TH0,         0x8C);
SFR(TH1,         0x8D);

/* ---- GPIO Port 1 ---------------------------------------------------------- */
SFR(P1,          0x90);
  SBIT(SCK,      0x90, 7);  /* SPI0 SCK  = P1.7 */
  SBIT(MISO,     0x90, 6);  /* SPI0 MISO = P1.6 */
  SBIT(MOSI,     0x90, 5);  /* SPI0 MOSI = P1.5 */
  SBIT(SCS,      0x90, 4);  /* SPI0 SCS  = P1.4 */
  SBIT(P1_3,     0x90, 3);
  SBIT(P1_2,     0x90, 2);
  SBIT(P1_1,     0x90, 1);
  SBIT(P1_0,     0x90, 0);
SFR(P1_MOD_OC,   0x92);
SFR(P1_DIR_PU,   0x93);
#define bSCK    0x80
#define bMISO   0x40
#define bMOSI   0x20
#define bSCS    0x10

/* ---- GPIO Port 3 ---------------------------------------------------------- */
SFR(P3,          0xB0);
  SBIT(P3_7,     0xB0, 7);
  SBIT(P3_6,     0xB0, 6);
  SBIT(T1,       0xB0, 5);
  SBIT(PWM2,     0xB0, 4);
  SBIT(INT1,     0xB0, 3);
  SBIT(INT0,     0xB0, 2);
  SBIT(TXD,      0xB0, 1);
  SBIT(RXD,      0xB0, 0);
SFR(P3_MOD_OC,   0x96);
SFR(P3_DIR_PU,   0x97);

SFR(PIN_FUNC,    0xC6);
#define bUSB_IO_EN    0x80
#define bUART1_PIN_X  0x20
#define bUART0_PIN_X  0x10

SFR(GPIO_IE,     0xC7);

/* ---- UART0 ---------------------------------------------------------------- */
SFR(SCON,        0x98);
  SBIT(SM0,      0x98, 7);
  SBIT(SM1,      0x98, 6);
  SBIT(SM2,      0x98, 5);
  SBIT(REN,      0x98, 4);
  SBIT(TB8,      0x98, 3);
  SBIT(RB8,      0x98, 2);
  SBIT(TI,       0x98, 1);
  SBIT(RI,       0x98, 0);
SFR(SBUF,        0x99);

/* ---- SPI0 ----------------------------------------------------------------- */
SFR(SPI0_STAT,   0xF8);
  SBIT(S0_FST_ACT,  0xF8, 7);
  SBIT(S0_IF_OV,    0xF8, 6);
  SBIT(S0_IF_FIRST, 0xF8, 5);
  SBIT(S0_IF_BYTE,  0xF8, 4);
  SBIT(S0_FREE,     0xF8, 3);  /* 1 = SPI0 idle/free */
  SBIT(S0_T_FIFO,   0xF8, 2);
  SBIT(S0_R_FIFO,   0xF8, 0);
SFR(SPI0_DATA,   0xF9);
SFR(SPI0_CTRL,   0xFA);
#define bS0_MISO_OE  0x80
#define bS0_MOSI_OE  0x40
#define bS0_SCK_OE   0x20
#define bS0_DATA_DIR 0x10
#define bS0_MST_CLK  0x08
#define bS0_2_WIRE   0x04
#define bS0_CLR_ALL  0x02
#define bS0_AUTO_IF  0x01
SFR(SPI0_CK_SE,  0xFB);  /* Fspi = Fsys / (2 * SPI0_CK_SE) */
SFR(SPI0_SETUP,  0xFC);
#define bS0_MODE_SLV    0x80
#define bS0_IE_FIFO_OV  0x40
#define bS0_IE_FIRST    0x20
#define bS0_IE_BYTE     0x10
#define bS0_BIT_ORDER   0x08

/* ---- USB ------------------------------------------------------------------ */
SFR(UDEV_CTRL,   0xD1);
#define bUD_PD_DIS    0x80
#define bUD_DP_PIN    0x20
#define bUD_DM_PIN    0x10
#define bUD_LOW_SPEED 0x04
#define bUD_GP_BIT    0x02
#define bUD_PORT_EN   0x01

/* Endpoint 1-3 control / length registers */
SFR(UEP1_CTRL,   0xD2);
SFR(UEP1_T_LEN,  0xD3);
SFR(UEP2_CTRL,   0xD4);
SFR(UEP2_T_LEN,  0xD5);
SFR(UEP3_CTRL,   0xD6);
SFR(UEP3_T_LEN,  0xD7);

/* USB interrupt flags (bit-addressable SFR at 0xD8) */
SFR(USB_INT_FG,  0xD8);
  SBIT(U_IS_NAK,    0xD8, 7);
  SBIT(U_TOG_OK,    0xD8, 6);
  SBIT(U_SIE_FREE,  0xD8, 5);
  SBIT(UIF_FIFO_OV, 0xD8, 4);
  SBIT(UIF_HST_SOF, 0xD8, 3);
  SBIT(UIF_SUSPEND, 0xD8, 2);
  SBIT(UIF_TRANSFER,0xD8, 1);
  SBIT(UIF_BUS_RST, 0xD8, 0);
  SBIT(UIF_DETECT,  0xD8, 0);

SFR(USB_INT_ST,  0xD9);
#define bUIS_IS_NAK    0x80
#define bUIS_TOG_OK    0x40
#define bUIS_TOKEN1    0x20
#define bUIS_TOKEN0    0x10
#define MASK_UIS_TOKEN 0x30
#define UIS_TOKEN_OUT   0x00
#define UIS_TOKEN_SOF   0x10
#define UIS_TOKEN_IN    0x20
#define UIS_TOKEN_SETUP 0x30
#define MASK_UIS_ENDP   0x0F
#define MASK_UIS_H_RES  0x0F

SFR(USB_MIS_ST,  0xDA);
#define bUMS_SOF_PRES   0x80
#define bUMS_SOF_ACT    0x40
#define bUMS_SIE_FREE   0x20
#define bUMS_R_FIFO_RDY 0x10
#define bUMS_BUS_RESET  0x08
#define bUMS_SUSPEND    0x04
#define bUMS_DM_LEVEL   0x02
#define bUMS_DEV_ATTACH 0x01

SFR(USB_RX_LEN,  0xDB);

/* Endpoint 0 and 4 control / length (share UEP0_DMA buffer) */
SFR(UEP0_CTRL,   0xDC);
SFR(UEP0_T_LEN,  0xDD);
SFR(UEP4_CTRL,   0xDE);
SFR(UEP4_T_LEN,  0xDF);

SFR(USB_INT_EN,  0xE1);
#define bUIE_DEV_SOF  0x80
#define bUIE_DEV_NAK  0x40
#define bUIE_FIFO_OV  0x10
#define bUIE_HST_SOF  0x08
#define bUIE_SUSPEND  0x04
#define bUIE_TRANSFER 0x02
#define bUIE_BUS_RST  0x01
#define bUIE_DETECT   0x01

SFR(USB_CTRL,    0xE2);
#define bUC_HOST_MODE  0x80
#define bUC_LOW_SPEED  0x40
#define bUC_DEV_PU_EN  0x20
#define bUC_SYS_CTRL1  0x20
#define bUC_SYS_CTRL0  0x10
#define MASK_UC_SYS_CTRL 0x30
#define bUC_INT_BUSY   0x08
#define bUC_RESET_SIE  0x04
#define bUC_CLR_ALL    0x02
#define bUC_DMA_EN     0x01

SFR(USB_DEV_AD,  0xE3);
#define bUDA_GP_BIT   0x80
#define MASK_USB_ADDR 0x7F

/* EP2/EP3 DMA buffer pointers (SFR16, little-endian) */
SFR16(UEP2_DMA,  0xE4);
SFR(UEP2_DMA_L,  0xE4);
SFR(UEP2_DMA_H,  0xE5);
SFR16(UEP3_DMA,  0xE6);
SFR(UEP3_DMA_L,  0xE6);
SFR(UEP3_DMA_H,  0xE7);

/* EP4/EP1 and EP2/EP3 mode registers */
SFR(UEP4_1_MOD,  0xEA);
#define bUEP1_RX_EN   0x80
#define bUEP1_TX_EN   0x40
#define bUEP1_BUF_MOD 0x10
#define bUEP4_RX_EN   0x08
#define bUEP4_TX_EN   0x04

SFR(UEP2_3_MOD,  0xEB);
#define bUEP3_RX_EN   0x80
#define bUEP3_TX_EN   0x40
#define bUEP3_BUF_MOD 0x10
#define bUEP2_RX_EN   0x08
#define bUEP2_TX_EN   0x04
#define bUEP2_BUF_MOD 0x01

/* EP0/EP1 DMA buffer pointers (SFR16, little-endian) */
SFR16(UEP0_DMA,  0xEC);
SFR(UEP0_DMA_L,  0xEC);
SFR(UEP0_DMA_H,  0xED);
SFR16(UEP1_DMA,  0xEE);
SFR(UEP1_DMA_L,  0xEE);
SFR(UEP1_DMA_H,  0xEF);

/* Endpoint control flag definitions (shared for UEP0..UEP4 CTRL registers) */
#define bUEP_R_TOG     0x80
#define bUEP_T_TOG     0x40
#define bUEP_AUTO_TOG  0x10
#define bUEP_R_RES1    0x08
#define bUEP_R_RES0    0x04
#define MASK_UEP_R_RES 0x0C
#define UEP_R_RES_ACK   0x00
#define UEP_R_RES_TOUT  0x04
#define UEP_R_RES_NAK   0x08
#define UEP_R_RES_STALL 0x0C
#define bUEP_T_RES1    0x02
#define bUEP_T_RES0    0x01
#define MASK_UEP_T_RES 0x03
#define UEP_T_RES_ACK   0x00
#define UEP_T_RES_TOUT  0x01
#define UEP_T_RES_NAK   0x02
#define UEP_T_RES_STALL 0x03

/* ---- Interrupt numbers ---------------------------------------------------- */
#define INT_NO_INT0   0
#define INT_NO_TMR0   1
#define INT_NO_INT1   2
#define INT_NO_TMR1   3
#define INT_NO_UART0  4
#define INT_NO_TMR2   5
#define INT_NO_SPI0   6
#define INT_NO_TKEY   7
#define INT_NO_USB    8
#define INT_NO_ADC    9
#define INT_NO_UART1  10
#define INT_NO_PWMX   11
#define INT_NO_GPIO   12
#define INT_NO_WDOG   13

/* ---- Flash/memory map ----------------------------------------------------- */
#define BOOT_LOAD_ADDR  0x3800
#define ROM_CFG_ADDR    0x3FF8
#define DATA_FLASH_ADDR 0xC000
#define XDATA_RAM_SIZE  0x0400

#endif /* __CH554_H__ */