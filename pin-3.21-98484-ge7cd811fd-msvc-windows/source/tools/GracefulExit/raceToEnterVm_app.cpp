/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test creates a race condition in which multiple threads race to enter the VM
 * while the main thread attempts to stop them in the thread depot. This condition
 * should not cause the process to hang.
 * The test is run with no tool.
 */

#include <Windows.h>
#include <cassert>
#include <iostream>

using std::cerr;
using std::endl;

HANDLE hEvent; // the secondary threads will wait on this event

// main function of the secondary threads
static DWORD WINAPI SecondaryThread(LPVOID lpParameter)
{
    // The threads will wait in a blocking system call until released for the first time.
    // Then, the event stays signaled, so we get a busy wait. Every iteration, the threads
    // enter the kernel and exit immediately, causing them to race back in to the VM.
    while (TRUE)
    {
        WaitForSingleObject(hEvent, INFINITE);
    }
}

int main()
{
    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL); // manual reset event
    cerr << "APP: Creating threads." << endl;
    for (int i = 0; i < 10; ++i)
    {
        DWORD tid;
        assert(CreateThread(NULL, 0, SecondaryThread, NULL, 0, &tid));
    }
    Sleep(2 * 1000);
    cerr << "APP: Releasing threads and exiting..." << endl;
    SetEvent(hEvent);
    return 0;
}
