#ifndef ASM386_H
#define ASM386_H

// move value 32 to mem 32
#define asm_movM32I32(addr, val) {       0xC7, 0x05,(addr)[0],(addr)[1],(addr)[2],(addr)[3], (val)[0], (val)[1], (val)[2], (val)[3] }
// move value 16 to mem 16
#define asm_movM16I16(addr, val) { 0x66, 0xC7, 0x05,(addr)[0],(addr)[1],(addr)[2],(addr)[3], (val)[0], (val)[1] }
// move value 8 to mem 8
#define asm_movM8I8(addr, val)   {       0xC6, 0x05,(addr)[0],(addr)[1],(addr)[2],(addr)[3], (val)[0] }

#endif