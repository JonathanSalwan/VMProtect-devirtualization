/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#if defined(TARGET_WINDOWS)
#define EXPORT_CSYM extern "C" __declspec(dllexport)
#else
#define EXPORT_CSYM extern "C"
#endif

EXPORT_CSYM int fp_func3(int i1, float f2, int i3)
{
    printf("fp_func3 parameters: %d, %f, %d\n", i1, f2, i3);
    fflush(stdout);
    return i1 + i3;
}

EXPORT_CSYM float fp_func4(double d1, float f2, int i3, double d4)
{
    printf("fp_func4 parameters: %e, %f, %d, %e\n", d1, f2, i3, d4);
    fflush(stdout);
    if (d4 < 6) return f2 - 0.4;
    return f2 + 0.1;
}

EXPORT_CSYM double fp_func1(double d1)
{
    if (d1 > 0.5)
    {
        return d1 - 0.1;
    }
    return d1 + 0.1;
}

int main()
{
    int ires = fp_func3(8, 0.53, 9);
    printf("fp_func3 result %d\n\n", ires);
    fflush(stdout);

    float fres = fp_func4(0.33333334444, 0.45, 5, 23.7878787878);
    printf("fp_func4 result %f\n\n", fres);
    fflush(stdout);

    double dres = fp_func1(0.33333334444);
    printf("fp_func1 result %e\n\n", dres);
    fflush(stdout);

    return 0;
}
