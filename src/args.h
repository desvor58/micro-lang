#ifndef ARGS_H
#define ARGS_H

#include <string.h>
#include "config.h"

typedef struct
{
    char   inputfile[MAX_SYMBOL_SIZE];
    char   outfile[MAX_SYMBOL_SIZE];
    size_t flags;
} args_t;

args_t args_parse(int argc, char **argv)
{
    args_t args = {0};

    for (size_t i = 0; i < argc; i++) {
        if (argv[i][0] = '-') {
            if (!strcmp(argv[i], "--output") || !strcmp(argv[i], "-o")) {
                strcpy(args.outfile, argv[++i]);
            }
        } else {
            strcpy(args.inputfile, argv[i]);
        }
    }
}

#endif