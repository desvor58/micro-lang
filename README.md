# µ micro-lang
> **Minimalistic. Lightweight. No SSA. No AST. Just compile.**

**micro** it's an experiment in the shape of IR code. Linear code, spiritually referencing C.

### ✨ Core philosophy
- **Flat**: Code is a linear list of instructions, which grouped to functions.
- **No AST**: The parser convert your text to list of instruction info. Or You can generate it, if you using micro as a library.
- **Blazing fast compilation**: No time wasted no node allocations, recursive traversals, or tree transformations.

### 🎯 Project Goals
- **Compilation speed** > Execution speed
- **Compiler simplicity** (< 4000 lines of code now, < 5000 line of code at 1.0.0 version)

-----------------

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
    
    \ f1 and f2 its a virtual registers \
    call f1 fib - n 1;
    call f2 fib - n 2;

    ret + f1 f2;

end_rec:
    ret n;
end
```

-----------------

### 🏛️ Compiler architecture now
If you use **micro** as a compiler
```
Source text
  ↓
Lexer
  ↓
Tokens
  ↓
Code-generator
  ↓
Binary (Or object file)
```

### 🏛️ Compiler architecture in the future
If you use **micro** as a compiler
```
Source text
  ↓
Lexer
  ↓
Tokens
  ↓
Parser
  ↓
Instruction infos
  ↓
Optimizer
  ↓
Code-generator
  ↓
Binary (Or object file)
```
If you use **micro** as a library
```
Special functions 'instruction-generators'
  ↓
Instruction infos
  ↓
Optimizer
  ↓
Code-generator
  ↓
Binary (Or object file)
```

-----------------

### 🏗️ Building
With make:
```
make CC=<comp>
```
Debug make compile:
```
make CC=<comp> MODE=debug
```
Now work with gcc and clang, maybe tcc.

-----------------

> [!BUILDING WITHOUT GCC OR LLVM] \
> makefile using ```gcc-ar``` by default for LTO in ```release``` mode for ```CC=gcc``` and ```llvm-ar``` for ```CC=clang```. \
> If you do not have GCC or llvm on your machine change ```AR := gcc-ar``` to ```AR := ar``` and delete ```-flto``` flag from 13 and 14 lines.

> [!NO STRICT ALIASING]  \
> micro using compile flag ```-fno-strict-aliasing```.  \
> If the compiler used to build micro does not support this flag then ```MODE=release``` version will not work

## 📜 Docs
- You can read about syntax of *micro* at [**language reference**](docs/micro-language-ref.md)
- About code style you can read at [**style reference**](docs/good-micro-code-style.md)
- If you are a developer read [**compiler description**](docs/compiler-description.md)
