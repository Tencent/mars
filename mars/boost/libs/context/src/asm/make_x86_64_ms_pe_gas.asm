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

.file	"make_x86_64_ms_pe_gas.asm"
.text
.p2align 4,,15
.globl	make_fcontext
.def	make_fcontext;	.scl	2;	.type	32;	.endef
.seh_proc	make_fcontext
make_fcontext:
.seh_endprologue

    /* first arg of make_fcontext() == top of context-stack */
    movq  %rcx, %rax

    /* shift address in RAX to lower 16 byte boundary */
    /* == pointer to fcontext_t and address of context stack */
    andq  $-16, %rax

    /* reserve space for context-data on context-stack */
    /* on context-function entry: (RSP -0x8) % 16 == 0 */
    leaq  -0xb8(%rax), %rax

    /* third arg of make_fcontext() == address of context-function */
    movq  %r8, 0x68(%rax)

    /* first arg of make_fcontext() == top of context-stack */
    /* save top address of context stack as 'base' */
    movq  %rcx, 0x18(%rax)
    /* second arg of make_fcontext() == size of context-stack */
    /* negate stack size for LEA instruction (== substraction) */
    negq  %rdx
    /* compute bottom address of context stack (limit) */
    leaq  (%rcx,%rdx), %rcx
    /* save bottom address of context stack as 'limit' */
    movq  %rcx, 0x10(%rax)
    /* save address of context stack limit as 'dealloction stack' */
    movq  %rcx, 0x8(%rax)

    /* compute address of transport_t */
    leaq  0x98(%rax), %rcx
    /* store address of transport_t in hidden field */
    movq %rcx, 0x60(%rax)

    /* compute abs address of label finish */
    leaq  finish(%rip), %rcx
    /* save address of finish as return-address for context-function */
    /* will be entered after context-function returns */
    movq  %rcx, 0x70(%rax)

    ret /* return pointer to context-data */

finish:
    /* 32byte shadow-space for _exit() */
    andq  $-32, %rsp
    /* 32byte shadow-space for _exit() are */
    /* already reserved by make_fcontext() */
    /* exit code is zero */
    xorq  %rcx, %rcx
    /* exit application */
    call  _exit
    hlt
.seh_endproc

.def	_exit;	.scl	2;	.type	32;	.endef  /* standard C library function */

.section .drectve
.ascii " -export:\"make_fcontext\""
