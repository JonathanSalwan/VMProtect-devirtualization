/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <windows.h>

int main()
{
    HMODULE hm = LoadLibraryEx("X86dll.dll", NULL, DONT_RESOLVE_DLL_REFERENCES);
    return (hm == NULL) ? 1 : 0;
}
