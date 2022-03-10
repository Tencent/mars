/*
            Copyright Oliver Kowalke 2009.
   Distributed under the Boost Software License, Version 1.0.
      (See accompanying file LICENSE_1_0.txt or copy at
          http://www.boost.org/LICENSE_1_0.txt)
*/

/******************************************************
 *                                                     *
 *  -------------------------------------------------  *
 *  |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  *
 *  -------------------------------------------------  *
 *  |  0  |  4  |  8  |  12 |  16 |  20 |  24 |  28 |  *
 *  -------------------------------------------------  *
 *  | R13 | R14 | R15 | R16 | R17 | R18 | R19 | R20 |  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  8  |  9  |  10 |  11 |  12 |  13 |  14 |  15 |  *
 *  -------------------------------------------------  *
 *  |  32 |  36 |  40 |  44 |  48 |  52 |  56 |  60 |  *
 *  -------------------------------------------------  *
 *  | R21 | R22 | R23 | R24 | R25 | R26 | R27 | R28 |  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  16 |  17 |  18 |  19 |  20 |  21 |  22 |  23 |  *
 *  -------------------------------------------------  *
 *  |  64 |  68 |  72 |  76 |  80 |  84 |  88 |  92 |  *
 *  -------------------------------------------------  *
 *  | R29 | R30 | R31 |hiddn|  CR |  LR |  PC |bchai|  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  24 |  25 |  26 |  27 |  28 |  29 |  30 |  31 |  *
 *  -------------------------------------------------  *
 *  |  96 | 100 | 104 | 108 | 112 | 116 | 120 | 124 |  *
 *  -------------------------------------------------  *
 *  |linkr| FCTX| DATA|                             |  * 
 *  -------------------------------------------------  *
 *                                                     *
 *******************************************************/

.text
.globl _make_fcontext
.align 2
_make_fcontext:
    ; save return address into R6
    mflr  r6

    ; first arg of make_fcontext() == top address of context-function
    ; shift address in R3 to lower 16 byte boundary
    clrrwi  r3, r3, 4

    ; reserve space for context-data on context-stack
    ; including 64 byte of linkage + parameter area (R1 % 16 == 0)
    subi  r3, r3, 172

    ; third arg of make_fcontext() == address of context-function
    stw  r5, 88(r3)

    ; set back-chain to zero
    li   r0, 0
    stw  r0, 92(r3)

    ; compute address of returned transfer_t
    addi  r0, r3, 100
    mr    r4, r0 
    stw   r4, 76(r3) 

    ; load LR
    mflr  r0
    ; jump to label 1
    bl  l1
l1:
    ; load LR into R4
    mflr  r4
    ; compute abs address of label finish
    addi  r4, r4, lo16((finish - .)+4)
    # restore LR
    mtlr  r0
    ; save address of finish as return-address for context-function
    ; will be entered after context-function returns
    stw  r4, 84(r3)

    ; restore return address from R6
    mtlr  r6

    blr  ; return pointer to context-data

finish:
    ; save return address into R0
    mflr  r0
    ; save return address on stack, set up stack frame
    stw  r0, 4(r1)
    ; allocate stack space, R1 % 16 == 0
    stwu  r1, -16(r1)

    ; exit code is zero
    li  r3, 0
    ; exit application
    bl  __exit
