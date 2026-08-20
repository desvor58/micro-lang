# Micro Language Reference

## Contents
1. [Introduction](#introduction)
2. [Comments](#comments)
3. [Types](#types)
4. [Expressions](#expressions)
5. [Virtual registers](#virtual-registers)
6. [Functions](#functions)
7. [Calling functions](#calling-functions)
8. [Returning values](#returning-values)
9. [Labels and jumps](#labels-and-jumps)

---

## Introduction

Micro is a low level IR language. It is a linear language: the code is a
plain list of instructions, and instructions are grouped into functions.
There are no blocks, no scopes, and no implicit control flow. You write
exactly what should happen, step by step.

A program is a set of functions. Each function has a name, a list of
arguments, an optional return type, and a body.

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

The table below is the full set of operators micro targets. Each operator
takes its operands in prefix form.

| operator          | C analog         | description                        |
|-------------------|------------------|------------------------------------|
| + <o1> <o2>       | <o1> + <o2>      | adds o1 and o2                     |
| - <o1> <o2>       | <o1> - <o2>      | subtracts o2 from o1               |
| ~ <o1>            | -<o1>            | negates o1                         |
| * <o1> <o2>       | <o1> * <o2>      | multiplies o1 and o2               |
| / <o1> <o2>       | <o1> / <o2>      | divides o1 by o2                   |
| # <i>             | &<i>             | address of i                       |
| $ <p>             | *<p>             | value at address p (see note 1)    |
| & <o1> <o2>       | <o1> & <o2>      | bitwise and of o1 and o2           |
| | <o1> <o2>       | <o1> | <o2>      | bitwise or of o1 and o2            |
| ^ <o1> <o2>       | <o1> ^ <o2>      | bitwise xor of o1 and o2           |
| ` <o1>            | ~<o1>            | bitwise not of o1                  |
| && <o1> <o2>      | <o1> && <o2>     | logical and of o1 and o2           |
| || <o1> <o2>      | <o1> || <o2>     | logical or of o1 and o2            |
| ! <o1>            | !<o1>            | logical not of o1                  |
| == <o1> <o2>      | <o1> == <o2>     | true if o1 equals o2               |
| != <o1> <o2>      | <o1> != <o2>     | true if o1 not equals o2           |
| > <o1> <o2>       | <o1> > <o2>      | true if o1 is bigger than o2       |
| < <o1> <o2>       | <o1> < <o2>      | true if o1 is less than o2         |
| >= <o1> <o2>      | <o1> >= <o2>     | true if o1 is bigger or equal      |
| <= <o1> <o2>      | <o1> <= <o2>     | true if o1 is less or equal        |

Notes:

1. `$` reads the value of the expected type from the given address.

```
set i32 val 4;
set ptr p #val;
set i16 other $val;  \ reads val as a pointer to i16 \
```

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
set i32 tmp 5;
set ptr slot #tmp;
set i32 $slot 42;        \ write 42 into tmp \
```

All virtual registers live only inside the function where they were
created. They disappear when the function ends.

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

Arguments act like virtual registers. Inside the body you can read them in
expressions.

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

Call without arguments:

```
fun main
start
    call _ empty;
end
```

Call with arguments:

```
fun main
    ret i32
start
    call _ add 3 4;
end
```

A full example with a declared result register:

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
    set i32 res;
    call res add 10 5;
    ret res;
end
```

---

## Returning values

The `ret` keyword ends the current function and returns a value.

```
ret <expression>;
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
    set counter 54;  \ this code never runs \
my_lbl:
    set i32 counter + counter 1;
    ret counter;
end
```

Unlike in C, labels are global. Any function can jump to any label:

```
fun a
    ret i32
start
    target:
    ret 7;
end

fun b
    ret i32
start
    goto target;
end
```

`goto` and labels are the only way to build loops and branches in micro.

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