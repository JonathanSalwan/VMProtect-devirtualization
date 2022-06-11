/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <Windows.h>
#include <stdio.h>
#define EXPORT_SYM __declspec(dllexport)

int dummy;

// Our pintool notices when we enter this function, and replays the same exception as before.
void EXPORT_SYM ReplayException() { dummy = 1; }

// Our pintool notices when we enter this function, so it knows the next exception it is notified about is
// the one caused in the __try of main.
void EXPORT_SYM ReadyForExceptionFromAppMain() { dummy = 1; }

int main(int argc, char** argv)
{
    ReadyForExceptionFromAppMain();
    __try
    {
        // Explicitly raising an exception does not cause a kernel exception in some versions
        // of Windows, and therefore is invisible to Pin.
        // RaiseException(99, 22, 0, 0);

        // To guarantee that we get a kernel exception we have to be more drastic...
        *(int volatile*)0 = 0;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        printf("Inner Exception code 0x%x\n", GetExceptionCode());
    }

    // Our tool reraises the same exception when ReplayException is
    // called, though it then terminates, because the process is in a funny state
    // at that point, and we can't easily continue to execute here.
    // Since the point of the test is to verify that the relevant Pin callbacks
    // are made, that doesn't matter.
    ReplayException();

    // We should never get here. If we do something is wrong.
    return 1;
}
