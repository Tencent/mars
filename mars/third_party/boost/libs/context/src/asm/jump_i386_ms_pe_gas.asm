/*
            Copyright Oliver Kowalke 2009.
            Copyright Thomas Sailer 2013.
   Distributed under the Boost Software License, Version 1.0.
      (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

/*************************************************************************************
*  --------------------------------------------------------------------------------- *
*  |    0    |    1    |    2    |    3    |    4    |    5    |    6    |    7    | *
*  --------------------------------------------------------------------------------- *
*  |    0h   |   04h   |   08h   |   0ch   |   010h  |   014h  |   018h  |   01ch  | *
*  --------------------------------------------------------------------------------- *
*  | fc_strg |fc_deallo|  limit  |   base  |  fc_seh |   EDI   |   ESI   |   EBX   | *
*  --------------------------------------------------------------------------------- *
*  --------------------------------------------------------------------------------- *
*  |    8    |    9    |   10    |    11   |    12   |    13   |    14   |    15   | *
*  --------------------------------------------------------------------------------- *
*  |   020h  |  024h   |  028h   |   02ch  |   030h  |   034h  |   038h  |   03ch  | *
*  --------------------------------------------------------------------------------- *
*  |   EBP   |   EIP   |    to   |   data  |         |  EH NXT |SEH HNDLR|         | *
*  --------------------------------------------------------------------------------- *
*************************************************************************************/

.file	"jump_i386_ms_pe_gas.asm"
.text
.p2align 4,,15
.globl	_jump_fcontext
.def	_jump_fcontext;	.scl	2;	.type	32;	.endef
_jump_fcontext:
    pushl  %ebp  /* save EBP */
    pushl  %ebx  /* save EBX */
    pushl  %esi  /* save ESI */
    pushl  %edi  /* save EDI */

    /* load NT_TIB */
    movl  %fs:(0x18), %edx

    /* load current SEH exception list */
    movl  (%edx), %eax
    push  %eax

    /* load current stack base */
    movl  0x04(%edx), %eax
    push  %eax

    /* load current stack limit */
    movl  0x08(%edx), %eax
    push  %eax
    
    /* load current dealloction stack */
    movl  0xe0c(%edx), %eax
    push  %eax
    
    /* load fiber local storage */
    movl  0x10(%edx), %eax
    push  %eax

    /* store ESP (pointing to context-data) in EAX */
    movl  %esp, %eax

    /* first arg of jump_fcontext() == fcontext to jump to */
    movl  0x28(%esp), %ecx

    /* restore ESP (pointing to context-data) from EDX */
    movl  %ecx, %esp

    /* load NT_TIB into ECX */
    movl  %fs:(0x18), %edx

    /* restore fiber local storage */
    popl  %ecx
    movl  %ecx, 0x10(%edx)

    /* restore current deallocation stack */
    popl  %ecx
    movl  %ecx, 0xe0c(%edx)

    /* restore current stack limit */
    popl  %ecx
    movl  %ecx, 0x08(%edx)

    /* restore current stack base */
    popl  %ecx
    movl  %ecx, 0x04(%edx)

    /* restore current SEH exception list */
    popl  %ecx
    movl  %ecx, (%edx)

    popl  %edi  /* save EDI */
    popl  %esi  /* save ESI */
    popl  %ebx  /* save EBX */
    popl  %ebp  /* save EBP */

    /* return transfer_t */
    /* FCTX == EAX, DATA == EDX */
    movl  0x2c(%eax), %edx

    /* jump to context */
    ret

.section .drectve
.ascii " -export:\"jump_fcontext\""
