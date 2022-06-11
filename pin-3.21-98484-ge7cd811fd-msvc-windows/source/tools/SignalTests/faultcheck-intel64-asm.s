/*
 * Copyright (C) 2009-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * void DoUnmappedRead(void *addr)
 */
.text
    .align 4
.globl DoUnmappedRead
.globl PinLab_UnmappedRead
DoUnmappedRead:
PinLab_UnmappedRead:
    mov     (%rdi), %rax
    ret

/*
 * void DoUnmappedWrite(void *addr)
 */
.text
    .align 4
.globl DoUnmappedWrite
.globl PinLab_UnmappedWrite
DoUnmappedWrite:
PinLab_UnmappedWrite:
    mov     %rax, (%rdi)
    ret

/*
 * void DoInaccessibleRead(void *addr)
 */
.text
    .align 4
.globl DoInaccessibleRead
.globl PinLab_InaccessibleRead
DoInaccessibleRead:
PinLab_InaccessibleRead:
    mov     (%rdi), %rax
    ret

/*
 * void DoInaccessibleWrite(void *addr)
 */
.text
    .align 4
.globl DoInaccessibleWrite
.globl PinLab_InaccessibleWrite
DoInaccessibleWrite:
PinLab_InaccessibleWrite:
    mov     %rax, (%rdi)
    ret

/*
 * void DoMisalignedRead(void *addr)
 */
.text
    .align 4
.globl DoMisalignedRead
.globl PinLab_MisalignedRead
DoMisalignedRead:
    /*
     * Set the AC flag so that the misaligned read raises an exception.
     */
    pushfq
    pop     %rax
    mov     %rax, %rcx
    or      $0x40000, %rax
    push    %rax
    popfq

PinLab_MisalignedRead:
    mov     (%rdi), %rax

    /*
     * Restore the AC flag.  If the exception happens (which is expected),
     * this code won't execute.  However the siglongjmp() will restore the
     * flags.
     */
    push    %rcx
    popfq
    ret

/*
 * void DoMisalignedWrite(void *addr)
 */
.text
    .align 4
.globl DoMisalignedWrite
.globl PinLab_MisalignedWrite
DoMisalignedWrite:
    /*
     * Set the AC flag so that the misaligned write raises an exception.
     */
    pushfq
    pop     %rax
    mov     %rax,  %rcx
    or      $0x40000, %rax
    push    %rax
    popfq

PinLab_MisalignedWrite:
    mov     %rax, (%rdi)

    /*
     * Restore the AC flag.  If the exception happens (which is expected),
     * this code won't execute.  However the siglongjmp() will restore the
     * flags.
     */
    push    %rcx
    popfq
    ret

/*
 * void DoUD2()
 */
.text
    .align 4
.globl DoUD2
.globl PinLab_IllegalInstruction
DoUD2:
PinLab_IllegalInstruction:
    ud2
    ret

/*
 * void DoPrivilegedInstruction()
 */
.text
    .align 4
.globl DoPrivilegedInstruction
.globl PinLab_PrivilegedInstruction
DoPrivilegedInstruction:
PinLab_PrivilegedInstruction:
    mov     %rax, %cr0
    ret

/*
 * void DoIntegerDivideByZero()
 */
.text
    .align 4
.globl DoIntegerDivideByZero
.globl PinLab_IntegerDivideByZero
DoIntegerDivideByZero:
    mov     $0x0, %rax
PinLab_IntegerDivideByZero:
    idiv    %rax
    ret

/*
 * void DoIntegerOverflowTrap()
 */
.text
    .align 4
.globl DoIntegerOverflowTrap
.globl PinLab_IntegerOverflowTrap
DoIntegerOverflowTrap:
PinLab_IntegerOverflowTrap:
    int     $4
    ret

/*
 * void DoX87DivideByZero()
 */
.text
    .align 4
.globl DoX87DivideByZero
.globl PinLab_X87DivideByZero
DoX87DivideByZero:
    subq    $32, %rsp
    finit                   /* initialize x87 */
    fstenv  0(%rsp)         /* store x87 state */
    btsw    $2, 4(%rsp)     /* set ZE bit (zero divide exception) */
    btrw    $2, 0(%rsp)     /* clear ZM bit (zero divide mask) */
    fldenv  0(%rsp)         /* restore x87 state */
PinLab_X87DivideByZero:
    fwait                   /* this raises the exception */
    addq    $32, %rsp
    ret

/*
 * void DoX87Overflow()
 */
.text
    .align 4
.globl DoX87Overflow
.globl PinLab_X87Overflow
DoX87Overflow:
    subq    $32, %rsp
    finit                   /* initialize x87 */
    fstenv  0(%rsp)         /* store x87 state */
    btsw    $3, 4(%rsp)     /* set OE bit (overflow exception) */
    btrw    $3, 0(%rsp)     /* clear OM bit (overflow mask) */
    fldenv  0(%rsp)         /* restore x87 state */
PinLab_X87Overflow:
    fwait                   /* this raises the exception */
    addq    $32, %rsp
    ret

/*
 * void DoX87Underflow()
 */
.text
    .align 4
.globl DoX87Underflow
.globl PinLab_X87Underflow
DoX87Underflow:
    subq    $32, %rsp
    finit                   /* initialize x87 */
    fstenv  0(%rsp)         /* store x87 state */
    btsw    $4, 4(%rsp)     /* set UE bit (underflow exception) */
    btrw    $4, 0(%rsp)     /* clear UM bit (underflow mask) */
    fldenv  0(%rsp)         /* restore x87 state */
PinLab_X87Underflow:
    fwait                   /* this raises the exception */
    addq    $32, %rsp
    ret

/*
 * void DoX87Precision()
 */
.text
    .align 4
.globl DoX87Precision
.globl PinLab_X87Precision
DoX87Precision:
    subq    $32, %rsp
    finit                   /* initialize x87 */
    fstenv  0(%rsp)         /* store x87 state */
    btsw    $5, 4(%rsp)     /* set PE bit (precision exception) */
    btrw    $5, 0(%rsp)     /* clear PM bit (precision mask) */
    fldenv  0(%rsp)         /* restore x87 state */
PinLab_X87Precision:
    fwait                   /* this raises the exception */
    addq    $32, %rsp
    ret

/*
 * void DoX87InvalidOperation()
 */
.text
    .align 4
.globl DoX87InvalidOperation
.globl PinLab_X87InvalidOperation
DoX87InvalidOperation:
    subq    $32, %rsp
    finit                   /* initialize x87 */
    fstenv  0(%rsp)         /* store x87 state */
    btsw    $0, 4(%rsp)     /* set IE bit (invalid operation exception) */
    btrw    $0, 0(%rsp)     /* clear IM bit (invalid operation mask) */
    fldenv  0(%rsp)         /* restore x87 state */
PinLab_X87InvalidOperation:
    fwait                   /* this raises the exception */
    addq    $32, %rsp
    ret

/*
 * void DoX87DenormalizedOperand()
 */
.text
    .align 4
.globl DoX87DenormalizedOperand
.globl PinLab_X87DenormalizedOperand
DoX87DenormalizedOperand:
    subq    $32, %rsp
    finit                   /* initialize x87 */
    fstenv  0(%rsp)         /* store x87 state */
    btsw    $1, 4(%rsp)     /* set DE bit (denormalized operand exception) */
    btrw    $1, 0(%rsp)     /* clear DM bit (denormalized operand mask) */
    fldenv  0(%rsp)         /* restore x87 state */
PinLab_X87DenormalizedOperand:
    fwait                   /* this raises the exception */
    addq    $32, %rsp
    ret

/*
 * void DoX87StackUnderflow()
 */
.text
    .align 4
.globl DoX87StackUnderflow
.globl PinLab_X87StackUnderflow
DoX87StackUnderflow:
    subq    $8, %rsp
    finit                   /* initialize x87 */
    fnstcw  0(%rsp)         /* get current FP control word */
    btrw    $0, 0(%rsp)     /* enable "invalid operation" exceptions */
    fldcw   0(%rsp)         /* store new FP control word */
    faddp
PinLab_X87StackUnderflow:
    fwait                   /* this raises the exception */
    addq    $8, %rsp
    ret

/*
 * void DoX87StackOverflow()
 */
.text
    .align 4
.globl DoX87StackOverflow
.globl PinLab_X87StackOverflow
DoX87StackOverflow:
    subq    $8, %rsp
    finit                   /* initialize x87 */
    fnstcw  0(%rsp)         /* get current FP control word */
    btrw    $0, 0(%rsp)     /* enable "invalid operation" exceptions */
    fldcw   0(%rsp)         /* store new FP control word */
    fld1
    fld1
    fld1
    fld1
    fld1
    fld1
    fld1
    fld1
    fld1                    /* this causes a stack overflow */
PinLab_X87StackOverflow:
    fwait                   /* this raises the exception */
    addq    $8, %rsp
    ret

/*
 * void DoX87MultipleExceptions()
 */
.text
    .align 4
.globl DoX87MultipleExceptions
.globl PinLab_X87MultipleExceptions
DoX87MultipleExceptions:
    subq    $32, %rsp
    finit                   /* initialize x87 */
    fstenv  0(%rsp)         /* store x87 state */
    btsw    $0, 4(%rsp)     /* set IE bit (invalid operation exception) */
    btsw    $1, 4(%rsp)     /* set DE bit (denormalized operand exception) */
    btsw    $2, 4(%rsp)     /* set ZE bit (zero divide exception) */
    btsw    $3, 4(%rsp)     /* set OE bit (overflow exception) */
    btsw    $4, 4(%rsp)     /* set UE bit (underflow exception) */
    btsw    $5, 4(%rsp)     /* set PE bit (precision exception) */
    btrw    $0, 0(%rsp)     /* clear IM bit (invalid operation mask) */
    btrw    $1, 0(%rsp)     /* clear DM bit (denormalized operand mask) */
    btrw    $2, 0(%rsp)     /* clear ZM bit (zero divide mask) */
    btrw    $3, 0(%rsp)     /* clear OM bit (overflow mask) */
    btrw    $4, 0(%rsp)     /* clear UM bit (underflow mask) */
    btrw    $5, 0(%rsp)     /* clear PM bit (precision mask) */
    fldenv  0(%rsp)         /* restore x87 state */
PinLab_X87MultipleExceptions:
    fwait                   /* this raises all the exceptions */
    addq    $32, %rsp
    ret

/*
 * void DoSIMDDivideByZero()
 */
.text
    .align 4
.globl DoSIMDDivideByZero
.globl PinLab_SIMDDivideByZero
DoSIMDDivideByZero:
    subq        $8, %rsp
    stmxcsr     0(%rsp)
    btrw        $9, 0(%rsp)     /* enable "zero divide" exceptions */
    ldmxcsr     0(%rsp)
    movl        $0, %eax
    cvtsi2ss    %eax, %xmm0     /* %xmm0 = 0.0 */
    movl        $1, %eax
    cvtsi2ss    %eax, %xmm1     /* %xmm1 = 1.0 */
PinLab_SIMDDivideByZero:
    divss       %xmm0, %xmm1    /* raise "zero divide" exception */
    addq        $8, %rsp
    ret

/*
 * void DoSIMDOverflow()
 */
.text
    .align 4
.globl DoSIMDOverflow
.globl PinLab_SIMDOverflow
DoSIMDOverflow:
    subq        $8, %rsp
    stmxcsr     0(%rsp)
    btrw        $10, 0(%rsp)            /* enable "overflow" exceptions */
    ldmxcsr     0(%rsp)
    movl        $0x7f7fffff, 0(%rsp)    /* largest 32-bit float value */
    movl        $0x7f7fffff, 4(%rsp)
    movq        0(%rsp), %xmm0
    movq        0(%rsp), %xmm1
PinLab_SIMDOverflow:
    mulss       %xmm0, %xmm1            /* raise "overflow" exception */
    addq        $8, %rsp
    ret

/*
 * void DoSIMDUnderflow()
 */
.text
    .align 4
.globl DoSIMDUnderflow
.globl PinLab_SIMDUnderflow
DoSIMDUnderflow:
    subq        $8, %rsp
    stmxcsr     0(%rsp)
    btrw        $11, 0(%rsp)            /* enable "underflow" exceptions */
    ldmxcsr     0(%rsp)
    movl        $0x00800001, 0(%rsp)    /* smallest 32-bit float value */
    movl        $0x00800001, 4(%rsp)
    movq        0(%rsp), %xmm0
    movq        0(%rsp), %xmm1
PinLab_SIMDUnderflow:
    mulss       %xmm0, %xmm1            /* raise "underflow" exception */
    addq        $8, %rsp
    ret

/*
 * void DoSIMDPrecision()
 */
.text
    .align 4
.globl DoSIMDPrecision
.globl PinLab_SIMDPrecision
DoSIMDPrecision:
    subq        $8, %rsp
    stmxcsr     0(%rsp)
    btrw        $12, 0(%rsp)    /* enable "precision" exceptions */
    ldmxcsr     0(%rsp)
    movl        $1, %eax
    cvtsi2ss    %eax, %xmm0     /* %xmm0 = 1.0 */
    movl        $2, %eax
    cvtsi2ss    %eax, %xmm1     /* %xmm1 = 2.0 */
    divss       %xmm1, %xmm0    /* %xmm0 = 0.5 */
PinLab_SIMDPrecision:
    cvtss2si    %xmm0, %eax     /* raise "precision" exception */
    addq        $8, %rsp
    ret

/*
 * void DoSIMDInvalidOperation()
 */
.text
    .align 4
.globl DoSIMDInvalidOperation
.globl PinLab_SIMDInvalidOperation
DoSIMDInvalidOperation:
    subq        $8, %rsp
    stmxcsr     0(%rsp)
    btrw        $7, 0(%rsp)     /* enable "invalid operation" exceptions */
    ldmxcsr     0(%rsp)
    movl        $-1, %eax
    cvtsi2ss    %eax, %xmm0     /* %xmm0 = -1.0 */
PinLab_SIMDInvalidOperation:
    sqrtss      %xmm0, %xmm0    /* raise "invalid operation" exception */
    addq        $8, %rsp
    ret

/*
 * void DoSIMDDenormalizedOperand()
 */
.text
    .align 4
.globl DoSIMDDenormalizedOperand
.globl PinLab_SIMDDenormalizedOperand
DoSIMDDenormalizedOperand:
    subq        $8, %rsp
    stmxcsr     0(%rsp)
    btrw        $6, 0(%rsp)             /* disable "denormals-are-zero" flag */
    btrw        $8, 0(%rsp)             /* enable "denormalized operand" exceptions */
    ldmxcsr     0(%rsp)
    movl        $0x00400000, 0(%rsp)    /* denormal 32-bit float */
    movl        $0x00400000, 4(%rsp)    /* denormal 32-bit float */
    movq        0(%rsp), %xmm0
    movq        0(%rsp), %xmm1
PinLab_SIMDDenormalizedOperand:
    addss       %xmm0, %xmm0            /* raise "denormalized operand" exception */
    addq        $8, %rsp
    ret

/*
 * void DoSIMDMultipleExceptions()
 */
.text
    .align 4
.globl DoSIMDMultipleExceptions
.globl PinLab_SIMDMultipleExceptions
DoSIMDMultipleExceptions:
    subq        $8, %rsp
    stmxcsr     0(%rsp)
    btrw        $11, 0(%rsp)            /* enable "underflow" exceptions */
    btrw        $12, 0(%rsp)            /* enable "precision" exceptions */
    ldmxcsr     0(%rsp)
    movl        $0x00800001, 0(%rsp)    /* smallest 32-bit float value */
    movl        $0x00800001, 4(%rsp)
    movq        0(%rsp), %xmm0
    movq        0(%rsp), %xmm1
PinLab_SIMDMultipleExceptions:
    mulss       %xmm0, %xmm1            /* raise "underflow" and "precision" exception */
    addq        $8, %rsp
    ret

/*
 * void DoBreakpointTrap()
 */
.text
    .align 4
.globl DoBreakpointTrap
.globl PinLab_BreakpointTrap
DoBreakpointTrap:
PinLab_BreakpointTrap:
    .byte 0xcc      /* INT3 instruction */
    ret

/*
 * void DoSingleStepTrap()
 */
.text
    .align 4
.globl DoSingleStepTrap
DoSingleStepTrap:
    /*
     * Set the TF (trap) flag.
     */
    pushfq
    pop     %rax
    mov     %rax, %rcx
    or      $0x100, %rax
    push    %rax
    popfq

    add     %rax, %rax      /* raises single-step trap fault */

    /*
     * Restore the TF flag.  If the exception happens (which is expected),
     * this code won't execute.  However the siglongjmp() will restore the
     * flags.
     */
    push    %rcx
    popfq
    ret

/*
 * void DoBadRegister()
 */
.text
    .align 4
.globl DoBadRegister
DoBadRegister:
    .byte 0x0f, 0x22, 0xff  /* move into %cr7.  The register %cr7 is invalid here */
    ret

/*
 * void ClearAC()
 */
.text
    .align 4
.globl ClearAC
ClearAC:
    pushfq
    btrl    $18, 0(%rsp)    /* clears the AC bit */
    popfq
    ret


/*
 * void DoIllegalSetOfSegReg1()
 */
 .globl DoIllegalSetOfSegReg1
 DoIllegalSetOfSegReg1:
    mov $5, %rax
    mov %rax, %gs
    ret

/*
 * void DoIllegalSetOfSegReg2()
 */
 .globl DoIllegalSetOfSegReg2
 DoIllegalSetOfSegReg2:
    mov $0, %rdx
    lgs (%rdx), %rax
    ret

/*
 * void DoIllegalGetOfSegReg1()
 */
 .globl DoIllegalGetOfSegReg1
 DoIllegalGetOfSegReg1:
    mov $0, %rdx
    mov %gs, (%rdx)
    ret

#if defined(TARGET_MAC)
#define EXPORT(N) \
.set _##N, N ; \
.globl _##N

EXPORT(DoUnmappedRead)
EXPORT(PinLab_UnmappedRead)
EXPORT(DoUnmappedWrite)
EXPORT(PinLab_UnmappedWrite)
EXPORT(DoInaccessibleRead)
EXPORT(PinLab_InaccessibleRead)
EXPORT(DoInaccessibleWrite)
EXPORT(PinLab_InaccessibleWrite)
EXPORT(DoMisalignedRead)
EXPORT(PinLab_MisalignedRead)
EXPORT(DoMisalignedWrite)
EXPORT(PinLab_MisalignedWrite)
EXPORT(DoUD2)
EXPORT(PinLab_IllegalInstruction)
EXPORT(DoPrivilegedInstruction)
EXPORT(PinLab_PrivilegedInstruction)
EXPORT(DoIntegerDivideByZero)
EXPORT(PinLab_IntegerDivideByZero)
EXPORT(DoIntegerOverflowTrap)
EXPORT(PinLab_IntegerOverflowTrap)
EXPORT(DoX87DivideByZero)
EXPORT(PinLab_X87DivideByZero)
EXPORT(DoX87Overflow)
EXPORT(PinLab_X87Overflow)
EXPORT(PinLab_X87Underflow)
EXPORT(PinLab_X87Precision)
EXPORT(PinLab_X87InvalidOperation)
EXPORT(PinLab_X87DenormalizedOperand)
EXPORT(PinLab_X87StackUnderflow)
EXPORT(PinLab_X87StackOverflow)
EXPORT(PinLab_X87MultipleExceptions)
EXPORT(PinLab_SIMDDivideByZero)
EXPORT(PinLab_SIMDOverflow)
EXPORT(PinLab_SIMDUnderflow)
EXPORT(PinLab_SIMDPrecision)
EXPORT(PinLab_SIMDInvalidOperation)
EXPORT(PinLab_SIMDDenormalizedOperand)
EXPORT(PinLab_SIMDMultipleExceptions)
EXPORT(PinLab_BreakpointTrap)
EXPORT(DoX87Underflow)
EXPORT(DoX87Precision)
EXPORT(DoX87InvalidOperation)
EXPORT(DoX87DenormalizedOperand)
EXPORT(DoX87StackUnderflow)
EXPORT(DoX87StackOverflow)
EXPORT(DoX87MultipleExceptions)
EXPORT(DoSIMDDivideByZero)
EXPORT(DoSIMDOverflow)
EXPORT(DoSIMDUnderflow)
EXPORT(DoSIMDPrecision)
EXPORT(DoSIMDInvalidOperation)
EXPORT(DoSIMDDenormalizedOperand)
EXPORT(DoSIMDMultipleExceptions)
EXPORT(DoBreakpointTrap)
EXPORT(DoBadRegister)
EXPORT(DoIllegalSetOfSegReg1)
EXPORT(DoIllegalSetOfSegReg2)
EXPORT(DoIllegalGetOfSegReg1)
EXPORT(ClearAC)

#endif
