; bubblesort.ll — Bubble sort on an integer array
; Equivalent test for micro-lang, LLVM IR, and GCC MIR backends
;
; void bubblesort(i32* %arr, i32 %n)
; Sorts arr[0..n-1] in ascending order using bubble sort.

define void @bubblesort(i32* %arr, i32 %n) {
entry:
  br label %outer

outer:
  %i = phi i32 [ 0, %entry ], [ %i.next, %inner_done ]
  %limit = sub i32 %n, 1
  %done_cmp = icmp sge i32 %i, %limit
  br i1 %done_cmp, label %done, label %inner_init

inner_init:
  %j.start = add i32 0, 0
  %limit2.base = sub i32 %n, %i
  %limit2 = sub i32 %limit2.base, 1
  br label %inner

inner:
  %j = phi i32 [ %j.start, %inner_init ], [ %j.next, %no_swap ]
  %inner_cmp = icmp sge i32 %j, %limit2
  br i1 %inner_cmp, label %inner_done, label %load

load:
  %off_j = mul i32 %j, 4
  %addr_j = getelementptr i32, i32* %arr, i32 %j
  %val_j = load i32, i32* %addr_j
  %jp1 = add i32 %j, 1
  %addr_j1 = getelementptr i32, i32* %arr, i32 %jp1
  %val_j1 = load i32, i32* %addr_j1
  %swap_cmp = icmp sgt i32 %val_j, %val_j1
  br i1 %swap_cmp, label %swap, label %no_swap

swap:
  store i32 %val_j1, i32* %addr_j
  store i32 %val_j, i32* %addr_j1
  br label %no_swap

no_swap:
  %j.next = add i32 %j, 1
  br label %inner

inner_done:
  %i.next = add i32 %i, 1
  br label %outer

done:
  ret void
}

; Entry point: sort a 20-element reverse-sorted array
define i32 @main() {
entry:
  %arr = alloca [20 x i32]
  %arr.ptr = getelementptr [20 x i32], [20 x i32]* %arr, i32 0, i32 0

  ; Initialize array: 20, 19, 18, ..., 1
  %p0  = getelementptr i32, i32* %arr.ptr, i32 0
  store i32 20, i32* %p0
  %p1  = getelementptr i32, i32* %arr.ptr, i32 1
  store i32 19, i32* %p1
  %p2  = getelementptr i32, i32* %arr.ptr, i32 2
  store i32 18, i32* %p2
  %p3  = getelementptr i32, i32* %arr.ptr, i32 3
  store i32 17, i32* %p3
  %p4  = getelementptr i32, i32* %arr.ptr, i32 4
  store i32 16, i32* %p4
  %p5  = getelementptr i32, i32* %arr.ptr, i32 5
  store i32 15, i32* %p5
  %p6  = getelementptr i32, i32* %arr.ptr, i32 6
  store i32 14, i32* %p6
  %p7  = getelementptr i32, i32* %arr.ptr, i32 7
  store i32 13, i32* %p7
  %p8  = getelementptr i32, i32* %arr.ptr, i32 8
  store i32 12, i32* %p8
  %p9  = getelementptr i32, i32* %arr.ptr, i32 9
  store i32 11, i32* %p9
  %p10 = getelementptr i32, i32* %arr.ptr, i32 10
  store i32 10, i32* %p10
  %p11 = getelementptr i32, i32* %arr.ptr, i32 11
  store i32 9, i32* %p11
  %p12 = getelementptr i32, i32* %arr.ptr, i32 12
  store i32 8, i32* %p12
  %p13 = getelementptr i32, i32* %arr.ptr, i32 13
  store i32 7, i32* %p13
  %p14 = getelementptr i32, i32* %arr.ptr, i32 14
  store i32 6, i32* %p14
  %p15 = getelementptr i32, i32* %arr.ptr, i32 15
  store i32 5, i32* %p15
  %p16 = getelementptr i32, i32* %arr.ptr, i32 16
  store i32 4, i32* %p16
  %p17 = getelementptr i32, i32* %arr.ptr, i32 17
  store i32 3, i32* %p17
  %p18 = getelementptr i32, i32* %arr.ptr, i32 18
  store i32 2, i32* %p18
  %p19 = getelementptr i32, i32* %arr.ptr, i32 19
  store i32 1, i32* %p19

  call void @bubblesort(i32* %arr.ptr, i32 20)
  ret i32 0
}
