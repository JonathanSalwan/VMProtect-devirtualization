/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <windows.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    HMODULE hMod = LoadLibrary("dummy_dll.dll");

    HMODULE hMod2 = LoadLibrary("dummy_dll2.dll");

    return 0;
}
