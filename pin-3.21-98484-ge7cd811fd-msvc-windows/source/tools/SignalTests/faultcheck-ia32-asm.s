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
    mov     4(%esp), %ecx
PinLab_UnmappedRead:
    mov     (%ecx), %eax
    ret

/*
 * void DoUnmappedWrite(void *addr)
 */
.text
    .align 4
.globl DoUnmappedWrite
.globl PinLab_UnmappedWrite
DoUnmappedWrite:
    mov     4(%esp), %ecx
PinLab_UnmappedWrite:
    mov     %eax, (%ecx)
    ret

/*
 * void DoInaccessibleRead(void *addr)
 */
.text
    .align 4
.globl DoInaccessibleRead
.globl PinLab_InaccessibleRead
DoInaccessibleRead:
    mov     4(%esp), %ecx
PinLab_InaccessibleRead:
    mov     (%ecx), %eax
    ret

/*
 * void DoInaccessibleWrite(void *addr)
 */
.text
    .align 4
.globl DoInaccessibleWrite
.globl PinLab_InaccessibleWrite
DoInaccessibleWrite:
    mov     4(%esp), %ecx
PinLab_InaccessibleWrite:
    mov     %eax, (%ecx)
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
    pushf
    pop     %eax
    mov     %eax, %ecx
    or      $0x40000, %eax
    push    %eax
    popf

    mov     4(%esp), %edx
PinLab_MisalignedRead:
    mov     (%edx), %eax

    /*
     * Restore the AC flag.  If the exception happens (which is expected),
     * this code won't execute.  However the siglongjmp() will restore the
     * flags.
     */
    push    %ecx
    popf
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
    pushf
    pop     %eax
    mov     %eax,  %ecx
    or      $0x40000, %eax
    push    %eax
    popf

    mov     4(%esp), %edx
PinLab_MisalignedWrite:
    mov     %eax, (%edx)

    /*
     * Restore the AC flag.  If the exception happens (which is expected),
     * this code won't execute.  However the siglongjmp() will restore the
     * flags.
     */
    push    %ecx
    popf
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
    mov     %eax, %cr0
    ret

/*
 * void DoIntegerDivideByZero()
 */
.text
    .align 4
.globl DoIntegerDivideByZero
.globl PinLab_IntegerDivideByZero
DoIntegerDivideByZero:
    mov     $0x0, %eax
PinLab_IntegerDivideByZero:
    idiv    %eax
    ret

/*
 * void DoIntegerOverflowTrap()
 */
.text
    .align 4
.globl DoIntegerOverflowTrap
.globl PinLab_IntegerOverflowTrap
DoIntegerOverflowTrap:
    mov     $0x7f, %al
    add     $1, %al
PinLab_IntegerOverflowTrap:
    into
    ret

#if !defined(TARGET_MAC)
/*
 * void DoBoundTrap()
 */
.text
    .align 4
.globl DoBoundTrap
.globl PinLab_BoundTrap
DoBoundTrap:
    subl    $8, %esp
    movl    $0, 0(%esp)     /* set the "bounds" to (0,0) */
    movl    $0, 4(%esp)
    mov     $0x80, %eax     /* set the "index" to 0x80 */
PinLab_BoundTrap:
    boundl  %eax, 0(%esp)   /* this should raise an exception */
    addl    $8, %esp
    ret
#endif

/*
 * void DoX87DivideByZero()
 */
.text
    .align 4
.globl DoX87DivideByZero
.globl PinLab_X87DivideByZero
DoX87DivideByZero:
    subl    $32, %esp
    finit                   /* initialize x87 */
    fstenv  0(%esp)         /* store x87 state */
    btsw    $2, 4(%esp)     /* set ZE bit (zero divide exception) */
    btrw    $2, 0(%esp)     /* clear ZM bit (zero divide mask) */
    fldenv  0(%esp)         /* restore x87 state */
PinLab_X87DivideByZero:
    fwait                   /* this raises the exception */
    addl    $32, %esp
    ret

/*
 * void DoX87Overflow()
 */
.text
    .align 4
.globl DoX87Overflow
.globl PinLab_X87Overflow
DoX87Overflow:
    subl    $32, %esp
    finit                   /* initialize x87 */
    fstenv  0(%esp)         /* store x87 state */
    btsw    $3, 4(%esp)     /* set OE bit (overflow exception) */
    btrw    $3, 0(%esp)     /* clear OM bit (overflow mask) */
    fldenv  0(%esp)         /* restore x87 state */
PinLab_X87Overflow:
    fwait                   /* this raises the exception */
    addl    $32, %esp
    ret

/*
 * void DoX87Underflow()
 */
.text
    .align 4
.globl DoX87Underflow
.globl PinLab_X87Underflow
DoX87Underflow:
    subl    $32, %esp
    finit                   /* initialize x87 */
    fstenv  0(%esp)         /* store x87 state */
    btsw    $4, 4(%esp)     /* set UE bit (underflow exception) */
    btrw    $4, 0(%esp)     /* clear UM bit (underflow mask) */
    fldenv  0(%esp)         /* restore x87 state */
PinLab_X87Underflow:
    fwait                   /* this raises the exception */
    addl    $32, %esp
    ret

/*
 * void DoX87Precision()
 */
.text
    .align 4
.globl DoX87Precision
.globl PinLab_X87Precision
DoX87Precision:
    subl    $32, %esp
    finit                   /* initialize x87 */
    fstenv  0(%esp)         /* store x87 state */
    btsw    $5, 4(%esp)     /* set PE bit (precision exception) */
    btrw    $5, 0(%esp)     /* clear PM bit (precision mask) */
    fldenv  0(%esp)         /* restore x87 state */
PinLab_X87Precision:
    fwait                   /* this raises the exception */
    addl    $32, %esp
    ret

/*
 * void DoX87InvalidOperation()
 */
.text
    .align 4
.globl DoX87InvalidOperation
.globl PinLab_X87InvalidOperation
DoX87InvalidOperation:
    subl    $32, %esp
    finit                   /* initialize x87 */
    fstenv  0(%esp)         /* store x87 state */
    btsw    $0, 4(%esp)     /* set IE bit (invalid operation exception) */
    btrw    $0, 0(%esp)     /* clear IM bit (invalid operation mask) */
    fldenv  0(%esp)         /* restore x87 state */
PinLab_X87InvalidOperation:
    fwait                   /* this raises the exception */
    addl    $32, %esp
    ret

/*
 * void DoX87DenormalizedOperand()
 */
.text
    .align 4
.globl DoX87DenormalizedOperand
.globl PinLab_X87DenormalizedOperand
DoX87DenormalizedOperand:
    subl    $32, %esp
    finit                   /* initialize x87 */
    fstenv  0(%esp)         /* store x87 state */
    btsw    $1, 4(%esp)     /* set DE bit (denormalized operand exception) */
    btrw    $1, 0(%esp)     /* clear DM bit (denormalized operand mask) */
    fldenv  0(%esp)         /* restore x87 state */
PinLab_X87DenormalizedOperand:
    fwait                   /* this raises the exception */
    addl    $32, %esp
    ret

/*
 * void DoX87StackUnderflow()
 */
.text
    .align 4
.globl DoX87StackUnderflow
.globl PinLab_X87StackUnderflow
DoX87StackUnderflow:
    subl    $8, %esp
    finit                   /* initialize x87 */
    fnstcw  0(%esp)         /* get current FP control word */
    btrw    $0, 0(%esp)     /* enable "invalid operation" exceptions */
    fldcw   0(%esp)         /* store new FP control word */
    faddp
PinLab_X87StackUnderflow:
    fwait                   /* this raises the exception */
    addl    $8, %esp
    ret

/*
 * void DoX87StackOverflow()
 */
.text
    .align 4
.globl DoX87StackOverflow
.globl PinLab_X87StackOverflow
DoX87StackOverflow:
    subl    $8, %esp
    finit                   /* initialize x87 */
    fnstcw  0(%esp)         /* get current FP control word */
    btrw    $0, 0(%esp)     /* enable "invalid operation" exceptions */
    fldcw   0(%esp)         /* store new FP control word */
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
    addl    $8, %esp
    ret

/*
 * void DoX87MultipleExceptions()
 */
.text
    .align 4
.globl DoX87MultipleExceptions
.globl PinLab_X87MultipleExceptions
DoX87MultipleExceptions:
    subl    $32, %esp
    finit                   /* initialize x87 */
    fstenv  0(%esp)         /* store x87 state */
    btsw    $0, 4(%esp)     /* set IE bit (invalid operation exception) */
    btsw    $1, 4(%esp)     /* set DE bit (denormalized operand exception) */
    btsw    $2, 4(%esp)     /* set ZE bit (zero divide exception) */
    btsw    $3, 4(%esp)     /* set OE bit (overflow exception) */
    btsw    $4, 4(%esp)     /* set UE bit (underflow exception) */
    btsw    $5, 4(%esp)     /* set PE bit (precision exception) */
    btrw    $0, 0(%esp)     /* clear IM bit (invalid operation mask) */
    btrw    $1, 0(%esp)     /* clear DM bit (denormalized operand mask) */
    btrw    $2, 0(%esp)     /* clear ZM bit (zero divide mask) */
    btrw    $3, 0(%esp)     /* clear OM bit (overflow mask) */
    btrw    $4, 0(%esp)     /* clear UM bit (underflow mask) */
    btrw    $5, 0(%esp)     /* clear PM bit (precision mask) */
    fldenv  0(%esp)         /* restore x87 state */
PinLab_X87MultipleExceptions:
    fwait                   /* this raises all the exceptions */
    addl    $32, %esp
    ret

/*
 * void DoSIMDDivideByZero()
 */
.text
    .align 4
.globl DoSIMDDivideByZero
.globl PinLab_SIMDDivideByZero
DoSIMDDivideByZero:
    subl        $8, %esp
    stmxcsr     0(%esp)
    btrw        $9, 0(%esp)     /* enable "zero divide" exceptions */
    ldmxcsr     0(%esp)
    movl        $0, %eax
    cvtsi2ss    %eax, %xmm0     /* %xmm0 = 0.0 */
    movl        $1, %eax
    cvtsi2ss    %eax, %xmm1     /* %xmm1 = 1.0 */
PinLab_SIMDDivideByZero:
    divss       %xmm0, %xmm1    /* raise "zero divide" exception */
    addl        $8, %esp
    ret

/*
 * void DoSIMDOverflow()
 */
.text
    .align 4
.globl DoSIMDOverflow
.globl PinLab_SIMDOverflow
DoSIMDOverflow:
    subl        $8, %esp
    stmxcsr     0(%esp)
    btrw        $10, 0(%esp)            /* enable "overflow" exceptions */
    ldmxcsr     0(%esp)
    movl        $0x7f7fffff, 0(%esp)    /* largest 32-bit float value */
    movl        $0x7f7fffff, 4(%esp)
    movq        0(%esp), %xmm0
    movq        0(%esp), %xmm1
PinLab_SIMDOverflow:
    mulss       %xmm0, %xmm1            /* raise "overflow" exception */
    addl        $8, %esp
    ret

/*
 * void DoSIMDUnderflow()
 */
.text
    .align 4
.globl DoSIMDUnderflow
.globl PinLab_SIMDUnderflow
DoSIMDUnderflow:
    subl        $8, %esp
    stmxcsr     0(%esp)
    btrw        $11, 0(%esp)            /* enable "underflow" exceptions */
    ldmxcsr     0(%esp)
    movl        $0x00800001, 0(%esp)    /* smallest 32-bit float value */
    movl        $0x00800001, 4(%esp)
    movq        0(%esp), %xmm0
    movq        0(%esp), %xmm1
PinLab_SIMDUnderflow:
    mulss       %xmm0, %xmm1            /* raise "underflow" exception */
    addl        $8, %esp
    ret

/*
 * void DoSIMDPrecision()
 */
.text
    .align 4
.globl DoSIMDPrecision
.globl PinLab_SIMDPrecision
DoSIMDPrecision:
    subl        $8, %esp
    stmxcsr     0(%esp)
    btrw        $12, 0(%esp)    /* enable "precision" exceptions */
    ldmxcsr     0(%esp)
    movl        $1, %eax
    cvtsi2ss    %eax, %xmm0     /* %xmm0 = 1.0 */
    movl        $2, %eax
    cvtsi2ss    %eax, %xmm1     /* %xmm1 = 2.0 */
    divss       %xmm1, %xmm0    /* %xmm0 = 0.5 */
PinLab_SIMDPrecision:
    cvtss2si    %xmm0, %eax     /* raise "precision" exception */
    addl        $8, %esp
    ret

/*
 * void DoSIMDInvalidOperation()
 */
.text
    .align 4
.globl DoSIMDInvalidOperation
.globl PinLab_SIMDInvalidOperation
DoSIMDInvalidOperation:
    subl        $8, %esp
    stmxcsr     0(%esp)
    btrw        $7, 0(%esp)     /* enable "invalid operation" exceptions */
    ldmxcsr     0(%esp)
    movl        $-1, %eax
    cvtsi2ss    %eax, %xmm0     /* %xmm0 = -1.0 */
PinLab_SIMDInvalidOperation:
    sqrtss      %xmm0, %xmm0    /* raise "invalid operation" exception */
    addl        $8, %esp
    ret

/*
 * void DoSIMDDenormalizedOperand()
 */
.text
    .align 4
.globl DoSIMDDenormalizedOperand
.globl PinLab_SIMDDenormalizedOperand
DoSIMDDenormalizedOperand:
    subl        $8, %esp
    stmxcsr     0(%esp)
    btrw        $6, 0(%esp)             /* disable "denormals-are-zero" flag */
    btrw        $8, 0(%esp)             /* enable "denormalized operand" exceptions */
    ldmxcsr     0(%esp)
    movl        $0x00400000, 0(%esp)    /* denormal 32-bit float */
    movl        $0x00400000, 4(%esp)    /* denormal 32-bit float */
    movq        0(%esp), %xmm0
    movq        0(%esp), %xmm1
PinLab_SIMDDenormalizedOperand:
    addss       %xmm0, %xmm0            /* raise "denormalized operand" exception */
    addl        $8, %esp
    ret

/*
 * void DoSIMDMultipleExceptions()
 */
.text
    .align 4
.globl DoSIMDMultipleExceptions
.globl PinLab_SIMDMultipleExceptions
DoSIMDMultipleExceptions:
    subl        $8, %esp
    stmxcsr     0(%esp)
    btrw        $11, 0(%esp)            /* enable "underflow" exceptions */
    btrw        $12, 0(%esp)            /* enable "precision" exceptions */
    ldmxcsr     0(%esp)
    movl        $0x00800001, 0(%esp)    /* smallest 32-bit float value */
    movl        $0x00800001, 4(%esp)
    movq        0(%esp), %xmm0
    movq        0(%esp), %xmm1
PinLab_SIMDMultipleExceptions:
    mulss       %xmm0, %xmm1            /* raise "underflow" and "precision" exception */
    addl        $8, %esp
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
    pushf
    pop     %eax
    mov     %eax, %ecx
    or      $0x100, %eax
    push    %eax
    popf

    add     %eax, %eax      /* raises single-step trap fault */

    /*
     * Restore the TF flag.  If the exception happens (which is expected),
     * this code won't execute.  However the siglongjmp() will restore the
     * flags.
     */
    push    %ecx
    popf
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
    pushf
    btrl    $18, 0(%esp)    /* clears the AC bit */
    popf
    ret

/*
 * void DoIllegalSetOfSegReg1()
 */
 .globl DoIllegalSetOfSegReg1
 DoIllegalSetOfSegReg1:
    mov $5, %eax
    mov %eax, %gs
    ret

/*
 * void DoIllegalSetOfSegReg2()
 */
 .globl DoIllegalSetOfSegReg2
 DoIllegalSetOfSegReg2:
    mov $0, %edx
    lfs (%edx), %eax
    ret

/*
 * void DoIllegalGetOfSegReg1()
 */
 .globl DoIllegalGetOfSegReg1
 DoIllegalGetOfSegReg1:
    mov $0, %edx
    mov %gs, (%edx)
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
