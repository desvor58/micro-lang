/*************************************************
 *               micro-lang compiler
 *                   by Desvor
 *               Version: dev-1.0.0
 * 
 * Compiler crated as util for
 * compiling ir-language of micro
 ************************************************/

#include <micro/micro.h>
#include <microc/lexer.h>
#include <microc/instrgen.h>
#include <microdebug/microdebug.h>
#include <string.h>
#include <stdio.h>
#include <SCT/string.h>

typedef struct {
    char inputfile[MICRO_MAX_SYMBOL_SIZE];
    char outfile[MICRO_MAX_SYMBOL_SIZE];
    u32   toks_put            : 1;
    u32   instrs_put          : 1;
    u32   asm_put             : 1;
    u32   skip_asmopting      : 1;
    u32   optimizing_inlining : 1;
    enum {
        STOPAFTER_NONE,
        STOPAFTER_FILE_READ,
        STOPAFTER_LEXER,
        STOPAFTER_INSTRGEN,
        STOPAFTER_ASMOPTING,
    } stop_at;
    micro_codegen_flags_t codegen_flags;
} mc_args_t;

static void print_usage(void)
{
    printf(
        "micro-lang dev-1.0.0\n"
        "dev: Desvor\n"
        "usage:\n"
        "    microc [flags] <input file>\n"
        "flags:\n"
        "    --help (-h)           - put this menu\n"
        "    --output (-o) <file>  - set output file (default: a.out)\n"
        "    -P                    - put some info\n"
        "      t                   - put tokens\n"
        "      i                   - put instructions\n"
        "      a                   - put assembly\n"
        "    -S                    - stop compiling\n"
        "      r                   - stop after reading file\n"
        "      l                   - stop after lexing\n"
        "      i                   - stop after instruction generation\n"
        "      a                   - stop after asm optimization stage\n"
        "    -N                    - skip stage of compiling\n"
        "      a                   - skip asm optimizing\n"
        "    -O                    - optimization flags\n"
        "      i                   - inlining\n"
        "    -Fno-err-outside-fun  - allow instructions outside of a function\n"
    );
}

static mc_args_t mc_args_parse(int argc, char **argv)
{
    mc_args_t args = {0};
    strcpy(args.outfile, "a.out");

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 'h' || !strcmp(argv[i], "--help")) {
                print_usage();
                exit(0);
            }
            if (!strcmp(argv[i], "--output") || !strcmp(argv[i], "-o")) {
                if (i + 1 == argc) {
                    puts("Error: Expected file name after output flag");
                    exit(1);
                }
                if (strlen(argv[i + 1]) > MICRO_MAX_SYMBOL_SIZE - 1) {
                    puts("Error: Output file name is too long");
                    exit(1);
                }
                strcpy(args.outfile, argv[++i]);
            } else
            if (argv[i][1] == 'P') {
                for (size_t j = 2; argv[i][j]; j++) {
                    if (argv[i][j] == 't') {
                        args.toks_put = 1;
                    } else
                    if (argv[i][j] == 'i') {
                        args.instrs_put = 1;
                    } else
                    if (argv[i][j] == 'a') {
                        args.asm_put = 1;
                    } else {
                        printf("Error: Unexpected symbol: '%c' (expected 't', 'i', 'a')\n", argv[i][j]);
                        exit(1);
                    }
                }
            } else
            if (argv[i][1] == 'S') {
                if (argv[i][2] == 'r') {
                    args.stop_at = STOPAFTER_FILE_READ;
                } else
                if (argv[i][2] == 'l') {
                    args.stop_at = STOPAFTER_LEXER;
                } else
                if (argv[i][2] == 'i') {
                    args.stop_at = STOPAFTER_INSTRGEN;
                } else
                if (argv[i][2] == 'a') {
                    args.stop_at = STOPAFTER_ASMOPTING;
                } else {
                    printf("Error: Unexpected symbol: '%c' (expected 'r', 'l', 'i', 'a')\n", argv[i][2]);
                    exit(1);
                }
            } else
            if (argv[i][1] == 'N') {
                for (size_t j = 2; argv[i][j]; j++) {
                    if (argv[i][j] == 'a') {
                        args.skip_asmopting = 1;
                    }
                }
            } else
            if (argv[i][1] == 'O') {
                for (size_t j = 2; argv[i][j]; j++) {
                    if (argv[i][j] == 'i') {
                        args.optimizing_inlining = 1;
                    }
                }
            } else
            if (!strcmp(argv[i], "-Fno-err-outside-fun")) {
                args.codegen_flags.no_err_outside_fun = 1;
            } else {
                printf("Error: Undefined flag %s\n", argv[i]);
                print_usage();
                exit(1);
            }
        } else {
            if (args.inputfile[0]) {
                puts("Error: Input file already set");
                print_usage();
                exit(1);
            }
            if (strlen(argv[i]) > MICRO_MAX_SYMBOL_SIZE - 1) {
                puts("Error: Input file name is too long");
                exit(1);
            }
            strcpy(args.inputfile, argv[i]);
        }
    }

    return args;
}

int main(int argc, char **argv)
{
    mc_args_t args = mc_args_parse(argc, argv);

    if (args.inputfile[0] == 0) {
        puts("Error: Expected input file name");
        print_usage();
        return 1;
    }

    sct_string_t input_text;
    sct_string_init(&input_text);

    FILE *infile = fopen(args.inputfile, "r");
    if (!infile) {
        puts("Error: Cannot open input file");
        return 1;
    }
    char c = 0;
    while ((c = getc(infile)) != EOF) {
        sct_string_push(&input_text, c);
    }
    fclose(infile);

    if (args.stop_at == STOPAFTER_FILE_READ) return 0;

    micro_init();
        sct_vector_t toks;
        sct_vector_init(&toks, sizeof(mc_token_t));
        mc_tokenize(input_text.cstr, input_text.size, &toks);

        sct_string_deinit(&input_text);

        for (size_t i = 0; i < micro_err_stk_size; i++) {
            micro_debug_put_err(args.inputfile, micro_err_stk[i]);
        }
        if (micro_err_stk_size) {
            puts("Lexing fail");
            return 2;
        }

        if (args.toks_put) {
            for (size_t i = 0; i < toks.size; i++) {
                mc_token_t tok = *(mc_token_t*)sct_vector_get(&toks, i);
                printf("%lu. ", i);
                micro_debug_put_tok(tok);
            }
        }

        if (args.stop_at == STOPAFTER_LEXER) return 0;

        mc_instrgen_t instrgen;
        mc_instrgen_init(&instrgen, &toks);
            mc_instrgen_gen(&instrgen);

            for (size_t i = 0; i < micro_err_stk_size; i++) {
                micro_debug_put_err(args.inputfile, micro_err_stk[i]);
            }
            if (micro_err_stk_size) {
                puts("Instruction generation fail");
                return 3;
            }

            micro_iropter_t iropter;
            int iropter_used = 0;
            if (args.optimizing_inlining) {
                micro_iropter_init(&iropter, &instrgen.instructions);
                micro_iropter_inlining_pass(&iropter);
                iropter_used = 1;
            }

            if (args.instrs_put) {
                micro_debug_print_instructions(&instrgen.instructions, 0);
            }

            if (args.stop_at == STOPAFTER_INSTRGEN) return 0;

            sct_vector_t asm_instrs;
            sct_vector_init(&asm_instrs, sizeof(micro_asm386_instruction_t));

            sct_vector_t outbuf;
            sct_vector_init(&outbuf, sizeof(u8));

            sct_arena_t arena;
            sct_arena_init(&arena);

            micro_codegen_t codegen;
            micro_codegen386_init(&codegen, args.codegen_flags, &asm_instrs, &arena, NULL);
                codegen.emit(&codegen, &instrgen.instructions);
                for (size_t i = 0; i < micro_err_stk_size; i++) {
                    micro_debug_put_err(args.inputfile, micro_err_stk[i]);
                }
                if (micro_err_stk_size) {
                    puts("Assembler generation fail");
                    return 4;
                }

        mc_instrgen_deinit(&instrgen);
        sct_vector_deinit(&toks);

                if (!args.skip_asmopting) {
                    micro_asm386_optimize(&asm_instrs);
                }

                if (args.asm_put) {
                    micro_debug_put_asm(&asm_instrs);
                }

                if (args.stop_at == STOPAFTER_ASMOPTING) {
                    return 0;
                }

                micro_asm386_emit(&asm_instrs, &outbuf);

            sct_arena_deinit(&arena);

                sct_vector_deinit(&asm_instrs);

                FILE *outfile = fopen(args.outfile, "wb");
                if (!outfile) {
                    puts("Error: Cannot open output file");
                    return 5;
                }
                fwrite(outbuf.data, sizeof(u8), outbuf.size, outfile);
                fclose(outfile);
            micro_codegen386_deinit(&codegen);
    /* the arena of the iropter holds the expressions the inlining created,
       so it has to outlive the code generator */
    if (iropter_used) {
        micro_iropter_deinit(&iropter);
    }
    micro_deinit();
    return 0;
}
