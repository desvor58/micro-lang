#ifndef CONFIG_H
#define CONFIG_H

// size by which error stack will be extended on each overflow
#define ERROR_STACK_EXTEND_SIZE 4
// size by which tokens buffer will be extended on each overflow
#define TOKEN_BUFFER_EXTEND_SIZE 32
// max size of symbols
#define MAX_SYMBOL_SIZE 128
// max errors and warnigns message size
#define MAX_ERROR_MESSAGE_SIZE 256
// size of buffer for reading input file
#define MAX_INPUT_CODE_SIZE 48 * 1024

#endif