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
    push    %ebp
    mov     %esp, %ebp
    push    %ebx          # CPUID clobbers ebx, which may be used as the PIC register
    push    %ecx
    push    %edx
    mov $1, %eax
    cpuid
    andl $0x18000000, %ecx
    cmpl $0x18000000, %ecx    # check both OSXSAVE and AVX feature flags
    jne NotSupported
                       # processor supports AVX instructions and XGETBV is enabled by OS
    mov $0, %ecx       # specify 0 for XFEATURE_ENABLED_MASK register
                       # 0xd0010f is xgetbv  - result in EDX:EAX
    .byte 0xf, 0x1, 0xd0
    andl $0x66, %eax
    cmpl $0x66, %eax   # check OS has enabled both XMM, YMM, ZMM and OPMASK state support
    jne NotSupported
    mov $7, %eax
    mov $0, %ecx
    cpuid
    andl $0x10000, %ebx
    cmpl $0x10000, %ebx
    jne NotSupported   # no AVX512
    mov $1, %eax
done:
    pop %edx
    pop %ecx
    pop %ebx
    leave
    ret

NotSupported:
    mov $0, %eax
    jmp done
