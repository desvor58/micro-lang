#include "../common.h"

micro_error_t *micro_err_stk;
size_t         micro_err_stk_size;
size_t       __micro_err_stk_real_size;

void micro_init()
{
    micro_err_stk = malloc(MICRO_ERROR_STACK_EXTEND_SIZE * sizeof(micro_error_t));
    micro_err_stk_size = 0;
    __micro_err_stk_real_size = MICRO_ERROR_STACK_EXTEND_SIZE;
}

void micro_push_err(micro_error_t err)
{
    if (micro_err_stk_size + 1 >= __micro_err_stk_real_size) {
        micro_error_t *new_errs = malloc((__micro_err_stk_real_size += MICRO_ERROR_STACK_EXTEND_SIZE) * sizeof(micro_error_t));
        memcpy(new_errs, micro_err_stk, sizeof(micro_error_t) * micro_err_stk_size);
        free(micro_err_stk);
        micro_err_stk = new_errs;
    }
    micro_err_stk[micro_err_stk_size++] = err;
}

void micro_deinit()
{
    free(micro_err_stk);
}
