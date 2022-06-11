/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This application tests for correctness when a tool calls PIN_ExitApplication/PIN_ExitProcess from within a thread-fini
 * or application-fini callback.
 * The following scenarios are tested:
 * 	1.  Call PIN_ExitApplication from the thread-fini callback of a thread executing its own thread-fini. We expect the
 * 	    test to fail with the error: "It is prohibited to call PIN_ExitApplication() from a thread-fini function".
 * 	2.  Call PIN_ExitApplication from the thread fini callback of a thread in a blocking system call. On Windows the
 * 	    thread-fini callback will be executed by a different OS thread than the one being terminated. We expect the test
 *      to fail with the same error as in scenario #1.
 *  3.  Call PIN_ExitApplication from an application-fini callback. We expect the test to fail with the error:
 *      "It is prohibited to call PIN_ExitApplication() from an application-fini function".
 *  4.  Call PIN_ExitProcess from the thread-fini callback of a thread executing its own thread-fini. We expect the test
 *      to pass.
 *  5.  Call PIN_ExitProcess from the thread-fini callback of a thread in a blocking system call. On Windows the
 *      thread-fini callback will be given by a different OS thread than the one exiting. We expect the test to pass.
 *  6.  Call PIN_ExitProcess from an application-fini callback. We expect the test to pass.
 *
 */

#include <sstream>
#include <cstdlib>
#include "threadUtils.h"

using std::stringstream;

extern "C" EXPORT_SYM volatile void* DoNewThread(void* scenarioPtr);

/**************************************************
 * Secondary thread's main functions              *
 **************************************************/
// The secondary thread simply exits, the tool will do the rest.
volatile void* DoNewThread(void* scenarioPtr)
{
    IncThreads();

    int scenario = *((int*)scenarioPtr);
    switch (scenario)
    {
        case 1:
        case 4:
            return NULL;
        case 2:
        case 3:
        case 5:
        case 6:
            exit(0);
        default:                           // invalid scenario
            ErrorExit(RES_UNKNOWN_OPTION); // never returns
            break;
    }

    // This can't be reached, simply for successful compilation.
    return NULL;
}

/**************************************************
 * Main function	                              *
 **************************************************/
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        ErrorExit(RES_INVALID_ARGS);
    }

    stringstream scenariostream(argv[1]);
    int scenario = 0;
    scenariostream >> scenario;
    if (scenariostream.fail())
    {
        ErrorExit(RES_INVALID_ARGS);
    }

    InitLocks();

    TidType tid;
    if (!CreateNewThread(&tid, (void*)DoNewThread, &scenario))
    {
        ErrorExit(RES_CREATE_FAILED);
    }

    DoSleep(300); // wait here to be terminated

    // Failsafe - this should not be reached but we want to avoid a hung test.
    ErrorExit(RES_EXIT_TIMEOUT); // never returns

    // This can't be reached, simply for successful compilation.
    return 0;
}
