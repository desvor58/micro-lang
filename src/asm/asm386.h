#ifndef ASM386_H
#define ASM386_H

typedef enum {
    REG32_EAX = 0,
    REG32_ECX = 1,
    REG32_EDX = 2,
    REG32_EBX = 3,
    REG32_ESP = 4,
    REG32_EBP = 5,
    REG32_ESI = 6,
    REG32_EDI = 7,
} reg32;
typedef enum {
    REG16_AX = 0,
    REG16_CX = 1,
    REG16_DX = 2,
    REG16_BX = 3,
    REG16_SP = 4,
    REG16_BP = 5,
    REG16_SI = 6,
    REG16_DI = 7,
} reg16;
typedef enum {
    REG8_AL = 0,
    REG8_CL = 1,
    REG8_DL = 2,
    REG8_BL = 3,
    REG8_AH = 4,
    REG8_CH = 5,
    REG8_DH = 6,
    REG8_BH = 7,
} reg8;

// move reg 32 to reg 32
#define asm_movR32R32(reg1, reg2) {       0x8B, 0b11000000 | ((reg2) << 3) | ((reg1)) }
// move reg 16 to reg 16
#define asm_movR16R16(reg1, reg2) { 0x66, 0x8B, 0b11000000 | ((reg2) << 3) | ((reg1)) }
// move reg 8 to reg 8
#define asm_movR8R8(reg1, reg2)   {       0x8A, 0b11000000 | ((reg2) << 3) | ((reg1)) }

// move value 32 to mem 32
#define asm_movM32I32(addr, val) {       0xC7, 0x05, (addr)[0], (addr)[1], (addr)[2], (addr)[3], (val)[0], (val)[1], (val)[2], (val)[3] }
// move value 16 to mem 16
#define asm_movM16I16(addr, val) { 0x66, 0xC7, 0x05, (addr)[0], (addr)[1], (addr)[2], (addr)[3], (val)[0], (val)[1]                     }
// move value 8 to mem 8
#define asm_movM8I8(addr, val)   {       0xC6, 0x05, (addr)[0], (addr)[1], (addr)[2], (addr)[3], (val)[0]                               }

// move mem 32 to reg 32
#define asm_movR32M32(reg, addr) {       0x8B, 0b00000101 | ((reg) << 3), (addr)[0], (addr)[1], (addr)[2], (addr)[3] }
// move mem 16 to reg 16
#define asm_movR16M16(reg, addr) { 0x66, 0x8B, 0b00000101 | ((reg) << 3), (addr)[0], (addr)[1], (addr)[2], (addr)[3] }
// move mem 8 to reg 8
#define asm_movR8M8(reg, addr)   {       0x8A, 0b00000101 | ((reg) << 3), (addr)[0], (addr)[1], (addr)[2], (addr)[3] }

// move reg 32 to mem 32
#define asm_movM32R32(addr, reg) {       0x89, 0b00000101 | ((reg) << 3), (addr)[0], (addr)[1], (addr)[2], (addr)[3] }
// move reg 16 to mem 16
#define asm_movM16R16(addr, reg) { 0x66, 0x89, 0b00000101 | ((reg) << 3), (addr)[0], (addr)[1], (addr)[2], (addr)[3] }
// move reg 8 to mem 8
#define asm_movM8R8(addr, reg)   {       0x88, 0b00000101 | ((reg) << 3), (addr)[0], (addr)[1], (addr)[2], (addr)[3] }

// mov value 32 to reg 32
#define asm_movR32I32(reg, val)  {       0xB8 + reg, (val)[0], (val)[1], (val)[2], (val)[3] }
// mov value 16 to reg 16
#define asm_movR16I16(reg, val)  { 0x66, 0xB8 + reg, (val)[0], (val)[1]  }
// mov value 8 to reg 8
#define asm_movR8I8(reg, val)    {       0xB0 + reg, (val)[0] }


// add reg 32 and other reg 32
#define asm_addR32R32(reg1, reg2) {       0x01, 0b11000000 | ((reg2) << 3) | ((reg1)) }
// add reg 16 and other reg 16
#define asm_addR16R16(reg1, reg2) { 0x66, 0x01, 0b11000000 | ((reg2) << 3) | ((reg1)) }
// add reg 8 and other reg 8
#define asm_addR8R8(reg1, reg2)   {       0x00, 0b11000000 | ((reg2) << 3) | ((reg1)) }

#endif