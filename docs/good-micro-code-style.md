# Naming
In micro, I recommend sticking to snake_case naming, but micro is an IR language, so naming conventions in micro code can be the same as those in the source language that compiles to micro.

# Function declaration
I recommend declarate function like this:
```
fun <function name>
    <param T 1> <param N 1>  <param T N> <param N N>
    ret <ret type>
start
    <body>
end
```