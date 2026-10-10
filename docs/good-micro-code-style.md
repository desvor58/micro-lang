# Good micro code style

## Comments

A comment is written between two backslashes. Everything inside is ignored
by the lexer, so a comment can hold any text, including newlines.

```
\ one line comment \

\
a comment
over several lines
\

set i32 x 5;  \ a comment after an instruction \
```

An unclosed comment is an error, so every `\` needs its pair.

## Naming

Use `snake_case` for virtual registers, labels and function names:

```
fun bubble_sort
    i32 elem_count
    ret i32
start
swap_again:
    ...
end
```

Micro is an IR language, so a name that comes from the source language can
keep the naming of that source language: `myStruct`, `my_field` and
`MyClass::method` style names are all valid.

## Function declaration

Declare a function like this:

```
fun <function name>
    <param type> <param name>
    ...
    ret <return type>
start
    <body>
end
```

The argument list comes first, then the return type, then the body between
`start` and `end`.

A short function with few arguments is worth splitting out: the compiler
inlines it into its callers with `-Oi`, which keeps the source readable and
the call free. Long functions and functions that call themselves are left as
calls, so a helper that is too big to inline is better written as a loop
than as a nest of small calls.

## Statements

End every instruction with a semicolon. Write one instruction per line and
use four spaces of indentation inside a function:

```
fun sum_to
    i32 n
    ret i32
start
    set i32 sum 0;
    set i32 i 0;
loop:
    if >= i n : done;
    set i32 sum + sum i;
    set i32 i + i 1;
    goto loop;
done:
    ret sum;
end
```

## Expressions

Write expressions in prefix form, in the shape the code generator can fold
into a single instruction when it can:

```
set i32 addr + base * index 4;     \ one lea \
set i32 next + index 1;            \ one inc \
set i32 tail + addr 4;             \ one lea \
```

Both folds need their operands in machine registers, which is what a
virtual register gets as soon as it holds a value. A fold also needs the
operator written out: `base * index 4` is `(base * index)` followed by a
stray literal, so `base + index * 4` has to be written `+ base * index 4`.

Put a space around every operator, keep the operand order that reads best
for you, and use a [lifetime hint](micro-language-ref.md#lifetime-hints) when
a register is not needed for the rest of the function:

```
set i32 limit - n 1 { lifetime: 2 };
```
