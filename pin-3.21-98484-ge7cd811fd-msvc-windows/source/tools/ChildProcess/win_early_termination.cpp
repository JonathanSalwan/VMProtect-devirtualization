/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <Dbghelp.h>

using std::cout;
using std::endl;
using std::flush;

extern "C" __declspec(dllimport) int Something();

//this application has static linking with dll which terminate the process in it's DllMain(PROCESS_ATTACH)
int main()
{
    //should never get here
    cout << "should never get here" << endl << flush;
    volatile int i = Something();
    HANDLE token;
    OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &token);
    GetTimestampForLoadedLibrary(GetModuleHandle("advapi32.dll"));

    exit(-1);
    return 0;
}
