/*
            Copyright Oliver Kowalke 2009.
            Copyright Thomas Sailer 2013.
   Distributed under the Boost Software License, Version 1.0.
      (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

/**************************************************************************************
 *                                                                                    *
 * ---------------------------------------------------------------------------------- *
 * |     0   |     1   |     2    |     3   |     4   |     5   |     6   |     7   | *
 * ---------------------------------------------------------------------------------- *
 * |    0x0  |    0x4  |    0x8   |    0xc  |   0x10  |   0x14  |   0x18  |   0x1c  | *
 * ---------------------------------------------------------------------------------- *
 * |      fbr_strg     |      fc_dealloc    |       limit       |        base       | *
 * ---------------------------------------------------------------------------------- *
 * ---------------------------------------------------------------------------------- *
 * |     8   |    9    |    10    |    11   |    12   |    13   |    14   |    15   | *
 * ---------------------------------------------------------------------------------- *
 * |   0x20  |  0x24   |   0x28   |   0x2c  |   0x30  |   0x34  |   0x38  |   0x3c  | *
 * ---------------------------------------------------------------------------------- *
 * |        R12        |         R13        |        R14        |        R15        | *
 * ---------------------------------------------------------------------------------- *
 * ---------------------------------------------------------------------------------- *
 * |    16   |    17   |    18   |    19    |    20   |    21   |    22   |    23   | *
 * ---------------------------------------------------------------------------------- *
 * |   0xe40  |   0x44 |   0x48  |   0x4c   |   0x50  |   0x54  |   0x58  |   0x5c  | *
 * ---------------------------------------------------------------------------------- *
 * |        RDI         |       RSI         |        RBX        |        RBP        | *
 * ---------------------------------------------------------------------------------- *
 * ---------------------------------------------------------------------------------- *
 * |    24   |   25    |    26    |   27    |    28   |    29   |    30   |    31   | *
 * ---------------------------------------------------------------------------------- *
 * |   0x60  |   0x64  |   0x68   |   0x6c  |   0x70  |   0x74  |   0x78  |   0x7c  | *
 * ---------------------------------------------------------------------------------- *
 * |        hidden     |         RIP        |        EXIT       |   parameter area  | *
 * ---------------------------------------------------------------------------------- *
 * ---------------------------------------------------------------------------------- *
 * |    32   |   32    |    33    |   34    |    35   |    36   |    37   |    38   | *
 * ---------------------------------------------------------------------------------- *
 * |   0x80  |   0x84  |   0x88   |   0x8c  |   0x90  |   0x94  |   0x98  |   0x9c  | *
 * ---------------------------------------------------------------------------------- *
 * |                       parameter area                       |        FCTX       | *
 * ---------------------------------------------------------------------------------- *
 * ---------------------------------------------------------------------------------- *
 * |    39   |   40    |    41    |   42    |    43   |    44   |    45   |    46   | *
 * ---------------------------------------------------------------------------------- *
 * |   0xa0  |   0xa4  |   0xa8   |   0xac  |   0xb0  |   0xb4  |   0xb8  |   0xbc  | *
 * ---------------------------------------------------------------------------------- *
 * |       DATA        |                    |                   |                   | *
 * ---------------------------------------------------------------------------------- *
 *                                                                                    *
 * ***********************************************************************************/

.file	"ontop_x86_64_ms_pe_gas.asm"
.text
.p2align 4,,15
.globl	ontop_fcontext
.def	ontop_fcontext;	.scl	2;	.type	32;	.endef
.seh_proc	ontop_fcontext
ontop_fcontext:
.seh_endprologue

    pushq  %rcx  /* save hidden address of transport_t */

    pushq  %rbp  /* save RBP */
    pushq  %rbx  /* save RBX */
    pushq  %rsi  /* save RSI */
    pushq  %rdi  /* save RDI */
    pushq  %r15  /* save R15 */
    pushq  %r14  /* save R14 */
    pushq  %r13  /* save R13 */
    pushq  %r12  /* save R12 */

    /* load NT_TIB */
    movq  %gs:(0x30), %r10
    /* save current stack base */
    movq  0x08(%r10), %rax
    pushq  %rax
    /* save current stack limit */
    movq  0x10(%r10), %rax
    pushq  %rax
    /* save current deallocation stack */
    movq  0x1478(%r10), %rax
    pushq  %rax
    /* save fiber local storage */
    movq  0x18(%r10), %rax
    pushq  %rax

    /* preserve RSP (pointing to context-data) in RCX */
    movq  %rsp, %rcx

    /* restore RSP (pointing to context-data) from RDX */
    movq  %rdx, %rsp

    /* load NT_TIB */
    movq  %gs:(0x30), %r10
    /* restore fiber local storage */
    popq  %rax
    movq  %rax, 0x18(%r10)
    /* restore deallocation stack */
    popq  %rax
    movq  %rax, 0x1478(%r10)
    /* restore stack limit */
    popq  %rax
    movq  %rax, 0x10(%r10)
    /* restore stack base */
    popq  %rax
    movq  %rax, 0x8(%r10)

    popq  %r12  /* restore R12 */
    popq  %r13  /* restore R13 */
    popq  %r14  /* restore R14 */
    popq  %r15  /* restore R15 */
    popq  %rdi  /* restore RDI */
    popq  %rsi  /* restore RSI */
    popq  %rbx  /* restore RBX */
    popq  %rbp  /* restore RBP */

    popq  %rax  /* restore hidden address of transport_t */

    /* keep return-address on stack */

    /* transport_t returned in RAX */
    /* return parent fcontext_t */
    movq  %rcx, (%rax)
    /* return data */
    movq  %r8, 0x8(%rax)

    /* transport_t as 1.arg of context-function */
    /* RCX contains address of returned (hidden) transfer_t */
    movq  %rax, %rcx
    /* RDX contains address of passed transfer_t */
    movq  %rax, %rdx

    /* indirect jump to context */
    jmp  *%r9
.seh_endproc

.section .drectve
.ascii " -export:\"ontop_fcontext\""
