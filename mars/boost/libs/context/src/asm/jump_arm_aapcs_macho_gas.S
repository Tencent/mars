/*
            Copyright Oliver Kowalke 2009.
   Distributed under the Boost Software License, Version 1.0.
      (See accompanying file LICENSE_1_0.txt or copy at
          http://www.boost.org/LICENSE_1_0.txt)
*/

/*******************************************************
 *                                                     *
 *  -------------------------------------------------  *
 *  |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  *
 *  -------------------------------------------------  *
 *  | 0x0 | 0x4 | 0x8 | 0xc | 0x10| 0x14| 0x18| 0x1c|  *
 *  -------------------------------------------------  *
 *  | sjlj|hiddn|  v1 |  v2 |  v3 |  v4 |  v5 |  v6 |  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  8  |  9  |  10 |  11 |  12 |  13 |  14 |  15 |  *
 *  -------------------------------------------------  *
 *  | 0x20| 0x24| 0x28| 0x2c| 0x30| 0x34| 0x38| 0x3c|  *
 *  -------------------------------------------------  *
 *  |  v7 |  v8 |  lr |  pc | FCTX| DATA|           |  *
 *  -------------------------------------------------  *
 *                                                     *
 *******************************************************/

#if defined(__arm__)
.text
.globl jump_fcontext
.align 2
jump_fcontext:
    @ save LR as PC
    push {lr}
    @ save hidden,V1-V8,LR
    push {a1,v1-v8,lr}

    @ locate TLS to save/restore SjLj handler
    mrc  p15, 0, v2, c13, c0, #3
    bic  v2, v2, #3

    @ load TLS[__PTK_LIBC_DYLD_Unwind_SjLj_Key]
    ldr  v1, [v2, #8]
    @ save SjLj handler
    push  {v1}

    @ store RSP (pointing to context-data) in A1
    mov  a1, sp

    @ restore RSP (pointing to context-data) from A2
    mov  sp, a2

    @ r#estore SjLj handler
    pop  {v1}
    @ store SjLj handler in TLS
    str  v1, [v2, #8]

    @ restore hidden,V1-V8,LR
    pop {a4,v1-v8,lr}

    @ return transfer_t from jump
    str  a1, [a4, #0]
    str  a3, [a4, #4]
    @ pass transfer_t as first arg in context function
    @ A1 == FCTX, A2 == DATA
    mov  a2, a3

    @ restore PC
    pop {pc}

#endif
