#include <stdio.h>
#include "args.h"
#include "types.h"
#include "lexer.h"

int main(int argc, char **argv)
{
    args_t *args = (args_t*)malloc(sizeof(args_t));
    *args = args_parse(argc, argv);

    char *text = (char*)malloc(sizeof(char) * MAX_INPUT_CODE_SIZE);
    size_t text_size = 0;

    FILE *infile;
    fopen_s(&infile, args->inputfile, "r");
    if (!infile) {
        return -1;
    }
    char c = 0;
    while ((c = getc(infile)) != EOF) {
        if (text_size >= MAX_INPUT_CODE_SIZE) {
            printf_s("Error: input file to large for reading");
            fclose(infile);
            return 1;
        }
        text[text_size++] = c;
    }

    lexer_init();
        lexing(text, text_size);

        for (size_t i = 0; i < lexer_err_stk_size; i++) {
            printf_s("Error:%s:%u:%u: %s",
                     args->inputfile,
                     lexer_err_stk[i].line_ref,
                     lexer_err_stk[i].chpos_ref,
                     lexer_err_stk[i].msg);
        }
        if (lexer_err_stk_size) {
            return 2;
        }

        for (size_t i = 0; i < toks_size; i++) {
            printf_s("%u. %u:%u type:%u, val:%s\n",
                     i,
                     toks[i].line_ref,
                     toks[i].chpos_ref,
                     toks[i].type,
                     toks[i].val);
        }
    lexer_delete();

    return 0;
}