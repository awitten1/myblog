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

; int64_t add_integers_asm_repeat(const int64_t* nums, size_t sz, size_t total);
;   Sums `total` int64s, cycling through nums[0..sz). Single ymm accumulator.
;   sz must be a power of 2 (>= 4); total must be a multiple of 4.
add_integers_asm_repeat:
    vpxor       ymm0, ymm0, ymm0
    test        rdx, rdx
    jz          .ar_reduce

    shr         rdx, 2                  ; rdx = total / 4 = number of vpaddqs
    lea         r8, [rsi*8]
    sub         r8, 1                   ; r8 = byte mask = sz*8 - 1
    xor         rcx, rcx                ; running byte offset
.ar_loop:
    mov         rax, rcx
    and         rax, r8
    vpaddq      ymm0, ymm0, [rdi + rax]
    add         rcx, 32
    sub         rdx, 1
    jnz         .ar_loop

.ar_reduce:
    vextracti128 xmm1, ymm0, 1
    vpaddq      xmm0, xmm0, xmm1
    vpsrldq     xmm1, xmm0, 8
    vpaddq      xmm0, xmm0, xmm1
    vmovq       rax, xmm0
    vzeroupper
    ret
