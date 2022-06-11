/*
 * Copyright (C) 2013-2017 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

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
# eax   - used (implicitly) by xsave
# ebx   - used for testing the gpr values
# ecx   - used for holding a pointer to the fp save area (used by fxsave)
# edx   - used (implicitly) by xsave
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
    push    %eax
    push    %ebx
    push    %ecx
    push    %edx

#ifdef CONTEXT_USING_AVX512F
    # Save the necessary mask registers
    kmovw   %k3, %eax
    push    %eax
#endif

    # Allign the fpSaveArea
    lea     fpSaveArea, %ecx
    add     $0x40,%ecx
    and     $0xffffffc0,%ecx
    # Save the floating-point state
#if defined(CONTEXT_USING_AVX) || defined(CONTEXT_USING_AVX512F)
    push    %edx
    xor     %edx, %edx
    mov     $7, %eax
    xsave   (%ecx)
#else
    fxsave  (%ecx)
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
    mov     $7, %eax
    xrstor  (%ecx)
    pop     %edx
#else
    fxrstor (%ecx)
#endif

#ifdef CONTEXT_USING_AVX512F
    # Restore the mask registers
    pop     %eax
    kmovw   %eax, %k3
#endif

    # Restore the GPRs
    pop     %edx
    pop     %ecx
    pop     %ebx
    pop     %eax
    ret

# void ChangeRegs();
# For register usage see ChangeRegsWrapper above.
#ifndef TARGET_MAC
.type ChangeRegs,  @function
#endif
.global ChangeRegs
ChangeRegs:
    # TEST: load the new value to ebx
    mov     gprval, %ebx
    # prepare the test value at the top of the FPU stack
    fldt    stval
    # TEST: load the new value to st2
    fst     %st(2)
    # TEST: load the new value to xmm0
    movdqu  xmmval, %xmm0
#ifdef CONTEXT_USING_AVX
    # TEST: load the new value to ymm1
    vmovdqu ymmval, %ymm1
#endif
#ifdef CONTEXT_USING_AVX512F
    # TEST: load the new value to zmm5
    vmovdqu32 zmmval, %zmm5
    # TEST: load the new value to k3
    kmovw   opmaskval, %k3
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
    # TEST: store the new value of ebx
    mov     %ebx, agprval
    # prepare the test value at the top of the FPU stack
    fld     %st(2)
    # TEST: store the new value of st2
    fstpt   astval
    # TEST: store the new value of xmm0
    movdqu  %xmm0, axmmval
#ifdef CONTEXT_USING_AVX
    # TEST: store the new value of ymm1
    vmovdqu %ymm1, aymmval
#endif
#ifdef CONTEXT_USING_AVX512F
    # TEST: store the new value of zmm5
    vmovdqu32 %zmm5, azmmval
    # TEST: store the new value of k3
    kmovw   %k3, aopmaskval
#endif
    ret
