/*
 * Copyright (C) 2015-2015 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
  This application causes exception in destructor of global object in a dynamically linked DLL
  after main thread called ExitProcess().
  The application is instrumented with active PinADX.
  The exception should not cause application hang after PinADX is disabled on exit.
*/
#include <Windows.h>

extern "C" __declspec(dllimport) int Something();

// main function of the secondary threads
static DWORD WINAPI SecondaryThread(LPVOID lpParameter)
{
    // Infinite loop.
    for (;;)
        ;
    return 0;
}

int main()
{
    CreateThread(NULL, 0, SecondaryThread, NULL, 0, NULL);
    Sleep(1000);
    Something();
    // Tell the process to exit immediately.
    // Destructor of myObj is not yet called.
    // It will be called after Pin starts exit flow.
    ExitProcess(0);
    return 0;
}
