/*
 * Copyright (C) 2007-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

# struct FarPointer16
# {
#     unsigned short _farPtr;
#     unsigned short _segVal;
#     unsigned int   _pad;
# };
#
# struct FarPointer32
# {
#     unsigned int _farPtr;
#     unsigned int _segVal;
#     unsigned int   _pad;
# };
# struct FarPointer64
# {
#     unsigned long _farPtr;
# 	  unsigned long _segVal;
# 	  unsigned long _pad;
# };
# 

# unsigned short SetGs16(const FarPointer16 *fp, unsigned long upperBits);
# unsigned int   SetGs32(const FarPointer32 *fp, unsigned long upperBits);
# unsigned long  SetGs64(const FarPointer64 *fp, unsigned long upperBits);


.global SetGs16
.type SetGs16, @function


SetGs16:
   mov %rsi, %rax
   lgs (%rdi), %ax
   ret

.global SetGs32
.type SetGs32, @function


SetGs32:
   mov %rsi, %rax
   lgs (%rdi), %eax
   ret

.global SetGs64
.type SetGs64, @function


SetGs64:
   mov %rsi, %rax
   lgs (%rdi), %rax
   ret

.global GetGsVal
.type GetGsVal, @function

GetGsVal:
   mov $0xffffffff88888888, %rax
   mov %gs, %eax
   ret

# void MoveMem16ToGs(unsigned int  *val32);
# void MoveMem64ToGs(unsigned long *val64);
# 
# void MoveGsToMem16(unsigned int  *val32);
# void MoveGsToMem64(unsigned long *val64);


.global MoveMem16ToGs
.type MoveMem16ToGs, @function

MoveMem16ToGs:
    movw (%rdi), %gs
    ret
    
.global MoveMem64ToGs
.type MoveMem64ToGs, @function

MoveMem64ToGs:
    mov (%rdi), %gs    
    ret
    
.global MoveGsToMem16
.type MoveGsToMem16, @function

MoveGsToMem16:
    movw %gs, (%rdi)
    ret
    
.global MoveGsToMem64
.type MoveGsToMem64, @function

MoveGsToMem64:
    mov %gs, (%rdi)  
    ret
