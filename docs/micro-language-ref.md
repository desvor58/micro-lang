# Coments
All code betwen '\' will be ignored
```
\ its a comments line \
var i32 a 5;
\
yooooooo
its a comment!!!
yeaaaaaaa
\
```


# Types
Micro have 7 base types:
| name | C analog            | size (in bytes) | min value          | max value          |
|------|---------------------|-----------------|--------------------|--------------------|
| i8   | int8_t              | 1               | -128               | 127                |
| i16  | int16_t             | 2               | -32768             | 32767              |
| i32  | int32_t (short)     | 4 (2)           | -2147483648        | 2147483647         |
| u8   | uint8_t             | 1               | 0                  | 256                |
| u16  | uint16_t            | 2               | 0                  | 65536              |
| u32  | uint32_t (uint16_t) | 4 (2)           | 0                  | 4294967296         |
| f32  | float               | 4 (2)           | 1.175494351 E - 38 | 3.402823466 E + 38 |
| ptr  | size_t              | 4 (2)           | 0                  | 4294967296         |

All pointers have ptr type

## note
If micro code builds for 16bits target, i32, u32, f32 and ptr type will be have 2 byte size


# Expresions
Micro lang use prefix notation like this:
```
+ 5 * 3 2  \ in standart notation: 3 * 2 + 5 \
* 5 + 3 2  \ in standart notation: 5 * (2 + 3) \
```

### note
In this document single literals is a expresion:
```
+ 5 4 \ expression \
5 \ also expresion \
"hello, world" \ expresion \
```

## Operators
Micro support +, -, * and / operators - addition, substruction, multipling, divising

| operator         | C analog         | description                      |
|------------------|------------------|----------------------------------|
| + \<o1> \<o2>    | \<o1> + \<o2>    | adds *o1* and *o2*               |
| - \<o1> \<o2>    | \<o1> - \<o2>    | from *o1* subtracts *o2*         |
| ~ \<o1>          | -\<o1>           | negate <o1>                      |
| * \<o1> \<o2>    | \<o1> * \<o2>    | mutliply *o1* and *o2*           |
| / \<o1> \<o2>    | \<o1> / \<o2>    | divides *o1* by *o2*             |
| # \<i>           | &\<i>            | get addres of <i>                |
| $ \<p>           | *\<p>            | get value (see note 1) by addres |
| & \<o1> \<o2>    | \<o1> & \<o2>    | bitwise *and* with *o1* and *o2* |
| \| \<o1> \<o2>   | \<o1> \| \<o2>   | bitwise *or* with *o1* and *o2*  |
| ^ \<o1> \<o2>    | \<o1> ^ \<o2>    | bitwise *xor* with *o1* and *o2* |
| ` \<o1>          | ~\<o2>           | bitwise *not* with *o1*          |
| && \<o1> \<o2>   | \<o1> && \<o2>   | logical *and* with *o1* and *o2* |
| \|\| \<o1> \<o2> | \<o1> \|\| \<o2> | logical *or* with *o1* and *o2*  |
| ! \<o1>          | !\<o2>           | logical *not* with *o1*          |

### notes
1. value for size of expected type


# Variables
Variables decl syntax:
```
var <type> <name>;
```
or
```
var <type> <name> <init value>;
```
If *init value* not set static variable will be initializate as 0

You cant using expression as *init value*!

---

Variables alocating on stack. *rsp* register stay on a return address:
```
var i32 a 5;
var i8  b 4;
```
```
stack:
|<ret addr>| 0x103 <- rsp
|<ret addr>| 0x102
|<ret addr>| 0x101
|<ret addr>| 0x100
|    0     | 0xFF  <- a
|    0     | 0xFE
|    0     | 0xFD
|    5     | 0xFC
|    4     | 0xFB  <- b
```

# Variable change
To change the value of variable use *set* keyword:
```
set <var name> <new expression>;
```

For example:
```
var u32 a 5; \ a = 5 \
set a + 8 5; \ a = 13 \
```


# Functuins
Functions in micro work like "functions" in most assembers: label is specified at the beginning of the function and 'ret' instruction is specified at end:
```
some_function_name:  ; start of function
    add ax, cx       ; some code
    ret              ; end of function
```

But these "function" desnt have parametrs and, for example, specified return type in language level, unlike micro

Functions in micro declaration in BNF notation:
```
fun <name> [<param type 1> <param name 1> <param type 2> <param name 2> <param type N> <param name N>] [ret <function return type>] start <body of fuction> end
```
note: check the [style guide of function declaration](./good-micro-code-style.md#function-declaration)

---

Functions can be called like in an assebler: with 'call' keyword:
```
\ declaration \
fun test:
start
    \ some code \
end

call _ test; \ calling function \
```
But keyword 'call' in micro requires destination variable for return value of function. If function do not have return value or if you dont wont save function return value, you can use '_' name

Arguments of function can be given after function name:
```
\ declaration \
fun test:
    u32 arg1
    u32 arg2
start
    \ some code \
end

var u32 a 5;
var u32 b 7;

call _ test a b; \ calling function \
```

To save return value of function:
```
\ declaration \
fun test:
    u32 arg1
    u32 arg2
    ret u32
start
    \ some code \
end

var u32 a 5;
var u32 b 7;
var u32 res;

call res test a b; \ calling function \
```