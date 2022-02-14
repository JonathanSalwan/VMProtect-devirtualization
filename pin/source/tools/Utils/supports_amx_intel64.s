/*
 * Copyright (C) 2021-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef TARGET_MAC
.global _SupportsAmx
_SupportsAmx:
#else
.type SupportsAmx, @function
.global SupportsAmx
SupportsAmx:
#endif
    push    %rbp
    mov     %rsp, %rbp
    push    %rbx
    push    %rcx
    push    %rdx
    mov $1, %rax
    cpuid
    andq $0x0c000000, %rcx
    cmpq $0x0c000000, %rcx    // check both OSXSAVE and XSAVE feature flags
    jne NotSupported
                       // processor supports AVX instructions and XGETBV is enabled by OS
    mov $0, %rcx       // specify 0 for XFEATURE_ENABLED_MASK register
                       // 0xd0010f is xgetbv  - result in EDX:EAX
    .byte 0xf, 0x1, 0xd0
    andq $0x60000, %rax
    cmpq $0x60000, %rax   // check OS has enabled both XTILECFG[17] and XTILEDATA[18] state support
    jne NotSupported
    mov $7, %rax
    mov $0, %rcx       // Check for AMX support on CPU
    cpuid
    andq $0x1000000, %rdx // bit 24 amx-tile
    cmpq $0x1000000, %rdx // bit 24 amx-tile
    jne NotSupported  // no AMX
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
