/*
 * Copyright (C) 2010-2017 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#
# Test for iret in 32 bit mode.
#

iret_func:
        mov $-1,%eax
        iret

#ifndef TARGET_MAC
.type iretTest, @function
#endif
.global iretTest
iretTest:
        # We have to build the stack frame ourselves
        sub     $12,%esp
        mov     $0, %eax
        mov     %eax,8(%esp)         #  Write the flags to zero
        mov     %cs, %eax
        mov     %eax,4(%esp)
        lea     here,%eax
        mov     %eax,0(%esp)
        jmp     iret_func
here:   
        ret

