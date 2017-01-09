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
.globl make_fcontext
.align 2
make_fcontext:
    @ shift address in A1 to lower 16 byte boundary
    bic  a1, a1, #15

    @ reserve space for context-data on context-stack
    sub  a1, a1, #64

    @ third arg of make_fcontext() == address of context-function
    str  a3, [a1, #44]

    @ compute address of returned transfer_t
    add  a2, a1, #48
    mov  a3, a2
    str  a3, [a1, #4]

    @ compute abs address of label finish
    adr  a2, finish
    @ save address of finish as return-address for context-function
    @ will be entered after context-function returns
    str  a2, [a1, #40]

    bx  lr @ return pointer to context-data

finish:
    @ exit code is zero
    mov  a1, #0
    @ exit application
    bl  _exit
#endif
