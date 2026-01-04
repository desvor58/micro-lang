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

// ret
#define asm_ret          { 0xC3 }

// call <lbl>
#define asm_call(addr)   { 0xE8, (addr)[0], (addr)[1], (addr)[2], (addr)[3] }

// push ebp   mov ebp, esp
#define asm_std_prelude  { 0x55, 0x89, 0xE5 }

// leave  ret
#define asm_std_epilogue { 0xC9, 0xC3 }

// exchange values
#define asm_xchgR32R32(reg1, reg2) {       0x87, 0b11000000 | ((reg1) << 3) | (reg2) }
#define asm_xchgR16R16(reg1, reg2) { 0x66, 0x87, 0b11000000 | ((reg1) << 3) | (reg2) }
#define asm_xchgR8R8(reg1, reg2)   {       0x86, 0b11000000 | ((reg1) << 3) | (reg2) }

// pushing reg 32 to stack
#define asm_pushR32(reg) {       0xFF, 0b11110000 | (reg) }
// pushing reg 16 to stack
#define asm_pushR16(reg) { 0x66, 0xFF, 0b11110000 | (reg) }

// poping reg 32 to stack
#define asm_popR32(reg) {       0x58 + (reg) }
// poping reg 16 to stack
#define asm_popR16(reg) { 0x66, 0x58 + (reg) }

// move reg 32 to reg 32
#define asm_movR32R32(reg1, reg2) {       0x8B, 0b11000000 | ((reg2) << 3) | (reg1) }
// move reg 16 to reg 16
#define asm_movR16R16(reg1, reg2) { 0x66, 0x8B, 0b11000000 | ((reg2) << 3) | (reg1) }
// move reg 8 to reg 8
#define asm_movR8R8(reg1, reg2)   {       0x8A, 0b11000000 | ((reg2) << 3) | (reg1) }

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

// move value 32 to reg 32
#define asm_movR32I32(reg, val)  {       0xB8 + reg, (val)[0], (val)[1], (val)[2], (val)[3] }
// move value 16 to reg 16
#define asm_movR16I16(reg, val)  { 0x66, 0xB8 + reg, (val)[0], (val)[1]  }
// move value 8 to reg 8
#define asm_movR8I8(reg, val)    {       0xB0 + reg, (val)[0] }

// move value 32 to stack with 32bits offset
#define asm_movS32I32(offset, val) {       0xC7, 0b10000100, 0b00100100, (offset)[0], (offset)[1], (offset)[2], (offset)[3], (val)[0], (val)[1], (val)[2], (val)[3] }
// move value 16 to stack with 32bits offset
#define asm_movS32I16(offset, val) { 0x66, 0xC7, 0b10000100, 0b00100100, (offset)[0], (offset)[1], (offset)[2], (offset)[3], (val)[0], (val)[1] }
// move value 8 to stack with 32bits offset
#define asm_movS32I8(offset, val)  {       0xC6, 0b10000100, 0b00100100, (offset)[0], (offset)[1], (offset)[2], (offset)[3], (val)[0] }

// move value from reg32 to stack with 32bits offset
#define asm_movS32R32(offset, reg) {       0x89, 0b10000100 | ((reg) << 3), 0b00100100, (offset)[0], (offset)[1], (offset)[2], (offset)[3] }
// move value from reg16 to stack with 32bits offset
#define asm_movS32R16(offset, reg) { 0x66, 0x89, 0b10000100 | ((reg) << 3), 0b00100100, (offset)[0], (offset)[1], (offset)[2], (offset)[3] }
// move value from reg8 to stack with 32bits offset
#define asm_movS32R8(offset, reg)  {       0x89, 0b10000100 | ((reg) << 3), 0b00100100, (offset)[0], (offset)[1], (offset)[2], (offset)[3] }

// move value to reg32 from stack with 32bits offset
#define asm_movR32S32(reg, offset) {       0x89, 0b10100000 | (reg), 0b00100100, (offset)[0], (offset)[1], (offset)[2], (offset)[3] }
// move value to reg16 from stack with 32bits offset
#define asm_movR16S32(reg, offset) { 0x66, 0x89, 0b10100000 | (reg), 0b00100100, (offset)[0], (offset)[1], (offset)[2], (offset)[3] }
// move value to reg8 from stack with 32bits offset
#define asm_movR8S32(reg, offset)  {       0x89, 0b10100000 | (reg), 0b00100100, (offset)[0], (offset)[1], (offset)[2], (offset)[3] }

// add reg 32 and other reg 32
#define asm_addR32R32(reg1, reg2) {       0x01, 0b11000000 | ((reg2) << 3) | ((reg1)) }
// add reg 16 and other reg 16
#define asm_addR16R16(reg1, reg2) { 0x66, 0x01, 0b11000000 | ((reg2) << 3) | ((reg1)) }
// add reg 8 and other reg 8
#define asm_addR8R8(reg1, reg2)   {       0x00, 0b11000000 | ((reg2) << 3) | ((reg1)) }

// sub reg 32 and other reg 32
#define asm_subR32R32(reg1, reg2) {       0x81, 0b11000000 | ((reg2) << 3) | ((reg1)) }
// sub reg 16 and other reg 16
#define asm_subR16R16(reg1, reg2) { 0x66, 0x81, 0b11000000 | ((reg2) << 3) | ((reg1)) }
// sub reg 8 and other reg 8
#define asm_subR8R8(reg1, reg2)   {       0x80, 0b11000000 | ((reg2) << 3) | ((reg1)) }

// multiply EAX and reg
#define asm_mulR32(reg) {       0xF7, 0b11100000 | (reg) }
// multiply AX and reg
#define asm_mulR16(reg) { 0x66, 0xF7, 0b11100000 | (reg) }
// multiply AL and reg
#define asm_mulR8(reg)  {       0xF6, 0b11100000 | (reg) }

// signed multiply reg 32 and reg 32
#define asm_imulR32R32(reg1, reg2) {       0x0F, 0xAF, 0b11000000 | ((reg1) << 3) | ((reg2)) }
// signed multiply reg 16 and reg 16
#define asm_imulR16R16(reg1, reg2) { 0x66, 0x0F, 0xAF, 0b11000000 | ((reg1) << 3) | ((reg2)) }
// signed multiply reg 8 and reg 8
#define asm_imulR8R8(reg1, reg2)   {       0x0F, 0xAF, 0b11000000 | ((reg1) << 3) | ((reg2)) }

// division EAX and reg
#define asm_divR32(reg) {       0xF7, 0b11110000 | (reg) }
// division AX and reg
#define asm_divR16(reg) { 0x66, 0xF7, 0b11110000 | (reg) }
// division AL and reg
#define asm_divR8(reg)  {       0xF6, 0b11110000 | (reg) }

// division EAX and reg
#define asm_idivR32(reg) {       0xF7, 0b11111000 | (reg) }
// division AX and reg
#define asm_idivR16(reg) { 0x66, 0xF7, 0b11111000 | (reg) }
// division AL and reg
#define asm_idivR8(reg)  {       0xF6, 0b11111000 | (reg) }

#endif