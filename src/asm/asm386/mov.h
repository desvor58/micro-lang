#ifndef ASM386_MOV_H
#define ASM386_MOV_H

#include "common.h"

// move 8 bits value from src register to dst register
void asm386_movR8R8(reg8 dst, reg8 src)
{
    __asm_realoc_check(2);
    mcbuf[mcbuf_size++] = 0x88;
    mcbuf[mcbuf_size++] = asm_mod_RR | (src << 3) | (dst << 0);
}

// move 16 bits value from src register to dst register
void asm386_movR16R16(reg16 dst, reg16 src)
{
    __asm_realoc_check(3);
    mcbuf[mcbuf_size++] = 0x66;
    mcbuf[mcbuf_size++] = 0x89;
    mcbuf[mcbuf_size++] = asm_mod_RR | (src << 3) | (dst << 0);
}

// move 32 bits value from src register to dst register
void asm386_movR32R32(reg32 dst, reg32 src)
{
    __asm_realoc_check(2);
    mcbuf[mcbuf_size++] = 0x89;
    mcbuf[mcbuf_size++] = asm_mod_RR | (src << 3) | (dst << 0);
}

// mov 8 bits value from src reg to mem of 32 bits dst_reg_base reg
// ```mov byte [dst_reg_base], src```
void asm386_movMIAR32R8(reg32 dst_reg_base, reg8 src)
{
    __asm_realoc_check(4);
    mcbuf[mcbuf_size++] = 0x88;
    if (dst_reg_base == reg32::EBP) {
        mcbuf[mcbuf_size++] = asm_mod_IA8 | (src << 3) | (dst_reg_base << 0);
        mcbuf[mcbuf_size++] = 0x00;
    } else {
        mcbuf[mcbuf_size++] = asm_mod_IAR | (src << 3) | (dst_reg_base << 0);
    }
    if (dst_reg_base == reg32::ESP) {
        mcbuf[mcbuf_size++] = 0x24;
    }
}

// mov 16 bits value from src reg to mem of 32 bits dst_reg_base reg
// ```mov word [dst_reg_base], src```
void asm386_movMIAR32R16(reg32 dst_reg_base, reg16 src)
{
    __asm_realoc_check(4);
    mcbuf[mcbuf_size++] = 0x66;
    mcbuf[mcbuf_size++] = 0x89;
    if (dst_reg_base == reg32::EBP) {
        mcbuf[mcbuf_size++] = asm_mod_IA8 | (src << 3) | (dst_reg_base << 0);
        mcbuf[mcbuf_size++] = 0x00;
    } else {
        mcbuf[mcbuf_size++] = asm_mod_IAR | (src << 3) | (dst_reg_base << 0);
    }
    if (dst_reg_base == reg32::ESP) {
        mcbuf[mcbuf_size++] = 0x24;
    }
}

// mov 32 bits value from src reg to mem of dst_reg_base reg
// ```mov dword [dst_reg_base], src```
void asm386_movMIAR32R32(reg32 dst_reg_base, reg32 src)
{
    __asm_realoc_check(3);
    mcbuf[mcbuf_size++] = 0x89;
    if (dst_reg_base == reg32::EBP) {
        mcbuf[mcbuf_size++] = asm_mod_IA8 | (src << 3) | (dst_reg_base << 0);
        mcbuf[mcbuf_size++] = 0x00;
    } else {
        mcbuf[mcbuf_size++] = asm_mod_IAR | (src << 3) | (dst_reg_base << 0);
    }
    if (dst_reg_base == reg32::ESP) {
        mcbuf[mcbuf_size++] = 0x24;
    }
}

// mov 8 bits value from memof src_reg_base reg to src reg
// ```mov dst, byte [src_reg_base]```
void asm386_movR8MIAR32(reg8 dst, reg32 src_reg_base)
{
    __asm_realoc_check(3);
    mcbuf[mcbuf_size++] = 0x8A;
    if (src_reg_base == reg32::EBP) {
        mcbuf[mcbuf_size++] = asm_mod_IA8 | (dst << 0) | (src_reg_base << 3);
        mcbuf[mcbuf_size++] = 0x00;
    } else {
        mcbuf[mcbuf_size++] = asm_mod_IAR | (dst << 0) | (src_reg_base << 3);
    }
    if (src_reg_base == reg32::ESP) {
        mcbuf[mcbuf_size++] = 0x24;
    }
}

// mov 16 bits value from memof src_reg_base reg to src reg
// ```mov dst, word [src_reg_base]```
void asm386_movR16MIAR32(reg16 dst, reg32 src_reg_base)
{
    __asm_realoc_check(4);
    mcbuf[mcbuf_size++] = 0x66;
    mcbuf[mcbuf_size++] = 0x8B;
    if (src_reg_base == reg32::EBP) {
        mcbuf[mcbuf_size++] = asm_mod_IA8 | (dst << 0) | (src_reg_base << 3);
        mcbuf[mcbuf_size++] = 0x00;
    } else {
        mcbuf[mcbuf_size++] = asm_mod_IAR | (dst << 0) | (src_reg_base << 3);
    }
    if (src_reg_base == reg32::ESP) {
        mcbuf[mcbuf_size++] = 0x24;
    }
}

// mov 32 bits value from memof src_reg_base reg to src reg
// ```mov dst, dword [src_reg_base]```
void asm386_movR32MIAR32(reg32 dst, reg32 src_reg_base)
{
    __asm_realoc_check(3);
    mcbuf[mcbuf_size++] = 0x8B;
    if (src_reg_base == reg32::EBP) {
        mcbuf[mcbuf_size++] = asm_mod_IA8 | (dst << 0) | (src_reg_base << 3);
        mcbuf[mcbuf_size++] = 0x00;
    } else {
        mcbuf[mcbuf_size++] = asm_mod_IAR | (dst << 0) | (src_reg_base << 3);
    }
    if (src_reg_base == reg32::ESP) {
        mcbuf[mcbuf_size++] = 0x24;
    }
}

// move 8 bits immediate value to 8 bits register 
void asm386_movR8I8(reg8 dst, u8 imm[1])
{
    __asm_realoc_check(2);
    mcbuf[mcbuf_size++] = 0xB0 + dst;
    mcbuf[mcbuf_size++] = imm[0];
}

// move 16 bits immediate value to 16 bits register 
void asm386_movR16I16(reg16 dst, u8 imm[2])
{
    __asm_realoc_check(2);
    mcbuf[mcbuf_size++] = 0xB8 + dst;
    mcbuf[mcbuf_size++] = imm[1];
    mcbuf[mcbuf_size++] = imm[0];
}

// move 32 bits immediate value to 32 bits register 
void asm386_movR32I32(reg32 dst, u8 imm[4])
{
    __asm_realoc_check(2);
    mcbuf[mcbuf_size++] = 0xB8 + dst;
    mcbuf[mcbuf_size++] = imm[3];
    mcbuf[mcbuf_size++] = imm[2];
    mcbuf[mcbuf_size++] = imm[1];
    mcbuf[mcbuf_size++] = imm[0];
}

#endif