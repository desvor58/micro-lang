#ifndef ASM386_H
#define ASM386_H

#include <string.h>
#include <SCT/string.h>
#include "../common.h"

#define __micro_defntbytes(...) { __VA_ARGS__, 0x00 }
#define __micro_write_instr(...)  {  \
    u8 I[] = { __VA_ARGS__ };  \
    memcpy(__micro_asm_instruction_stack[__micro_asm_instruction_stack_top].instr, I, sizeof(I) / sizeof(*I));  \
    __micro_asm_instruction_stack[__micro_asm_instruction_stack_top].size = sizeof(I) / sizeof(*I);  \
    __micro_asm_instruction_stack_top++;  \
}

extern sct_string_t *micro_outbuf;
extern size_t micro_pos;

typedef enum {
    REG32_EAX = 0,
    REG32_ECX = 1,
    REG32_EDX = 2,
    REG32_EBX = 3,
    REG32_ESP = 4,
    REG32_EBP = 5,
    REG32_ESI = 6,
    REG32_EDI = 7,
    
    REG16_AX = 0,
    REG16_CX = 1,
    REG16_DX = 2,
    REG16_BX = 3,
    REG16_SP = 4,
    REG16_BP = 5,
    REG16_SI = 6,
    REG16_DI = 7,

    REG8_AL = 0,
    REG8_CL = 1,
    REG8_DL = 2,
    REG8_BL = 3,
    REG8_AH = 4,
    REG8_CH = 5,
    REG8_DH = 6,
    REG8_BH = 7,
} asm386_reg;

#define MICRO_ASM386_MAX_INSTRUCTION_SIZE 15

typedef struct {
    u8 instr[MICRO_ASM386_MAX_INSTRUCTION_SIZE];
    u8 size;
} __micro_asm386_instruction_t;

extern __micro_asm386_instruction_t __micro_asm_instruction_stack[MICRO_INSTRUCTION_STACK_SIZE];
extern u8 __micro_asm_instruction_stack_top;

void asm_put_instructions();

void asm_put_instructions_to_addr(size_t addr);

void asm386_ret();
void asm386_call(micro_addr_le_t addr);
void asm386_prelude();
void asm386_epilogue();

void asm386_cmpR32R32(asm386_reg reg1, asm386_reg reg2);
void asm386_cmpR16R16(asm386_reg reg1, asm386_reg reg2);
void asm386_cmpR8R8(asm386_reg reg1, asm386_reg reg2);

void asm386_xchgR32R32(asm386_reg reg1, asm386_reg reg2);
void asm386_xchgR16R16(asm386_reg reg1, asm386_reg reg2);
void asm386_xchgR8R8(asm386_reg reg1, asm386_reg reg2);

void asm386_pushR32(asm386_reg reg);
void asm386_pushR16(asm386_reg reg);

void asm386_popR32(asm386_reg reg);
void asm386_popR16(asm386_reg reg);

void asm386_movR32R32(asm386_reg reg1, asm386_reg reg2);
void asm386_movR16R16(asm386_reg reg1, asm386_reg reg2);
void asm386_movR8R8(asm386_reg reg1, asm386_reg reg2);

void asm386_movM32I32(micro_addr_le_t addr, micro_imm_le_t val);
void asm386_movM16I16(micro_addr_le_t addr, micro_imm_le_t val);
void asm386_movM8I8(micro_addr_le_t addr, micro_imm_le_t val);

void asm386_movR32M32(asm386_reg reg, micro_addr_le_t addr);
void asm386_movR16M16(asm386_reg reg, micro_addr_le_t addr);
void asm386_movR8M8(asm386_reg reg, micro_addr_le_t addr);

void asm386_movR32MR32(asm386_reg reg1, asm386_reg reg2);
void asm386_movR16MR16(asm386_reg reg1, asm386_reg reg2);
void asm386_movR8MR8(asm386_reg reg1, asm386_reg reg2);

void asm386_movM32R32(micro_addr_le_t addr, asm386_reg reg);
void asm386_movM16R16(micro_addr_le_t addr, asm386_reg reg);
void asm386_movM8R8(micro_addr_le_t addr, asm386_reg reg);

void asm386_movR32I32(asm386_reg reg, micro_imm_le_t val);
void asm386_movR16I16(asm386_reg reg, micro_imm_le_t val);
void asm386_movR8I8(asm386_reg reg, micro_imm_le_t val);

void asm386_movS32I32(micro_imm_le_t offset, micro_imm_le_t val);
void asm386_movS32I16(micro_imm_le_t offset, micro_imm_le_t val);
void asm386_movS32I8(micro_imm_le_t offset, micro_imm_le_t val);

void asm386_movS32R32(micro_imm_le_t offset, asm386_reg reg);
void asm386_movS32R16(micro_imm_le_t offset, asm386_reg reg);
void asm386_movS32R8(micro_imm_le_t offset, asm386_reg reg);

void asm386_movR32S32(asm386_reg reg, micro_imm_le_t offset);
void asm386_movR16S32(asm386_reg reg, micro_imm_le_t offset);
void asm386_movR8S32(asm386_reg reg, micro_imm_le_t offset);

void asm386_addR32R32(asm386_reg reg1, asm386_reg reg2);
void asm386_addR16R16(asm386_reg reg1, asm386_reg reg2);
void asm386_addR8R8(asm386_reg reg1, asm386_reg reg2);

void asm386_subR32R32(asm386_reg reg1, asm386_reg reg2);
void asm386_subR16R16(asm386_reg reg1, asm386_reg reg2);
void asm386_subR8R8(asm386_reg reg1, asm386_reg reg2);

void asm386_mulR32(asm386_reg reg);
void asm386_mulR16(asm386_reg reg);
void asm386_mulR8(asm386_reg reg);

void asm386_imulR32R32(asm386_reg reg1, asm386_reg reg2);
void asm386_imulR16R16(asm386_reg reg1, asm386_reg reg2);
void asm386_imulR8R8(asm386_reg reg1, asm386_reg reg2);

void asm386_divR32(asm386_reg reg);
void asm386_divR16(asm386_reg reg);
void asm386_divR8(asm386_reg reg);

void asm386_idivR32(asm386_reg reg);
void asm386_idivR16(asm386_reg reg);
void asm386_idivR8(asm386_reg reg);

void asm386_negR32(asm386_reg reg);
void asm386_negR16(asm386_reg reg);
void asm386_negR8(asm386_reg reg);

void asm386_leaR32S32(asm386_reg reg, micro_imm_le_t offset);
void asm386_leaR16S32(asm386_reg reg, micro_imm_le_t offset);

void asm386_jmpL32(micro_imm_le_t lbl);

#endif