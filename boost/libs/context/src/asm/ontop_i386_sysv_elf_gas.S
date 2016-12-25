/*
            Copyright Oliver Kowalke 2009.
   Distributed under the Boost Software License, Version 1.0.
      (See accompanying file LICENSE_1_0.txt or copy at
          http://www.boost.org/LICENSE_1_0.txt)
*/

/*****************************************************************************************
 *                                                                                       *
 *  -----------------------------------------------------------------------------------  *
 *  |    0    |    1    |    2    |    3    |    4     |    5    |    6     |    7    |  *
 *  -----------------------------------------------------------------------------------  *
 *  |   0x0   |   0x4   |   0x8   |   0xc   |   0x10   |   0x14  |   0x18   |   0x1c  |  *
 *  -----------------------------------------------------------------------------------  *
 *  |   EDI   |   ESI   |   EBX   |   EBP   |   EIP    |  hidden |    to    |   data  |  *
 *  -----------------------------------------------------------------------------------  *
 *                                                                                       *
 *****************************************************************************************/

.text
.globl ontop_fcontext
.align 2
.type ontop_fcontext,@function
ontop_fcontext:
    pushl  %ebp  /* save EBP */
    pushl  %ebx  /* save EBX */
    pushl  %esi  /* save ESI */
    pushl  %edi  /* save EDI */

    /* store fcontext_t in ECX */
    movl  %esp, %ecx

    /* first arg of ontop_fcontext() == fcontext to jump to */
    movl  0x18(%esp), %eax

    /* pass parent fcontext_t */
    movl  %ecx, 0x18(%eax)

    /* second arg of ontop_fcontext() == data to be transferred */
    movl  0x1c(%esp), %ecx

    /* pass data */
    movl %ecx, 0x1c(%eax)

    /* third arg of ontop_fcontext() == ontop-function */
    movl  0x20(%esp), %ecx

    /* restore ESP (pointing to context-data) from EDX */
    movl  %eax, %esp

    popl  %edi  /* restore EDI */
    popl  %esi  /* restore ESI */
    popl  %ebx  /* restore EBX */
    popl  %ebp  /* restore EBP */

    /* jump to context */
    jmp *%ecx
.size ontop_fcontext,.-ontop_fcontext

/* Mark that we don't need executable stack.  */
.section .note.GNU-stack,"",%progbits
