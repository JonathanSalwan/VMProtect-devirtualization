/*
 * Copyright (C) 2014-2017 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "asm_macros.h"

.data
.extern xsaveArea
.extern flags

.text

# void DoXsave();
# This function calls xsave and stores the FP state in the given dst area.
# The caller is expected to allocate enough space for the xsave area.
# The function expects the given dst pointer to be properly aligned for the xsave instruction.
DECLARE_FUNCTION_AS(DoXsave)
DoXsave:
    lea     flags(%rip), %rcx
    mov     (%rcx), %eax
    lea     xsaveArea(%rip), %rcx
    xor     %rdx, %rdx

    # Do xsave
    xsave   (%rcx)

    ret

# void DoXsaveOpt();
# This function calls xsaveopt and stores the FP state in the given dst area.
# The caller is expected to allocate enough space for the xsaveopt area.
# The function expects the given dst pointer to be properly aligned for the xsaveopt instruction.
DECLARE_FUNCTION_AS(DoXsaveOpt)
DoXsaveOpt:
    lea     flags(%rip), %rcx
    mov     (%rcx), %eax
    lea     xsaveArea(%rip), %rcx
    lea     xsaveArea(%rip), %rcx
    xor     %rdx, %rdx

    # Do xsaveopt
    xsaveopt   (%rcx)

    ret

# void DoXrstor();
# This function calls xrstor and restores the specified thetures from the xsave dst area.
# The function expects the given dst pointer to be properly aligned
DECLARE_FUNCTION_AS(DoXrstor)
DoXrstor:
    lea     flags(%rip), %rcx
    mov     (%rcx), %eax
    lea     xsaveArea(%rip), %rcx
    xor     %rdx, %rdx

    # Do xsaveopt
    xrstor  (%rcx)

    ret

# void DoFxsave();
# This function calls fxsave and stores the legacy FP state in the given dst area.
# The caller is expected to allocate enough space for the fxsave area.
# The function expects the given dst pointer to be properly aligned for the xsave instruction.
DECLARE_FUNCTION_AS(DoFxsave)
DoFxsave:
    lea     xsaveArea(%rip), %rcx

    # Do fxsave
    fxsave   (%rcx)

    ret

# void DoFxrstor();
# This function calls fxrstor and restores the legacy FP state fxsave dst area.
# The function expects the given dst pointer to be properly aligned
DECLARE_FUNCTION_AS(DoFxrstor)
DoFxrstor:
    lea     xsaveArea(%rip), %rcx

    # Do fxrstor
    fxrstor   (%rcx)

    ret
