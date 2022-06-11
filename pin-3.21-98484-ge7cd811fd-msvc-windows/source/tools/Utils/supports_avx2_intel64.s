/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef TARGET_MAC
.global _SupportsAvx2
_SupportsAvx2:
#else
.type SupportsAvx2, @function
.global SupportsAvx2
SupportsAvx2:
#endif
    push %rbp
    mov  %rsp, %rbp
    push %rbx
    push %rcx
    push %rdx
    mov $1, %rax
    cpuid
    andq $0x18000000, %rcx
    cmpq $0x18000000, %rcx    # check both OSXSAVE and AVX feature flags
    jne NotSupported         
                       # processor supports AVX instructions and XGETBV is enabled by OS
    mov $0, %rcx       # specify 0 for XFEATURE_ENABLED_MASK register
                       # 0xd0010f is xgetbv  - result in EDX:EAX 
    .byte 0xf, 0x1, 0xd0
    andq $6, %rax
    cmpq $6, %rax      # check OS has enabled both XMM and YMM state support
    jne NotSupported
    mov $7, %rax
    mov $0, %rcx       // Check for AVX2 support on CPU
    cpuid
    andq $0x20, %rbx   // bit 5 avx2
    cmpq $0x20, %rbx   // bit 5 avx2 
    jne NotSupported  # no AVX2
    mov $1, %rax
done:
    pop %rdx
    pop %rcx
    pop %rbx
    leave
    ret

NotSupported:
    mov $0, %rax
    jmp done
