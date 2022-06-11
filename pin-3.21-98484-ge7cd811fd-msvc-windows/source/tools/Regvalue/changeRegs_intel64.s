/*
 * Copyright (C) 2013-2019 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

# On macOS*, we need to use RIP relative addressing (%rip) to access nonlocal
# data. This has no effect on Linux.

.data

.extern gprval
.extern agprval
.extern stval
.extern astval
.extern xmmval
.extern axmmval
#ifdef CONTEXT_USING_AVX
.extern ymmval
.extern aymmval
#endif
#ifdef CONTEXT_USING_AVX512F
.extern zmmval
.extern azmmval
.extern opmaskval
.extern aopmaskval
#endif
.extern fpSaveArea

.text

# void ChangeRegsWrapper();
# This wrapper saves and restores the registers used by ChangeRegs.
# This is done in the wrapper since we want ChangeRegs to actually
# change the register values but not to affect the application itself.
# The tool may intercept ChangeRegs and replace it with its own function.
#
# Register usage:
# rax   - used (implicitly) by xsave
# rbx   - used for testing the gpr values
# rcx   - used for holding a pointer to the fp save area (used by fxsave)
# rdx   - used (implicitly) by xsave
# st0   - used (implicitly) for loading a value to the FPU stack
# st2   - used for testing the FPU values
# xmm0  - used for testing the sse (xmm) values
# ymm1  - used for testing the avx (ymm) values
# zmm5  - used for testing the avx512 (zmm) values
# k3    - used for testing the opmask register values
#ifndef TARGET_MAC
.type ChangeRegsWrapper,  @function
#endif
.global ChangeRegsWrapper
ChangeRegsWrapper:
    # Save the necessary GPRs
    push    %rax
    push    %rbx
    push    %rcx
    push    %rdx

#ifdef CONTEXT_USING_AVX512F
    # Save the necessary mask registers
    kmovw   %k3, %eax
    push    %rax
#endif

    # Allign the fpSaveArea
    lea     fpSaveArea(%rip), %rcx
    add     $0x40, %rcx
    and     $0xffffffffffffffc0, %rcx
    # Save the floating-point state
#if defined(CONTEXT_USING_AVX) || defined(CONTEXT_USING_AVX512F)
    push    %rdx
    xor     %rdx, %rdx
    mov     $7, %rax
    xsave   (%rcx)
#else
    fxsave  (%rcx)
#endif

    # Now call ChangeRegs - do the actual test.
    # The tool may intercept this function and modify the register values itself.
    call    ChangeRegs

    # Placeholder for PIN_ExecuteAt
    call    ExecuteAt

    # Save the modified values to memory so the tool can ispect them.
    # This is relevant only when the tool modifies the values.
    call    SaveRegsToMem

    # Restore the floating-point state
#if defined(CONTEXT_USING_AVX) || defined(CONTEXT_USING_AVX512F)
    mov     $7, %rax
    xrstor  (%rcx)
    pop     %rdx
#else
    fxrstor (%rcx)
#endif

#ifdef CONTEXT_USING_AVX512F
    # Restore the mask registers
    pop     %rax
    kmovw   %eax, %k3
#endif

    # Restore the GPRs
    pop     %rdx
    pop     %rcx
    pop     %rbx
    pop     %rax
    ret

# void ChangeRegs();
# For register usage see ChangeRegsWrapper above.
#ifndef TARGET_MAC
.type ChangeRegs,  @function
#endif
.global ChangeRegs
ChangeRegs:
    # TEST: load the new value to rbx
    mov     gprval(%rip), %rbx
    # prepare the test value at the top of the FPU stack
    fldt    stval(%rip)
    # TEST: load the new value to st2
    fst     %st(2)
    # TEST: load the new value to xmm0
    movdqu  xmmval(%rip), %xmm0
#ifdef CONTEXT_USING_AVX
    # TEST: load the new value to ymm1
    vmovdqu ymmval(%rip), %ymm1
#endif
#ifdef CONTEXT_USING_AVX512F
    # TEST: load the new value to zmm5
    vmovdqu64 zmmval(%rip), %zmm5
    # TEST: load the new value to k3
    kmovw   opmaskval(%rip), %k3
#endif
    ret

# void ExecuteAt();
#ifndef TARGET_MAC
.type ExecuteAt,  @function
#endif
.global ExecuteAt
ExecuteAt:
    ret

# void SaveRegsToMem();
# Save the necessary registers to memory.
# The tool will then compare the value stored in memory to the ones it expects to find.
# For register usage see ChangeRegsWrapper above.
#ifndef TARGET_MAC
.type SaveRegsToMem,  @function
#endif
.global SaveRegsToMem
SaveRegsToMem:
    # TEST: store the new value of rbx
    mov     %rbx, agprval(%rip)
    # prepare the test value at the top of the FPU stack
    fld     %st(2)
    # TEST: store the new value of st2
    fstpt   astval(%rip)
    # TEST: store the new value of xmm0
    movdqu  %xmm0, axmmval(%rip)
#ifdef CONTEXT_USING_AVX
    # TEST: store the new value of ymm1
    vmovdqu %ymm1, aymmval(%rip)
#endif
#ifdef CONTEXT_USING_AVX512F
    # TEST: store the new value of zmm5
    vmovdqu64 %zmm5, azmmval(%rip)
    # TEST: store the new value of k3
    kmovw   %k3, aopmaskval(%rip)
#endif
    ret
