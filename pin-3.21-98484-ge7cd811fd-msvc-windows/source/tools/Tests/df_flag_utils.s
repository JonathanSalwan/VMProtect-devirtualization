/*
 * Copyright (C) 2015-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "asm_macros.h"

DECLARE_FUNCTION(setdf)

.global NAME(setdf)

NAME(setdf):
    std
    ret


DECLARE_FUNCTION(cleardf)

.global NAME(cleardf)

NAME(cleardf):
    cld
    ret
