/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
    fprintf(stderr, "Calling sleep\n");
    sleep(10);
    return 0;
}
