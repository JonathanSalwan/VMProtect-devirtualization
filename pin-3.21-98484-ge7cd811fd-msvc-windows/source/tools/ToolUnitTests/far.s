/*
 * Copyright (C) 2008-2011 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

# Add enter and leave as well, just for amusement

farret:
        enter   $32,$0
        mov $5,  %eax
        leave
        lret

.type FarCallTest, @function
.global FarCallTest
FarCallTest:
        push %cs
        call farret
        ret     

#
# Expose the MaskMovQ instruction
#       MaskMovQ(char *dest, UINT64 src, UINT64 mask) 
.type MaskMovQ, @function
.global MaskMovQ
MaskMovQ:
        mov      4(%esp),%edi
        movq     8(%esp),%mm1
        movq    16(%esp),%mm0
        maskmovq %mm0,%mm1
        ret

#
# Some unlikely (but legal) push and pop instructions
# VOID *  pushEsp(VOID * stack)
#
.type pushESP, @function
.global pushESP
pushESP:        
        mov     %esp, %ecx      # Save esp
        mov     4(%esp),%esp
        push    %esp            # Do the op
        mov     %esp, %eax      # Result
        mov     %ecx, %esp      # Stack with return address
        ret

#
# VOID *  popESP(VOID * stack)
#
.type popESP, @function
.global popESP
popESP:                
        mov     %esp, %ecx      # Save esp
        mov     4(%esp),%esp    # stack to play with
        pop     %esp            # Do the op
        mov     %esp, %eax      # Result
        mov     %ecx, %esp      # Stack with return address
        ret
#
# VOID *  pushStarESP(VOID * stack)
#
.type pushStarESP, @function
.global pushStarESP
pushStarESP:                
        mov     %esp, %ecx      # Save esp
        mov     4(%esp),%esp    # stack to play with
        push    (%esp)          # Do the op
        mov     %esp, %eax      # Result
        mov     %ecx, %esp      # Stack with return address
        ret

#
# VOID *  push4ESP(VOID * stack)
#
.type push4ESP, @function
.global push4ESP
push4ESP:                
        mov     %esp, %ecx      # Save esp
        mov     4(%esp),%esp    # stack to play with
        push    4(%esp)         # Do the op
        mov     %esp, %eax      # Result
        mov     %ecx, %esp      # Stack with return address
        ret

#
# VOID *  popStarESP(VOID * stack)
#
.type popStarESP, @function
.global popStarESP
popStarESP:                
        mov     %esp, %ecx      # Save esp
        mov     4(%esp),%esp    # stack to play with
        pop     (%esp)          # Do the op
        mov     %esp, %eax      # Result
        mov     %ecx, %esp      # Stack with return address
        ret

#
# VOID *  pop4ESP(VOID * stack)
#
.type pop4ESP, @function
.global pop4ESP
pop4ESP:                
        mov     %esp, %ecx      # Save esp
        mov     4(%esp),%esp    # stack to play with
        pop     4(%esp)         # Do the op
        mov     %esp, %eax      # Result
        mov     %ecx, %esp      # Stack with return address
        ret

#
# VOID *  pushFlags(VOID * stack)
#
# This actually forces the flags first, and then pushes them
.type pushFlags, @function
.global pushFlags
pushFlags:      
        mov     %esp, %ecx      # Save esp
        pushl   $0x282          # Force flags
        popf                    #
        mov     4(%esp),%esp    # stack to play with
        pushf                   # Do the op
        mov     %esp, %eax      # Result
        mov     %ecx, %esp      # Stack with return address
        ret
        
#
# VOID *  pushIZero(VOID * stack)
# Push an immediate zero
.type pushIZero, @function
.global pushIZero
pushIZero:      
        mov     %esp, %ecx      # Save esp
        mov     4(%esp),%esp    # stack to play with
        pushl   $0              # Do the op
        mov     %esp, %eax      # Result
        mov     %ecx, %esp      # Stack with return address
        ret
        
#
# VOID *  pushWIZero(VOID * stack)
# Push an immediate zero
.type pushWIZero, @function
.global pushWIZero
pushWIZero:      
        mov     %esp, %ecx      # Save esp
        mov     4(%esp),%esp    # stack to play with
        pushw   $0              # Do the op
        mov     %esp, %eax      # Result
        mov     %ecx, %esp      # Stack with return address
        ret
        
#
# VOID *  pushIM1(VOID * stack)
# Push an immediate minus -1 (one byte sign extended...)
.type pushIM1, @function
.global pushIM1
pushIM1:      
        mov     %esp, %ecx      # Save esp
        mov     4(%esp),%esp    # stack to play with
        pushl   $-1             # Do the op
        mov     %esp, %eax      # Result
        mov     %ecx, %esp      # Stack with return address
        ret
        
#
# VOID *  pushIs16(VOID * stack)
# Push an immediate which fits in 16 (sign extended) bits
# (though the assembler leaves it as 32 bits).
.type pushIs16, @function
.global pushIs16
pushIs16:      
        mov     %esp, %ecx      # Save esp
        mov     4(%esp),%esp    # stack to play with
        pushl   $-32768         # Do the op
        mov     %esp, %eax      # Result
        mov     %ecx, %esp      # Stack with return address
        ret

#
# VOID *  pushMemAbs(VOID * stack)
# Push data from an absolute address
        .data
value:       .long  0x01234567
        .text
        .type pushMemAbs, @function
.global pushMemAbs
pushMemAbs:      
        mov     %esp, %ecx      # Save esp
        mov     4(%esp),%esp    # stack to play with
        pushl   value           # Do the op
        mov     %esp, %eax      # Result
        mov     %ecx, %esp      # Stack with return address
        ret
        
#
# VOID *  pushCS(VOID * stack)
# Push CS
        .text
        .type pushCS, @function
.global pushCS
pushCS:      
        mov     %esp, %ecx      # Save esp
        mov     4(%esp),%esp    # stack to play with
        pushl   %cs             # Do the op 
        mov     %esp, %eax      # Result
        mov     %ecx, %esp      # Stack with return address
        ret
        
#
# UINT16  readCS()
# Return the value of %cs
        .text
        .type readCS, @function
.global readCS
readCS:      
        mov     %cs, %eax
        ret

#
# VOID *  pushFS(VOID * stack)
# Push FS
        .text
        .type pushFS, @function
.global pushFS
pushFS:      
        mov     %esp, %ecx      # Save esp
        mov     4(%esp),%esp    # stack to play with
        pushl   %fs             # Do the op
        mov     %esp, %eax      # Result
        mov     %ecx, %esp      # Stack with return address
        ret
        
#
# UINT16  readFS()
# Return the value of %fs
        .text
        .type readFS, @function
.global readFS
readFS:      
        mov     %fs, %eax
        ret
        
