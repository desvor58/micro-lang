# µ micro-lang
> **Minimalistic. Lightweight. No SSA. No AST. Just compile.**

**micro** it's an experiment in the shape of IR code. Linear code, spiritually referencing C.

Current version: **dev-1.0.0**. It is an unstable MVP: the whole pipeline
works and is tested, but the language and the code generator are still
moving. See [status](#status) for what to expect.

### ✨ Core philosophy
- **Flat**: Code is a linear list of instructions, which grouped to functions.
- **No AST**: The parser convert your text to list of instruction info. Or You can generate it, if you using micro as a library.
- **Blazing fast compilation**: No time wasted no node allocations, recursive traversals, or tree transformations.

### 🎯 Project goals
- **Compilation speed** > execution speed
- **Compiler simplicity**: about 5700 lines at all and about 4200 lines at libmicro only

### 📌 Status

| part                | state                                                        |
|---------------------|--------------------------------------------------------------|
| lexer               | done                                                        |
| instruction builder | done                                                        |
| i386 code generator | done, one `lea` based code selection pass and a peephole     |
| ir optimizer        | one pass done: inlining of small calls, `-Oi` to enable it  |
| tests               | 191 unit tests over the lexer, instructions and the backend  |
| library API         | done, the compiler itself is built on it                     |
| trampolines         | done, `tramp` in source and host handlers through an API map  |
| object file output  | not implemented, the output is a raw binary                  |
| other backends      | not implemented                                             |
| f32, bitwise ops    | reserved in the grammar, not implemented                     |

### 💡 example code
**C** code:
```
int fib(int n)
{
    if (n <= 1) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}
```

**micro** code
```
fun fib
    i32 n
    ret i32
start
    if <= n 1 : end_rec;

    set i32 f1;
    set i32 f2;
    
    call f1 fib - n 1;
    call f2 fib - n 2;

    ret + f1 f2;

end_rec:
    ret n;
end
```

### ⚙️ What the code generator does with it

Each line below becomes a single instruction, without asking for it:

```
set i32 base 16;
set i32 index 3;
set i32 addr + base * index 4;  \ lea addr, [base + index * 4] \
set i32 next + index 1;         \ inc next \
set i32 tail + addr 4;          \ lea tail, [addr + 4] \
```

Each fold needs its operands to sit in machine registers, so it fires
inside a function body and not on untouched arguments.

### 🏛️ Compiler architecture
If you use **microc** as a compiler
```
Source text
  ↓
Lexer
  ↓
Tokens
  ↓
Instruction generator
  ↓
Instruction infos
  ↓
IR optimizer (optional, `-Oi`)
  ↓
Code generator
  ↓
Asm instructions
  ↓
Asm optimizer
  ↓
Assembler
  ↓
Binary
```

If you use **micro** as a library
```
Instruction infos
  ↓
IR optimizer (optional)
  ↓
Code generator
  ↓
Asm instructions
  ↓
Asm optimizer
  ↓
Assembler
  ↓
Binary
```

### 🏗️ Building
With make:
```
make
```
Debug make compile:
```
make MODE=debug
```
Now work with gcc and clang, maybe tcc. `MODE` is one of `debug`,
`release-fast` (default) and `release-size`.

Run the tests:
```
make test
```
It builds and runs the suite twice, in `debug` and in `release-fast`.

Build the examples:
```
make examples
```

`examples/simple1` compiles a single instruction and prints the bytes it
produced. `examples/simple2` builds a function, maps the bytes into
executable memory and calls it. `examples/simple3` goes the whole way: it
takes micro source text, runs the lexer and the instruction generator, and
calls the compiled `fib`.

> [!BUILDING WITHOUT GCC OR LLVM] \
> The makefile uses ```gcc-ar``` by default for LTO in ```release``` mode for ```CC=gcc``` and ```llvm-ar``` for ```CC=clang```. \
> If you do not have GCC or llvm on your machine change ```AR := gcc-ar``` to ```AR := ar``` and delete the ```-flto``` flags from the ```release-fast``` and ```release-size``` blocks.

> [!32 BIT TOOLCHAIN] \
> micro generates 32 bit code and the build is ```-m32```. You need a multilib gcc, ```libc6-dev-i386``` on debian, ```lib32-glibc``` and ```lib32-gcc-libs``` on arch.

> [!NO STRICT ALIASING]  \
> micro using compile flag ```-fno-strict-aliasing```.  \
> If the compiler used to build micro does not support this flag then ```release``` mode will not work

## 📜 Docs
- You can read about syntax of *micro* at [**language reference**](docs/micro-language-ref.md)
- About code style you can read at [**style reference**](docs/good-micro-code-style.md)
- If you are a developer read [**compiler description**](docs/compiler-description.md)
