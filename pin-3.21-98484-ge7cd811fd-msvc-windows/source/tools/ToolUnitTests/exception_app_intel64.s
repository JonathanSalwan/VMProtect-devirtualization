/*
 * Copyright (C) 2010-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

.text
.global RaiseIntDivideByZeroException
## ADDRINT RaiseIntDivideByZeroException(ADDRINT catchAddr , ADDRINT value);

.type RaiseIntDivideByZeroException, @function
RaiseIntDivideByZeroException:
    push %rbp
    mov %rsp, %rbp
    push %rbx # save rbx
    mov %rdi, %rbx # fptr
    # value in %rsi
    push %rsi
    xor %rax, %rax
    idiv %rax
    leave
    ret
    
.global CatchIntDivideByZeroException

.type CatchIntDivideByZeroException, @function
CatchIntDivideByZeroException:
    pop %rax # exc code -> %rax
    pop %rbx
    leave
    ret
    
.global UnmaskFpZeroDivide
.type UnmaskFpZeroDivide, @function
UnmaskFpZeroDivide:
  push %rbp
  mov %rsp, %rbp
  call GetFCW
  mov $0x4, %rdi
  not %rdi
  and %rax, %rdi
  call SetFCW
  leave
  ret
  
.global MaskFpZeroDivide
.type MaskFpZeroDivide, @function
MaskFpZeroDivide:
  push %rbp
  mov %rsp, %rbp
  call GetFCW
  mov $0x4, %rdi
  or %rax, %rdi
  call SetFCW
  fnclex
  leave
  ret

.global GetFCW
.type GetFCW, @function
GetFCW:
 xor %rax, %rax
 push %rax
 fstcw (%rsp)
 pop %rax
 ret


.global SetFCW
.type SetFCW, @function
SetFCW:
   push %rbp
   mov %rsp, %rbp
   fnclex
   push %rdi
   fldcw (%rsp)
   fnclex
   leave
   ret

.global UnmaskZeroDivideInMxcsr
.type UnmaskZeroDivideInMxcsr, @function
UnmaskZeroDivideInMxcsr:
   call GetMxcsr
   mov $0x200, %rdi
   not %rdi
   and %rax, %rdi
   call SetMxcsr
   ret
   
.global MaskZeroDivideInMxcsr
.type MaskZeroDivideInMxcsr, @function
MaskZeroDivideInMxcsr:
   call GetMxcsr
   mov $0x200, %rdi
   or %rax, %rdi
   call SetMxcsr
   ret

.global GetMxcsr
.type GetMxcsr, @function
GetMxcsr:
  push %rax
  stmxcsr (%rsp)
  pop %rax
  ret
  
.global SetMxcsr
.type SetMxcsr, @function
SetMxcsr:
  push %rdi
  ldmxcsr (%rsp)
  pop %rax
  ret

