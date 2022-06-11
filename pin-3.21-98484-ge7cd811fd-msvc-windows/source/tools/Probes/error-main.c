/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//  this application fails so we can check errno.

#include <stdio.h>

#if defined(TARGET_WINDOWS)
#include <windows.h>
#define EXPORT_SYM __declspec(dllexport)
#else
#include <errno.h>
#define EXPORT_SYM extern
#endif

EXPORT_SYM void CheckError();

int main(int argc, char* argv[])
{
    FILE* f = fopen("non-existent-file", "r");

    if (!f)
    {
        CheckError();
        fprintf(stdout, "App: cannot open non-existent file\n");
        return 0;
    }
}
