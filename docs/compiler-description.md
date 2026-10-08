# Compiler Description

Version: **dev-1.0.0** (unstable MVP)

# Usage

```
microc [flags] <input file>
```

`microc` reads one source file and writes a raw 32 bit x86 binary. The name
of the output file is `a.out` unless `-o` is given.

# Options

| full name                      | short name   | description                                              |
|--------------------------------|--------------|----------------------------------------------------------|
| --help                         | -h           | print the menu and exit                                  |
| --output \<file>               | -o \<file>   | output file name, `a.out` by default                     |
| put info                       | -P \<flags>  | print what a stage produced: `t` tokens, `i` instructions, `a` assembly |
| stop compiling                 | -S \<stage>  | stop after a stage: `r` file read, `l` lexing, `i` instruction generation, `a` asm optimizing |
| skip stage of compiling        | -N \<stage>  | skip a stage, currently only `a` asm optimizing          |
| no errors outside of a function| -Fno-err-outside-fun | allow instructions outside of a function        |

The exit code is 0 on success, 1 for a bad command line or an unreadable
file, 2 for a lexing error, 3 for an instruction generation error, 4 for a
code generation error and 5 when the output file cannot be created.

`microc` is a thin driver around the same library that other programs embed,
so everything it does is available through the API described below.

# Stages

```
source text
  ↓
lexer                        mc_tokenize
  ↓
tokens                       mc_token_t
  ↓
instruction generator        mc_instrgen_gen
  ↓
instruction infos            micro_instruction_t
  ↓
code generator               micro_codegen386_init + emit
  ↓
asm instructions             micro_asm386_instruction_t
  ↓
asm optimizer                micro_asm386_optimize
  ↓
assembler                    micro_asm386_emit
  ↓
raw binary
```

The stages after the instruction generator are the library itself. A host
program can start at any of them: build the instruction infos with the
`micro_instr_gen_*` functions, or even build them by hand, and pass them to
the code generator.

# Lexer

## Token structure

Every token keeps its type, its value in a fixed size buffer and the
position it started at.

```C
typedef struct {
    mc_token_type_t type;
    char            val[MICRO_MAX_SYMBOL_SIZE];
    size_t          line_ref;
    size_t          chpos_ref;
} mc_token_t;
```

## Token types

| name                | description                                        |
|---------------------|----------------------------------------------------|
| MC_TOK_NULL         | not used                                           |
| MC_TOK_PLUS         | `+`                                                |
| MC_TOK_MINUS        | `-`                                                |
| MC_TOK_STAR         | `*`                                                |
| MC_TOK_SLASH        | `/`                                                |
| MC_TOK_AMPERSAND    | `&`, reserved                                      |
| MC_TOK_DOLLAR       | `$`                                                |
| MC_TOK_HASH         | `#`, reserved                                      |
| MC_TOK_APOSTROPHE   | `` ` ``, reserved                                  |
| MC_TOK_TILDE        | `~`, reserved                                      |
| MC_TOK_EQ           | `=`                                                |
| MC_TOK_EXCLAMATION  | `!`                                                |
| MC_TOK_GREAT        | `>`                                                |
| MC_TOK_LESS         | `<`                                                |
| MC_TOK_GREAT_OR_EQ  | `>=`                                               |
| MC_TOK_LESS_OR_EQ   | `<=`                                               |
| MC_TOK_IDENT        | name, an `_` or a letter followed by name chars    |
| MC_TOK_LIT_INT      | integer literal, `-5` is one literal               |
| MC_TOK_LIT_FLOAT    | float literal                                      |
| MC_TOK_LIT_STR      | string literal                                     |
| MC_TOK_DOT          | `.`                                                |
| MC_TOK_COMA         | `,`, used inside a hint block                      |
| MC_TOK_COLON        | `:`                                                |
| MC_TOK_SEMICOLON    | `;`                                                |
| MC_TOK_TYPE_NAME    | type name, see the language reference              |
| MC_TOK_KW_FUN       | `fun`                                              |
| MC_TOK_KW_SET       | `set`                                              |
| MC_TOK_KW_IF        | `if`                                               |
| MC_TOK_KW_ELSE      | `else`, reserved                                   |
| MC_TOK_KW_WHILE     | `while`, reserved                                  |
| MC_TOK_KW_START     | `start`                                            |
| MC_TOK_KW_END       | `end`                                              |
| MC_TOK_KW_RET       | `ret`                                              |
| MC_TOK_KW_CALL      | `call`                                             |
| MC_TOK_KW_GOTO      | `goto`                                             |
| MC_TOK_LBRACE       | `{`                                                |
| MC_TOK_RBRACE       | `}`                                                |

# Instruction generator

The generator walks the token list and produces `micro_instruction_t`
values: one per `fun`, `set`, `call`, `ret`, `goto`, `if` and label. Each
instruction carries a `micro_instruction_hints_t` with the lifetime of the
names it introduces.

Every generated instruction is a plain struct, so a host program can build
the same list by hand. `micro_make_expr` turns a string into the
`micro_expr_tok_t` list an instruction expects.

A `lifetime` hint is an instruction index, not a count: it is the index of
the last instruction that may still use the name, and `-1` means the name
lives until the end of the function. The generator turns a source hint of
`{lifetime: 2}` on the third instruction of a body into `2 + 2`, so a host
that builds the same list by hand adds the index of the instruction it is
writing to the number it wants.

# Code generator

The code generator lowers every instruction into 32 bit x86 instructions.
Its model is small and predictable:

- Six machine registers are allocatable: `eax`, `ecx`, `edx`, `ebx`, `esi`,
  `edi`. `esp` and `ebp` are reserved for the frame.
- A virtual register gets a machine register while it is alive, and the
  stack when the registers run out. Its machine register is released when
  its lifetime hint expires.
- Functions follow the cdecl convention: arguments are read from `ebp + 8`
  and up, the result is returned in `eax`.
- Every operand of a virtual register can be a machine register, a frame
  slot or a data section address, so each operation has a form for every
  combination. A temporary that does not fit in a register is spilled to
  the frame.

## Code selection

Two patterns are recognized while an expression is lowered, and they are
only used when every operand already lives in a machine register and a
register is free for the result:

- `lea r, [base + index * scale + disp]`, where the scale is 2, 4 or 8. The
  index and the base are both optional, so a plain `base + disp` folds into
  the same instruction. The displacement is folded into the instruction
  whenever the expression adds a literal to the scaled term, and the
  shortest of the no displacement, the 8 bit displacement and the 32 bit
  displacement form is chosen.
- `inc r` and `dec r` for a value plus or minus one.

## Assembler optimizer

The peephole optimizer runs over the instruction list before it is encoded:

- a move of a register into itself is dropped,
- `jmp` to the label that follows it is dropped,
- an addition or a subtraction of zero is dropped,
- an addition or a subtraction of one to a register becomes `inc` or `dec`.

# Assembler

The assembler encodes each instruction and resolves the labels. The output
is raw machine code without any header, so it can be copied into
executable memory and called, which is what the `examples` do.

## Trampolines

A trampoline is a host function that compiled micro code can call in place of
a compiled function. Use it when the callee lives outside the generated code:
an interpreted function, a VM entry point, or anything else the compiler
cannot see.

In source, a trampoline is declared where functions are declared, with `tramp`
instead of `fun` and no body:

```
tramp vm_add
    i32 a
    i32 b
    ret i32
end
```

Declare the same signature with `micro_instr_gen_tramp` when the code is built
through the API, and hand the code generator a map from that name to the
handler:

```C
static i32 vm_add(const micro_tramp_frame_t *frame)
{
    return frame->args[0] + frame->args[1] * 10;
}

sct_hashmap_t tramps;
sct_hashmap_init(&tramps, sizeof(micro_tramp_t));
micro_tramp_t handler = vm_add;
sct_hashmap_add(&tramps, "vm_add", &handler);

micro_codegen386_init(&codegen, flags, &asm_instrs, &arena, &tramps);
```

The call itself is an ordinary `call` instruction in micro code, and the
argument count and result type are checked against the declaration exactly as
they are for a compiled function. Passing `NULL` instead of the map disables
the feature, and every `tramp` then reports `No trampoline in the map for this
name`. `microc` always passes `NULL`, so the command line compiler rejects every
`tramp` declaration.

### Calling convention

Arguments are pushed like any other micro call, one 32 bit word per declared
argument in declaration order, and the result comes back in `eax`. On top of
that the handler receives one extra first argument, a pointer to a frame that
describes the call:

```C
typedef struct {
    i32         *args;      /* one word per argument, in declaration order */
    size_t       args_num;
    micro_type_t ret_type;
} micro_tramp_frame_t;

typedef i32 (*micro_tramp_t)(const micro_tramp_frame_t *frame);
```

The frame is how a single handler serves many signatures: read `args_num` and
index `args` instead of relying on a matching C prototype, which is what lets
one VM trampoline dispatch functions of different arity. `args` stays valid
for the duration of the call. The map value is a `micro_tramp_t *`, so a host
can replace the handler of an already declared trampoline.

Arguments narrower than 32 bits arrive in the low bytes of their word, the same
way they reach a compiled function; `ret_type` tells the handler how micro
intends to use the value it returns.

A trampoline call costs a few instructions more than a direct call: the frame
is built on the stack and the handler address is loaded into a register
instead of being encoded as a label displacement. That keeps the call correct
no matter how far the handler is from the generated code.

# Library

`micro.h` is the only header a host program needs to compile micro code at
runtime:

```C
micro_init();
    micro_codegen_t codegen;
    micro_codegen386_init(&codegen, flags, &asm_instrs, &arena, tramps);
    codegen.emit(&codegen, &instructions);
    micro_asm386_optimize(&asm_instrs);
    micro_asm386_emit(&asm_instrs, &outbuf);
micro_deinit();
```

The arena has to outlive the assembler: label names are allocated there and
the instruction list keeps pointing at them while labels are resolved, so
`micro_codegen386_deinit` and `sct_arena_deinit` come after
`micro_asm386_emit`.

`microc` itself is built on top of these calls, together with the lexer,
the instruction generator and the debug printers of `microdebug`.
