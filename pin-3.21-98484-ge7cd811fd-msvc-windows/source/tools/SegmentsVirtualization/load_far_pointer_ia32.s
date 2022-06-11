/*
 * Copyright (C) 2007-2012 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

# 
#  struct FarPointer
#  {
#      unsigned int _farPtr;
#      unsigned int _segVal;
#  };

#  int SetGs(const FarPointer *fp);


.global SetGs
.type SetGs, @function


SetGs:
   push %ebp
   mov %esp, %ebp
   mov 0x8(%ebp), %eax
   lgs (%eax), %eax
   leave
   ret

#  int SetFs(const FarPointer *fp);

.global SetFs
.type SetFs, @function

SetFs:
   push %ebp
   mov %esp, %ebp
   mov 0x8(%ebp), %eax
   lfs (%eax), %eax
   leave
   ret
   
#  unsigned int GetGsBase();
#  unsigned int GetFsBase();

.global GetGsBase
.type GetGsBase, @function


GetGsBase:
   mov %gs:0x0, %eax
   ret
   
.global GetFsBase
.type GetFsBase, @function

GetFsBase:
   mov %fs:0x0, %eax
   ret
   

