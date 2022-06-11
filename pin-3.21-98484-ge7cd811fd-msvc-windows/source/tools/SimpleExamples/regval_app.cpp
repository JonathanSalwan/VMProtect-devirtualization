/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#if defined(TARGET_WINDOWS)
#include <Windows.h>
#define EXPORT_SYM __declspec(dllexport)
#else
#define EXPORT_SYM
#endif

// Empty function - this is simply a placeholder for the tool to start printing register values.
extern "C" EXPORT_SYM void Start()
{
    // do nothing
}

// Empty function - this is simply a placeholder for the tool to stop printing register values.
extern "C" EXPORT_SYM void Stop()
{
    // do nothing
}

// Do some calculations and let the tool print the registers.
long double Work()
{
    int inta = 1;
    int intb = 2;
    int intc = inta - intb;
    int intd = intb - inta;
    int inte = intc * intd;

    long double lda = 1234.5678;
    long double ldb = 8765.4321;
    long double ldc = lda / ldb;
    long double ldd = ldb / lda;
    long double lde = ldc + ldd;
    return (lde * (long double)inte);
}

int main()
{
    Start();
    Work();
    Stop();
}
