/*
 * Copyright (C) 2006-2006 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef TARGET_MAC
#define NAME(x) _##x
#else
#define NAME(x) x
#endif

.text
    .align 4
.globl NAME(DoInt)
NAME(DoInt):
    int     $12
    ret
