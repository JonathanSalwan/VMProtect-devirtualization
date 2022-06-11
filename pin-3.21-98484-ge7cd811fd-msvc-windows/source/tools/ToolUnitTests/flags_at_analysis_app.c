/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

#if !defined(TARGET_WINDOWS)

#define EXPORT_SYM

#else //defined(TARGET_WINDOWS)

#include <windows.h>
// declare all functions as exported so pin can find them,
// must be all functions since only way to find end of one function is the begining of the next
// Another way is to compile application with debug info (Zi) - pdb file, but that causes probelms
// in the running of the script
#define EXPORT_SYM __declspec(dllexport)

#endif
void SetOfFlag_asm();
EXPORT_SYM
int main()
{
    SetOfFlag_asm();
    exit(0);
}
