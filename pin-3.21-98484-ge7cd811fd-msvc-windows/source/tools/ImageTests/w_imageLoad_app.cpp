/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <windows.h>
#include <string>
#include <iostream>

#define EXPORT_SYM extern "C" __declspec(dllexport)

EXPORT_SYM int AfterAttach();

const char* FIRST_DLL_NAME = "my_dll.dll";

const char* SECOND_DLL_NAME = "my_dll_1.dll";

enum ExitType
{
    RES_SUCCESS = 0, //0
    RES_LOAD_FAILED, //1
};

/**************************************************/

void WindowsOpen(const char* filename)
{
    HMODULE hdll = LoadLibrary(filename);
    if (hdll == NULL)
    {
        fflush(stderr);
        exit(RES_LOAD_FAILED);
    }
    FreeLibrary(hdll);
}

int AfterAttach()
{
    // Pin sets an anslysis function here to notify the application when Pin attaches to it.
    return 0;
}

int main()
{
    WindowsOpen(FIRST_DLL_NAME);
    while (!AfterAttach())
    {
        Sleep(1 * 1000);
    }
    WindowsOpen(SECOND_DLL_NAME);
    return RES_SUCCESS;
}
