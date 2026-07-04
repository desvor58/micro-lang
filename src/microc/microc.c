/************************************************
 *               Micro IR language
 *                   by Desvor
 * 
 *    Language created for Simple C Compiler,
 *     you can use it for your own compilers.
 *     All documentation in `docs` directory
 **********************************************/

#include <micro/micro.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    char inputfile[MICRO_MAX_SYMBOL_SIZE];
    char outfile[MICRO_MAX_SYMBOL_SIZE];
    u8   toks_put   : 1;
    u8   instrs_put : 1;
} micro_args_t;


micro_args_t micro_args_parse(int argc, char **argv)
{
    micro_args_t args;
    args.inputfile[0] = 0;
    args.outfile[0] = 0;
    args.toks_put = 0;
    args.instrs_put = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (!strcmp(argv[i], "--output") || !strcmp(argv[i], "-o")) {
                strcpy(args.outfile, argv[++i]);
            } else
            if (argv[i][1] == 'P') {
                for (size_t j = 2; argv[i][j]; j++) {
                    if (argv[i][j] == 't') {
                        args.toks_put = 1;
                    } else
                    if (argv[i][j] == 'i') {
                        args.instrs_put = 1;
                    }
                }
            }
        } else {
            strcpy(args.inputfile, argv[i]);
        }
    }

    return args;
}

void put_err(char *file, size_t line, size_t chpos, char *msg)
{
    printf("Error: %s:%lu:%lu: %s\n", file, line, chpos, msg);
}

void print_tok(micro_token_t tok)
{
    printf("%lu:%lu type:%s, val:%s\n",
                       tok.line_ref,
                       tok.chpos_ref,
                       micro_token_type2str[tok.type],
                       tok.val);
}

char *str_type[] = {
    [MICRO_TYPE_NULL] = "null",
    [MICRO_TYPE_I8]   = "i8",
    [MICRO_TYPE_U8]   = "u8",
    [MICRO_TYPE_I16]  = "i16",
    [MICRO_TYPE_U16]  = "u16",
    [MICRO_TYPE_I32]  = "i32",
    [MICRO_TYPE_U32]  = "u32",
    [MICRO_TYPE_F32]  = "f32",
    [MICRO_TYPE_PTR]  = "ptr"
};

u8 _op_args_num[] = {
    [MICRO_TOK_PLUS]        = 2,
    [MICRO_TOK_MINUS]       = 2,
    [MICRO_TOK_STAR]        = 2,
    [MICRO_TOK_SLASH]       = 2,
    [MICRO_TOK_AMPERSAND]   = 1,
    [MICRO_TOK_DOLLAR]      = 1,
    [MICRO_TOK_HASH]        = 1,
    [MICRO_TOK_APOSTROPHE]  = 1,
    [MICRO_TOK_TILDE]       = 1,
    [MICRO_TOK_EQ]          = 2,
    [MICRO_TOK_EXCLAMATION] = 1,
    [MICRO_TOK_NOT_EQ]      = 2,
    [MICRO_TOK_GREAT]       = 2,
    [MICRO_TOK_LESS]        = 2,
    [MICRO_TOK_GREAT_OR_EQ] = 2,
    [MICRO_TOK_LESS_OR_EQ]  = 2,
};

size_t print_expr(micro_token_t *start, size_t tab)
{
    if (!start) return 0;
    for (size_t i = 0; i < tab; i++) {
        putchar(' ');
    }
    print_tok(*start);
    if (_micro_tok_is_lit(start->type) || start->type == MICRO_TOK_IDENT) {
        return 1;
    }
    if (_micro_tok_is_op(start->type)) {
        u8 num = _op_args_num[start->type];
        size_t offset = 0;
        while (num) {
            offset += print_expr(start + offset + 1, tab);
            num--;
        }
        return offset;
    }
    puts("print_expr error!");
    return 0;
}

void print_instructions(sct_vector_t *instrs, size_t tab)
{
    for (size_t i = 0; i < instrs->size; i++) {
        micro_instruction_t *instr = sct_vector_get(instrs, i);
        for (size_t i = 0; i < tab; i++) {
            putchar(' ');
        }
        switch (instr->type) {
            case MICRO_INSTR_SET:
                printf("SET: type:%s, name:'%s'\n", str_type[instr->set.type], instr->set.reg_name);
                print_expr(instr->set.val_expr, tab + 5);
                break;

            case MICRO_INSTR_DRSET:
                printf("DRSET: type:%s, name:'%s'\n", str_type[instr->drset.type], instr->drset.reg_name);
                print_expr(instr->set.val_expr, tab + 5);
                break;

            case MICRO_INSTR_FUN:
                printf("FUN: ret_type:%s, name:'%s'\n", str_type[instr->fun.ret_type], instr->fun.name);
                printf("     args:\n");
                for (size_t i = 0; i < instr->fun.args.size; i++) {
                    micro_instruction_fun_arg_t *arg = sct_vector_get(&instr->fun.args, i);
                    printf("       type:%s, name:'%s'\n", str_type[arg->type], arg->name);
                }
                printf("     body:\n");
                print_instructions(&instr->fun.body, tab + 7);
                break;

            case MICRO_INSTR_RET:
                printf("RET\n");
                print_expr(instr->ret.val_expr, tab + 5);
                break;

            case MICRO_INSTR_CALL:
                printf("CALL: res_reg:'%s', fun:'%s'\n", instr->call.ret_reg_name, instr->call.fun_name);
                printf("       args:\n");
                for (size_t j = 0; j < instr->call.arg_exprs.size; j++) {
                    print_expr(*(micro_token_t**)sct_vector_get(&instr->call.arg_exprs, j), tab + 6);
                }
                break;

            case MICRO_INSTR_LBL:
                printf("LBL '%s'\n", instr->lbl.name);
                break;

            case MICRO_INSTR_GOTO:
                printf("GOTO '%s'\n", instr->goto_lbl.lbl);
                break;

            default:
                puts("wrong instr->type");
                break;
        }
    }
}

int main(int argc, char **argv)
{
    micro_args_t *args = (micro_args_t*)malloc(sizeof(micro_args_t));

    *args = micro_args_parse(argc, argv);
    // strcpy(args->inputfile, "../test.micro");
    // strcpy(args->outfile,  "../test.bin");

    if (args->inputfile[0] == 0) {
        puts("Error: Expected input file name");
        return 1;
    }
    if (args->outfile[0] == 0) {
        puts("Error: Expected output file name");
        return 1;
    }

    char *text = (char*)malloc(sizeof(char) * MICRO_MAX_INPUT_CODE_SIZE);
    size_t text_size = 0;

    FILE *infile = fopen(args->inputfile, "r");
    if (!infile) {
        puts("Error: Input file not be opening");
        return 1;
    }
    char c = 0;
    while ((c = getc(infile)) != EOF) {
        if (text_size >= MICRO_MAX_INPUT_CODE_SIZE) {
            puts("Error: Input file to large for reading");
            fclose(infile);
            return 1;
        }
        text[text_size++] = c;
    }
    fclose(infile);

    micro_init();
        sct_vector_t toks;
        sct_vector_init(&toks, sizeof(micro_token_t));
        micro_tokenize(text, text_size, &toks);

        for (size_t i = 0; i < micro_err_stk_size; i++) {
            put_err(args->inputfile,
                    micro_err_stk[i].line_ref,
                    micro_err_stk[i].chpos_ref,
                    micro_err_stk[i].msg);
        }
        if (micro_err_stk_size) {
            return 2;
        }

        if (args->toks_put) {
            for (size_t i = 0; i < toks.size; i++) {
                micro_token_t tok = *(micro_token_t*)sct_vector_get(&toks, i);
                printf("%lu. ", i);
                print_tok(tok);
            }
        }

        micro_instrgen_t instrgen;
        micro_instrgen_init(&instrgen, &toks);
            micro_instrgen_gen(&instrgen);

            for (size_t i = 0; i < micro_err_stk_size; i++) {
                put_err(args->inputfile,
                        micro_err_stk[i].line_ref,
                        micro_err_stk[i].chpos_ref,
                        micro_err_stk[i].msg);
            }
            if (micro_err_stk_size) {
                return 3;
            }
            
            if (args->instrs_put) {
                print_instructions(&instrgen.instructions, 0);
            }
        micro_instrgen_deinit(&instrgen);
    micro_deinit();

    return 0;
}