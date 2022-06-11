/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

#if defined(__cplusplus)
extern "C"
#endif
    int
    one()
{
    // make the literal 2 be part of the code
    fprintf(stderr, "%d\n", 2);

    return 2;
}
