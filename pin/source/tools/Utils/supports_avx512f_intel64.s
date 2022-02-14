/*
 * Copyright (C) 2010-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef TARGET_MAC
.global _SupportsAvx512f
_SupportsAvx512f:
#else
.type SupportsAvx512f, @function
.global SupportsAvx512f
SupportsAvx512f:
#endif
    push    %rbp
    mov     %rsp, %rbp
    push    %rbx
    push    %rcx
    push    %rdx
    mov $1, %rax
    cpuid
    andq $0x18000000, %rcx
    cmpq $0x18000000, %rcx    # check both OSXSAVE and AVX feature flags
    jne NotSupported
                       # processor supports AVX instructions and XGETBV is enabled by OS
    mov $0, %rcx       # specify 0 for XFEATURE_ENABLED_MASK register
                       # 0xd0010f is xgetbv  - result in EDX:EAX
    .byte 0xf, 0x1, 0xd0
    andq $0xe6, %rax
    cmpq $0xe6, %rax   # check OS has enabled both XMM, YMM, ZMM and OPMASK state support
    jne NotSupported
    mov $7, %rax
    mov $0, %rcx       # specify 0 for XFEATURE_ENABLED_MASK register
    cpuid
    andq $0x10000, %rbx
    cmpq $0x10000, %rbx
    jne NotSupported  # no AVX512f
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
