#include <micro/asm/asm386.h>

#define instr_handle(instr, size, ...)  \
    case instr:  \
        sct_vector_push_array(outbuf, &(u8[])__VA_ARGS__, (size));  \
        return

#define instr_handle_lbl(instr, S, N, ...)  \
    case instr:  \
        sct_vector_push_array(outbuf, &(u8[])__VA_ARGS__, (S));  \
        sct_vector_push(outbuf, &(deferred_lbl_t){ .addr = outbuf->size, .lbl_name = (N) });  \
        sct_vector_push_array(outbuf, &(u8[]){ 0, 0, 0, 0 }, 4);  \
        return

sct_hashmap_t lbls;

typedef struct {
    size_t addr;
    char  *lbl_name;
} deferred_lbl_t;

sct_vector_t deferred_lbls;

static inline void emit_instr(micro_asm386_instruction_t *instr, sct_vector_t *outbuf)
{
    switch (instr->opcode) {
        instr_handle(MICRO_ASM386_INSTR_MOV_R32R32, 2, {       0x8B, 0b11000000 | (instr->operand1.reg << 3) | instr->operand2.reg });
        instr_handle(MICRO_ASM386_INSTR_MOV_R16R16, 3, { 0x66, 0x8B, 0b11000000 | (instr->operand1.reg << 3) | instr->operand2.reg });
        instr_handle(MICRO_ASM386_INSTR_MOV_R8R8,   2, {       0x8A, 0b11000000 | (instr->operand1.reg << 3) | instr->operand2.reg });

        instr_handle(MICRO_ASM386_INSTR_MOV_R32I32, 5, {       0xB8 + instr->operand1.reg, instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1], instr->operand2.imm.bytes[2], instr->operand2.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_MOV_R16I16, 4, { 0x66, 0xB8 + instr->operand1.reg, instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1] });
        instr_handle(MICRO_ASM386_INSTR_MOV_R8I8,   2, {       0xB0 + instr->operand1.reg, instr->operand2.imm.bytes[0] });
      
        instr_handle(MICRO_ASM386_INSTR_MOV_M32R32, 6, {       0x89, 0b00000101 | (instr->operand2.reg << 3), instr->operand1.addr.bytes[0], instr->operand1.addr.bytes[1], instr->operand1.addr.bytes[2], instr->operand1.addr.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_MOV_M16R16, 7, { 0x66, 0x89, 0b00000101 | (instr->operand2.reg << 3), instr->operand1.addr.bytes[0], instr->operand1.addr.bytes[1], instr->operand1.addr.bytes[2], instr->operand1.addr.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_MOV_M8R8,   6, {       0x88, 0b00000101 | (instr->operand2.reg << 3), instr->operand1.addr.bytes[0], instr->operand1.addr.bytes[1], instr->operand1.addr.bytes[2], instr->operand1.addr.bytes[3] });

        instr_handle(MICRO_ASM386_INSTR_MOV_R32M32, 6, {       0x8B, 0b00000101 | (instr->operand1.reg << 3), instr->operand2.addr.bytes[0], instr->operand2.addr.bytes[1], instr->operand2.addr.bytes[2], instr->operand2.addr.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_MOV_R16M16, 7, { 0x66, 0x8B, 0b00000101 | (instr->operand1.reg << 3), instr->operand2.addr.bytes[0], instr->operand2.addr.bytes[1], instr->operand2.addr.bytes[2], instr->operand2.addr.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_MOV_R8M8,   6, {       0x8A, 0b00000101 | (instr->operand1.reg << 3), instr->operand2.addr.bytes[0], instr->operand2.addr.bytes[1], instr->operand2.addr.bytes[2], instr->operand2.addr.bytes[3] });
    
        instr_handle(MICRO_ASM386_INSTR_MOV_M32I32, 10, {       0xC7, 0b00000101, instr->operand1.addr.bytes[0], instr->operand1.addr.bytes[1], instr->operand1.addr.bytes[2], instr->operand1.addr.bytes[3], instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1], instr->operand2.imm.bytes[2], instr->operand2.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_MOV_M16I16, 9,  { 0x66, 0xC7, 0b00000101, instr->operand1.addr.bytes[0], instr->operand1.addr.bytes[1], instr->operand1.addr.bytes[2], instr->operand1.addr.bytes[3], instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1] });
        instr_handle(MICRO_ASM386_INSTR_MOV_M8I8,   7,  {       0xC6, 0b00000101, instr->operand1.addr.bytes[0], instr->operand1.addr.bytes[1], instr->operand1.addr.bytes[2], instr->operand1.addr.bytes[3], instr->operand2.imm.bytes[0] });
    
        instr_handle(MICRO_ASM386_INSTR_MOV_S32I32, 10, {       0xC7, 0b10000101, instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3], instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1], instr->operand2.imm.bytes[2], instr->operand2.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_MOV_S32I16, 9,  { 0x66, 0xC7, 0b10000101, instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3], instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1] });
        instr_handle(MICRO_ASM386_INSTR_MOV_S32I8,  7,  {       0xC6, 0b10000101, instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3], instr->operand2.imm.bytes[0] });
    
        instr_handle(MICRO_ASM386_INSTR_MOV_S32R32, 6, {       0x89, 0b10000101 | (instr->operand2.reg << 3), instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_MOV_S32R16, 7, { 0x66, 0x89, 0b10000101 | (instr->operand2.reg << 3), instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_MOV_S32R8,  6, {       0x88, 0b10000101 | (instr->operand2.reg << 3), instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3] });
    
        instr_handle(MICRO_ASM386_INSTR_MOV_R32S32, 6, {       0x8B, 0b10000101 | (instr->operand1.reg << 3), instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1], instr->operand2.imm.bytes[2], instr->operand2.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_MOV_R16S32, 7, { 0x66, 0x8B, 0b10000101 | (instr->operand1.reg << 3), instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1], instr->operand2.imm.bytes[2], instr->operand2.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_MOV_R8S32,  6, {       0x8A, 0b10000101 | (instr->operand1.reg << 3), instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1], instr->operand2.imm.bytes[2], instr->operand2.imm.bytes[3] });
    
        instr_handle(MICRO_ASM386_INSTR_MOV_R32MR32, 2, {       0x8B, 0b00000000 | (instr->operand1.reg << 3) | instr->operand2.reg });
        instr_handle(MICRO_ASM386_INSTR_MOV_R16MR16, 3, { 0x66, 0x8B, 0b00000000 | (instr->operand1.reg << 3) | instr->operand2.reg });
        instr_handle(MICRO_ASM386_INSTR_MOV_R8MR8,   2, {       0x8A, 0b00000000 | (instr->operand1.reg << 3) | instr->operand2.reg });

        instr_handle_lbl(MICRO_ASM386_INSTR_MOV_R32L32, 2, instr->operand2.lbl_name, { 0xB8 + instr->operand1.reg });

        instr_handle(MICRO_ASM386_INSTR_ADD_R32R32, 2, {       0x01, 0b11000000 | (instr->operand2.reg << 3) | instr->operand1.reg });
        instr_handle(MICRO_ASM386_INSTR_ADD_R16R16, 3, { 0x66, 0x01, 0b11000000 | (instr->operand2.reg << 3) | instr->operand1.reg });
        instr_handle(MICRO_ASM386_INSTR_ADD_R8R8,   2, {       0x00, 0b11000000 | (instr->operand2.reg << 3) | instr->operand1.reg });
    
        instr_handle(MICRO_ASM386_INSTR_ADD_R32I32, 6, {       0x81, 0b11000000 | instr->operand1.reg, instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1], instr->operand2.imm.bytes[2], instr->operand2.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_ADD_R16I16, 5, { 0x66, 0x81, 0b11000000 | instr->operand1.reg, instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1] });
        instr_handle(MICRO_ASM386_INSTR_ADD_R8I8,   3, {       0x80, 0b11000000 | instr->operand1.reg, instr->operand2.imm.bytes[0] });

        instr_handle(MICRO_ASM386_INSTR_ADD_M32R32, 6, {       0x01, 0b00000101 | (instr->operand2.reg << 3), instr->operand1.addr.bytes[0], instr->operand1.addr.bytes[1], instr->operand1.addr.bytes[2], instr->operand1.addr.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_ADD_M16R16, 7, { 0x66, 0x01, 0b00000101 | (instr->operand2.reg << 3), instr->operand1.addr.bytes[0], instr->operand1.addr.bytes[1], instr->operand1.addr.bytes[2], instr->operand1.addr.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_ADD_M8R8,   6, {       0x00, 0b00000101 | (instr->operand2.reg << 3), instr->operand1.addr.bytes[0], instr->operand1.addr.bytes[1], instr->operand1.addr.bytes[2], instr->operand1.addr.bytes[3] });

        instr_handle(MICRO_ASM386_INSTR_ADD_M32I32, 10, {       0x81, 0b00000101, instr->operand1.addr.bytes[0], instr->operand1.addr.bytes[1], instr->operand1.addr.bytes[2], instr->operand1.addr.bytes[3], instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1], instr->operand2.imm.bytes[2], instr->operand2.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_ADD_M16I16, 9,  { 0x66, 0x81, 0b00000101, instr->operand1.addr.bytes[0], instr->operand1.addr.bytes[1], instr->operand1.addr.bytes[2], instr->operand1.addr.bytes[3], instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1] });
        instr_handle(MICRO_ASM386_INSTR_ADD_M8I8,   7,  {       0x80, 0b00000101, instr->operand1.addr.bytes[0], instr->operand1.addr.bytes[1], instr->operand1.addr.bytes[2], instr->operand1.addr.bytes[3], instr->operand2.imm.bytes[0] });

        instr_handle(MICRO_ASM386_INSTR_ADD_S32I32, 10, {       0x81, 0b10000101, instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3], instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1], instr->operand2.imm.bytes[2], instr->operand2.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_ADD_S32I16, 9,  { 0x66, 0x81, 0b10000101, instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3], instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1] });
        instr_handle(MICRO_ASM386_INSTR_ADD_S32I8,  7,  {       0x80, 0b10000101, instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3], instr->operand2.imm.bytes[0] });

        instr_handle(MICRO_ASM386_INSTR_ADD_S32R32, 6, {       0x01, 0b10000101 | (instr->operand2.reg << 3), instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_ADD_S32R16, 7, { 0x66, 0x01, 0b10000101 | (instr->operand2.reg << 3), instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_ADD_S32R8,  6, {       0x00, 0b10000101 | (instr->operand2.reg << 3), instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3] });

        instr_handle(MICRO_ASM386_INSTR_ADD_R32S32, 6, {       0x03, 0b10000101 | (instr->operand1.reg << 3), instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1], instr->operand2.imm.bytes[2], instr->operand2.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_ADD_R16S32, 7, { 0x66, 0x03, 0b10000101 | (instr->operand1.reg << 3), instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1], instr->operand2.imm.bytes[2], instr->operand2.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_ADD_R8S32,  6, {       0x02, 0b10000101 | (instr->operand1.reg << 3), instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1], instr->operand2.imm.bytes[2], instr->operand2.imm.bytes[3] });

        instr_handle(MICRO_ASM386_INSTR_SUB_R32R32, 2, {       0x29, 0b11000000 | (instr->operand2.reg << 3) | instr->operand1.reg });
        instr_handle(MICRO_ASM386_INSTR_SUB_R16R16, 3, { 0x66, 0x29, 0b11000000 | (instr->operand2.reg << 3) | instr->operand1.reg });
        instr_handle(MICRO_ASM386_INSTR_SUB_R8R8,   2, {       0x28, 0b11000000 | (instr->operand2.reg << 3) | instr->operand1.reg });

        instr_handle(MICRO_ASM386_INSTR_SUB_M32I32, 10, {       0x81, 0b00101101, instr->operand1.addr.bytes[0], instr->operand1.addr.bytes[1], instr->operand1.addr.bytes[2], instr->operand1.addr.bytes[3], instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1], instr->operand2.imm.bytes[2], instr->operand2.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_SUB_M16I16, 9,  { 0x66, 0x81, 0b00101101, instr->operand1.addr.bytes[0], instr->operand1.addr.bytes[1], instr->operand1.addr.bytes[2], instr->operand1.addr.bytes[3], instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1] });
        instr_handle(MICRO_ASM386_INSTR_SUB_M8I8,   7,  {       0x80, 0b00101101, instr->operand1.addr.bytes[0], instr->operand1.addr.bytes[1], instr->operand1.addr.bytes[2], instr->operand1.addr.bytes[3], instr->operand2.imm.bytes[0] });

        instr_handle(MICRO_ASM386_INSTR_SUB_M32R32, 6, {       0x29, 0b00000101 | (instr->operand2.reg << 3), instr->operand1.addr.bytes[0], instr->operand1.addr.bytes[1], instr->operand1.addr.bytes[2], instr->operand1.addr.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_SUB_M16R16, 7, { 0x66, 0x29, 0b00000101 | (instr->operand2.reg << 3), instr->operand1.addr.bytes[0], instr->operand1.addr.bytes[1], instr->operand1.addr.bytes[2], instr->operand1.addr.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_SUB_M8R8,   6, {       0x28, 0b00000101 | (instr->operand2.reg << 3), instr->operand1.addr.bytes[0], instr->operand1.addr.bytes[1], instr->operand1.addr.bytes[2], instr->operand1.addr.bytes[3] });

        instr_handle(MICRO_ASM386_INSTR_SUB_S32I32, 10, {       0x81, 0b10101101, instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3], instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1], instr->operand2.imm.bytes[2], instr->operand2.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_SUB_S32I16, 9,  { 0x66, 0x81, 0b10101101, instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3], instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1] });
        instr_handle(MICRO_ASM386_INSTR_SUB_S32I8,  7,  {       0x80, 0b10101101, instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3], instr->operand2.imm.bytes[0] });

        instr_handle(MICRO_ASM386_INSTR_SUB_S32R32, 6, {       0x29, 0b10000101 | (instr->operand2.reg << 3), instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_SUB_S32R16, 7, { 0x66, 0x29, 0b10000101 | (instr->operand2.reg << 3), instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_SUB_S32R8,  6, {       0x28, 0b10000101 | (instr->operand2.reg << 3), instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3] });

        instr_handle(MICRO_ASM386_INSTR_SUB_R32I32, 6, {       0x81, 0b11101000 | instr->operand1.reg, instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1], instr->operand2.imm.bytes[2], instr->operand2.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_SUB_R16I16, 5, { 0x66, 0x81, 0b11101000 | instr->operand1.reg, instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1] });
        instr_handle(MICRO_ASM386_INSTR_SUB_R8I8,   3, {       0x80, 0b11101000 | instr->operand1.reg, instr->operand2.imm.bytes[0] });

        instr_handle(MICRO_ASM386_INSTR_SUB_R32S32, 6, {       0x2B, 0b10000101 | (instr->operand1.reg << 3), instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1], instr->operand2.imm.bytes[2], instr->operand2.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_SUB_R16S32, 7, { 0x66, 0x2B, 0b10000101 | (instr->operand1.reg << 3), instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1], instr->operand2.imm.bytes[2], instr->operand2.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_SUB_R8S32,  6, {       0x2A, 0b10000101 | (instr->operand1.reg << 3), instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1], instr->operand2.imm.bytes[2], instr->operand2.imm.bytes[3] });

        instr_handle(MICRO_ASM386_INSTR_MUL_R32, 2, {       0xF7, 0b11100000 | instr->operand1.reg });
        instr_handle(MICRO_ASM386_INSTR_MUL_R16, 3, { 0x66, 0xF7, 0b11100000 | instr->operand1.reg });
        instr_handle(MICRO_ASM386_INSTR_MUL_R8,  2, {       0xF6, 0b11100000 | instr->operand1.reg });

        instr_handle(MICRO_ASM386_INSTR_IMUL_R32R32, 3, {       0x0F, 0xAF, 0b11000000 | (instr->operand1.reg << 3) | instr->operand2.reg });
        instr_handle(MICRO_ASM386_INSTR_IMUL_R16R16, 4, { 0x66, 0x0F, 0xAF, 0b11000000 | (instr->operand1.reg << 3) | instr->operand2.reg });
        instr_handle(MICRO_ASM386_INSTR_IMUL_R8R8,   3, {       0x0F, 0xAF, 0b11000000 | (instr->operand1.reg << 3) | instr->operand2.reg });

        instr_handle(MICRO_ASM386_INSTR_DIV_R32, 2, {       0xF7, 0b11110000 | instr->operand1.reg });
        instr_handle(MICRO_ASM386_INSTR_DIV_R16, 3, { 0x66, 0xF7, 0b11110000 | instr->operand1.reg });
        instr_handle(MICRO_ASM386_INSTR_DIV_R8,  2, {       0xF6, 0b11110000 | instr->operand1.reg });

        instr_handle(MICRO_ASM386_INSTR_IDIV_R32, 2, {       0xF7, 0b11111000 | instr->operand1.reg });
        instr_handle(MICRO_ASM386_INSTR_IDIV_R16, 3, { 0x66, 0xF7, 0b11111000 | instr->operand1.reg });
        instr_handle(MICRO_ASM386_INSTR_IDIV_R8,  2, {       0xF6, 0b11111000 | instr->operand1.reg });

        instr_handle(MICRO_ASM386_INSTR_NEG_R32, 2, {       0xF7, 0b11011000 | instr->operand1.reg });
        instr_handle(MICRO_ASM386_INSTR_NEG_R16, 3, { 0x66, 0xF7, 0b11011000 | instr->operand1.reg });
        instr_handle(MICRO_ASM386_INSTR_NEG_R8,  2, {       0xF6, 0b11011000 | instr->operand1.reg });

        instr_handle(MICRO_ASM386_INSTR_LEA_R32S32, 7, {       0x8D, 0b10000100 | (instr->operand1.reg << 3), 0b00100100, instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1], instr->operand2.imm.bytes[2], instr->operand2.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_LEA_R16S32, 8, { 0x66, 0x8D, 0b10000100 | (instr->operand1.reg << 3), 0b00100100, instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1], instr->operand2.imm.bytes[2], instr->operand2.imm.bytes[3] });

        instr_handle(MICRO_ASM386_INSTR_RET,        1, { 0xC3 });
        instr_handle(MICRO_ASM386_INSTR_CALL_S32,   5, { 0xE8, instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3] });

        instr_handle(MICRO_ASM386_INSTR_PRELUDE,    3, { 0x55, 0x89, 0xE5 });
        instr_handle(MICRO_ASM386_INSTR_EPILOGUE,   2, { 0xC9, 0xC3 });

        instr_handle(MICRO_ASM386_INSTR_CMP_R32R32, 2, {       0x39, 0b11000000 | (instr->operand1.reg << 3) | instr->operand2.reg });
        instr_handle(MICRO_ASM386_INSTR_CMP_R16R16, 3, { 0x66, 0x39, 0b11000000 | (instr->operand1.reg << 3) | instr->operand2.reg });
        instr_handle(MICRO_ASM386_INSTR_CMP_R8R8,   2, {       0x38, 0b11000000 | (instr->operand1.reg << 3) | instr->operand2.reg });

        instr_handle(MICRO_ASM386_INSTR_SETZ_R8,   3, { 0x0F, 0x94, 0xC0 | instr->operand1.reg });
        instr_handle(MICRO_ASM386_INSTR_SETNZ_R8,  3, { 0x0F, 0x95, 0xC0 | instr->operand1.reg });
        instr_handle(MICRO_ASM386_INSTR_SETG_R8,   3, { 0x0F, 0x9F, 0xC0 | instr->operand1.reg });
        instr_handle(MICRO_ASM386_INSTR_SETGE_R8,  3, { 0x0F, 0x9D, 0xC0 | instr->operand1.reg });
        instr_handle(MICRO_ASM386_INSTR_SETL_R8,   3, { 0x0F, 0x9C, 0xC0 | instr->operand1.reg });
        instr_handle(MICRO_ASM386_INSTR_SETLE_R8,  3, { 0x0F, 0x9E, 0xC0 | instr->operand1.reg });

        instr_handle(MICRO_ASM386_INSTR_MOVZX_R32R8, 3, {       0x0F, 0xB6, 0b11000000 | (instr->operand1.reg << 3) | instr->operand2.reg });
        instr_handle(MICRO_ASM386_INSTR_MOVZX_R16R8, 4, { 0x66, 0x0F, 0xB6, 0b11000000 | (instr->operand1.reg << 3) | instr->operand2.reg });

        instr_handle(MICRO_ASM386_INSTR_TEST_R32R32, 2, {       0x85, 0b11000000 | (instr->operand1.reg << 3) | instr->operand2.reg });
        instr_handle(MICRO_ASM386_INSTR_TEST_R16R16, 3, { 0x66, 0x85, 0b11000000 | (instr->operand1.reg << 3) | instr->operand2.reg });
        instr_handle(MICRO_ASM386_INSTR_TEST_R8R8,   2, {       0x84, 0b11000000 | (instr->operand1.reg << 3) | instr->operand2.reg });

        instr_handle(MICRO_ASM386_INSTR_TEST_M32I32, 10, {       0xF7, 0x05, instr->operand1.addr.bytes[0], instr->operand1.addr.bytes[1], instr->operand1.addr.bytes[2], instr->operand1.addr.bytes[3], instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1], instr->operand2.imm.bytes[2], instr->operand2.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_TEST_M16I16, 9,  { 0x66, 0xF7, 0x05, instr->operand1.addr.bytes[0], instr->operand1.addr.bytes[1], instr->operand1.addr.bytes[2], instr->operand1.addr.bytes[3], instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1] });
        instr_handle(MICRO_ASM386_INSTR_TEST_M8I8,   7,  {       0xF6, 0x05, instr->operand1.addr.bytes[0], instr->operand1.addr.bytes[1], instr->operand1.addr.bytes[2], instr->operand1.addr.bytes[3], instr->operand2.imm.bytes[0] });

        instr_handle(MICRO_ASM386_INSTR_TEST_S32I32, 10, {       0xF7, 0b10000101, instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3], instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1], instr->operand2.imm.bytes[2], instr->operand2.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_TEST_S32I16, 9,  { 0x66, 0xF7, 0b10000101, instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3], instr->operand2.imm.bytes[0], instr->operand2.imm.bytes[1] });
        instr_handle(MICRO_ASM386_INSTR_TEST_S32I8,   7,  {       0xF6, 0b10000101, instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3], instr->operand2.imm.bytes[0] });

        instr_handle(MICRO_ASM386_INSTR_JZ_S32,   6, { 0x0F, 0x84, instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_JNZ_S32,  6, { 0x0F, 0x85, instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3] });
        instr_handle(MICRO_ASM386_INSTR_JMP_S32,  5, { 0xE9, instr->operand1.imm.bytes[0], instr->operand1.imm.bytes[1], instr->operand1.imm.bytes[2], instr->operand1.imm.bytes[3] });

        instr_handle(MICRO_ASM386_INSTR_XCHG_R32R32, 2, {       0x87, 0b11000000 | (instr->operand1.reg << 3) | instr->operand2.reg });
        instr_handle(MICRO_ASM386_INSTR_XCHG_R16R16, 3, { 0x66, 0x87, 0b11000000 | (instr->operand1.reg << 3) | instr->operand2.reg });
        instr_handle(MICRO_ASM386_INSTR_XCHG_R8R8,   2, {       0x86, 0b11000000 | (instr->operand1.reg << 3) | instr->operand2.reg });

        instr_handle(MICRO_ASM386_INSTR_PUSH_R32, 1, {       0x50 + instr->operand1.reg });
        instr_handle(MICRO_ASM386_INSTR_PUSH_R16, 2, { 0x66, 0x50 + instr->operand1.reg });
        instr_handle(MICRO_ASM386_INSTR_POP_R32,  1, {       0x58 + instr->operand1.reg });
        instr_handle(MICRO_ASM386_INSTR_POP_R16,  2, { 0x66, 0x58 + instr->operand1.reg });

        case MICRO_ASM386_INSTR_LBL:
            sct_hashmap_add(&lbls, instr->operand1.lbl_name, &outbuf->size);
            return;
        
        default:
            puts("undefined instr");
            return;
    };
}

void micro_asm386_emit(sct_vector_t *instrs, sct_vector_t *outbuf)
{
    sct_hashmap_init(&lbls, sizeof(size_t));
    sct_vector_init(&deferred_lbls, sizeof(deferred_lbl_t));

    for (size_t i = 0; i < instrs->size; i++) {
        micro_asm386_instruction_t *instr = sct_vector_get(instrs, i);
        if (unlikely(!instr)) {
            return;
        }
        emit_instr(instr, outbuf);
    }

    for (size_t i = 0; i < deferred_lbls.size; i++) {
        deferred_lbl_t *dlbl = sct_vector_get(&deferred_lbls, i);

        size_t *lbl_val = sct_hashmap_get(&lbls, dlbl->lbl_name);
        if (!lbl_val) {
            puts("Internal asm error: undefined label");
            return;
        }

        micro_imm_le_t lbl_imm = micro_imm_le_gen(*lbl_val);
        sct_vector_set(outbuf, dlbl->addr,     &lbl_imm.bytes[0]);
        sct_vector_set(outbuf, dlbl->addr + 1, &lbl_imm.bytes[1]);
        sct_vector_set(outbuf, dlbl->addr + 2, &lbl_imm.bytes[2]);
        sct_vector_set(outbuf, dlbl->addr + 3, &lbl_imm.bytes[3]);
    }

    sct_vector_deinit(&deferred_lbls);
    sct_vector_deinit(instrs);
    sct_vector_init(instrs, sizeof(micro_asm386_instruction_t));

    sct_hashmap_deinit(&lbls);
}