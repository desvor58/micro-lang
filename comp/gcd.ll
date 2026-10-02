; gcd.ll — Greatest Common Divisor (Euclidean algorithm)
; Equivalent test for micro-lang, LLVM IR, and GCC MIR backends
;
; i32 gcd(i32 %a, i32 %b)
; Returns the greatest common divisor of a and b.

define i32 @gcd(i32 %a, i32 %b) {
entry:
  %bz = icmp eq i32 %b, 0
  br i1 %bz, label %done, label %recurse

recurse:
  %rem = srem i32 %a, %b
  %result = call i32 @gcd(i32 %b, i32 %rem)
  ret i32 %result

done:
  ret i32 %a
}

; Entry point: compute gcd(48, 18)
define i32 @main() {
entry:
  %r = call i32 @gcd(i32 48, i32 18)
  ret i32 %r
}
