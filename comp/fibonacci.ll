; fibonacci.ll — Recursive Fibonacci
; Equivalent test for micro-lang, LLVM IR, and GCC MIR backends
;
; Recursive Fibonacci: returns fib(n)
; fib(0) = 0, fib(1) = 1, fib(n) = fib(n-1) + fib(n-2)

define i32 @fibonacci(i32 %n) {
entry:
  %cmp = icmp sle i32 %n, 1
  br i1 %cmp, label %base, label %recurse

base:
  ret i32 %n

recurse:
  %n1 = sub i32 %n, 1
  %a = call i32 @fibonacci(i32 %n1)
  %n2 = sub i32 %n, 2
  %b = call i32 @fibonacci(i32 %n2)
  %result = add i32 %a, %b
  ret i32 %result
}

; Entry point: call fibonacci(30)
define i32 @main() {
entry:
  %r = call i32 @fibonacci(i32 30)
  ret i32 %r
}
