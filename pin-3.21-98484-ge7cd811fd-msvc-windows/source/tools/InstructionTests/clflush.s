/*
 * Copyright (C) 2017-2017 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <asm_macros.h>

.data
.extern funcPtr

.text

DECLARE_FUNCTION(ClFlushFunc)
DECLARE_FUNCTION(ClFlushOptFunc)
DECLARE_FUNCTION(ClwbFunc)

# void ClFlushFunc();
# This function calls clflush
.global NAME(ClFlushFunc)
NAME(ClFlushFunc):

    mov       PIC_VAR(funcPtr), GCX_REG
    #clflush  (GCX_REG)
    .byte     0x0F, 0xAE, 0x39

    ret

# void ClFlushOptFunc();
# This function calls clflushopt
.global NAME(ClFlushOptFunc)
NAME(ClFlushOptFunc):

    mov          PIC_VAR(funcPtr), GCX_REG
    #clflushopt  (GCX_REG)
    .byte        0x66, 0x0F, 0xAE, 0x39

    ret

# void ClwbFunc();
# This function calls clwb
.global NAME(ClwbFunc)
NAME(ClwbFunc):

    mov      PIC_VAR(funcPtr), GCX_REG
    #clwb    (GCX_REG)
    .byte    0x66, 0x0F, 0xAE, 0x31

    ret
