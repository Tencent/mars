
;           Copyright Oliver Kowalke 2009.
;  Distributed under the Boost Software License, Version 1.0.
;     (See accompanying file LICENSE_1_0.txt or copy at
;           http://www.boost.org/LICENSE_1_0.txt)

;  ---------------------------------------------------------------------------------
;  |    0    |    1    |    2    |    3    |    4    |    5    |    6    |    7    |
;  ---------------------------------------------------------------------------------
;  |    0h   |   04h   |   08h   |   0ch   |   010h  |   014h  |   018h  |   01ch  |
;  ---------------------------------------------------------------------------------
;  | fc_strg |fc_deallo|  limit  |   base  |  fc_seh |   EDI   |   ESI   |   EBX   |
;  ---------------------------------------------------------------------------------
;  ---------------------------------------------------------------------------------
;  |    8    |    9    |   10    |    11   |    12   |    13   |    14   |    15   |
;  ---------------------------------------------------------------------------------
;  |   020h  |  024h   |  028h   |   02ch  |   030h  |   034h  |   038h  |   03ch  |
;  ---------------------------------------------------------------------------------
;  |   EBP   |   EIP   |    to   |   data  |         |  EH NXT |SEH HNDLR|         |
;  ---------------------------------------------------------------------------------

.386
.XMM
.model flat, c
.code

jump_fcontext PROC BOOST_CONTEXT_EXPORT
    push  ebp  ; save EBP 
    push  ebx  ; save EBX 
    push  esi  ; save ESI 
    push  edi  ; save EDI 

    assume  fs:nothing
    ; load NT_TIB into ECX
    mov  edx, fs:[018h]
    assume  fs:error

    ; load current SEH exception list
    mov  eax, [edx]
    push  eax

    ; load current stack base
    mov  eax, [edx+04h]
    push  eax

    ; load current stack limit
    mov  eax, [edx+08h]
    push  eax

    ; load current deallocation stack
    mov  eax, [edx+0e0ch]
    push  eax

    ; load fiber local storage
    mov  eax, [edx+010h]
    push  eax

    ; store ESP (pointing to context-data) in EAX
    mov  eax, esp

    ; firstarg of jump_fcontext() == fcontext to jump to
    mov  ecx, [esp+028h]
    
    ; restore ESP (pointing to context-data) from EAX
    mov  esp, ecx

    assume  fs:nothing
    ; load NT_TIB into EDX
    mov  edx, fs:[018h]
    assume  fs:error

    ; restore fiber local storage
    pop  ecx
    mov  [edx+010h], ecx

    ; restore current deallocation stack
    pop  ecx
    mov  [edx+0e0ch], ecx

    ; restore current stack limit
    pop  ecx
    mov  [edx+08h], ecx

    ; restore current stack base
    pop  ecx
    mov  [edx+04h], ecx

    ; restore current SEH exception list
    pop  ecx
    mov  [edx], ecx

    pop  edi  ; save EDI 
    pop  esi  ; save ESI 
    pop  ebx  ; save EBX 
    pop  ebp  ; save EBP

    ; return transfer_t
    ; FCTX == EAX, DATA == EDX
    mov  edx, [eax+02ch]

    ; jump to context
    ret
jump_fcontext ENDP
END
