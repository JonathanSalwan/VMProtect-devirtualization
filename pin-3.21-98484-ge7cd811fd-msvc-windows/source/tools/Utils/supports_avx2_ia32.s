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
    push    %ebp
    mov     %esp, %ebp
    push %ebx          # CPUID clobbers ebx, which may be used as the PIC register
    push %ecx
    push %edx
    mov $1, %eax
    cpuid
    andl $0x18000000, %ecx
    cmpl $0x18000000, %ecx    # check both OSXSAVE and AVX feature flags
    jne NotSupported         
                       # processor supports AVX instructions and XGETBV is enabled by OS
    mov $0, %ecx       # specify 0 for XFEATURE_ENABLED_MASK register
                       # 0xd0010f is xgetbv  - result in EDX:EAX 
    .byte 0xf, 0x1, 0xd0
    andl $6, %eax
    cmpl $6, %eax      # check OS has enabled both XMM and YMM state support
    jne NotSupported
    mov $7, %eax
    mov $0, %ecx       // Check for AVX2 support on CPU
    cpuid
    andl $0x20, %ebx   // bit 5 avx2
    cmpl $0x20, %ebx   // bit 5 avx2
    jne NotSupported   # no AVX2
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
