; matrixmul.ll — Matrix multiplication C = A * B
; Equivalent test for micro-lang, LLVM IR, and GCC MIR backends
;
; void matrixmul(i32* %A, i32* %B, i32* %C, i32 %N)
; Multiplies N x N matrices: C = A * B

define void @matrixmul(i32* %A, i32* %B, i32* %C, i32 %N) {
entry:
  br label %row_loop

row_loop:
  %i = phi i32 [ 0, %entry ], [ %i.next, %next_row ]
  %done_cmp = icmp sge i32 %i, %N
  br i1 %done_cmp, label %done, label %col_init

col_init:
  br label %col_loop

col_loop:
  %j = phi i32 [ 0, %col_init ], [ %j.next, %store ]
  %col_cmp = icmp sge i32 %j, %N
  br i1 %col_cmp, label %next_row, label %dot_init

dot_init:
  %sum.start = add i32 0, 0
  %k.start = add i32 0, 0
  %row_idx = mul i32 %i, %N
  %row_off = mul i32 %row_idx, 4
  br label %dot_loop

dot_loop:
  %k = phi i32 [ %k.start, %dot_init ], [ %k.next, %dot_continue ]
  %sum = phi i32 [ %sum.start, %dot_init ], [ %sum.next, %dot_continue ]
  %dot_cmp = icmp sge i32 %k, %N
  br i1 %dot_cmp, label %store, label %load_a

load_a:
  ; addr_a = A + (i * N + k)
  %a_idx = mul i32 %i, %N
  %a_idx2 = add i32 %a_idx, %k
  %addr_a = getelementptr i32, i32* %A, i32 %a_idx2
  %va = load i32, i32* %addr_a

  ; addr_b = B + (k * N + j)
  %b_idx = mul i32 %k, %N
  %b_idx2 = add i32 %b_idx, %j
  %addr_b = getelementptr i32, i32* %B, i32 %b_idx2
  %vb = load i32, i32* %addr_b

  ; sum += va * vb
  %prod = mul i32 %va, %vb
  %sum.next = add i32 %sum, %prod

  %k.next = add i32 %k, 1
  br label %dot_continue

dot_continue:
  br label %dot_loop

store:
  ; addr_c = C + (i * N + j)
  %c_idx = mul i32 %i, %N
  %c_idx2 = add i32 %c_idx, %j
  %addr_c = getelementptr i32, i32* %C, i32 %c_idx2
  store i32 %sum, i32* %addr_c

  %j.next = add i32 %j, 1
  br label %col_loop

next_row:
  %i.next = add i32 %i, 1
  br label %row_loop

done:
  ret void
}

; Entry point: multiply two 10x10 matrices
define i32 @main() {
entry:
  %A = alloca [100 x i32]
  %B = alloca [100 x i32]
  %C = alloca [100 x i32]

  %A.ptr = getelementptr [100 x i32], [100 x i32]* %A, i32 0, i32 0
  %B.ptr = getelementptr [100 x i32], [100 x i32]* %B, i32 0, i32 0
  %C.ptr = getelementptr [100 x i32], [100 x i32]* %C, i32 0, i32 0

  ; Initialize A: fill with 1s
  br label %init_a

init_a:
  %ai = phi i32 [ 0, %entry ], [ %ai.next, %init_a.body ]
  %ai.cmp = icmp sge i32 %ai, 100
  br i1 %ai.cmp, label %init_b, label %init_a.body

init_a.body:
  %a.ptr = getelementptr i32, i32* %A.ptr, i32 %ai
  store i32 1, i32* %a.ptr
  %ai.next = add i32 %ai, 1
  br label %init_a

init_b:
  %bi = phi i32 [ 0, %init_a ], [ %bi.next, %init_b.body ]
  %bi.cmp = icmp sge i32 %bi, 100
  br i1 %bi.cmp, label %multiply, label %init_b.body

init_b.body:
  %b.ptr = getelementptr i32, i32* %B.ptr, i32 %bi
  store i32 2, i32* %b.ptr
  %bi.next = add i32 %bi, 1
  br label %init_b

multiply:
  call void @matrixmul(i32* %A.ptr, i32* %B.ptr, i32* %C.ptr, i32 10)
  ret i32 0
}
