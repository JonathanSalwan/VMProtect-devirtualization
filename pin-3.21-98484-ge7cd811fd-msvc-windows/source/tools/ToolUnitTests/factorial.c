/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef TARGET_WINDOWS
#define EXPORT_CSYM __declspec(dllexport)
#else
#define EXPORT_CSYM
#endif

EXPORT_CSYM int factorial(int n)
{
    if ((n < 0) || (n > 30))
    {
        return 0;
    }
    else if (n == 0)
    {
        return 1;
    }
    else
    {
        return n * factorial(n - 1);
    }
}

int main(int argc, char* argv[])
{
    int n;
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s number_string\n", argv[0]);
        return 1;
    }
    n = atoi(argv[1]);
    fprintf(stdout, "%d!=%d\n", n, factorial(n));
    return 0;
}
