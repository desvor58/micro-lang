#include <micro/common.h>

micro_error_t *micro_err_stk;
size_t         micro_err_stk_size;
size_t        _micro_err_stk_real_size;

micro_size_t micro_type_to_size[] = {
    [MICRO_TYPE_I8]  = MICRO_SIZE_8,
    [MICRO_TYPE_U8]  = MICRO_SIZE_8,
    [MICRO_TYPE_I16] = MICRO_SIZE_16,
    [MICRO_TYPE_U16] = MICRO_SIZE_16,
    [MICRO_TYPE_I32] = MICRO_SIZE_32,
    [MICRO_TYPE_U32] = MICRO_SIZE_32,
    [MICRO_TYPE_F32] = MICRO_SIZE_32,
    [MICRO_TYPE_PTR] = MICRO_SIZE_32,
};

void micro_init()
{
    micro_err_stk = malloc(MICRO_ERROR_STACK_EXTEND_SIZE * sizeof(micro_error_t));
    micro_err_stk_size = 0;
    _micro_err_stk_real_size = MICRO_ERROR_STACK_EXTEND_SIZE;
}

void micro_deinit()
{
    free(micro_err_stk);
}

void micro_push_err(micro_error_t err)
{
    if (micro_err_stk_size + 1 >= _micro_err_stk_real_size) {
        micro_error_t *new_errs = malloc((_micro_err_stk_real_size += MICRO_ERROR_STACK_EXTEND_SIZE) * sizeof(micro_error_t));
        memcpy(new_errs, micro_err_stk, sizeof(micro_error_t) * micro_err_stk_size);
        free(micro_err_stk);
        micro_err_stk = new_errs;
    }
    micro_err_stk[micro_err_stk_size++] = err;
}

micro_imm_be_t micro_imm_be_gen(i32 val)
{
    return (micro_imm_be_t){
        .val = val,
        .bytes = {
            (u8)(val >> 24), (u8)(val >> 16), (u8)(val >> 8), (u8)val
        }
    };
}

micro_imm_le_t micro_imm_le_gen(i32 val)
{
    return (micro_imm_le_t){
        .val = val,
        .bytes = {
            (u8)val, (u8)(val >> 8), (u8)(val >> 16), (u8)(val >> 24)
        }
    };
}
