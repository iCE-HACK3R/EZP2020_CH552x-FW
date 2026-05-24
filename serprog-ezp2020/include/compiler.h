/*
 * compiler.h - SDCC compatibility macros for CH552/CH554 SFR definitions
 *
 * These macros wrap SDCC's __sfr/__sbit built-in keywords so that ch554.h
 * can be compiled with SDCC targeting the MCS-51 architecture.
 */

#ifndef __COMPILER_H__
#define __COMPILER_H__

/* Special Function Register (SFR) at a given byte address */
#define SFR(name, addr)           __sfr __at(addr) name

/* 16-bit SFR pair: low byte at addr, high byte at addr+1 */
#define SFR16(name, addr)         __sfr16 __at(((addr+1)<<8)|(addr)) name

/* Single bit addressable SFR bit: byteAddr must be in 0x80-0xFF SFR area,
 * SDCC bit address = byteAddr + bit (0..7) */
#define SBIT(name, addr, bit)     __sbit __at((addr)+(bit)) name

#endif /* __COMPILER_H__ */
