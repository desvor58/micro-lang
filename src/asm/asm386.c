#include "asm386.h"

__micro_asm386_instruction_t __micro_asm_instruction_stack[MICRO_INSTRUCTION_STACK_SIZE];
u8 __micro_asm_instruction_stack_top = 0;

void asm_put_instructions()
{
    for (size_t i = 0; i < __micro_asm_instruction_stack_top; i++) {
        for (size_t j = 0; j < __micro_asm_instruction_stack[i].size; j++) {
            sct_string_push_back(micro_outbuf, __micro_asm_instruction_stack[i].instr[j]);
        }
    }
    __micro_asm_instruction_stack_top = 0;
}

void asm_put_instructions_to_addr(size_t addr)
{
    for (size_t i = 0; i < __micro_asm_instruction_stack_top; i++) {
        for (size_t j = 0; j < __micro_asm_instruction_stack[i].size; j++) {
            micro_outbuf->str[addr + i] = __micro_asm_instruction_stack[i].instr[j];
        }
    }
    __micro_asm_instruction_stack_top = 0;
}

void asm386_ret()                      { __micro_write_instr(0xC3); }
void asm386_call(micro_addr_le_t addr) { __micro_write_instr(0xE8, (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3]); }
void asm386_prelude()                  { __micro_write_instr(0x55, 0x89, 0xE5); }
void asm386_epilogue()                 { __micro_write_instr(0xC9, 0xC3); }

void asm386_cmpR32R32(asm386_reg reg1, asm386_reg reg2) { __micro_write_instr(      0x39, 0b11000000 | ((reg1) << 3) | (reg2)); }
void asm386_cmpR16R16(asm386_reg reg1, asm386_reg reg2) { __micro_write_instr(0x66, 0x39, 0b11000000 | ((reg1) << 3) | (reg2)); }
void asm386_cmpR8R8(asm386_reg reg1, asm386_reg reg2)   { __micro_write_instr(      0x38, 0b11000000 | ((reg1) << 3) | (reg2)); }

void asm386_xchgR32R32(asm386_reg reg1, asm386_reg reg2) { __micro_write_instr(      0x87, 0b11000000 | ((reg1) << 3) | (reg2)); }
void asm386_xchgR16R16(asm386_reg reg1, asm386_reg reg2) { __micro_write_instr(0x66, 0x87, 0b11000000 | ((reg1) << 3) | (reg2)); }
void asm386_xchgR8R8(asm386_reg reg1, asm386_reg reg2)   { __micro_write_instr(      0x86, 0b11000000 | ((reg1) << 3) | (reg2)); }

void asm386_pushR32(asm386_reg reg) { __micro_write_instr(      0x50 + reg); }
void asm386_pushR16(asm386_reg reg) { __micro_write_instr(0x66, 0x50 + reg); }

void asm386_popR32(asm386_reg reg) { __micro_write_instr(      0x58 + (reg)); }
void asm386_popR16(asm386_reg reg) { __micro_write_instr(0x66, 0x58 + (reg)); }

void asm386_movR32R32(asm386_reg reg1, asm386_reg reg2) { __micro_write_instr(      0x8B, 0b11000000 | ((reg2) << 3) | (reg1)); }
void asm386_movR16R16(asm386_reg reg1, asm386_reg reg2) { __micro_write_instr(0x66, 0x8B, 0b11000000 | ((reg2) << 3) | (reg1)); }
void asm386_movR8R8(asm386_reg reg1, asm386_reg reg2)   { __micro_write_instr(      0x8A, 0b11000000 | ((reg2) << 3) | (reg1)); }

void asm386_movM32I32(micro_addr_le_t addr, micro_imm_le_t val) { __micro_write_instr(      0xC7, 0x05, (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3], (val).bytes[0], (val).bytes[1], (val).bytes[2], (val).bytes[3]); }
void asm386_movM16I16(micro_addr_le_t addr, micro_imm_le_t val) { __micro_write_instr(0x66, 0xC7, 0x05, (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3], (val).bytes[0], (val).bytes[1]                                ); }
void asm386_movM8I8(micro_addr_le_t addr, micro_imm_le_t val)   { __micro_write_instr(      0xC6, 0x05, (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3], (val).bytes[0]                                                ); }

void asm386_movR32M32(asm386_reg reg, micro_addr_le_t addr) { __micro_write_instr(      0x8B, 0b00000101 | ((reg) << 3), (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3]); }
void asm386_movR16M16(asm386_reg reg, micro_addr_le_t addr) { __micro_write_instr(0x66, 0x8B, 0b00000101 | ((reg) << 3), (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3]); }
void asm386_movR8M8(asm386_reg reg, micro_addr_le_t addr)   { __micro_write_instr(      0x8A, 0b00000101 | ((reg) << 3), (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3]); }

void asm386_movR32MR32(asm386_reg reg1, asm386_reg reg2) { __micro_write_instr(      0x8B, 0b00000000 | ((reg1) << 3) | (reg2)); }
void asm386_movR16MR16(asm386_reg reg1, asm386_reg reg2) { __micro_write_instr(0x66, 0x8B, 0b00000000 | ((reg1) << 3) | (reg2)); }
void asm386_movR8MR8(asm386_reg reg1, asm386_reg reg2)   { __micro_write_instr(      0x8A, 0b00000000 | ((reg1) << 3) | (reg2)); }

void asm386_movM32R32(micro_addr_le_t addr, asm386_reg reg) { __micro_write_instr(      0x89, 0b00000101 | ((reg) << 3), (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3]); }
void asm386_movM16R16(micro_addr_le_t addr, asm386_reg reg) { __micro_write_instr(0x66, 0x89, 0b00000101 | ((reg) << 3), (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3]); }
void asm386_movM8R8(micro_addr_le_t addr, asm386_reg reg)   { __micro_write_instr(      0x88, 0b00000101 | ((reg) << 3), (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3]); }

void asm386_movR32I32(asm386_reg reg, micro_imm_le_t val) { __micro_write_instr(      0xB8 + reg, (val).bytes[0], (val).bytes[1], (val).bytes[2], (val).bytes[3]); }
void asm386_movR16I16(asm386_reg reg, micro_imm_le_t val) { __micro_write_instr(0x66, 0xB8 + reg, (val).bytes[0], (val).bytes[1]                                ); }
void asm386_movR8I8(asm386_reg reg, micro_imm_le_t val)   { __micro_write_instr(      0xB0 + reg, (val).bytes[0]                                                ); }

void asm386_movS32I32(micro_imm_le_t offset, micro_imm_le_t val) { __micro_write_instr(      0xC7, 0b10000101, (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3], (val).bytes[0], (val).bytes[1], (val).bytes[2], (val).bytes[3]); }
void asm386_movS32I16(micro_imm_le_t offset, micro_imm_le_t val) { __micro_write_instr(0x66, 0xC7, 0b10000101, (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3], (val).bytes[0], (val).bytes[1]                                ); }
void asm386_movS32I8(micro_imm_le_t offset, micro_imm_le_t val)  { __micro_write_instr(      0xC6, 0b10000101, (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3], (val).bytes[0]                                                ); }

void asm386_movS32R32(micro_imm_le_t offset, asm386_reg reg) { __micro_write_instr(      0x89, 0b10000101 | ((reg) << 3), (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3]); }
void asm386_movS32R16(micro_imm_le_t offset, asm386_reg reg) { __micro_write_instr(0x66, 0x89, 0b10000101 | ((reg) << 3), (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3]); }
void asm386_movS32R8(micro_imm_le_t offset, asm386_reg reg)  { __micro_write_instr(      0x89, 0b10000101 | ((reg) << 3), (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3]); }

void asm386_movR32S32(asm386_reg reg, micro_imm_le_t offset) { __micro_write_instr(      0x8B, 0b10000101 | ((reg) << 3), (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3]); }
void asm386_movR16S32(asm386_reg reg, micro_imm_le_t offset) { __micro_write_instr(0x66, 0x8B, 0b10000101 | ((reg) << 3), (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3]); }
void asm386_movR8S32(asm386_reg reg, micro_imm_le_t offset)  { __micro_write_instr(      0x8A, 0b10000101 | ((reg) << 3), (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3]); }

void asm386_addR32R32(asm386_reg reg1, asm386_reg reg2) { __micro_write_instr(      0x01, 0b11000000 | ((reg2) << 3) | ((reg1))); }
void asm386_addR16R16(asm386_reg reg1, asm386_reg reg2) { __micro_write_instr(0x66, 0x01, 0b11000000 | ((reg2) << 3) | ((reg1))); }
void asm386_addR8R8(asm386_reg reg1, asm386_reg reg2)   { __micro_write_instr(      0x00, 0b11000000 | ((reg2) << 3) | ((reg1))); }

void asm386_subR32R32(asm386_reg reg1, asm386_reg reg2) { __micro_write_instr(      0x29, 0b11000000 | ((reg2) << 3) | ((reg1))); }
void asm386_subR16R16(asm386_reg reg1, asm386_reg reg2) { __micro_write_instr(0x66, 0x29, 0b11000000 | ((reg2) << 3) | ((reg1))); }
void asm386_subR8R8(asm386_reg reg1, asm386_reg reg2)   { __micro_write_instr(      0x28, 0b11000000 | ((reg2) << 3) | ((reg1))); }

void asm386_mulR32(asm386_reg reg) { __micro_write_instr(      0xF7, 0b11100000 | (reg)); }
void asm386_mulR16(asm386_reg reg) { __micro_write_instr(0x66, 0xF7, 0b11100000 | (reg)); }
void asm386_mulR8(asm386_reg reg)  { __micro_write_instr(      0xF6, 0b11100000 | (reg)); }

void asm386_imulR32R32(asm386_reg reg1, asm386_reg reg2) { __micro_write_instr(      0x0F, 0xAF, 0b11000000 | ((reg1) << 3) | ((reg2))); }
void asm386_imulR16R16(asm386_reg reg1, asm386_reg reg2) { __micro_write_instr(0x66, 0x0F, 0xAF, 0b11000000 | ((reg1) << 3) | ((reg2))); }
void asm386_imulR8R8(asm386_reg reg1, asm386_reg reg2)   { __micro_write_instr(      0x0F, 0xAF, 0b11000000 | ((reg1) << 3) | ((reg2))); }

void asm386_divR32(asm386_reg reg) { __micro_write_instr(      0xF7, 0b11110000 | (reg)); }
void asm386_divR16(asm386_reg reg) { __micro_write_instr(0x66, 0xF7, 0b11110000 | (reg)); }
void asm386_divR8(asm386_reg reg)  { __micro_write_instr(      0xF6, 0b11110000 | (reg)); }

void asm386_idivR32(asm386_reg reg) { __micro_write_instr(      0xF7, 0b11111000 | (reg)); }
void asm386_idivR16(asm386_reg reg) { __micro_write_instr(0x66, 0xF7, 0b11111000 | (reg)); }
void asm386_idivR8(asm386_reg reg)  { __micro_write_instr(      0xF6, 0b11111000 | (reg)); }

void asm386_negR32(asm386_reg reg) { __micro_write_instr(      0xF7, 0b11011000 | (reg)); }
void asm386_negR16(asm386_reg reg) { __micro_write_instr(0x66, 0xF7, 0b11011000 | (reg)); }
void asm386_negR8(asm386_reg reg)  { __micro_write_instr(      0xF6, 0b11011000 | (reg)); }

void asm386_leaR32S32(asm386_reg reg, micro_imm_le_t offset) { __micro_write_instr(      0x8D, 0b10000100 | ((reg) << 3), 0b00100100, (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3]) }
void asm386_leaR16S32(asm386_reg reg, micro_imm_le_t offset) { __micro_write_instr(0x66, 0x8D, 0b10000100 | ((reg) << 3), 0b00100100, (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3]) }

void asm386_jmpL32(micro_imm_le_t lbl) { __micro_write_instr(0xE9, (lbl).bytes[0], (lbl).bytes[1], (lbl).bytes[2], (lbl).bytes[3]); }

