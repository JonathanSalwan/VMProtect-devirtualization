/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Test that we can read / write a thread's MXCSR register via the Windows CONTEXT.MxCsr field.
 */

#include <windows.h>
#include <iostream>
#include <cstring>

const unsigned MXCSR_INITIAL = 0x1f80; // All exceptions masked
const unsigned MXCSR_NEW     = 0x1d80; // Enable divide-by-zero exceptions

volatile bool TestThreadReady = false;
volatile bool MxcsrChanged    = false;

extern "C" void SetMxcsr(unsigned);
extern "C" unsigned GetMxcsr();

static DWORD WINAPI TestThread(LPVOID);

int main()
{
    HANDLE thd = CreateThread(0, 0, TestThread, 0, 0, 0);
    if (!thd)
    {
        std::cerr << "Unable to create thread" << std::endl;
        return 1;
    }

    while (!TestThreadReady)
        Sleep(0);

    if (SuspendThread(thd) == -1)
    {
        std::cerr << "Error from SuspendThread()" << std::endl;
        return 1;
    }

    CONTEXT ctxt;
    std::memset(&ctxt, 0, sizeof(ctxt));

    // Must specify CONTEXT_CONTROL here to avoid Windows bug on Vista and earlier (?).
    // On those systems, the SetThreadContext() call below will try to write the CONTROL
    // registers even though we don't specify CONTEXT_CONTROL in that call.  Therefore,
    // we read the CONTROL registers here in case the SetThreadContext() call tries to
    // write them back.
    //
    ctxt.ContextFlags = (CONTEXT_FLOATING_POINT | CONTEXT_CONTROL);
    if (GetThreadContext(thd, &ctxt) == 0)
    {
        std::cerr << "Error from GetThreadContext()" << std::endl;
        return 1;
    }

    if (ctxt.MxCsr != MXCSR_INITIAL)
    {
        std::cout << "Read incorrect initial MXCSR value (0x" << std::hex << ctxt.MxCsr << ")" << std::endl;
        return 1;
    }

    ctxt.ContextFlags = CONTEXT_FLOATING_POINT;
    ctxt.MxCsr        = MXCSR_NEW;
    if (SetThreadContext(thd, &ctxt) == 0)
    {
        std::cerr << "Error from SetThreadContext()" << std::endl;
        return 1;
    }
    if (ResumeThread(thd) == -1)
    {
        std::cerr << "Error from ResumeThread()" << std::endl;
        return 1;
    }

    MxcsrChanged = true;
    WaitForSingleObject(thd, INFINITE);

    DWORD ret = 1;
    GetExitCodeThread(thd, &ret);
    return ret;
}

static DWORD WINAPI TestThread(LPVOID)
{
    SetMxcsr(MXCSR_INITIAL);
    TestThreadReady = true;

    // Wait for the main thread to change our MXCSR value.  Note that we do not call Sleep here
    // because we want to force Pin to suspend us outside of a system call.  When the thread is
    // suspended inside a system call, Pin executes the SetThreadContext() syscall directly (instead
    // of emulating it).  The test would still be valid in that case, but it's a better test
    // if it exercises the emulation support in Pin.
    //
    while (!MxcsrChanged)
        ;

    unsigned mxcsr = GetMxcsr();
    if (mxcsr != MXCSR_NEW)
    {
        std::cout << "New MXCSR value (0x" << std::hex << mxcsr << ") is wrong" << std::endl;
        return 1;
    }
    return 0;
}
