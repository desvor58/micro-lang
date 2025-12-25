# Usage
```
micro [Options] inputfile_name
```

# Options
| full name                      | short name               | description                                   |
|--------------------------------|--------------------------|-----------------------------------------------|
| --outfile \<file>              | -o \<file>               | output file name (if not exist creating file) |
| --target \<[target](#targets)> | -t \<[target](#targets)> | set target to compiling                       |
| --toks-put                     | -T                       | put tokens after [lexical analis](#stages)    |

## targets
| target name | description                                     | micro-comp  |
|-------------|-------------------------------------------------|-------------|
| i386        | 32 bits raw bin out                             | micro-386   |
| win32       | 32 bits .o file for windows 32 (COFF format)    | micro-win32 |
| elf32       | 32 bits .o file for linux/macOS 32 (ELF format) | micro-elf32 |

# Stages
Micro compiling have 4 stage:
1. [lexical analis](#lexing)
2. [code-generation and semantic analis](#code-generation-and-semantic-analis)

# Lexical analis
## Token structure
Token structure contains:
- [type](#token-types)
- value - char buffer size of MAX_SYMBOL_SIZE
- line referense - line num where token have starting
- char pos reference - char position in line where token have starting

is C is structure:
```C
struct Token
{
    token_type type;
    char       val[MAX_SYMBOL_SIZE];
    size_t     line_ref;
    size_t     chpos_ref;
};
```

### Token types
| int number | name         | description                                  |
|------------|--------------|----------------------------------------------|
| 0          | TT_NULL      | not used on lexical stage                    |
| 0          | TT_TYPE_NAME | types (i8, u32, ptr, f16)                    |
| 2          | TT_IDENT     | identifires (some_ident, foo, bar)           |
| 3          | TT_LIT_INT   | integer literal (1, 0, -54)                  |
| 4          | TT_LIT_FLOAT | floating literal (1.5, 0.01, -54.825)        |
| 5          | TT_LIT_STR   | string literal ("some str", "Hello, world!") |
| 6          | TT_KW_VAR    | keyword 'var'                                |
| 7          | TT_KW_FUN    | keyword 'fun'                                |
| 8          | TT_KW_SET    | keyword 'set'                                |
| 9          | TT_KW_IF     | keyword 'if'                                 |
| 10         | TT_KW_ELSE   | keyword 'else'                               |
| 11         | TT_KW_WHILE  | keyword 'while'                              |
| 12         | TT_PLUS      | '+'                                          |
| 13         | TT_MINUS     | '-'                                          |
| 14         | TT_STAR      | '*'                                          |
| 15         | TT_SLASH     | '/'                                          |
| 16         | TT_DOT       | '.'                                          |
| 17         | TT_COMA      | ','                                          |
| 18         | TT_COLON     | ':'                                          |
| 19         | TT_SEMICOLON | ';'                                          |


# Code-generation and semantic analis
