#ifndef ASM386_H
#define ASM386_H

#include <string.h>
#include <SCT/string.h>
#include <SCT/vector.h>
#include <micro/common.h>

extern sct_vector_t micro_asm386_instr_stack;

typedef enum {
    MICRO_ASM386_REG32_EAX = 0,
    MICRO_ASM386_REG32_ECX = 1,
    MICRO_ASM386_REG32_EDX = 2,
    MICRO_ASM386_REG32_EBX = 3,
    MICRO_ASM386_REG32_ESP = 4,
    MICRO_ASM386_REG32_EBP = 5,
    MICRO_ASM386_REG32_ESI = 6,
    MICRO_ASM386_REG32_EDI = 7,
    
    MICRO_ASM386_REG16_AX = 0,
    MICRO_ASM386_REG16_CX = 1,
    MICRO_ASM386_REG16_DX = 2,
    MICRO_ASM386_REG16_BX = 3,
    MICRO_ASM386_REG16_SP = 4,
    MICRO_ASM386_REG16_BP = 5,
    MICRO_ASM386_REG16_SI = 6,
    MICRO_ASM386_REG16_DI = 7,

    MICRO_ASM386_REG8_AL = 0,
    MICRO_ASM386_REG8_CL = 1,
    MICRO_ASM386_REG8_DL = 2,
    MICRO_ASM386_REG8_BL = 3,
    MICRO_ASM386_REG8_AH = 4,
    MICRO_ASM386_REG8_CH = 5,
    MICRO_ASM386_REG8_DH = 6,
    MICRO_ASM386_REG8_BH = 7,
} micro_asm386_reg_t;

void micro_asm386_init();

void micro_asm386_deinit();

void micro_asm386_put_instructions(sct_vector_t *vec);

void micro_asm386_put_instructions_to_addr(sct_vector_t *vec, size_t addr);

void micro_asm386_insert_instructions_to_addr(sct_vector_t *vec, size_t addr);

void micro_asm386_ret();
void micro_asm386_call(micro_addr_le_t addr);
void micro_asm386_prelude();
void micro_asm386_epilogue();

void micro_asm386_cmpR32R32(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);
void micro_asm386_cmpR16R16(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);
void micro_asm386_cmpR8R8(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);

void micro_asm386_setzR8(micro_asm386_reg_t reg);
void micro_asm386_setnzR8(micro_asm386_reg_t reg);
void micro_asm386_setgR8(micro_asm386_reg_t reg);
void micro_asm386_setgeR8(micro_asm386_reg_t reg);
void micro_asm386_setlR8(micro_asm386_reg_t reg);
void micro_asm386_setleR8(micro_asm386_reg_t reg);

void micro_asm386_movzxR32R8(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);
void micro_asm386_movzxR16R8(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);

void micro_asm386_testR8R8(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);
void micro_asm386_testR16R16(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);
void micro_asm386_testR32R32(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);

void micro_asm386_testM8I8(micro_addr_le_t addr, micro_imm_le_t val);
void micro_asm386_testM16I16(micro_addr_le_t addr, micro_imm_le_t val);
void micro_asm386_testM32I32(micro_addr_le_t addr, micro_imm_le_t val);

void micro_asm386_testS32I8(micro_addr_le_t offset, micro_imm_le_t val);
void micro_asm386_testS32I16(micro_addr_le_t offset, micro_imm_le_t val);
void micro_asm386_testS32I32(micro_addr_le_t offset, micro_imm_le_t val);

// returns offset to address byte
i32  micro_asm386_jzL32(micro_imm_le_t lbl);
i32  micro_asm386_jnzL32(micro_imm_le_t lbl);
i32  micro_asm386_jmpL32(micro_imm_le_t lbl);

void micro_asm386_xchgR32R32(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);
void micro_asm386_xchgR16R16(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);
void micro_asm386_xchgR8R8(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);

void micro_asm386_pushR32(micro_asm386_reg_t reg);
void micro_asm386_pushR16(micro_asm386_reg_t reg);

void micro_asm386_popR32(micro_asm386_reg_t reg);
void micro_asm386_popR16(micro_asm386_reg_t reg);

void micro_asm386_movR32R32(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);
void micro_asm386_movR16R16(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);
void micro_asm386_movR8R8(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);

void micro_asm386_movM32I32(micro_addr_le_t addr, micro_imm_le_t val);
void micro_asm386_movM16I16(micro_addr_le_t addr, micro_imm_le_t val);
void micro_asm386_movM8I8(micro_addr_le_t addr, micro_imm_le_t val);

void micro_asm386_movR32M32(micro_asm386_reg_t reg, micro_addr_le_t addr);
void micro_asm386_movR16M16(micro_asm386_reg_t reg, micro_addr_le_t addr);
void micro_asm386_movR8M8(micro_asm386_reg_t reg, micro_addr_le_t addr);

void micro_asm386_movR32MR32(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);
void micro_asm386_movR16MR16(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);
void micro_asm386_movR8MR8(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);

void micro_asm386_movM32R32(micro_addr_le_t addr, micro_asm386_reg_t reg);
void micro_asm386_movM16R16(micro_addr_le_t addr, micro_asm386_reg_t reg);
void micro_asm386_movM8R8(micro_addr_le_t addr, micro_asm386_reg_t reg);

void micro_asm386_movR32I32(micro_asm386_reg_t reg, micro_imm_le_t val);
void micro_asm386_movR16I16(micro_asm386_reg_t reg, micro_imm_le_t val);
void micro_asm386_movR8I8(micro_asm386_reg_t reg, micro_imm_le_t val);

void micro_asm386_movS32I32(micro_imm_le_t offset, micro_imm_le_t val);
void micro_asm386_movS32I16(micro_imm_le_t offset, micro_imm_le_t val);
void micro_asm386_movS32I8(micro_imm_le_t offset, micro_imm_le_t val);

void micro_asm386_movS32R32(micro_imm_le_t offset, micro_asm386_reg_t reg);
void micro_asm386_movS32R16(micro_imm_le_t offset, micro_asm386_reg_t reg);
void micro_asm386_movS32R8(micro_imm_le_t offset, micro_asm386_reg_t reg);

void micro_asm386_movR32S32(micro_asm386_reg_t reg, micro_imm_le_t offset);
void micro_asm386_movR16S32(micro_asm386_reg_t reg, micro_imm_le_t offset);
void micro_asm386_movR8S32(micro_asm386_reg_t reg, micro_imm_le_t offset);

void micro_asm386_addR32R32(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);
void micro_asm386_addR16R16(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);
void micro_asm386_addR8R8(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);

void micro_asm386_addM32I32(micro_addr_le_t addr, micro_imm_le_t val);
void micro_asm386_addM16I16(micro_addr_le_t addr, micro_imm_le_t val);
void micro_asm386_addM8I8(micro_addr_le_t addr, micro_imm_le_t val);

void micro_asm386_addM32R32(micro_addr_le_t addr, micro_asm386_reg_t reg);
void micro_asm386_addM16R16(micro_addr_le_t addr, micro_asm386_reg_t reg);
void micro_asm386_addM8R8(micro_addr_le_t addr, micro_asm386_reg_t reg);

void micro_asm386_addS32I32(micro_imm_le_t offset, micro_imm_le_t val);
void micro_asm386_addS32I16(micro_imm_le_t offset, micro_imm_le_t val);
void micro_asm386_addS32I8(micro_imm_le_t offset, micro_imm_le_t val);

void micro_asm386_addS32R32(micro_imm_le_t offset, micro_asm386_reg_t reg);
void micro_asm386_addS32R16(micro_imm_le_t offset, micro_asm386_reg_t reg);
void micro_asm386_addS32R8(micro_imm_le_t offset, micro_asm386_reg_t reg);

void micro_asm386_addR32I32(micro_asm386_reg_t reg, micro_imm_le_t val);
void micro_asm386_addR16I16(micro_asm386_reg_t reg, micro_imm_le_t val);
void micro_asm386_addR8I8(micro_asm386_reg_t reg, micro_imm_le_t val);

void micro_asm386_addR32S32(micro_asm386_reg_t reg, micro_imm_le_t offset);
void micro_asm386_addR16S32(micro_asm386_reg_t reg, micro_imm_le_t offset);
void micro_asm386_addR8S32(micro_asm386_reg_t reg, micro_imm_le_t offset);

void micro_asm386_subR32R32(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);
void micro_asm386_subR16R16(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);
void micro_asm386_subR8R8(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);

void micro_asm386_subM32I32(micro_addr_le_t addr, micro_imm_le_t val);
void micro_asm386_subM16I16(micro_addr_le_t addr, micro_imm_le_t val);
void micro_asm386_subM8I8(micro_addr_le_t addr, micro_imm_le_t val);

void micro_asm386_subM32R32(micro_addr_le_t addr, micro_asm386_reg_t reg);
void micro_asm386_subM16R16(micro_addr_le_t addr, micro_asm386_reg_t reg);
void micro_asm386_subM8R8(micro_addr_le_t addr, micro_asm386_reg_t reg);

void micro_asm386_subS32I32(micro_imm_le_t offset, micro_imm_le_t val);
void micro_asm386_subS32I16(micro_imm_le_t offset, micro_imm_le_t val);
void micro_asm386_subS32I8(micro_imm_le_t offset, micro_imm_le_t val);

void micro_asm386_subS32R32(micro_imm_le_t offset, micro_asm386_reg_t reg);
void micro_asm386_subS32R16(micro_imm_le_t offset, micro_asm386_reg_t reg);
void micro_asm386_subS32R8(micro_imm_le_t offset, micro_asm386_reg_t reg);

void micro_asm386_subR32I32(micro_asm386_reg_t reg, micro_imm_le_t val);
void micro_asm386_subR16I16(micro_asm386_reg_t reg, micro_imm_le_t val);
void micro_asm386_subR8I8(micro_asm386_reg_t reg, micro_imm_le_t val);

void micro_asm386_subR32S32(micro_asm386_reg_t reg, micro_imm_le_t offset);
void micro_asm386_subR16S32(micro_asm386_reg_t reg, micro_imm_le_t offset);
void micro_asm386_subR8S32(micro_asm386_reg_t reg, micro_imm_le_t offset);

void micro_asm386_mulR32(micro_asm386_reg_t reg);
void micro_asm386_mulR16(micro_asm386_reg_t reg);
void micro_asm386_mulR8(micro_asm386_reg_t reg);

void micro_asm386_imulR32R32(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);
void micro_asm386_imulR16R16(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);
void micro_asm386_imulR8R8(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2);

void micro_asm386_divR32(micro_asm386_reg_t reg);
void micro_asm386_divR16(micro_asm386_reg_t reg);
void micro_asm386_divR8(micro_asm386_reg_t reg);

void micro_asm386_idivR32(micro_asm386_reg_t reg);
void micro_asm386_idivR16(micro_asm386_reg_t reg);
void micro_asm386_idivR8(micro_asm386_reg_t reg);

void micro_asm386_negR32(micro_asm386_reg_t reg);
void micro_asm386_negR16(micro_asm386_reg_t reg);
void micro_asm386_negR8(micro_asm386_reg_t reg);

void micro_asm386_leaR32S32(micro_asm386_reg_t reg, micro_imm_le_t offset);
void micro_asm386_leaR16S32(micro_asm386_reg_t reg, micro_imm_le_t offset);

#endif