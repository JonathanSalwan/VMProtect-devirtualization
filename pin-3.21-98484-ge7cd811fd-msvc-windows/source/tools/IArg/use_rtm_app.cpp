/*
 * Copyright (C) 2018-2018 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <immintrin.h>

int main()
{
    int temp = 5;
    if (_xbegin() == _XBEGIN_STARTED)
    {
        temp++;
        _xend();
    }
    else
    {
        temp--;
    }
    printf("temp is %d\n", temp);
    return 0;
}
