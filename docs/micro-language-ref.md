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

# Variables
Variables decl syntax:
```
var <type> <name>;
```
or
```
var <type> <name> <init expression>;
```
If *init expression* not set static variable will be initializate as 0
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

## Variable change
To change the value of variable use *set* keyword:
```
set <var name> <new expression>;
```
