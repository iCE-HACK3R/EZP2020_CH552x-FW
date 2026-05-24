/*
 * ch554_usb.h - USB register definitions and data structures for CH554/CH552
 *
 * Extracted from the ieiao/ch554_sdcc USB stack.
 * Covers USB setup packet structures, request types, and descriptor constants
 * needed by the CDC ACM USB device implementation in main.c.
 */

#ifndef __CH554_USB_H__
#define __CH554_USB_H__

#include <stdint.h>

/* ---- Buffer sizes -------------------------------------------------------- */
#define DEFAULT_ENDP0_SIZE  64
#define DEFAULT_ENDP1_SIZE  64
#define MAX_PACKET_SIZE     64

/* ---- USB standard request types ----------------------------------------- */
#define USB_REQ_TYP_MASK        0x60
#define USB_REQ_TYP_STANDARD    0x00
#define USB_REQ_TYP_CLASS       0x20
#define USB_REQ_TYP_VENDOR      0x40

#define USB_REQ_RECIP_MASK      0x1F
#define USB_REQ_RECIP_DEVICE    0x00
#define USB_REQ_RECIP_INTERF    0x01
#define USB_REQ_RECIP_ENDP      0x02

#define USB_REQ_TYP_IN          0x80
#define USB_REQ_TYP_OUT         0x00

/* ---- USB standard request codes ------------------------------------------ */
#define USB_GET_STATUS          0x00
#define USB_CLEAR_FEATURE       0x01
#define USB_SET_FEATURE         0x03
#define USB_SET_ADDRESS         0x05
#define USB_GET_DESCRIPTOR      0x06
#define USB_SET_DESCRIPTOR      0x07
#define USB_GET_CONFIGURATION   0x08
#define USB_SET_CONFIGURATION   0x09
#define USB_GET_INTERFACE       0x0A
#define USB_SET_INTERFACE       0x0B

/* ---- USB descriptor types ------------------------------------------------ */
#define USB_DESCR_TYP_DEVICE    0x01
#define USB_DESCR_TYP_CONFIG    0x02
#define USB_DESCR_TYP_STRING    0x03
#define USB_DESCR_TYP_INTERF    0x04
#define USB_DESCR_TYP_ENDP      0x05
#define USB_DESCR_TYP_HID       0x21
#define USB_DESCR_TYP_REPORT    0x22

/* ---- USB CDC class codes ------------------------------------------------- */
#define USB_DEV_CLASS_COMMUNIC  0x02
#define USB_DEV_SUBCLASS_CDC    0x00
#define USB_DEV_PROTOCOL_CDC    0x00

/* ---- USB CDC class-specific requests ------------------------------------- */
#define CDC_SET_LINE_CODING     0x20
#define CDC_GET_LINE_CODING     0x21
#define CDC_SET_CONTROL_LINE    0x22
#define CDC_SEND_BREAK          0x23

/* ---- USB setup request packet structure ---------------------------------- */
typedef struct _USB_SETUP_REQ {
    uint8_t  bRequestType;
    uint8_t  bRequest;
    uint8_t  wValueL;
    uint8_t  wValueH;
    uint8_t  wIndexL;
    uint8_t  wIndexH;
    uint8_t  wLengthL;
    uint8_t  wLengthH;
} USB_SETUP_REQ, *PUSB_SETUP_REQ;

#endif /* __CH554_USB_H__ */
