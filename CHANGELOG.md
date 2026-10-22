# Changelog

## dev-1.0.0-1

- language: operators `|`, `^`, `&&`, `||`; `else` and `while` are no longer keywords
- lexer: too long identifiers, numbers and string literals are reported as errors
- codegen: 8-bit values never go to `esi`/`edi` (they were encoded as `dh`/`bh` and corrupted `edx`/`ebx`)
- codegen: `esp`/`ebp` are never handed out as temporaries
- codegen: used-register bookkeeping is restored correctly for nested expressions
