/*
 * Copyright (C) 2015-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <asm_macros.h>

.data
.extern xsaveArea


.text

DECLARE_FUNCTION_AS(Do_Fxsave)
Do_Fxsave:
    lea     PIC_VAR(xsaveArea), GCX_REG
    fxsave (GCX_REG)
    ret
END_FUNCTION(Do_Fxsave)
    
DECLARE_FUNCTION_AS(Do_Fxrstor)
Do_Fxrstor:
    lea     PIC_VAR(xsaveArea), GCX_REG
    fxrstor (GCX_REG)
    ret
END_FUNCTION(Do_Fxrstor)
