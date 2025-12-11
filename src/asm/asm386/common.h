#ifndef ASM386_COMMON_H
#define ASM386_COMMON_H

#include "asm.h"

typedef enum
{
    EAX = 0b000,
    ECX = 0b001,
    EDX = 0b010,
    EBX = 0b011,
    ESP = 0b100,
    EBP = 0b101,
    ESI = 0b110,
    EDI = 0b111,
} reg32;

typedef enum
{
    AX = 0b000,
    CX = 0b001,
    DX = 0b010,
    BX = 0b011,
    SP = 0b100,
    BP = 0b101,
    SI = 0b110,
    DI = 0b111,
} reg16;

typedef enum
{
    AL = 0b000,
    CL = 0b001,
    DL = 0b010,
    BL = 0b011,
    AH = 0b100,
    CH = 0b101,
    DH = 0b110,
    BH = 0b111,
} reg8;

typedef enum
{
    ES = 0b000,
    CS = 0b001,
    SS = 0b010,
    DS = 0b011,
} regSeg;

// register
#define asm_mod_IAR  0b00000000
// register + 8bits offset
#define asm_mod_IA8  0b01000000
// register + 32bits offset
#define asm_mod_IA32 0b10000000
// register, register
#define asm_mod_RR   0b11000000

#endif