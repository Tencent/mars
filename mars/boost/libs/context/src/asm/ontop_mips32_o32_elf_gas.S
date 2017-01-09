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
 *  |  0  |  4  |  8  |  12 |  16 |  20 |  24 |  28 |  *
 *  -------------------------------------------------  *
 *  |  S0 |  S1 |  S2 |  S3 |  S4 |  S5 |  S6 |  S7 |  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  8  |  9  |  10 |  11 |  12 |  13 |  14 |  15 |  *
 *  -------------------------------------------------  *
 *  |  32 |  36 |  40 |  44 |  48 |  52 |  56 |  60 |  *
 *  -------------------------------------------------  *
 *  |  FP |hiddn|  RA |  PC |  GP | FCTX| DATA|     |  *
 *  -------------------------------------------------  *
 *                                                     *
 * *****************************************************/

.text
.globl ontop_fcontext
.align 2
.type ontop_fcontext,@function
.ent ontop_fcontext
ontop_fcontext:
    # reserve space on stack
    addiu $sp, $sp, -112

    sw  $s0, ($sp)  # save S0
    sw  $s1, 4($sp)  # save S1
    sw  $s2, 8($sp)  # save S2
    sw  $s3, 12($sp)  # save S3
    sw  $s4, 16($sp)  # save S4
    sw  $s5, 20($sp)  # save S5
    sw  $s6, 24($sp)  # save S6
    sw  $s7, 28($sp)  # save S7
    sw  $fp, 32($sp)  # save FP
    sw  $a0, 36($sp)  # save hidden, address of returned transfer_t
    sw  $ra, 40($sp)  # save RA
    sw  $ra, 44($sp)  # save RA as PC

    # store SP (pointing to context-data) in A0
    move  $a0, $sp

    # restore SP (pointing to context-data) from A1
    move  $sp, $a1

    lw  $s0, ($sp)  # restore S0
    lw  $s1, 4($sp)  # restore S1
    lw  $s2, 8($sp)  # restore S2
    lw  $s3, 12($sp)  # restore S3
    lw  $s4, 16($sp)  # restore S4
    lw  $s5, 20($sp)  # restore S5
    lw  $s6, 24($sp)  # restore S6
    lw  $s7, 28($sp)  # restore S7
    lw  $fp, 32($sp)  # restore FP
    lw  $t0, 36($sp)  # restore hidden, address of returned transfer_t
    lw  $ra, 40($sp)  # restore RA

    # load PC
    lw  $t9, 44($sp)

    # adjust stack
    addiu $sp, $sp, 112
    
    # return transfer_t from jump
    sw  $a0, ($t0)  # fctx of transfer_t
    sw  $a2, 4($t0) # data of transfer_t
    # pass transfer_t as first arg in context function
    # A0 == hidden, A1 == fctx, A2 == data
    move  $a1, $a0 
    move  $a0, $t0 

    # jump to context
    jr  $a3
.end ontop_fcontext
.size ontop_fcontext, .-ontop_fcontext

/* Mark that we don't need executable stack.  */
.section .note.GNU-stack,"",%progbits
