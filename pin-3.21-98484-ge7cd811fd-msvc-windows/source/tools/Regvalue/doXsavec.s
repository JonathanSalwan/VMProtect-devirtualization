/*
 * Copyright (C) 2015-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <asm_macros.h>

.data
.extern xsaveArea
.extern flags

.text

# void DoXsavec();
# This function calls xsave and stores the FP state in the given dst area.
# The caller is expected to allocate enough space for the xsave area.
# The function expects the given dst pointer to be properly aligned for the xsave instruction.
.global NAME(DoXsavec)
NAME(DoXsavec):

    lea     PIC_VAR(flags), GCX_REG
    mov     (GCX_REG), GAX_REG
    lea     PIC_VAR(xsaveArea), GCX_REG
    xor     GDX_REG, GDX_REG

    # Do xsave
    xsavec   (GCX_REG)

    ret

# void DoXsaveOpt();
# This function calls xsaveopt and stores the FP state in the given dst area.
# The caller is expected to allocate enough space for the xsaveopt area.
# The function expects the given dst pointer to be properly aligned for the xsaveopt instruction.
.global NAME(DoXsaveOpt)
NAME(DoXsaveOpt):

    lea     PIC_VAR(flags), GCX_REG
    mov     (GCX_REG), GAX_REG
    lea     PIC_VAR(xsaveArea), GCX_REG
    xor     GDX_REG, GDX_REG

    # Do xsaveopt
    xsaveopt   (GCX_REG)

    ret

# void DoXrstor();
# This function calls xrstor and restores the specified thetures from the xsave dst area.
# The function expects the given dst pointer to be properly aligned
.global NAME(DoXrstor)
NAME(DoXrstor):

    lea     PIC_VAR(flags), GCX_REG
    mov     (GCX_REG), GAX_REG
    lea     PIC_VAR(xsaveArea), GCX_REG
    xor     GDX_REG, GDX_REG

    # Do xsaveopt
    xrstor   (GCX_REG)

    ret
