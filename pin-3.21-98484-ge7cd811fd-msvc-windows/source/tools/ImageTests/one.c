/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

#if defined(__cplusplus)
extern "C"
#endif

#ifdef TARGET_WINDOWS
    __declspec(dllexport)
#endif
        int one()
{
    // make the literal 1 be part of the code
    fprintf(stderr, "%d\n", 1);

    return 1;
}
