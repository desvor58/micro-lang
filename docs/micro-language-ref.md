# Micro Language Reference

Version: **dev-1.0.0** (unstable MVP)

## Contents
1. [Introduction](#introduction)
2. [Comments](#comments)
3. [Types](#types)
4. [Expressions](#expressions)
5. [Virtual registers](#virtual-registers)
6. [Lifetime hints](#lifetime-hints)
7. [Functions](#functions)
8. [Calling functions](#calling-functions)
9. [Returning values](#returning-values)
10. [Labels and jumps](#labels-and-jumps)
11. [Conditional jumps](#conditional-jumps)
12. [Code selection](#code-selection)
13. [Full example](#full-example)

---

## Introduction

Micro is a low level IR language. It is a linear language: the code is a
plain list of instructions, and instructions are grouped into functions.
There are no blocks, no scopes, and no implicit control flow. You write
exactly what should happen, step by step.

A program is a set of functions. Each function has a name, a list of
arguments, an optional return type, and a body.

The dev-1.0.0 backend generates raw 32 bit x86 code.

---

## Comments

A comment starts and ends with a backslash `\`. Everything between two
backslashes is ignored.

```
\ this is a comment \
\ this whole block is ignored,
  even across multiple lines \
```

Comments can be placed between instructions and on their own lines.

---

## Types

Micro has 8 base types. Every value in the program has one of these types.

| name | C analog   | size in bytes | signed | min value  | max value  |
|------|------------|---------------|--------|------------|------------|
| i8   | int8_t     | 1             | yes    | -128       | 127        |
| u8   | uint8_t    | 1             | no     | 0          | 255        |
| i16  | int16_t    | 2             | yes    | -32768     | 32767      |
| u16  | uint16_t   | 2             | no     | 0          | 65535      |
| i32  | int32_t    | 4             | yes    | -2147483648| 2147483647 |
| u32  | uint32_t   | 4             | no     | 0          | 4294967295 |
| f32  | float      | 4             | -      | -          | -          |
| ptr  | void*      | 4             | no     | 0          | 4294967295 |

Notes:
- `ptr` is the type of every pointer and address.
- Integer literals are signed unless they are stored into an unsigned type.
- Literals in micro are written the usual way: `5`, `-3`, `2.5`.
- Comparison operators pick the signed or the unsigned form from the type
  of their operands.
- `f32` is parsed and takes 4 bytes, but arithmetic on floats is not
  implemented yet: a float literal is truncated to an integer.
- Every virtual register is allocated to a 32 bit machine register, and
  values narrower than 4 bytes are written with instructions of their own
  width. Widening such a value back to 32 bits is not implemented yet, so
  use `i32`, `u32` and `ptr` for values you read as a whole register.

---

## Expressions

Micro uses prefix notation. The operator comes first, then its operands.
This is also called Polish notation.

```
+ 5 * 3 2     means  5 + (3 * 2)
* 5 + 3 2     means  5 * (3 + 2)
```

A single literal or a single name is also a valid expression:

```
+ 5 4          \ expression \
5              \ also an expression \
my_vreg        \ also an expression \
"hello, world" \ also an expression \
```

### Operators

The table below is the full set of operators micro implements. Each operator
takes its operands in prefix form.

| operator      | C analog     | description                                  |
|---------------|--------------|----------------------------------------------|
| + <o1> <o2>   | <o1> + <o2>  | adds o1 and o2                               |
| - <o1> <o2>   | <o1> - <o2>  | subtracts o2 from o1                         |
| * <o1> <o2>   | <o1> * <o2>  | multiplies o1 and o2                         |
| / <o1> <o2>   | <o1> / <o2>  | divides o1 by o2                             |
| $ <p>         | *<p>         | value at address p (see note 1)              |
| = <o1> <o2>   | <o1> == <o2> | 1 if o1 equals o2, otherwise 0               |
| < <o1> <o2>   | <o1> < <o2>  | 1 if o1 is less than o2, otherwise 0         |
| > <o1> <o2>   | <o1> > <o2>  | 1 if o1 is bigger than o2, otherwise 0       |
| <= <o1> <o2>  | <o1> <= <o2> | 1 if o1 is less or equal, otherwise 0        |
| >= <o1> <o2>  | <o1> >= <o2> | 1 if o1 is bigger or equal, otherwise 0     |

Notes:

1. `$` reads a 32 bit value from the given address.

```
fun f
    ptr p
    ret i32
start
    set i32 val $p;      \ read through the pointer \
    set i32 $p + val 1;  \ write back through it \
end
```

The symbols `&`, `#`, `` ` `` and `~` are reserved: the lexer knows them,
but there is no code behind them yet. Using one gives an
`Expression parse error`. `!` is not an expression operator either, it
negates an `if` jump, see [conditional jumps](#conditional-jumps).

Expressions can be nested without limit, as long as each operator receives
the right number of operands.

---

## Virtual registers

Micro stores values in virtual registers. A virtual register has a name, a
type, and an optional starting value.

You create or set a virtual register with the `set` keyword:

```
set <type> <name> [<expression>];
```

If you skip the expression, the register is created without a value:

```
set i32 count;          \ empty register, filled later \
set i32 number 5;       \ register with value 5 \
set i32 total + number 2;  \ register with result of expression \
```

A `set` without an initial value can also be used to write a new value into
an existing register:

```
set i32 count 1;
set i32 count + count 1;  \ now count holds 2 \
```

To store a value through a pointer, put `$` before the name. The value is
written to the address stored in that register:

```
set ptr slot;
set i32 $slot 42;
```

A register cannot have the same name as a function, and it cannot change
its type after it is created.

---

## Lifetime hints

By default a virtual register lives until the end of its function and keeps
its machine register. A hint tells the compiler to release it earlier, so
that later registers can reuse that machine register:

```
set <type> <name> [<expression>] { <hint>: <value>, ... };
```

| hint           | value | description                                  |
|----------------|-------|----------------------------------------------|
| `lifetime`     | int   | release the register after this many further instructions |
| `forced_stack` | bool  | reserved, parsed and ignored                 |
| `lazy_init`    | bool  | reserved, parsed and ignored                 |

`lifetime: 0` releases the register right away, so the next instruction
must not use it. `lifetime: 4` keeps it for four more instructions.

```
set i32 nn - n 1 { lifetime: 4 };  \ only needed by the next 4 instructions \
```

Using a name that is not a defined function, or as a jump target, gives
`Identifier is not a virtual register`.

---

## Functions

A function is a named block of code. Functions look like labels in an
assembler, but they also carry argument and return type information.

```
fun <name>
    <type> <name>
    <type> <name>
    ...
    ret <type>
start
    <body>
end
```

- The list of arguments is optional.
- The `ret <type>` line is optional and sets the return type.
- The body sits between `start` and `end`.
- A hint block, like the one of a `set`, can follow the `end` keyword.

A minimal function:

```
fun empty
start
end
```

A function with one argument and no return value:

```
fun print_num
    i32 num
start
    \ some code \
end
```

A function with arguments and a return type:

```
fun add
    i32 a
    i32 b
    ret i32
start
    ret + a b;
end
```

Arguments are placed on the stack and are used as virtual registers inside
the body. A copy of an argument placed into a new virtual register can live
in a machine register, so read arguments through such a copy when the
argument takes part in many computations.

---

## Calling functions

You call a function with the `call` keyword:

```
call <result_register> <function_name> <arg1> <arg2> ... ;
```

- The first name is the register that receives the return value.
- If the function has no return value, or you do not want to keep it, use
  `_` as the result register.
- Arguments are expressions, separated by spaces.
- The called function must be defined before the call.

Call without arguments:

```
fun empty
start
end

fun main
start
    call _ empty;
end
```

Call with arguments:

```
fun add
    i32 a
    i32 b
    ret i32
start
    ret + a b;
end

fun main
    ret i32
start
    call _ add 3 4;
end
```

---

## Returning values

The `ret` keyword ends the current function and returns a value.

```
ret [<expression>];
```

If the function has a return type, the expression result must match it:

```
fun get_five
    ret i32
start
    ret 5;
end
```

A function without a return type can still call `ret` without a value:

```
fun done
start
    ret;
end
```

`ret` always jumps to the end of the function, so nothing after it runs.

---

## Labels and jumps

A label is a named point in the code. You can jump to it with `goto`.

A label is written as a name followed by a colon:

```
my_label:
```

You jump to it with `goto`:

```
fun loop
    ret i32
start
    set i32 counter 0;
    goto my_lbl;
    set i32 counter 54;  \ this code never runs \
my_lbl:
    set i32 counter + counter 1;
    ret counter;
end
```

`goto` and labels are the only way to build loops and branches in micro.
For conditional branching use [`if`](#conditional-jumps).

---

## Conditional jumps

The `if` keyword jumps to a label when a condition is true:

```
if <expression> : <label>;
```

The condition is a full expression. A virtual register is true when it
holds a non-zero value, a comparison operator produces 1 or 0, and any
other expression is true when its result is not zero.

```
fun f
    i32 n
    ret i32
start
    if n : non_zero;          \ jump when n != 0 \
    if <= n 1 : small;        \ jump when n <= 1 \
    if = * n 4 8 : eight;     \ jump when n * 4 == 8 \
    ret 0;
small:
    ret 1;
eight:
    ret 8;
non_zero:
    ret 2;
end
```

To jump when the condition is *false* instead, prefix it with `!`:

```
if ! <= n 1 : big;   \ jump when n > 1 \
```

Several `!` in a row are allowed: `if ! ! n : lbl;` jumps when `n` is
zero.

After the jump target, the code continues normally, so `if` acts like a
conditional `goto`. If the target label does not exist, compilation fails.

---

## Code selection

The code generator picks the shortest instruction sequence for the common
addressing patterns. You do not have to ask for it, but writing the
expression in the shape below lets the compiler find it.

| expression             | generated                        |
|------------------------|----------------------------------|
| `* i 4`                | `lea r, [i * 4]`                 |
| `+ b * i 4`            | `lea r, [b + i * 4]`             |
| `+ + b * i 4 8`        | `lea r, [b + i * 4 + 8]`         |
| `+ * i 4 8`            | `lea r, [i * 4 + 8]`             |
| `+ b * + i 1 4`        | `lea r, [b + i * 4 + 4]`         |
| `+ i 1`                | `inc r`                          |
| `- i 1`                | `dec r`                          |

A scale of 2, 4 or 8 fits into one instruction. Any other literal, and
every value that does not live in a machine register, falls back to the
plain `imul` and `add` sequence.

---

## Full example

A small program that shows functions, arguments, calls, registers and
returns working together.

```
fun add
    i32 a
    i32 b
    ret i32
start
    ret + a b;
end

fun sub
    i32 a
    i32 b
    ret i32
start
    ret - a b;
end

fun main
    ret i32
start
    set i32 sum;
    call sum add 10 5;

    set i32 diff;
    call diff sub sum 3;

    ret diff;
end
```
