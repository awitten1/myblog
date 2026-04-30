; int64_t add_integers_asm(const int64_t* nums, size_t sz);
;   System V AMD64: rdi=nums, rsi=sz, returns rax.
;   Requires sz % 4 == 0 (benchmark uses power-of-two sizes).

default rel
section .text
global add_integers_asm
global add_integers_asm_repeat

add_integers_asm:
    vpxor       ymm0, ymm0, ymm0
    test        rsi, rsi
    jz          .empty

    mov         rax, rdi
    lea         rcx, [rdi + rsi*8]      ; end pointer

.loop:
    vpaddq      ymm0, ymm0, [rax]
    add         rax, 32
    cmp         rax, rcx
    jb          .loop

    vextracti128 xmm1, ymm0, 1
    vpaddq      xmm0, xmm0, xmm1
    vpsrldq     xmm1, xmm0, 8
    vpaddq      xmm0, xmm0, xmm1
    vmovq       rax, xmm0
    vzeroupper
    ret

.empty:
    xor         rax, rax
    ret

; int64_t add_integers_asm_repeat(const int64_t* nums, size_t sz, size_t repeat);
;   Sums `nums[0..sz)` `repeat` times into ONE accumulator.
;   Only one prologue/horizontal-reduce per call, so per-call overhead
;   amortizes away as `repeat` grows. sz % 4 == 0 required.
add_integers_asm_repeat:
    vpxor       ymm0, ymm0, ymm0
    test        rsi, rsi
    jz          .ar_empty
    test        rdx, rdx
    jz          .ar_reduce

.ar_outer:
    mov         rax, rdi
    lea         rcx, [rdi + rsi*8]
.ar_inner:
    vpaddq      ymm0, ymm0, [rax]
    add         rax, 32
    cmp         rax, rcx
    jb          .ar_inner
    sub         rdx, 1
    jnz         .ar_outer

.ar_reduce:
    vextracti128 xmm1, ymm0, 1
    vpaddq      xmm0, xmm0, xmm1
    vpsrldq     xmm1, xmm0, 8
    vpaddq      xmm0, xmm0, xmm1
    vmovq       rax, xmm0
    vzeroupper
    ret

.ar_empty:
    xor         rax, rax
    ret
