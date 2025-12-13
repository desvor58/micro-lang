# Usage
```
micro [Options] inputfile_name
```

# Options
| full name                      | short name               | description                                   |
|--------------------------------|--------------------------|-----------------------------------------------|
| --outfile \<file>              | -o \<file>               | output file name (if not exist creating file) |
| --target \<[target](#targets)> | -t \<[target](#targets)> | set target to compiling               |

## targets
| target name | description                                     | micro-comp  |
|-------------|-------------------------------------------------|-------------|
| i386        | 32 bits raw bin out                             | micro-386   |
| win32       | 32 bits .o file for windows 32 (COFF format)    | micro-win32 |
| elf32       | 32 bits .o file for linux/macOS 32 (ELF format) | micro-elf32 |

# Stages
Micro compiling have 4 stage:
1. [lexing](#lexing)
2. 

# Lexing
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
In micro tokenizer have types:
- 0 TT_NULL - using no lexic analis stage
- 1 TT_IDENT - identifires (some_ident, foo, bar)
- 2 TT_LIT_INT - integer literal (1, 0, -54)
- 3 TT_LIT_FLOAT - floating literal (1.5, 0.01, -54.825)
- 4 TT_LIT_STR - string literal ("some str", "Hello, world!")
- 5 TT_KW_VAR - keyword 'var'
- 6 TT_KW_FUN - keyword 'fun'
- 7 TT_PLUS - '+'
- 8 TT_MINUS - '-'
- 9 TT_STAR - '*'
- 10 TT_SLASH - '/'
