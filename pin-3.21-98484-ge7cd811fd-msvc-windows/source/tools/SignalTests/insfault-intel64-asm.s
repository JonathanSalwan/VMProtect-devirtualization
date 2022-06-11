/*
 * Copyright (C) 2011-2017 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

# Each test sets these to register values at the time of the fault.
#
.comm   ExpectedPC,8,8
.comm   ExpectedR8,8,8
.comm   ExpectedR9,8,8
.comm   ExpectedR10,8,8
.comm   ExpectedR11,8,8
.comm   ExpectedR12,8,8
.comm   ExpectedR13,8,8
.comm   ExpectedR14,8,8
.comm   ExpectedR15,8,8
.comm   ExpectedRDI,8,8
.comm   ExpectedRSI,8,8
.comm   ExpectedRBP,8,8
.comm   ExpectedRBX,8,8
.comm   ExpectedRDX,8,8
.comm   ExpectedRAX,8,8
.comm   ExpectedRCX,8,8
.comm   ExpectedRSP,8,8
.comm   ExpectedEFLAGS,8,8

# Some scratch memory used by the tests below.
#
.comm   Scratch,4096,8


.text

# SetRegisters()
#
#   Set the integer registers and their associated 'Expected*' variables
#   to weird values.
#
    .align 4
.globl SetRegisters
SetRegisters:
    # Set flags to:
    #
    #   AC=0, OF=1, DF=1, TF=0, SF=0, ZF=1, AF=1, PF=0, CF=1
    #
    movq    $0xc51, %rax
    pushq   %rax
    popfq
    movq    $0xc51, ExpectedEFLAGS(%rip)

    movq    $1, %r8
    movq    $2, %r9
    movq    $3, %r10
    movq    $4, %r11
    movq    $5, %r12
    movq    $6, %r13
    movq    $7, %r14
    movq    $8, %r15
    movq    $9, %rdi
    movq    $10, %rsi
    movq    $11, %rbp
    movq    $12, %rbx
    movq    $13, %rdx
    movq    $14, %rax
    movq    $15, %rcx

    movq    %r8, ExpectedR8(%rip)
    movq    %r9, ExpectedR9(%rip)
    movq    %r10, ExpectedR10(%rip)
    movq    %r11, ExpectedR11(%rip)
    movq    %r12, ExpectedR12(%rip)
    movq    %r13, ExpectedR13(%rip)
    movq    %r14, ExpectedR14(%rip)
    movq    %r15, ExpectedR15(%rip)
    movq    %rdi, ExpectedRDI(%rip)
    movq    %rsi, ExpectedRSI(%rip)
    movq    %rbp, ExpectedRBP(%rip)
    movq    %rbx, ExpectedRBX(%rip)
    movq    %rdx, ExpectedRDX(%rip)
    movq    %rax, ExpectedRAX(%rip)
    movq    %rcx, ExpectedRCX(%rip)
    ret


# DoFaultRetSp()
#
#   Cause RET to fault by making $SP invalid.
#
    .align 4
.globl DoFaultRetSp
DoFaultRetSp:
    call    SetRegisters
    push    %rcx
    lea     DoFaultRetSpLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    pop     %rcx
    movq    $0, ExpectedRSP(%rip)
    movq    $0, %rsp
DoFaultRetSpLab:
    ret


# DoFaultRetTarg()
#
#   Cause RET to fault due to invalid target.
#
    .align 4
.globl DoFaultRetTarg
DoFaultRetTarg:
    call    SetRegisters
    movq    $0, ExpectedPC(%rip)
    mov     %rsp, ExpectedRSP(%rip)
    pushq   $0
    ret


# DoFaultRetImmSp()
#
#   Cause "RET <imm>" to fault by making $SP invalid.
#
    .align 4
.globl DoFaultRetImmSp
DoFaultRetImmSp:
    call    SetRegisters
    push    %rcx
    lea     DoFaultRetImmSpLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    pop     %rcx
    movq    $0, ExpectedRSP(%rip)
    movq    $0, %rsp
DoFaultRetImmSpLab:
    ret     $0x40


# DoFaultRetImmTarg()
#
#   Cause "RET <imm>" to fault due to invalid target.
#
    .align 4
.globl DoFaultRetImmTarg
DoFaultRetImmTarg:
    movq    $0, ExpectedPC(%rip)
#   Save the stack pointer in rax
    mov     %rsp, %rax
    addq    $0x40, %rax
    mov     %rax, ExpectedRSP(%rip)
    call    SetRegisters
    pushq   $0
    ret     $0x40


# DoFaultCallSp()
#
#   Cause direct CALL to fault because $SP is invalid.
#
    .align 4
.globl DoFaultCallSp
DoFaultCallSp:
    call    SetRegisters
    push    %rcx
    lea     DoFaultCallSpLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    pop     %rcx
    movq    $0, ExpectedRSP(%rip)
    movq    $0, %rsp
DoFaultCallSpLab:
    call    DoFaultCallSp


# DoFaultCallTarg()
#
#   Cause direct CALL to fault because target is invalid.
#
    .align 4
.globl DoFaultCallTarg
DoFaultCallTarg:
    push    %rcx
    lea     Unmapped(%rip), %rcx
    mov     %rcx, ExpectedPC(%rip)
    pop     %rcx
#   Save the stack pointer in rax
    mov     %rsp, %rax
    subq    $8, %rax
    mov     %rax, ExpectedRSP(%rip)
    call    SetRegisters
    call    Unmapped


# DoFaultCallRegSp()
#
#   Cause "CALL *%rx" to fault because $SP is invalid.
#
    .align 4
.globl DoFaultCallRegSp
DoFaultCallRegSp:
    call    SetRegisters
    push    %rcx
    lea     DoFaultCallRegSpLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    pop     %rcx
    lea     DoFaultCallRegSp(%rip), %rdi
    movq    %rdi, ExpectedRDI(%rip)
    movq    $0, ExpectedRSP(%rip)
    movq    $0, %rsp
    lea     DoFaultCallRegSp(%rip), %rdi
DoFaultCallRegSpLab:
    call    *%rdi


# DoFaultCallRegTarg()
#
#   Cause "CALL *%rx" to fault because target is invalid.
#
    .align 4
.globl DoFaultCallRegTarg
DoFaultCallRegTarg:
    push    %rcx
    lea     Unmapped(%rip), %rcx
    mov     %rcx, ExpectedPC(%rip)
    pop     %rcx
#   Save the stack pointer in rax
    mov     %rsp, %rax
    subq    $8, %rax
    mov     %rax, ExpectedRSP(%rip)
    call    SetRegisters
    lea     Unmapped(%rip), %rdi
    mov     %rdi, ExpectedRDI(%rip)
    call    *%rdi


# DoFaultCallMemSp()
#
#   Cause "CALL *[mem]" to fault because $SP is invalid.
#
    .align 4
.globl DoFaultCallMemSp
DoFaultCallMemSp:
    call    SetRegisters
    push    %rcx
    lea     DoFaultCallMemSpLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    lea     DoFaultCallMemSp(%rip), %rcx
    movq    %rcx, Scratch(%rip)
    pop     %rcx
    movq    $0, ExpectedRSP(%rip)
    movq    $0, %rsp
DoFaultCallMemSpLab:
    call    *Scratch(%rip)


# DoFaultCallMemTarg()
#
#   Cause "CALL *[mem]" to fault because target is invalid.
#
    .align 4
.globl DoFaultCallMemTarg
DoFaultCallMemTarg:
    lea     Unmapped(%rip), %rax
    mov     %rax, ExpectedPC(%rip)
#   Save the stack pointer in rax
    mov     %rsp, %rax
    subq    $8, %rax
    mov     %rax, ExpectedRSP(%rip)
    call    SetRegisters
    push    %rcx
    lea     Unmapped(%rip), %rcx
    movq    %rcx, Scratch(%rip)
    pop     %rcx
    call    *Scratch(%rip)


# DoFaultCallMemBadMem()
#
#   Cause "CALL *[mem]" to fault because [mem] is invalid.
#
    .align 4
.globl DoFaultCallMemBadMem
DoFaultCallMemBadMem:
    push    %rcx
    lea     DoFaultCallMemBadMemLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    pop     %rcx
    movq    %rsp, ExpectedRSP(%rip)
    call    SetRegisters
DoFaultCallMemBadMemLab:
    call    *Unmapped(%rip)


# DoFaultSegMov()
#
#   Cause MOV to fault because of illegal segment selector.
#
    .align 4
.globl DoFaultSegMov
DoFaultSegMov:
    movq    $0, %rax
    mov     %gs, %ax
    call    SetRegisters
    push    %rcx
    lea     DoFaultSegMovLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    pop     %rcx
    movq    %rsp, ExpectedRSP(%rip)
DoFaultSegMovLab:
    mov     %gs:0, %rdi
    ret


# DoFaultStringOp()
#
#   Cause MOVS to fault.
#
    .align 4
.globl DoFaultStringOp
DoFaultStringOp:
    call    SetRegisters
    lea     DoFaultStringOpLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    movq    $5, ExpectedRCX(%rip)
    lea     Unmapped(%rip), %rcx
    mov     %rcx, ExpectedRSI(%rip)
    lea     Scratch+5*4(%rip), %rcx
    mov     %rcx, ExpectedRDI(%rip)
    movq    %rsp, ExpectedRSP(%rip)
    movq    $0x851, ExpectedEFLAGS(%rip)   # Expect Flags to have DF=0
    movq    $10, %rcx
    lea     Unmapped-5*4(%rip), %rsi
    lea     Scratch(%rip), %rdi
    cld
DoFaultStringOpLab:
    rep movsd
    ret


# DoFaultPushF()
#
#   Cause PUSHF to fault by making $SP invalid.
#
    .align 4
.globl DoFaultPushF
DoFaultPushF:
    call    SetRegisters
    push    %rcx
    lea     DoFaultPushFLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    pop     %rcx
    movq    $0, ExpectedRSP(%rip)
    movq    $0, %rsp
DoFaultPushFLab:
    pushf


# DoFaultPopF()
#
#   Cause POPF to fault by making $SP invalid.
#
    .align 4
.globl DoFaultPopF
DoFaultPopF:
    call    SetRegisters
    push    %rcx
    lea     DoFaultPopFLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    pop     %rcx
    movq    $0, ExpectedRSP(%rip)
    movq    $0, %rsp
DoFaultPopFLab:
    popf


# DoFaultPush()
#
#   Cause PUSH to fault by making $SP invalid.
#
    .align 4
.globl DoFaultPush
DoFaultPush:
    call    SetRegisters
    push    %rcx
    lea     DoFaultPushLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    pop     %rcx
    movq    $0, ExpectedRSP(%rip)
    movq    $0, %rsp
DoFaultPushLab:
    pushq   %rdi


# DoFaultPop()
#
#   Cause POP to fault by making $SP invalid.
#
    .align 4
.globl DoFaultPop
DoFaultPop:
    call    SetRegisters
    push    %rcx
    lea     DoFaultPopLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    pop     %rcx
    movq    $0, ExpectedRSP(%rip)
    movq    $0, %rsp
DoFaultPopLab:
    popq    %rdi


# DoFaultPushMem()
#
#   Cause "PUSH *[mem]" to fault because [mem] is invalid.
#
    .align 4
.globl DoFaultPushMem
DoFaultPushMem:
    call    SetRegisters
    push    %rcx
    lea     DoFaultPushMemLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    pop     %rcx
    movq    %rsp, ExpectedRSP(%rip)
DoFaultPushMemLab:
    push    Unmapped(%rip)


# DoFaultPopMem()
#
#   Cause "POP *[mem]" to fault because [mem] is invalid.
#
    .align 4
.globl DoFaultPopMem
DoFaultPopMem:
    call    SetRegisters
    push    %rcx
    lea     DoFaultPopMemLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    pop     %rcx
    movq    %rsp, ExpectedRSP(%rip)
DoFaultPopMemLab:
    popq    Unmapped(%rip)


# DoFaultEnter()
#
#   Cause ENTER to fault by making $SP invalid.
#
    .align 4
.globl DoFaultEnter
DoFaultEnter:
    call    SetRegisters
    push    %rcx
    lea     DoFaultEnterLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    pop     %rcx
    movq    $0, ExpectedRSP(%rip)
    movq    $0, %rsp
DoFaultEnterLab:
    enterq  $0, $0


# DoFaultLeave()
#
#   Cause LEAVE to fault by making $SP invalid.
#
    .align 4
.globl DoFaultLeave
DoFaultLeave:
    call    SetRegisters
    push    %rcx
    lea     DoFaultLeaveLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    pop     %rcx
    movq    $0, ExpectedRSP(%rip)
    movq    $0, %rsp
DoFaultLeaveLab:
    leave


# DoFaultMaskmovdqu()
#
#   Cause MASKMOVDQU to fault.
#
    .align 4
.globl DoFaultMaskmovdqu
DoFaultMaskmovdqu:
    subq    $16, %rsp
    call    SetRegisters
    movl    $0xffffffff, 0(%rsp)
    movl    $0xffffffff, 4(%rsp)
    movl    $0xffffffff, 8(%rsp)
    movl    $0xffffffff, 12(%rsp)
    movq    0(%rsp), %xmm0
    movq    0(%rsp), %xmm1
    lea     Unmapped(%rip), %rdi
    push    %rcx
    lea     DoFaultMaskmovdquLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    pop     %rcx
    movq    %rsp, ExpectedRSP(%rip)
    movq    %rdi, ExpectedRDI(%rip)
DoFaultMaskmovdquLab:
    maskmovdqu  %xmm0, %xmm1


# DoFaultBitTest()
#
#   Cause BT to fault.
#
    .align 4
.globl DoFaultBitTest
DoFaultBitTest:
    call    SetRegisters
    push    %rcx
    lea     DoFaultBitTestLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    pop     %rcx
    movq    %rsp, ExpectedRSP(%rip)
    movq    $16, ExpectedRDI(%rip)
    movq    $16, %rdi
    lea     Unmapped(%rip), %rcx
    movq    %rcx, ExpectedRCX(%rip)
DoFaultBitTestLab:
    bt      %rdi, (%rcx)


# DoFaultMovSegSelector()
#
#   Cause "MOV %fs, [mem]" to fault.
#
    .align 4
.globl DoFaultMovSegSelector
DoFaultMovSegSelector:
    call    SetRegisters
    push    %rcx
    lea     DoFaultMovSegSelectorLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    pop     %rcx
    movq    %rsp, ExpectedRSP(%rip)
    lea     Unmapped(%rip), %rcx
    movq    %rcx, ExpectedRCX(%rip)
DoFaultMovSegSelectorLab:
    mov     %fs, (%rcx)


# DoFaultJumpMemBadMem()
#
#   Cause "JMP *[mem]" to fault because [mem] is invalid.
#
    .align 4
.globl DoFaultJumpMemBadMem
DoFaultJumpMemBadMem:
    push    %rcx
    lea     DoFaultJumpMemBadMemLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    pop     %rcx
    movq    %rsp, ExpectedRSP(%rip)
    call    SetRegisters
DoFaultJumpMemBadMemLab:
    jmp     *Unmapped(%rip)


# DoFaultBadLoad()
#
#   Cause "MOV [mem], %rx" to fault.
#
    .align 4
.globl DoFaultBadLoad
DoFaultBadLoad:
    push    %rcx
    lea     DoFaultBadLoadLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    pop     %rcx
    movq    %rsp, ExpectedRSP(%rip)
    call    SetRegisters
DoFaultBadLoadLab:
    mov     Unmapped(%rip), %rdi


# DoFaultBadStore()
#
#   Cause "MOV %rx, [mem]" to fault.
#
    .align 4
.globl DoFaultBadStore
DoFaultBadStore:
    push    %rcx
    lea     DoFaultBadStoreLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    pop     %rcx
    movq    %rsp, ExpectedRSP(%rip)
    call    SetRegisters
DoFaultBadStoreLab:
    mov     %rdi, Unmapped(%rip)


# DoFaultCmov()
#
#   Cause "CMOVcc %rx, [mem]" to fault.
#
    .align 4
.globl DoFaultCmov
DoFaultCmov:
    push    %rcx
    lea     DoFaultCmovLab(%rip), %rcx
    movq    %rcx, ExpectedPC(%rip)
    pop     %rcx
    movq    %rsp, ExpectedRSP(%rip)
    call    SetRegisters
DoFaultCmovLab:
    cmovb   Unmapped(%rip), %rdi  # SetRegisters() sets CF=1, so the condition is true.


# Unmapped()
#
#   This label is on an unmapped page.  (The C++ part of the test program unmaps the page.)
    .align 4096
.globl Unmapped
Unmapped:
    .zero 4096
