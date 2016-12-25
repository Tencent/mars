.globl .jump_fcontext
.globl  jump_fcontext[DS]
.align 2 
.csect	jump_fcontext[DS]
jump_fcontext:
  .long	.jump_fcontext
.jump_fcontext:
    # reserve space on stack
    subi 1, 1, 92

    stw  13, 0(1)  # save R13
    stw  14, 4(1)  # save R14
    stw  15, 8(1)  # save R15
    stw  16, 12(1)  # save R16
    stw  17, 16(1)  # save R17
    stw  18, 20(1)  # save R18
    stw  19, 24(1)  # save R19
    stw  20, 28(1)  # save R20
    stw  21, 32(1)  # save R21
    stw  22, 36(1)  # save R22
    stw  23, 40(1)  # save R23
    stw  24, 44(1)  # save R24
    stw  25, 48(1)  # save R25
    stw  26, 52(1)  # save R26
    stw  27, 56(1)  # save R27
    stw  28, 60(1)  # save R28
    stw  29, 64(1)  # save R29
    stw  30, 68(1)  # save R30
    stw  31, 72(1)  # save R31
    stw  3,  76(1)  # save hidden

    # save CR
    mfcr 0
    stw 0, 80(1)
    # save LR
    mflr 0
    stw 0, 84(1)
    # save LR as PC
    stw 0, 88(1)

    # store RSP (pointing to context-data) in R6
    mr  6, 1

    # restore RSP (pointing to context-data) from R4
    mr 1, 4

    lwz  13, 0(1)  # restore R13
    lwz  14, 4(1)  # restore R14
    lwz  15, 8(1)  # restore R15
    lwz  16, 12(1)  # restore R16
    lwz  17, 16(1)  # restore R17
    lwz  18, 20(1)  # restore R18
    lwz  19, 24(1)  # restore R19
    lwz  20, 28(1)  # restore R20
    lwz  21, 32(1)  # restore R21
    lwz  22, 36(1)  # restore R22
    lwz  23, 40(1)  # restore R23
    lwz  24, 44(1)  # restore R24
    lwz  25, 48(1)  # restore R25
    lwz  26, 52(1)  # restore R26
    lwz  27, 56(1)  # restore R27
    lwz  28, 60(1)  # restore R28
    lwz  29, 64(1)  # restore R29
    lwz  30, 68(1)  # restore R30
    lwz  31, 72(1)  # restore R31
    lwz  3,  76(1)  # restore hidden

    # restore CR
    lwz 0, 80(1)
    mtcr 0
    # restore LR
    lwz 0, 84(1)
    mtlr 0

    # load PC
    lwz 0, 88(1)
    # restore CTR
    mtctr 0

    # adjust stack
    addi 1, 1, 92

    # return transfer_t 
    stw  6, 0(3)
    stw  5, 4(3)

    # jump to context
    bctr
