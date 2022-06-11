/*
 * Copyright (C) 2009-2016 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "asm_macros.h"

DECLARE_FUNCTION(ReadFpContext)
NAME(ReadFpContext):
	push %ebp
	mov %esp, %ebp
	mov 8(%ebp), %eax
	fxsave (%eax)
	leave
	ret
END_FUNCTION(ReadFpContext)

DECLARE_FUNCTION(WriteFpContext)
NAME(WriteFpContext):
	push %ebp
	mov %esp, %ebp
	mov 8(%ebp), %eax
	fxrstor (%eax)
	leave
	ret
END_FUNCTION(WriteFpContext)

.global sched_yield

// void GetLock(long *mutex, long newVal)
DECLARE_FUNCTION(GetLock)
NAME(GetLock):
    push %ebp
    mov %esp, %ebp
    push %esi
    push %edi
    mov 8(%ebp), %esi # %esi <- mutex
    mov 0xc(%ebp), %edi # %edi <- new value
    xor %eax, %eax

try_again:
    lock cmpxchg %edi, (%esi)
    je done
    call PLT_ADDRESS(sched_yield)
    jmp try_again
done:
    pop %edi
    pop %esi
    leave
    ret
END_FUNCTION(GetLock)
        
// void ReleaseLock(long *mutex)

DECLARE_FUNCTION(ReleaseLock)
NAME(ReleaseLock):
    push %ebp
    mov %esp, %ebp
    push %edi
    mov 8(%ebp), %edi
    xor %eax, %eax
    lock xchg %eax, (%edi)
    pop %edi
    leave
    ret
END_FUNCTION(ReleaseLock)

// void InitLock(long *mutex)    
DECLARE_FUNCTION(InitLock)
NAME(InitLock):
    push %ebp
    mov %esp, %ebp
    push %edi
    mov 8(%ebp), %edi
    xor %eax, %eax
    lock xchg %eax, (%edi)
    pop %edi
    leave
    ret
END_FUNCTION(InitLock)

// extern "C" void SetXmmRegs(long v1, long v2, long v3);
// extern "C" void GetXmmRegs(long *v1, long *v2, long *v3);



DECLARE_FUNCTION(SetXmmRegs)
NAME(SetXmmRegs):
  movss 0x4(%esp), %xmm1
  movss 0x8(%esp), %xmm2
  movss 0xc(%esp), %xmm3
  ret
END_FUNCTION(SetXmmRegs)
 
DECLARE_FUNCTION(GetXmmRegs)
NAME(GetXmmRegs):
  mov 0x4(%esp), %eax
  movss %xmm1, (%eax)
  mov 0x8(%esp), %eax
  movss %xmm2, (%eax)
  mov 0xc(%esp), %eax
  movss %xmm3, (%eax)
  ret
END_FUNCTION(GetXmmRegs)

