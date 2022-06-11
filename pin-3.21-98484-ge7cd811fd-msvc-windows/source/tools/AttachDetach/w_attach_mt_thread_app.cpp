/*
 * Copyright (C) 2017-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <string>
#include <windows.h>

extern "C" __declspec(noinline, dllexport) int PinIsAttached(unsigned int numOfThreads) { return 0; }

using std::fprintf;
using std::string;

const unsigned long num_threads = 10;
HANDLE threadCreatedSemaphore;
HANDLE neverReleasedSemaphore;
volatile bool loop = true;

int ThreadRoutine(LPVOID lpParam)
{
    bool enterEndlessSyscall = *(bool*)lpParam;
    if (!ReleaseSemaphore(threadCreatedSemaphore, // handle to semaphore
                          1,                      // increase count by one
                          NULL))                  // not interested in previous count
    {
        printf("ReleaseSemaphore error: %d\n", GetLastError());
    }

    if (enterEndlessSyscall)
    { // One thread enter endless system call. In order to check test pass in this case as well
        WaitForSingleObject(neverReleasedSemaphore, // handle to semaphore
                            INFINITE);              // wait until signaled
    }

    while (loop)
    {
        void* h = malloc(13);
        if (h)
        {
            free(h);
        }
    }
    return 0;
}

void ThreadCreation()
{
    unsigned long thread_id = 0;
    unsigned long cnt_th    = 0;

    fprintf(stderr, "  App: creating %d additional threads \n", num_threads);

    bool firstThread = true;
    for (cnt_th = 0; cnt_th < num_threads; cnt_th++)
    {
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadRoutine, &firstThread, 0, (LPDWORD)&thread_id);
        WaitForSingleObject(threadCreatedSemaphore, // handle to semaphore
                            INFINITE);              // wait until signaled
        firstThread = false;
    }
}

int main()
{
    threadCreatedSemaphore = CreateSemaphore(NULL,  // default security attributes
                                             0,     // initial count
                                             1,     // maximum count
                                             NULL); // unnamed semaphore
    neverReleasedSemaphore = CreateSemaphore(NULL,  // default security attributes
                                             0,     // initial count
                                             1,     // maximum count
                                             NULL); // unnamed semaphore

    if ((threadCreatedSemaphore == NULL) || (neverReleasedSemaphore == NULL))
    {
        printf("CreateSemaphore error: %d\n", GetLastError());
        return 1;
    }

    //
    // Create Threads
    //
    ThreadCreation();

    //
    // Ready to be attached by Pin. Notify app launcher it can proceed by releasing the below semaphore
    //
    HANDLE readySemaphore;
    std::ostringstream stream;
    stream << GetCurrentProcessId();
    string semaphoreHandleName = "semaphore_handle_" + stream.str();
    readySemaphore             = OpenSemaphore(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, TRUE, semaphoreHandleName.c_str());
    assert(readySemaphore != NULL);

    fprintf(stderr, "  App: ready to be attached by Pin, about to release %s\n", semaphoreHandleName.c_str());
    if (!ReleaseSemaphore(readySemaphore, // handle to semaphore
                          1,              // increase count by one
                          NULL))          // not interested in previous count
    {
        printf("ReleaseSemaphore error: %d\n", GetLastError());
    }

    //
    // Waiting for Pin to attach to the current process
    //
    while (!PinIsAttached(num_threads + 1))
        SwitchToThread();

    loop = false;

    CloseHandle(threadCreatedSemaphore);
    CloseHandle(neverReleasedSemaphore);

    fprintf(stderr, "  App: ended successfully\n");

    return 0;
}
