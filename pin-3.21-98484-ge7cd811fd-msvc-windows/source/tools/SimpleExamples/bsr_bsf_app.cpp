/*
 * Copyright (C) 2015-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>

extern "C" int bsr_func(int);
extern "C" int bsf_func(int);

int main()
{
    // Call 'bsr_func' and 'bsf_func' implemented in assembly
    printf("BSR of 0 is %d\n", bsr_func(0));
    printf("BSR of 888 is %d\n", bsr_func(888));
    printf("BSF of 0 is %d\n", bsf_func(0));
    printf("BSF of 888 is %d\n", bsf_func(888));
    return 0;
}
