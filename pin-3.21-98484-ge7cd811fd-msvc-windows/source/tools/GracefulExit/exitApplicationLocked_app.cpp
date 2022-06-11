/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This application tests for graceful exit when a tool calls PIN_ExitApplication while holding the client lock. The main thread
 * waits in a blocking system call to be terminated. The secondary thread will never execute its code since the tool will call
 * PIN_ExitApplication from the thread-start callback.
 *
 */

#include <sstream>
#include <cstdlib>
#include "threadUtils.h"

using std::stringstream;

/**************************************************
 * Secondary thread's main functions              *
 **************************************************/
// The secondary thread simply exits, the tool will do the rest.
static void* DoNewThread(void* v)
{
    IncThreads();
    ErrorExit(RES_EXIT_FAILED); // never returns
    return NULL;                // simply for successful compilation
}

/**************************************************
 * Main function                                  *
 **************************************************/
int main(int argc, char* argv[])
{
    InitLocks();

    TidType tid;
    if (!CreateNewThread(&tid, (void*)DoNewThread, NULL))
    {
        ErrorExit(RES_CREATE_FAILED);
    }

    DoSleep(300); // wait here to be terminated

    // Failsafe - this should not be reached but we want to avoid a hung test.
    ErrorExit(RES_EXIT_TIMEOUT); // never returns

    // This can't be reached, simply for successful compilation.
    return 0;
}
