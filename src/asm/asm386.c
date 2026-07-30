#include <micro/asm/asm386.h>

sct_vector_t micro_asm386_instr_stack;

#define  _micro_write_instr(...) do {  \
    u8 I[] = { __VA_ARGS__ };  \
    for (size_t i = 0; i < sizeof(I)/sizeof(*I); i++) {  \
        sct_vector_push(&micro_asm386_instr_stack, &I[i]);  \
    }  \
} while (0)

void micro_asm386_init()
{
    sct_vector_init(&micro_asm386_instr_stack, sizeof(u8));
}

void micro_asm386_deinit()
{
    sct_vector_deinit(&micro_asm386_instr_stack);
}

void micro_asm386_put_instructions(sct_vector_t *vec)
{
    for (size_t i = 0; i < micro_asm386_instr_stack.size; i++) {
        sct_vector_push(vec, sct_vector_get(&micro_asm386_instr_stack, i));
    }

    micro_asm386_deinit();
    micro_asm386_init();
}

void micro_asm386_put_instructions_to_addr(sct_vector_t *vec, size_t addr)
{
    for (size_t i = 0; i < micro_asm386_instr_stack.size; i++) {
        memcpy(vec->data + i * vec->_item_size, sct_vector_get(&micro_asm386_instr_stack, i), vec->_item_size);
    }

    micro_asm386_deinit();
    micro_asm386_init();
}

void micro_asm386_ret()                      { _micro_write_instr(0xC3); }
void micro_asm386_call(micro_addr_le_t addr) { _micro_write_instr(0xE8, (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3]); }
void micro_asm386_prelude()                  { _micro_write_instr(0x55, 0x89, 0xE5); }
void micro_asm386_epilogue()                 { _micro_write_instr(0xC9, 0xC3); }

void micro_asm386_cmpR32R32(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2) { _micro_write_instr(      0x39, 0b11000000 | ((reg1) << 3) | (reg2)); }
void micro_asm386_cmpR16R16(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2) { _micro_write_instr(0x66, 0x39, 0b11000000 | ((reg1) << 3) | (reg2)); }
void micro_asm386_cmpR8R8(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2)   { _micro_write_instr(      0x38, 0b11000000 | ((reg1) << 3) | (reg2)); }

void micro_asm386_setzR8(micro_asm386_reg_t reg)  { _micro_write_instr(0x0F, 0x94, 0xC0 | (reg)); }
void micro_asm386_setnzR8(micro_asm386_reg_t reg) { _micro_write_instr(0x0F, 0x95, 0xC0 | (reg)); }
void micro_asm386_setgR8(micro_asm386_reg_t reg)  { _micro_write_instr(0x0F, 0x9F, 0xC0 | (reg)); }
void micro_asm386_setgeR8(micro_asm386_reg_t reg) { _micro_write_instr(0x0F, 0x9D, 0xC0 | (reg)); }
void micro_asm386_setlR8(micro_asm386_reg_t reg)  { _micro_write_instr(0x0F, 0x9C, 0xC0 | (reg)); }
void micro_asm386_setleR8(micro_asm386_reg_t reg) { _micro_write_instr(0x0F, 0x9E, 0xC0 | (reg)); }

void micro_asm386_movzxR32R8(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2) { _micro_write_instr(      0x0F, 0xB6, 0b11000000 | ((reg1) << 3) | (reg2)); }
void micro_asm386_movzxR16R8(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2) { _micro_write_instr(0x66, 0x0F, 0xB6, 0b11000000 | ((reg1) << 3) | (reg2)); }

void micro_asm386_testR8R8(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2)   { _micro_write_instr(      0x84, 0x11000000 | ((reg1) << 3) | (reg2)); }
void micro_asm386_testR16R16(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2) { _micro_write_instr(0x66, 0x85, 0x11000000 | ((reg1) << 3) | (reg2)); }
void micro_asm386_testR32R32(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2) { _micro_write_instr(      0x85, 0x11000000 | ((reg1) << 3) | (reg2)); }

void micro_asm386_testM8I8(micro_addr_le_t addr, micro_imm_le_t val)   { _micro_write_instr(      0xF6, 0x05, (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3], (val).bytes[0]); }
void micro_asm386_testM16I16(micro_addr_le_t addr, micro_imm_le_t val) { _micro_write_instr(0x66, 0xF7, 0x05, (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3], (val).bytes[0], (val).bytes[1]); }
void micro_asm386_testM32I32(micro_addr_le_t addr, micro_imm_le_t val) { _micro_write_instr(      0xF7, 0x05, (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3], (val).bytes[0], (val).bytes[1], (val).bytes[2], (val).bytes[3]); }

void micro_asm386_testS32I8(micro_addr_le_t offset, micro_imm_le_t val)  { _micro_write_instr(      0xF6, 0b10000101, (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3], (val).bytes[0]); }
void micro_asm386_testS32I16(micro_addr_le_t offset, micro_imm_le_t val) { _micro_write_instr(0x66, 0xF7, 0b10000101, (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3], (val).bytes[0], (val).bytes[1]); }
void micro_asm386_testS32I32(micro_addr_le_t offset, micro_imm_le_t val) { _micro_write_instr(      0xF7, 0b10000101, (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3], (val).bytes[0], (val).bytes[1], (val).bytes[2], (val).bytes[3]); }

i32  micro_asm386_jzL32(micro_imm_le_t lbl)  { _micro_write_instr(0x0F, 0x84, (lbl).bytes[0], (lbl).bytes[1], (lbl).bytes[2], (lbl).bytes[3]); return 2; }
i32  micro_asm386_jnzL32(micro_imm_le_t lbl) { _micro_write_instr(0x0F, 0x85, (lbl).bytes[0], (lbl).bytes[1], (lbl).bytes[2], (lbl).bytes[3]); return 2; }
i32  micro_asm386_jmpL32(micro_imm_le_t lbl) { _micro_write_instr(0xE9, (lbl).bytes[0], (lbl).bytes[1], (lbl).bytes[2], (lbl).bytes[3]); return 1; }

void micro_asm386_xchgR32R32(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2) { _micro_write_instr(      0x87, 0b11000000 | ((reg1) << 3) | (reg2)); }
void micro_asm386_xchgR16R16(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2) { _micro_write_instr(0x66, 0x87, 0b11000000 | ((reg1) << 3) | (reg2)); }
void micro_asm386_xchgR8R8(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2)   { _micro_write_instr(      0x86, 0b11000000 | ((reg1) << 3) | (reg2)); }

void micro_asm386_pushR32(micro_asm386_reg_t reg) { _micro_write_instr(      0x50 + reg); }
void micro_asm386_pushR16(micro_asm386_reg_t reg) { _micro_write_instr(0x66, 0x50 + reg); }

void micro_asm386_popR32(micro_asm386_reg_t reg) { _micro_write_instr(      0x58 + (reg)); }
void micro_asm386_popR16(micro_asm386_reg_t reg) { _micro_write_instr(0x66, 0x58 + (reg)); }

void micro_asm386_movR32R32(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2) { _micro_write_instr(      0x8B, 0b11000000 | ((reg2) << 3) | (reg1)); }
void micro_asm386_movR16R16(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2) { _micro_write_instr(0x66, 0x8B, 0b11000000 | ((reg2) << 3) | (reg1)); }
void micro_asm386_movR8R8(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2)   { _micro_write_instr(      0x8A, 0b11000000 | ((reg2) << 3) | (reg1)); }

void micro_asm386_movM32I32(micro_addr_le_t addr, micro_imm_le_t val) { _micro_write_instr(      0xC7, 0x05, (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3], (val).bytes[0], (val).bytes[1], (val).bytes[2], (val).bytes[3]); }
void micro_asm386_movM16I16(micro_addr_le_t addr, micro_imm_le_t val) { _micro_write_instr(0x66, 0xC7, 0x05, (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3], (val).bytes[0], (val).bytes[1]                                ); }
void micro_asm386_movM8I8(micro_addr_le_t addr, micro_imm_le_t val)   { _micro_write_instr(      0xC6, 0x05, (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3], (val).bytes[0]                                                ); }

void micro_asm386_movR32M32(micro_asm386_reg_t reg, micro_addr_le_t addr) { _micro_write_instr(      0x8B, 0b00000101 | ((reg) << 3), (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3]); }
void micro_asm386_movR16M16(micro_asm386_reg_t reg, micro_addr_le_t addr) { _micro_write_instr(0x66, 0x8B, 0b00000101 | ((reg) << 3), (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3]); }
void micro_asm386_movR8M8(micro_asm386_reg_t reg, micro_addr_le_t addr)   { _micro_write_instr(      0x8A, 0b00000101 | ((reg) << 3), (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3]); }

void micro_asm386_movR32MR32(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2) { _micro_write_instr(      0x8B, 0b00000000 | ((reg1) << 3) | (reg2)); }
void micro_asm386_movR16MR16(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2) { _micro_write_instr(0x66, 0x8B, 0b00000000 | ((reg1) << 3) | (reg2)); }
void micro_asm386_movR8MR8(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2)   { _micro_write_instr(      0x8A, 0b00000000 | ((reg1) << 3) | (reg2)); }

void micro_asm386_movM32R32(micro_addr_le_t addr, micro_asm386_reg_t reg) { _micro_write_instr(      0x89, 0b00000101 | ((reg) << 3), (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3]); }
void micro_asm386_movM16R16(micro_addr_le_t addr, micro_asm386_reg_t reg) { _micro_write_instr(0x66, 0x89, 0b00000101 | ((reg) << 3), (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3]); }
void micro_asm386_movM8R8(micro_addr_le_t addr, micro_asm386_reg_t reg)   { _micro_write_instr(      0x88, 0b00000101 | ((reg) << 3), (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3]); }

void micro_asm386_movR32I32(micro_asm386_reg_t reg, micro_imm_le_t val) { _micro_write_instr(      0xB8 + reg, (val).bytes[0], (val).bytes[1], (val).bytes[2], (val).bytes[3]); }
void micro_asm386_movR16I16(micro_asm386_reg_t reg, micro_imm_le_t val) { _micro_write_instr(0x66, 0xB8 + reg, (val).bytes[0], (val).bytes[1]                                ); }
void micro_asm386_movR8I8(micro_asm386_reg_t reg, micro_imm_le_t val)   { _micro_write_instr(      0xB0 + reg, (val).bytes[0]                                                ); }

void micro_asm386_movS32I32(micro_imm_le_t offset, micro_imm_le_t val) { _micro_write_instr(      0xC7, 0b10000101, (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3], (val).bytes[0], (val).bytes[1], (val).bytes[2], (val).bytes[3]); }
void micro_asm386_movS32I16(micro_imm_le_t offset, micro_imm_le_t val) { _micro_write_instr(0x66, 0xC7, 0b10000101, (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3], (val).bytes[0], (val).bytes[1]                                ); }
void micro_asm386_movS32I8(micro_imm_le_t offset, micro_imm_le_t val)  { _micro_write_instr(      0xC6, 0b10000101, (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3], (val).bytes[0]                                                ); }

void micro_asm386_movS32R32(micro_imm_le_t offset, micro_asm386_reg_t reg) { _micro_write_instr(      0x89, 0b10000101 | ((reg) << 3), (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3]); }
void micro_asm386_movS32R16(micro_imm_le_t offset, micro_asm386_reg_t reg) { _micro_write_instr(0x66, 0x89, 0b10000101 | ((reg) << 3), (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3]); }
void micro_asm386_movS32R8(micro_imm_le_t offset, micro_asm386_reg_t reg)  { _micro_write_instr(      0x89, 0b10000101 | ((reg) << 3), (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3]); }

void micro_asm386_movR32S32(micro_asm386_reg_t reg, micro_imm_le_t offset) { _micro_write_instr(      0x8B, 0b10000101 | ((reg) << 3), (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3]); }
void micro_asm386_movR16S32(micro_asm386_reg_t reg, micro_imm_le_t offset) { _micro_write_instr(0x66, 0x8B, 0b10000101 | ((reg) << 3), (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3]); }
void micro_asm386_movR8S32(micro_asm386_reg_t reg, micro_imm_le_t offset)  { _micro_write_instr(      0x8A, 0b10000101 | ((reg) << 3), (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3]); }

void micro_asm386_addR32R32(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2) { _micro_write_instr(      0x01, 0b11000000 | ((reg2) << 3) | ((reg1))); }
void micro_asm386_addR16R16(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2) { _micro_write_instr(0x66, 0x01, 0b11000000 | ((reg2) << 3) | ((reg1))); }
void micro_asm386_addR8R8(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2)   { _micro_write_instr(      0x00, 0b11000000 | ((reg2) << 3) | ((reg1))); }

void micro_asm386_addM32I32(micro_addr_le_t addr, micro_imm_le_t val) { _micro_write_instr(      0x81, 0b00000101, (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3], (val).bytes[0], (val).bytes[1], (val).bytes[2], (val).bytes[3]); }
void micro_asm386_addM16I16(micro_addr_le_t addr, micro_imm_le_t val) { _micro_write_instr(0x66, 0x81, 0b00000101, (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3], (val).bytes[0], (val).bytes[1]                                ); }
void micro_asm386_addM8I8(micro_addr_le_t addr, micro_imm_le_t val)   { _micro_write_instr(      0x83, 0b00000101, (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3], (val).bytes[0]                                                ); }

void micro_asm386_addM32R32(micro_addr_le_t addr, micro_asm386_reg_t reg) { _micro_write_instr(      0x01, 0b00000101 | ((reg) << 3), (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3]); }
void micro_asm386_addM16R16(micro_addr_le_t addr, micro_asm386_reg_t reg) { _micro_write_instr(0x66, 0x01, 0b00000101 | ((reg) << 3), (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3]); }
void micro_asm386_addM8R8(micro_addr_le_t addr, micro_asm386_reg_t reg)   { _micro_write_instr(      0x00, 0b00000101 | ((reg) << 3), (addr).bytes[0], (addr).bytes[1], (addr).bytes[2], (addr).bytes[3]); }

void micro_asm386_addS32I32(micro_imm_le_t offset, micro_imm_le_t val) { _micro_write_instr(      0x81, 0b10000101, (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3], (val).bytes[0], (val).bytes[1], (val).bytes[2], (val).bytes[3]); }
void micro_asm386_addS32I16(micro_imm_le_t offset, micro_imm_le_t val) { _micro_write_instr(0x66, 0x81, 0b10000101, (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3], (val).bytes[0], (val).bytes[1]                                ); }
void micro_asm386_addS32I8(micro_imm_le_t offset, micro_imm_le_t val)  { _micro_write_instr(      0x83, 0b10000101, (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3], (val).bytes[0]                                                ); }

void micro_asm386_addS32R32(micro_imm_le_t offset, micro_asm386_reg_t reg) { _micro_write_instr(      0x01, 0b10000101 | ((reg) << 3), (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3]); }
void micro_asm386_addS32R16(micro_imm_le_t offset, micro_asm386_reg_t reg) { _micro_write_instr(0x66, 0x01, 0b10000101 | ((reg) << 3), (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3]); }
void micro_asm386_addS32R8(micro_imm_le_t offset, micro_asm386_reg_t reg)  { _micro_write_instr(      0x89, 0b10000101 | ((reg) << 3), (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3]); }

void micro_asm386_addR32I32(micro_asm386_reg_t reg, micro_imm_le_t val) { _micro_write_instr(      0x81, 0b11000000 | (reg), (val).bytes[0], (val).bytes[1], (val).bytes[2], (val).bytes[3]); }
void micro_asm386_addR16I16(micro_asm386_reg_t reg, micro_imm_le_t val) { _micro_write_instr(0x66, 0x81, 0b11000000 | (reg), (val).bytes[0], (val).bytes[1]                                ); }
void micro_asm386_addR8I8(micro_asm386_reg_t reg, micro_imm_le_t val)   { _micro_write_instr(      0x80, 0b11000000 | (reg), (val).bytes[0]                                                ); }

void micro_asm386_addR32S32(micro_asm386_reg_t reg, micro_imm_le_t offset) { _micro_write_instr(      0x03, 0b10000101 | ((reg) << 3), (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3]); }
void micro_asm386_addR16S32(micro_asm386_reg_t reg, micro_imm_le_t offset) { _micro_write_instr(0x66, 0x03, 0b10000101 | ((reg) << 3), (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3]); }
void micro_asm386_addR8S32(micro_asm386_reg_t reg, micro_imm_le_t offset)  { _micro_write_instr(      0x02, 0b10000101 | ((reg) << 3), (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3]); }

void micro_asm386_subR32R32(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2) { _micro_write_instr(      0x29, 0b11000000 | ((reg2) << 3) | ((reg1))); }
void micro_asm386_subR16R16(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2) { _micro_write_instr(0x66, 0x29, 0b11000000 | ((reg2) << 3) | ((reg1))); }
void micro_asm386_subR8R8(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2)   { _micro_write_instr(      0x28, 0b11000000 | ((reg2) << 3) | ((reg1))); }

void micro_asm386_mulR32(micro_asm386_reg_t reg) { _micro_write_instr(      0xF7, 0b11100000 | (reg)); }
void micro_asm386_mulR16(micro_asm386_reg_t reg) { _micro_write_instr(0x66, 0xF7, 0b11100000 | (reg)); }
void micro_asm386_mulR8(micro_asm386_reg_t reg)  { _micro_write_instr(      0xF6, 0b11100000 | (reg)); }

void micro_asm386_imulR32R32(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2) { _micro_write_instr(      0x0F, 0xAF, 0b11000000 | ((reg1) << 3) | ((reg2))); }
void micro_asm386_imulR16R16(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2) { _micro_write_instr(0x66, 0x0F, 0xAF, 0b11000000 | ((reg1) << 3) | ((reg2))); }
void micro_asm386_imulR8R8(micro_asm386_reg_t reg1, micro_asm386_reg_t reg2)   { _micro_write_instr(      0x0F, 0xAF, 0b11000000 | ((reg1) << 3) | ((reg2))); }

void micro_asm386_divR32(micro_asm386_reg_t reg) { _micro_write_instr(      0xF7, 0b11110000 | (reg)); }
void micro_asm386_divR16(micro_asm386_reg_t reg) { _micro_write_instr(0x66, 0xF7, 0b11110000 | (reg)); }
void micro_asm386_divR8(micro_asm386_reg_t reg)  { _micro_write_instr(      0xF6, 0b11110000 | (reg)); }

void micro_asm386_idivR32(micro_asm386_reg_t reg) { _micro_write_instr(      0xF7, 0b11111000 | (reg)); }
void micro_asm386_idivR16(micro_asm386_reg_t reg) { _micro_write_instr(0x66, 0xF7, 0b11111000 | (reg)); }
void micro_asm386_idivR8(micro_asm386_reg_t reg)  { _micro_write_instr(      0xF6, 0b11111000 | (reg)); }

void micro_asm386_negR32(micro_asm386_reg_t reg) { _micro_write_instr(      0xF7, 0b11011000 | (reg)); }
void micro_asm386_negR16(micro_asm386_reg_t reg) { _micro_write_instr(0x66, 0xF7, 0b11011000 | (reg)); }
void micro_asm386_negR8(micro_asm386_reg_t reg)  { _micro_write_instr(      0xF6, 0b11011000 | (reg)); }

void micro_asm386_leaR32S32(micro_asm386_reg_t reg, micro_imm_le_t offset) { _micro_write_instr(      0x8D, 0b10000100 | ((reg) << 3), 0b00100100, (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3]); }
void micro_asm386_leaR16S32(micro_asm386_reg_t reg, micro_imm_le_t offset) { _micro_write_instr(0x66, 0x8D, 0b10000100 | ((reg) << 3), 0b00100100, (offset).bytes[0], (offset).bytes[1], (offset).bytes[2], (offset).bytes[3]); }
