/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This application tests Pin's graceful termination when the application is terminated by an exception.
 * The application will generate a segmentation fault and we expect to receive the threadFini callbacks
 * on both threads.
 *
 */

#include "threadUtils.h"

/**************************************************
 * Global variables                               *
 **************************************************/
int* segfault                   = 0; // This is a global variable so that the compiler won't optimize it out.
const int numOfSecondaryThreads = 1;
TidType threads[numOfSecondaryThreads];

/**************************************************
 * Secondary thread's main functions              *
 **************************************************/
// The secondary thread spins in a busy wait.
extern "C" EXPORT_SYM void* DoNewThread(void* dummy)
{
    IncThreads();
    while (true)
        ;

    // This can't be reached, simply for successful compilation.
    return NULL;
}

/**************************************************
 * Utility functions                              *
 **************************************************/
extern "C" EXPORT_SYM void doExit() {} // the tool expects to find this function, otherwise the test will fail.

static bool createThreads()
{
    for (int i = 0; i < numOfSecondaryThreads; ++i)
    {
        threads[i] = 0;
        if (!CreateNewThread(&threads[i], (void*)DoNewThread, NULL))
        {
            return false;
        }
    }
    return true;
}

static void waitForThreads()
{
    // Wait for all threads to be created.
    while (NumOfThreads() != numOfSecondaryThreads)
    {
        DoYield();
    }
}

int main(int argc, char* argv[])
{
    InitLocks();

    if (!createThreads())
    { // returns true if all threads were created successfully
        ErrorExit(RES_CREATE_FAILED);
    }
    waitForThreads(); // wait for the secondary thread(s) to be created

    *segfault = 5;
    ErrorExit(RES_UNEXPECTED_EXIT);

    // This can't be reached, simply for successful compilation.
    return 0;
}
