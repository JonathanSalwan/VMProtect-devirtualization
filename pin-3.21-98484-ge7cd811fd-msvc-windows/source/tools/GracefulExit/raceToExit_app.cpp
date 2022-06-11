/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies that no new threads register in the depot after the termination request. If that happens,
 * the test should hang until the timeout and the result would be the RES_EXIT_TIMEOUT error. Also, we check
 * that the number of threadStart callbacks equals the number of threadFini callbacks. There is no requirement
 * that all threads will be created before termination.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "threadUtils.h"

/**************************************************
 * Global variables                               *
 **************************************************/
const int numOfFirstCreateLoop  = 30;
const int numOfSecondCreateLoop = 50;
const int numOfSecondaryThreads = numOfFirstCreateLoop + numOfSecondCreateLoop + 1; // Total number of secondary threads.
TidType threads[numOfSecondaryThreads];

/**************************************************
 * Utility functions                              *
 **************************************************/
extern "C" EXPORT_SYM void doExit() {} // the tool expects to find this function, otherwise the test will fail.

static bool createThreads(int numOfThreads, void* func)
{
    for (int i = 0; i < numOfThreads; ++i)
    {
        threads[i] = 0;
        if (!CreateNewThread(&threads[i], func, NULL))
        {
            return false;
        }
    }
    return true;
}

/**************************************************
 * Secondary threads' main functions              *
 **************************************************/
void* DoExitThread(void* dummy)
{
    Print("Exit thread is now going to exit");
    IncThreads();
    exit(RES_SUCCESS);
}

void* DoDummyThread(void* dummy)
{
    IncThreads();
    DoYield();
    return NULL;
}

/**************************************************
 * Main                                           *
 **************************************************/
int main()
{
    InitLocks();

    if (!createThreads(numOfFirstCreateLoop, (void*)DoDummyThread))
    { // create first batch of dummy threads
        ErrorExit(RES_CREATE_FAILED);
    }
    Print("Creating the exit thread");
    if (!createThreads(1, (void*)DoExitThread))
    { // create the exit thread
        ErrorExit(RES_CREATE_FAILED);
    }
    if (!createThreads(numOfSecondCreateLoop, (void*)DoDummyThread))
    { // create second batch of dummy threads
        ErrorExit(RES_CREATE_FAILED);
    }

    DoSleep(1000); // wait here to be terminated

    // Failsafe - this should not be reached but we want to avoid a hung test.
    ErrorExit(RES_EXIT_TIMEOUT); // never returns

    // This can't be reached, simply for successful compilation.
    return RES_SUCCESS;
}
