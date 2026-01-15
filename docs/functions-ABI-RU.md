# Возвращаемое значение
возвращаемое передаеться в eax (всегда)

# Пролог и эпилог финкции
в начале функции должен быть выполнен пролог:
```
push ebp
mov ebp, esp
```
а в конце функции эпилог:
```
mov esp, ebp  ; можно опустить если esp не был сдвинут
pop ebp
ret
```
или
```
leave
ret
```

# Аргументы функций
Аргументы передаються через стэк в обратном порядке расширяясь до 32 бит
```
fun F
    u16 a1  i8 a2  ptr a3
start
end

call _ F 5 32 1

stack:
0xFF|      0      | \
0xFE|      0      | |
0xFD|      0      | | ptr (u32) a3
0xFC|      1      | /
0xFB|      0      | \
0xFA|      0      | |
0xF9|      0      | | i8 a2
0xF8|      32     | /
0xF7|      0      | \
0xF6|      0      | |
0xF5|      0      | | u16 a1
0xF4|      5      | /
0xF3| return addr |
0xF2| return addr |
0xF1| return addr |
0xF0| return addr |
0xEF|  saves ebp  | <-- esp <-- ebp
0xEE|  saves ebp  |
0xED|  saves ebp  |
0xEC|  saves ebp  |
```

для получения Nного аргумента:
```
[ebp + 4 + ((N - 1) * 4 + sizeof(aN))]
```