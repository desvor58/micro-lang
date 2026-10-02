; smoke.ll — architecture neutral checksum loop
; Equivalent test for micro-lang, LLVM IR, and GCC MIR backends
;
; i32 checksum(i32* data, i32 n)
; Sums k * data[k] over the array.

define i32 @checksum(i32* %data, i32 %n) {
entry:
  br label %loop

loop:
  %i = phi i32 [ 0, %entry ], [ %i.next, %body ]
  %acc = phi i32 [ 0, %entry ], [ %acc.next, %body ]
  %addr = phi i32* [ %data, %entry ], [ %addr.next, %body ]
  %cmp = icmp sge i32 %i, %n
  br i1 %cmp, label %done, label %body

body:
  %word = load i32, i32* %addr, align 4
  %scaled = mul i32 %i, %word
  %acc.next = add i32 %acc, %scaled
  %addr.next = getelementptr i32, i32* %addr, i32 1
  %i.next = add i32 %i, 1
  br label %loop

done:
  ret i32 %acc
}

; Entry point: checksum over the 64 word reverse-sorted array
define i32 @main() {
entry:
  %arr = alloca [64 x i32], align 16
  %data = getelementptr inbounds [64 x i32], [64 x i32]* %arr, i32 0, i32 0
  br label %init

init:
  %i = phi i32 [ 0, %entry ], [ %i.next, %fill ]
  %addr = phi i32* [ %data, %entry ], [ %addr.next, %fill ]
  %cmp = icmp sge i32 %i, 64
  br i1 %cmp, label %run, label %fill

fill:
  %val = sub i32 64, %i
  store i32 %val, i32* %addr, align 4
  %addr.next = getelementptr i32, i32* %addr, i32 1
  %i.next = add i32 %i, 1
  br label %init

run:
  %r = call i32 @checksum(i32* %addr, i32 64)
  ret i32 %r
}