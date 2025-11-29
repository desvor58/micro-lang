# Coments
All code betwen '\' will be ignored
```
\ its a comments line \
var i32 a = 5;
\
yooooooo
its a comment!!!
yeaaaaaaa
\
```

# Types
Micro have 7 base types:
| name | C analog | size (in bytes) | min value          | max value          |
|------|----------|-----------------|--------------------|--------------------|
| i8   | char     | 1               | -128               | 127                |
| i16  | short    | 2               | -32768             | 32767              |
| i32  | int      | 4               | -2147483648        | 2147483647         |
| u8   | uint8_t  | 1               | 0                  | 256                |
| u16  | uint16_t | 2               | 0                  | 65536              |
| u32  | uint32_t | 4               | 0                  | 4294967296         |
| f32  | float    | 4               | 1.175494351 E - 38 | 3.402823466 E + 38 |

All pointers have u32 type


# Variables
Variables decl syntax:
```
var <type> <name>;
```
or
```
var <type> <name> = <init value>;
```
---
Variables alocating on stack. *rsp* register stay on a return address:
```
var i32 a = 5;
var i8  b = 4;
```
on stack:
```
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

# Expressions
due to the minimalizm of micro compiler expressions writes in prefix notation
For example:
```
\ standart notation \
2 + 5 * 3  \ 17 \

\ prefix notation \
* 3 + 5 2  \ also 17 \
```