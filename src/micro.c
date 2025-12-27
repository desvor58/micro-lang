/************************************************
 *               Micro IR language
 *                   by Desvor
 * 
 *    Language created for Simple C Compiler,
 *     you can use it for your own compiers.
 *     All documentation in `docs` directory
 **********************************************/

#include <stdio.h>

#include "args.h"
#include "types.h"
#include "lexer.h"

#include "codegen/codegen_386.h"

int main(int argc, char **argv)
{
    micro_args_t *args = (micro_args_t*)malloc(sizeof(micro_args_t));

    *args = micro_args_parse(argc, argv);

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

    micro_lexer_init();
        micro_lexing(text, text_size);

        for (size_t i = 0; i < micro_lexer_err_stk_size; i++) {
            printf("Error:%s:%lu:%lu: %s",
                     args->inputfile,
                     micro_lexer_err_stk[i].line_ref,
                     micro_lexer_err_stk[i].chpos_ref,
                     micro_lexer_err_stk[i].msg);
        }
        if (micro_lexer_err_stk_size) {
            return 2;
        }

        if (args->flags & MICRO_AF_TOKS_PUT) {
            for (size_t i = 0; i < micro_toks_size; i++) {
                printf("%lu. %lu:%lu type:%u, val:%s\n",
                        i,
                        micro_toks[i].line_ref,
                        micro_toks[i].chpos_ref,
                        micro_toks[i].type,
                        micro_toks[i].val);
            }
        }

        micro_codegen_386_init();
            micro_codegen_386();

            for (size_t i = 0; i < micro_codegen_386_err_stk_size; i++) {
                printf("Error:%s:%lu:%lu: %s\n",
                        args->inputfile,
                        micro_codegen_386_err_stk[i].line_ref,
                        micro_codegen_386_err_stk[i].chpos_ref,
                        micro_codegen_386_err_stk[i].msg);
            }
            if (micro_codegen_386_err_stk_size) {
                return 2;
            }

            FILE *outfile = fopen(args->outfile, "w");
            fwrite(micro_outbuf->str, sizeof(char), micro_outbuf->size, outfile);
            fclose(outfile);
        micro_codegen_386_delete();
    micro_lexer_delete();

    return 0;
}