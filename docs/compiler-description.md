# Stages
Micro compiling have 4 stage:
1. [tokenization](#Tokenization)
2. [lexical analis](#lexacal-analis)

# Tokenization
# Token
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

## Token types
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

# Lexacal analis