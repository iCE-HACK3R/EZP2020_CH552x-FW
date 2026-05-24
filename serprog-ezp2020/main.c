/*****************************************************************************
 * main.c - serprog USB-CDC firmware for EZP2020 (CH552T)
 *
 * Derived from ieiao/ch554_sdcc (serprog branch), examples/serprog/main.c
 * Original: Copyright (C) ieiao, 2017  https://github.com/ieiao/ch554_sdcc
 *
 * EZP2020 hardware changes vs original:
 *   CS_PIN : 0 -> 4  (chip-select on P1.4 instead of P1.0)
 *   PGMNAME: "ch552-serprog" -> "ezp2020-serprog"
 *
 * Usage after flashing:
 *   Linux  : flashrom -p serprog:dev=/dev/ttyACM0:4000000
 *   Windows: flashrom -p serprog:dev=COM3:4000000
 *****************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <ch554.h>
#include <ch554_usb.h>
#include <spi.h>
#include <debug.h>
#include "serprog.h"

/* ---- Pin assignments ------------------------------------------------------ */
#define LED_PIN  4
SBIT(LED, 0xB0, LED_PIN);   /* P3.4 - green activity LED (active low); P3.3 is red power LED, HW driven */
#define CS_PIN   4           /* EZP2020: CS on P1.4 */
SBIT(CS,  0x90, CS_PIN);    /* P1.4 - SPI flash chip-select */

/* ---- Serprog identity ----------------------------------------------------- */
#define PGMNAME "ezp2020-serprog"
#define CMD_MAP (\
    (1L << S_CMD_NOP)        | \
    (1L << S_CMD_Q_IFACE)    | \
    (1L << S_CMD_Q_CMDMAP)   | \
    (1L << S_CMD_Q_PGMNAME)  | \
    (1L << S_CMD_Q_SERBUF)   | \
    (1L << S_CMD_Q_BUSTYPE)  | \
    (1L << S_CMD_SYNCNOP)    | \
    (1L << S_CMD_S_BUSTYPE)  | \
    (1L << S_CMD_O_SPIOP)    | \
    (1L << S_CMD_S_SPI_FREQ) \
)
#define BUS_SPI (1 << 3)

/* ---- USB CDC request codes ------------------------------------------------ */
#define SET_LINE_CODING         0x20
#define GET_LINE_CODING         0x21
#define SET_CONTROL_LINE_STATE  0x22

/* ---- Endpoint buffers (XRAM) ---------------------------------------------- */
/* EP0: 64-byte control buffer at 0x0000 */
__xdata __at (0x0000) uint8_t Ep0Buffer[DEFAULT_ENDP0_SIZE];
/* EP1: 64-byte CDC notification buffer at 0x0040 */
__xdata __at (0x0040) uint8_t Ep1Buffer[DEFAULT_ENDP1_SIZE];
/* EP2: 128-byte (2×64) bidirectional CDC data buffer at 0x0080
 *       [0..63]  = OUT (host→device, filled by USB DMA)
 *       [64..127]= IN  (device→host, filled by firmware) */
__xdata __at (0x0080) uint8_t Ep2Buffer[2*MAX_PACKET_SIZE];

/* ---- Global state --------------------------------------------------------- */
uint16_t SetupLen;
uint8_t  SetupReq, UsbConfig;
const uint8_t *pDescr;

#define UsbSetupBuf  ((PUSB_SETUP_REQ)Ep0Buffer)

/* ---- USB Descriptors ------------------------------------------------------ */
/* Device descriptor */
__code uint8_t DevDesc[] = {
    0x12, 0x01,             /* bLength=18, bDescriptorType=Device */
    0x10, 0x01,             /* bcdUSB=1.10 */
    0x02, 0x00, 0x00,       /* bDeviceClass=CDC, SubClass=0, Protocol=0 */
    DEFAULT_ENDP0_SIZE,     /* bMaxPacketSize0=8 */
    0x86, 0x1a,             /* idVendor=0x1A86 (WCH) */
    0x22, 0x57,             /* idProduct=0x5722 (CH552 CDC) */
    0x00, 0x01,             /* bcdDevice=1.00 */
    0x01, 0x02, 0x03,       /* iManufacturer, iProduct, iSerialNumber */
    0x01                    /* bNumConfigurations=1 */
};

/* Configuration descriptor: CDC ACM with two interfaces */
__code uint8_t CfgDesc[] = {
    /* Configuration */
    0x09, 0x02, 0x43, 0x00, 0x02, 0x01, 0x00, 0xa0, 0x32,
    /* Interface 0 - CDC control */
    0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02, 0x01, 0x00,
    /* CDC header functional descriptor */
    0x05, 0x24, 0x00, 0x10, 0x01,
    /* CDC call management descriptor */
    0x05, 0x24, 0x01, 0x00, 0x00,
    /* CDC ACM functional descriptor */
    0x04, 0x24, 0x02, 0x02,
    /* CDC union functional descriptor */
    0x05, 0x24, 0x06, 0x00, 0x01,
    /* EP1 IN - interrupt (CDC notification) */
    0x07, 0x05, 0x81, 0x03, 0x08, 0x00, 0xFF,
    /* Interface 1 - CDC data */
    0x09, 0x04, 0x01, 0x00, 0x02, 0x0a, 0x00, 0x00, 0x00,
    /* EP2 OUT - bulk */
    0x07, 0x05, 0x02, 0x02, 0x40, 0x00, 0x00,
    /* EP2 IN  - bulk */
    0x07, 0x05, 0x82, 0x02, 0x40, 0x00, 0x00,
};

/* String descriptors */
__code uint8_t LangDes[]  = { 0x04, 0x03, 0x09, 0x04 };  /* English */
__code uint8_t SerDes[]   = {  /* Serial: "202201" */
    0x0e, 0x03,
    0x32,0x00, 0x30,0x00, 0x32,0x00, 0x32,0x00, 0x30,0x00, 0x31,0x00,
};
__code uint8_t Prod_Des[] = {  /* Product: "ezp2020-serprog" (UTF-16LE) */
    0x20, 0x03,
    0x65,0x00, 0x7A,0x00, 0x70,0x00, 0x32,0x00, 0x30,0x00,
    0x32,0x00, 0x30,0x00, 0x2D,0x00, 0x73,0x00, 0x65,0x00,
    0x72,0x00, 0x70,0x00, 0x72,0x00, 0x6F,0x00, 0x67,0x00,
};
__code uint8_t Manuf_Des[] = {  /* Manufacturer: "ieiao" */
    0x0c, 0x03,
    0x69,0x00, 0x65,0x00, 0x69,0x00, 0x61,0x00, 0x6f,0x00,
};

/* CDC line-coding default: 57600 8N1 */
__xdata uint8_t LineCoding[7] = { 0x00, 0xe1, 0x00, 0x00, 0x00, 0x00, 0x08 };

/* ---- USB state flags (written from ISR, read from main loop) -------------- */
volatile __idata uint8_t USBByteCount  = 0;  /* bytes available in EP2 OUT */
volatile __idata uint8_t UpPoint2_Busy = 0;  /* EP2 IN transmit busy */

/* ---- USB device init ------------------------------------------------------ */
void USBDeviceCfg(void)
{
    USB_CTRL = 0x00;
    USB_CTRL &= ~bUC_HOST_MODE;
    USB_CTRL |= bUC_DEV_PU_EN | bUC_INT_BUSY | bUC_DMA_EN;
    USB_DEV_AD = 0x00;
    USB_CTRL &= ~bUC_LOW_SPEED;
    UDEV_CTRL &= ~bUD_LOW_SPEED;
    UDEV_CTRL = bUD_PD_DIS;   /* disable D+/D- pull-down */
    UDEV_CTRL |= bUD_PORT_EN; /* enable USB physical port */
}

void USBDeviceIntCfg(void)
{
    USB_INT_EN |= bUIE_SUSPEND;
    USB_INT_EN |= bUIE_TRANSFER;
    USB_INT_EN |= bUIE_BUS_RST;
    USB_INT_FG |= 0x1F;       /* clear all interrupt flags */
    IE_USB = 1;
    EA = 1;
}

void USBDeviceEndPointCfg(void)
{
    UEP1_DMA   = (uint16_t)Ep1Buffer;
    UEP2_DMA   = (uint16_t)Ep2Buffer;
    UEP2_3_MOD = 0xCC;  /* EP2+EP3: RX+TX enabled */
    UEP2_CTRL  = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;
    UEP1_CTRL  = bUEP_AUTO_TOG | UEP_T_RES_NAK;
    UEP0_DMA   = (uint16_t)Ep0Buffer;
    UEP4_1_MOD = 0x40;  /* EP1 TX buffer enabled */
    UEP0_CTRL  = UEP_R_RES_ACK | UEP_T_RES_NAK;
}

/* ---- USB ISR -------------------------------------------------------------- */
void DeviceInterrupt(void) __interrupt (INT_NO_USB)
{
    uint16_t len;
    if (UIF_TRANSFER)
    {
        switch (USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))
        {
        case UIS_TOKEN_IN | 1:
            UEP1_T_LEN = 0;
            UEP1_CTRL  = UEP1_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_NAK;
            break;

        case UIS_TOKEN_IN | 2:
            UEP2_T_LEN  = 0;
            UEP2_CTRL   = UEP2_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_NAK;
            UpPoint2_Busy = 0;
            break;

        case UIS_TOKEN_OUT | 2:
            if (U_TOG_OK)
            {
                USBByteCount = USB_RX_LEN;
                UEP2_CTRL = UEP2_CTRL & ~MASK_UEP_R_RES | UEP_R_RES_NAK;
            }
            break;

        case UIS_TOKEN_SETUP | 0:
            len = USB_RX_LEN;
            if (len == (sizeof(USB_SETUP_REQ)))
            {
                SetupLen = ((uint16_t)UsbSetupBuf->wLengthH << 8) |
                           UsbSetupBuf->wLengthL;
                len = 0;
                SetupReq = UsbSetupBuf->bRequest;
                if ((UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK) !=
                    USB_REQ_TYP_STANDARD)
                {
                    /* Non-standard (class) request */
                    switch (SetupReq)
                    {
                    case GET_LINE_CODING:
                        pDescr = LineCoding;
                        len = sizeof(LineCoding);
                        len = SetupLen >= DEFAULT_ENDP0_SIZE ?
                              DEFAULT_ENDP0_SIZE : SetupLen;
                        memcpy(Ep0Buffer, pDescr, len);
                        SetupLen -= len;
                        pDescr   += len;
                        break;
                    case SET_CONTROL_LINE_STATE:
                        break;
                    case SET_LINE_CODING:
                        break;
                    default:
                        len = 0xFF;
                        break;
                    }
                }
                else
                {
                    /* Standard request */
                    switch (SetupReq)
                    {
                    case USB_GET_DESCRIPTOR:
                        switch (UsbSetupBuf->wValueH)
                        {
                        case 1:
                            pDescr = DevDesc;
                            len    = sizeof(DevDesc);
                            break;
                        case 2:
                            pDescr = CfgDesc;
                            len    = sizeof(CfgDesc);
                            break;
                        case 3:
                            if      (UsbSetupBuf->wValueL == 0) { pDescr = LangDes;  len = sizeof(LangDes);  }
                            else if (UsbSetupBuf->wValueL == 1) { pDescr = Manuf_Des; len = sizeof(Manuf_Des);}
                            else if (UsbSetupBuf->wValueL == 2) { pDescr = Prod_Des; len = sizeof(Prod_Des); }
                            else                                { pDescr = SerDes;   len = sizeof(SerDes);   }
                            break;
                        default:
                            len = 0xFF;
                            break;
                        }
                        if (SetupLen > len) SetupLen = len;
                        len = SetupLen >= DEFAULT_ENDP0_SIZE ?
                              DEFAULT_ENDP0_SIZE : SetupLen;
                        memcpy(Ep0Buffer, pDescr, len);
                        SetupLen -= len;
                        pDescr   += len;
                        break;

                    case USB_SET_ADDRESS:
                        SetupLen = UsbSetupBuf->wValueL;
                        break;

                    case USB_GET_CONFIGURATION:
                        Ep0Buffer[0] = UsbConfig;
                        if (SetupLen >= 1) len = 1;
                        break;

                    case USB_SET_CONFIGURATION:
                        UsbConfig = UsbSetupBuf->wValueL;
                        break;

                    case USB_GET_INTERFACE:
                        break;

                    case USB_CLEAR_FEATURE:
                        if ((UsbSetupBuf->bRequestType & 0x1F) ==
                            USB_REQ_RECIP_DEVICE)
                        {
                            if (((uint16_t)UsbSetupBuf->wValueH << 8 |
                                 UsbSetupBuf->wValueL) == 0x01)
                            {
                                if (CfgDesc[7] & 0x20) { /* wakeup */ }
                                else                     { len = 0xFF; }
                            }
                            else { len = 0xFF; }
                        }
                        else if ((UsbSetupBuf->bRequestType &
                                  USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP)
                        {
                            switch (UsbSetupBuf->wIndexL)
                            {
                            case 0x83: UEP3_CTRL = UEP3_CTRL & ~(bUEP_T_TOG|MASK_UEP_T_RES) | UEP_T_RES_NAK; break;
                            case 0x03: UEP3_CTRL = UEP3_CTRL & ~(bUEP_R_TOG|MASK_UEP_R_RES) | UEP_R_RES_ACK; break;
                            case 0x82: UEP2_CTRL = UEP2_CTRL & ~(bUEP_T_TOG|MASK_UEP_T_RES) | UEP_T_RES_NAK; break;
                            case 0x02: UEP2_CTRL = UEP2_CTRL & ~(bUEP_R_TOG|MASK_UEP_R_RES) | UEP_R_RES_ACK; break;
                            case 0x81: UEP1_CTRL = UEP1_CTRL & ~(bUEP_T_TOG|MASK_UEP_T_RES) | UEP_T_RES_NAK; break;
                            case 0x01: UEP1_CTRL = UEP1_CTRL & ~(bUEP_R_TOG|MASK_UEP_R_RES) | UEP_R_RES_ACK; break;
                            default:   len = 0xFF; break;
                            }
                        }
                        else { len = 0xFF; }
                        break;

                    case USB_SET_FEATURE:
                        if ((UsbSetupBuf->bRequestType & 0x1F) ==
                            USB_REQ_RECIP_DEVICE)
                        {
                            if (((uint16_t)UsbSetupBuf->wValueH << 8 |
                                 UsbSetupBuf->wValueL) == 0x01)
                            {
                                if (CfgDesc[7] & 0x20)
                                {
                                    while (XBUS_AUX & bUART0_TX) { ; }
                                    SAFE_MOD = 0x55;
                                    SAFE_MOD = 0xAA;
                                    WAKE_CTRL = bWAK_BY_USB | bWAK_RXD0_LO | bWAK_RXD1_LO;
                                    PCON |= PD;
                                    SAFE_MOD = 0x55;
                                    SAFE_MOD = 0xAA;
                                    WAKE_CTRL = 0x00;
                                }
                                else { len = 0xFF; }
                            }
                            else { len = 0xFF; }
                        }
                        else if ((UsbSetupBuf->bRequestType & 0x1F) ==
                                 USB_REQ_RECIP_ENDP)
                        {
                            if (((uint16_t)UsbSetupBuf->wValueH << 8 |
                                 UsbSetupBuf->wValueL) == 0x00)
                            {
                                switch ((uint16_t)UsbSetupBuf->wIndexH << 8 |
                                        UsbSetupBuf->wIndexL)
                                {
                                case 0x83: UEP3_CTRL = UEP3_CTRL & ~bUEP_T_TOG | UEP_T_RES_STALL; break;
                                case 0x03: UEP3_CTRL = UEP3_CTRL & ~bUEP_R_TOG | UEP_R_RES_STALL; break;
                                case 0x82: UEP2_CTRL = UEP2_CTRL & ~bUEP_T_TOG | UEP_T_RES_STALL; break;
                                case 0x02: UEP2_CTRL = UEP2_CTRL & ~bUEP_R_TOG | UEP_R_RES_STALL; break;
                                case 0x81: UEP1_CTRL = UEP1_CTRL & ~bUEP_T_TOG | UEP_T_RES_STALL; break;
                                case 0x01: UEP1_CTRL = UEP1_CTRL & ~bUEP_R_TOG | UEP_R_RES_STALL; break;
                                default:   len = 0xFF;                                              break;
                                }
                            }
                            else { len = 0xFF; }
                        }
                        else { len = 0xFF; }
                        break;

                    case USB_GET_STATUS:
                        Ep0Buffer[0] = 0x00;
                        Ep0Buffer[1] = 0x00;
                        len = SetupLen >= 2 ? 2 : SetupLen;
                        break;

                    default:
                        len = 0xFF;
                        break;
                    }
                }
            }
            else { len = 0xFF; }

            if (len == 0xFF)
            {
                SetupReq = 0xFF;
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG |
                            UEP_R_RES_STALL | UEP_T_RES_STALL;
            }
            else if (len <= DEFAULT_ENDP0_SIZE)
            {
                UEP0_T_LEN = len;
                UEP0_CTRL  = bUEP_R_TOG | bUEP_T_TOG |
                             UEP_R_RES_ACK | UEP_T_RES_ACK;
            }
            else
            {
                UEP0_T_LEN = 0;
                UEP0_CTRL  = bUEP_R_TOG | bUEP_T_TOG |
                             UEP_R_RES_ACK | UEP_T_RES_ACK;
            }
            break;

        case UIS_TOKEN_IN | 0:
            switch (SetupReq)
            {
            case USB_GET_DESCRIPTOR:
                len = SetupLen >= DEFAULT_ENDP0_SIZE ?
                      DEFAULT_ENDP0_SIZE : SetupLen;
                memcpy(Ep0Buffer, pDescr, len);
                SetupLen -= len;
                pDescr   += len;
                UEP0_T_LEN = len;
                UEP0_CTRL ^= bUEP_T_TOG;
                break;
            case USB_SET_ADDRESS:
                USB_DEV_AD = USB_DEV_AD & bUDA_GP_BIT | SetupLen;
                UEP0_CTRL  = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
            default:
                UEP0_T_LEN = 0;
                UEP0_CTRL  = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
            }
            break;

        case UIS_TOKEN_OUT | 0:
            if (SetupReq == SET_LINE_CODING)
            {
                if (U_TOG_OK)
                {
                    memcpy(LineCoding, UsbSetupBuf, USB_RX_LEN);
                    UEP0_T_LEN = 0;
                    UEP0_CTRL |= UEP_R_RES_ACK | UEP_T_RES_ACK;
                }
            }
            else
            {
                UEP0_T_LEN = 0;
                UEP0_CTRL |= UEP_R_RES_ACK | UEP_T_RES_NAK;
            }
            break;

        default:
            break;
        }
        UIF_TRANSFER = 0;
    }

    if (UIF_BUS_RST)
    {
        UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK;
        UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;
        USB_DEV_AD    = 0x00;
        UIF_SUSPEND   = 0;
        UIF_TRANSFER  = 0;
        UIF_BUS_RST   = 0;
        USBByteCount  = 0;
        UsbConfig     = 0;
        UpPoint2_Busy = 0;
    }

    if (UIF_SUSPEND)
    {
        UIF_SUSPEND = 0;
        if (USB_MIS_ST & bUMS_SUSPEND)
        {
            while (XBUS_AUX & bUART0_TX) { ; }
            SAFE_MOD = 0x55;
            SAFE_MOD = 0xAA;
            WAKE_CTRL = bWAK_BY_USB | bWAK_RXD0_LO | bWAK_RXD1_LO;
            PCON |= PD;
            SAFE_MOD = 0x55;
            SAFE_MOD = 0xAA;
            WAKE_CTRL = 0x00;
        }
    }
    else
    {
        USB_INT_FG = 0xFF;
    }
}

/* ---- Serprog I/O ---------------------------------------------------------- */
__idata uint32_t slen;
__idata uint32_t rlen;
__idata int8_t   usb_send_length = 0;
__idata uint8_t  recv_index = 0;

void usb_send(uint8_t length)
{
    UEP2_T_LEN = length;
    UEP2_CTRL  = UEP2_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_ACK;
    UpPoint2_Busy = 1;
}

uint8_t recv_buf_getc(void)
{
    uint8_t tmp;
    while (USBByteCount == 0);
    tmp = Ep2Buffer[recv_index];
    recv_index++;
    if (recv_index == USBByteCount)
    {
        recv_index   = 0;
        USBByteCount = 0;
        UEP2_CTRL = UEP2_CTRL & ~MASK_UEP_R_RES | UEP_R_RES_ACK;
    }
    return tmp;
}

/* ---- Command handler ------------------------------------------------------ */
void handle_command(void)
{
    uint32_t i = 0;
    uint32_t remaining = 0;
    uint8_t  j = 0, k = 0;
    uint8_t  c = recv_buf_getc();

    usb_send_length = 0;   /* reset for each command; O_SPIOP manages its own sends */
    while (UpPoint2_Busy);
    LED = 0;   /* flash LED ON for duration of command */

    switch (c)
    {
    case S_CMD_NOP:
        Ep2Buffer[64 + 0] = S_ACK;
        usb_send_length = 1;
        break;

    case S_CMD_Q_IFACE:
        Ep2Buffer[64 + 0] = S_ACK;
        Ep2Buffer[64 + 1] = 0x01;
        Ep2Buffer[64 + 2] = 0x00;
        usb_send_length = 3;
        break;

    case S_CMD_Q_CMDMAP:
        /* CMD_MAP = 0x001D003F (32 bytes, little-endian, one bit per command):
         *   byte 0 = 0x3F: bits 0-5  -> NOP,Q_IFACE,Q_CMDMAP,Q_PGMNAME,Q_SERBUF,Q_BUSTYPE
         *   byte 1 = 0x00: bits 8-15 -> none
         *   byte 2 = 0x1D: bit 16 -> SYNCNOP(0x10)
         *                  bit 18 -> S_BUSTYPE(0x12)
         *                  bit 19 -> O_SPIOP(0x13)
         *                  bit 20 -> S_SPI_FREQ(0x14)
         *   byte 3 = 0x00: bits 24-31 -> none */
        Ep2Buffer[64 + 0] = S_ACK;
        Ep2Buffer[64 + 1] = 0x3F;
        Ep2Buffer[64 + 2] = 0x00;
        Ep2Buffer[64 + 3] = 0x1D;
        Ep2Buffer[64 + 4] = 0x00;
        for (i = 5; i < 33; i++) Ep2Buffer[64 + i] = 0;
        usb_send_length = 33;
        break;

    case S_CMD_Q_PGMNAME:
        Ep2Buffer[64 + 0] = S_ACK;
        while (PGMNAME[i]) { Ep2Buffer[64 + i + 1] = PGMNAME[i]; i++; }
        for (; i < 16; i++) Ep2Buffer[64 + i + 1] = 0;
        usb_send_length = 17;
        break;

    case S_CMD_Q_SERBUF:
        Ep2Buffer[64 + 0] = S_ACK;
        Ep2Buffer[64 + 1] = 0xFF;
        Ep2Buffer[64 + 2] = 0xFF;
        usb_send_length = 3;
        break;

    case S_CMD_Q_BUSTYPE:
        Ep2Buffer[64 + 0] = S_ACK;
        Ep2Buffer[64 + 1] = BUS_SPI;
        usb_send_length = 2;
        break;

    case S_CMD_SYNCNOP:
        Ep2Buffer[64 + 0] = S_NAK;
        Ep2Buffer[64 + 1] = S_ACK;
        usb_send_length = 2;
        break;

    case S_CMD_S_BUSTYPE:
        Ep2Buffer[64 + 0] = ((recv_buf_getc() | BUS_SPI) == BUS_SPI) ?
                            S_ACK : S_NAK;
        usb_send_length = 1;
        break;

    case S_CMD_S_SPI_FREQ:
        {
            uint32_t freq, ck_se, actual;
            freq = (uint32_t)recv_buf_getc()         |
                   ((uint32_t)recv_buf_getc() <<  8) |
                   ((uint32_t)recv_buf_getc() << 16) |
                   ((uint32_t)recv_buf_getc() << 24);
            if (freq == 0) {
                Ep2Buffer[64 + 0] = S_NAK;
                usb_send_length = 1;
                break;
            }
            ck_se = (uint32_t)FREQ_SYS / (2UL * freq);
            if (ck_se < 1)   ck_se = 1;
            if (ck_se > 255) ck_se = 255;
            SPI0_CK_SE = (uint8_t)ck_se;
            actual = (uint32_t)FREQ_SYS / (2UL * (uint32_t)SPI0_CK_SE);
            Ep2Buffer[64 + 0] = S_ACK;
            Ep2Buffer[64 + 1] = (uint8_t)(actual);
            Ep2Buffer[64 + 2] = (uint8_t)(actual >>  8);
            Ep2Buffer[64 + 3] = (uint8_t)(actual >> 16);
            Ep2Buffer[64 + 4] = (uint8_t)(actual >> 24);
            usb_send_length = 5;
        }
        break;

    case S_CMD_O_SPIOP:
        /* Read slen and rlen (3 bytes each, little-endian) */
        slen = (uint32_t)recv_buf_getc()         |
               ((uint32_t)recv_buf_getc() <<  8) |
               ((uint32_t)recv_buf_getc() << 16);
        rlen = (uint32_t)recv_buf_getc()         |
               ((uint32_t)recv_buf_getc() <<  8) |
               ((uint32_t)recv_buf_getc() << 16);

        CS = 0;
        mDelayuS(2);   /* CS setup time: let chip see CS low before first clock */

        /* Write phase */
        if (slen > 0)
        {
            for (i = 0; i < slen; i++)
                CH554SPIMasterWrite(recv_buf_getc());
        }

        /* Read phase:
         * For rlen == 0: just send ACK (1 byte).
         * For 1 <= rlen <= 63: send ACK + all data in one 64-byte packet
         *   (ACK at Ep2Buffer[64], data at Ep2Buffer[65..64+rlen]).
         * For rlen >= 64: send ACK as first byte of first chunk, then
         *   subsequent chunks of 64. For rlen that is a multiple of 64, the
         *   first packet is 64 bytes (1 ACK + 63 data) and subsequent packets
         *   hold the remaining data. */
        if (rlen == 0)
        {
            /* Write-only operation: just ACK. */
            Ep2Buffer[64 + 0] = S_ACK;
            usb_send(1);
            while (UpPoint2_Busy);
        }
        else if (rlen < 64)
        {
            /* Small read: collect all data first, then send ACK+data together. */
            k = (uint8_t)rlen;
            Ep2Buffer[64] = S_ACK;
            for (j = 65; j < (uint8_t)(k + 65); j++)
            {
                SPI0_DATA = 0xFF;
                while (S0_FREE == 0);
                Ep2Buffer[j] = SPI0_DATA;
            }
            while (UpPoint2_Busy);
            usb_send((uint8_t)(k + 1));
            while (UpPoint2_Busy);
        }
        else
        {
            /* Large read (rlen >= 64):
             * First packet: ACK + first 63 bytes of SPI data (64 bytes total).
             * Subsequent packets: 64 bytes each.
             * Final partial packet if any. */
            /* Block host OUT while we stream IN data. */
            UEP2_CTRL = UEP2_CTRL & ~MASK_UEP_R_RES | UEP_R_RES_NAK;

            /* Build and send first 64-byte packet: ACK + 63 SPI bytes.
             * Buffer must be filled BEFORE kicking usb_send so the DMA
             * doesn't race with writes to Ep2Buffer[64..127]. */
            Ep2Buffer[64] = S_ACK;
            for (j = 65; j < 128; j++)
            {
                SPI0_DATA = 0xFF;
                while (S0_FREE == 0);
                Ep2Buffer[j] = SPI0_DATA;
            }
            usb_send(64);   /* Busy=1; DMA now owns Ep2Buffer[64..127] */

            /* Remaining SPI bytes after first 63. */
            remaining = rlen - 63;

            /* Full 64-byte chunks.
             * Always wait for the previous USB packet to finish BEFORE
             * touching the buffer — otherwise we overwrite bytes the DMA
             * is still reading, corrupting the outgoing packet. */
            for (i = 0; i < (remaining / 64); i++)
            {
                while (UpPoint2_Busy);   /* buffer now free to reuse */
                for (j = 64; j < 128; j++)
                {
                    SPI0_DATA = 0xFF;
                    while (S0_FREE == 0);
                    Ep2Buffer[j] = SPI0_DATA;
                }
                usb_send(64);
            }

            /* Final partial chunk. */
            k = (uint8_t)(remaining % 64);
            if (k > 0)
            {
                while (UpPoint2_Busy);
                for (j = 64; j < (uint8_t)(k + 64); j++)
                {
                    SPI0_DATA = 0xFF;
                    while (S0_FREE == 0);
                    Ep2Buffer[j] = SPI0_DATA;
                }
                usb_send(k);
            }

            while (UpPoint2_Busy);
            UEP2_CTRL = UEP2_CTRL & ~MASK_UEP_R_RES | UEP_R_RES_ACK;
        }

        CS = 1;
        break;

    default:
        Ep2Buffer[64 + 0] = S_NAK;
        usb_send_length = 1;
        break;
    }

    if (usb_send_length > 0)
        usb_send(usb_send_length);
}

/* ---- main ----------------------------------------------------------------- */
void main(void)
{
    CfgFsys();
    mDelaymS(5);

    /* LED (P3.4): push-pull output (green activity LED; P3.3 is red power LED, HW driven) */
    P3_DIR_PU |= (1 << LED_PIN);
    P3_MOD_OC &= ~(1 << LED_PIN);

    /* SPI pins (P1): configure MOSI/SCK/MISO/CS
     *   CS(P1.4)/MOSI(P1.5)/SCK(P1.7): MOC=0, DIR_PU=1 → push-pull output
     *   MISO(P1.6): MOC=1, DIR_PU=1 → quasi-bidirectional (open-drain + pull-up)
     *               This is the correct SPI master input mode for CH55x. */
    P1_MOD_OC = 0x00 | (1 << 6);   /* P1.6 open-drain; all others push-pull */
    P1_DIR_PU = 0;
    P1_DIR_PU |= (1 << CS_PIN) | (1 << 5) | (1 << 6) | (1 << 7);

    /* SPI0: master, mode 0, MSB first
     * CK_SE=8 → Fspi = 24 MHz / (2*8) = 1.5 MHz (conservative default;
     * use flashrom spispeed= or S_CMD_S_SPI_FREQ to go faster) */
    SPI0_SETUP = 0x00;
    SPI0_CTRL  = 0x60;   /* bS0_MOSI_OE | bS0_SCK_OE */
    SPI0_CK_SE = 0x08;
    CS = 1;   /* ensure CS is deasserted (high) at startup */

    /* --- Startup RDID diagnostic ---
     * Read the JEDEC ID (cmd 0x9F) before USB init.
     * LED pattern tells you whether a chip is responding in the socket:
     *   8 fast blinks (80 ms)  → chip found (ID != 0xFFFFFF)
     *   3 slow blinks (400 ms) → no chip / MISO floating (all 0xFF)
     * This is independent of the USB/serprog stack, so you can confirm
     * the SPI bus and chip seating before even running flashrom. */
    {
        uint8_t id0, id1, id2;
        uint8_t blinks, half_ms;
        uint8_t bi;

        CS = 0;
        CH554SPIMasterWrite(0x9F);
        id0 = CH554SPIMasterRead();
        id1 = CH554SPIMasterRead();
        id2 = CH554SPIMasterRead();
        CS = 1;

        if (id0 == 0xFF && id1 == 0xFF && id2 == 0xFF)
        {
            blinks   = 3;    /* slow: no chip / MISO floating */
            half_ms  = 200;
        }
        else
        {
            blinks   = 8;    /* fast: chip found */
            half_ms  = 60;
        }
        for (bi = 0; bi < blinks; bi++)
        {
            LED = 0; mDelaymS(half_ms);
            LED = 1; mDelaymS(half_ms);
        }
    }

    /* USB init */
    USBDeviceCfg();
    USBDeviceEndPointCfg();
    USBDeviceIntCfg();
    UEP0_T_LEN = 0;
    UEP1_T_LEN = 0;
    UEP2_T_LEN = 0;

    /* Wait for USB enumeration, blink LED */
    while (!UsbConfig)
    {
        LED = !LED;
        mDelaymS(50);
    }
    LED = 1;   /* LED off after enumeration */

    /* Main serprog command loop */
    LED = 1;   /* LED off when idle */
    while (1)
    {
        handle_command();
        LED = 1;   /* LED off between commands */
    }
}