# µ micro-lang
> **Minimalistic. Lightweight. No AST. No SAA. Just optimizations.**

**micro** it's an experiment in the shape of IR code. Liner code, spiritually referencing C.

### ✨ Core philosophy
- **Flat**: Code is a liner list of instructions, witch grouped to functions.
- **No AST**: The parser convert your text to list of instruction info. Or You can generate it, if you using micro as a library.
- **Blazing fast compilation**: No time wasted no node allocations, recursive traversals, or tree transformations.

### 🎯 Project Goals
- **Compilation speed** > Execution speed
- **Compiler simplicity** (< 3000 lines of code)

-----------------

### 💡 example code
**C** code:
```
int fib(int n) {
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

    var i32 f1;
    var i32 f2;
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
make
```
Debug make compile:
```
make MODE=debug
```
Or [**dcr**](https://github.com/dexoron/dcr):
```
dcr build --release
```
Debug [**dcr**](https://github.com/dexoron/dcr) compile:
```
dcr build --debug
```

## 📜 Docs
- You can read about sintax of *micro* at [**language reference**](docs/micro-language-ref.md)
- About code style you can read at [**style reference**](docs/good-micro-code-style.md)
- If you an developer read [**compiler description**](docs/compiler-description.md)
