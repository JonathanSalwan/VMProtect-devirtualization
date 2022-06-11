/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <windows.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    HMODULE hMod = LoadLibrary("dummy_dll.dll");
    FreeLibrary(hMod);

    hMod = LoadLibrary("dummy_dll.dll");
    FreeLibrary(hMod);

    return 0;
}
