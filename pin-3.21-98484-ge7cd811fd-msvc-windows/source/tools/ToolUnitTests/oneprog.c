/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

#ifdef TARGET_WINDOWS
// declare all functions as exported so pin can find them,
// must be all functions since only way to find end of one function is the begining of the next
// Another way is to compile application with debug info (Zi) - pdb file, but that causes probelms
// in the running of the script
#define EXPORT_SYM __declspec(dllexport)
#else
#define EXPORT_SYM
#endif
#include <stdio.h>
EXPORT_SYM
int one() { return 1; }

EXPORT_SYM
int main()
{
    int o = one();
    printf("Result %d\n", o);
    return 0;
}
