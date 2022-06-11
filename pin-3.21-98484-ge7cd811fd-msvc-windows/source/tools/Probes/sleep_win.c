/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

int main()
{
    fprintf(stderr, "Calling Sleep\n");
    Sleep(10);
    return 0;
}
