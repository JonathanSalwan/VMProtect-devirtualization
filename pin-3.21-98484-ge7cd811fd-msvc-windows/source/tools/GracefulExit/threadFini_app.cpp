/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This application creates 6 threads including the main thread. We check 5 different exit scenarios (depending on the
 * parameter passed to the application) and expect all threadFini callbacks to be given. During the test, some threads
 * wait (busy-wait or sleep), while others exit without terminating the application.
 *
 * The exit scenarios are as follows:
 * 0. Tool calls PIN_ExitApplication on an application thread.
 * 1. Tool calls PIN_ExitApplication on an internal thread.
 * 2. Secondary thread calls exit().
 * 3. Main thread calls exit().
 * 4. Main thread exits normally with "return".
 *
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "threadUtils.h"

/**************************************************
 * enums and typedefs                             *
 **************************************************/
enum ExitType
{

    EXIT_TOOL_APP = 0,  // the tool will call PIN_ExitApplication in the application's main thread
    EXIT_TOOL_INTERNAL, // the tool will call PIN_ExitApplication in an internal thread
    EXIT_SEC_EXIT,      // the application's ROLE_EXIT thread will call exit
    EXIT_MAIN_EXIT,     // the application's main thread will call exit
    EXIT_RETURN         // the application's main thread will call return
};

enum ThreadRole
{
    ROLE_WAIT = 0, // waits in a blocking system call
    ROLE_LOOP,     // waits in a busy wait (without sched_yield) - this simulates a thread in a running state
    ROLE_FINISH,   // exits normally by calling return from its main function
    ROLE_EXIT,     // This thread has two ways to exit depending on a given parameter:
                   //  1. exit using the pthread_exit function which only terminates the thread but not the process
                   //  2. exit via the exit() system call which terminates the entire process
    ROLE_CANCELED, // waits in a blocking system call to be terminated via pthread_cancel
    ROLE_SIZE      // used for iteration
};

/**************************************************
 * Global variables                               *
 **************************************************/
ExitType exitType           = EXIT_TOOL_APP; // Defines how the application will exit:
volatile bool readyToCancel = false;
TidType threads[ROLE_SIZE];

// These variables are used as arguments to the secondary threads' main functions, therefore they can't be temporary
// and must be defined throughout the entire program. This is simpler than allocating them dynamically.
ThreadRole roles[ROLE_SIZE] = {ROLE_WAIT, ROLE_LOOP, ROLE_FINISH, ROLE_EXIT, ROLE_CANCELED};

const string roleStrings[ROLE_SIZE] = {"WAIT", "LOOP", "FINISH", "EXIT", "CANCELED"};

/**************************************************
 * Function declarations                          *
 **************************************************/
extern "C" EXPORT_SYM volatile void doExit(bool appThread);
extern "C" EXPORT_SYM volatile void* DoNewThread(void* threadNumArg);

/**************************************************
 * Utility functions                              *
 **************************************************/
static bool createThreads()
{
    for (int i = 0; i < ROLE_SIZE; ++i)
    {
        threads[i] = 0;
        if (!CreateNewThread(&threads[i], (void*)DoNewThread, &roles[i]))
        {
            return false;
        }
    }
    return true;
}

/**************************************************
 * Secondary threads main function                *
 **************************************************/
volatile void* DoNewThread(void* threadNumArg)
{
    int threadNum = *((int*)threadNumArg);
    Print(roleStrings[threadNum] + " thread was created succesfully."); // FOR DEBUG
    IncThreads();
    switch (threadNum)
    {
        case ROLE_WAIT:
            DoSleep(1000);
            ErrorExit(RES_EXIT_TIMEOUT); // the ROLE_WAIT thread reached its timeout of 1000 seconds - something is wrong

        case ROLE_LOOP:
            while (1)
                ;                           // never exits from here
            ErrorExit(RES_UNEXPECTED_EXIT); // Should be never called

        case ROLE_FINISH:
            break; // exits normally, process continues

        case ROLE_EXIT:
            if (exitType == EXIT_SEC_EXIT)
            {
                while (NumOfThreads() != ROLE_SIZE)
                {
                    DoYield(); // wait here until all threads are ready
                }
                Print("ROLE_EXIT thread is calling exit()"); // FOR DEBUG

                // exit and kill the entire process
                exit(RES_SUCCESS); // never returns
            }
            // thread exits but process continues
            ThreadExit(); // never returns

        case ROLE_CANCELED:
            // On Windows, this thread will be canceled using the TerminateThread API. The documentation states that
            // it is unsafe to use this function if the target thread is executing certain kernel32 calls etc.
            // On Linux, the thread must enter a function which is defined as a cancellation point. See pthreads entry
            // in the chapter 7 of the manual for further details.
            readyToCancel = true;
            EnterSafeCancellationPoint();
    }

    return NULL;
}

/**************************************************
 * Main thread's functions                        *
 **************************************************/
static void parseArgs(int argc, const char* argv[])
{
    if (argc != 2)
    {
        ErrorExit(RES_INVALID_ARGS);
    }
    if (strcmp(argv[1], "0") == 0)
    {
        exitType = EXIT_TOOL_APP;
    }
    else if (strcmp(argv[1], "1") == 0)
    {
        exitType = EXIT_TOOL_INTERNAL;
    }
    else if (strcmp(argv[1], "2") == 0)
    {
        exitType = EXIT_SEC_EXIT;
    }
    else if (strcmp(argv[1], "3") == 0)
    {
        exitType = EXIT_MAIN_EXIT;
    }
    else if (strcmp(argv[1], "4") == 0)
    {
        exitType = EXIT_RETURN;
    }
    else
    {
        ErrorExit(RES_UNKNOWN_OPTION);
    }
}

static void waitForThreads()
{
    // Wait for all threads to be created.
    while (NumOfThreads() != ROLE_SIZE)
    {
        DoYield();
    }

    // Wait for the ROLE_CANCELED thread to be ready for cancellation
    while (!readyToCancel)
    {
        DoYield();
    }

    // If the secondary thread calls exit, the internal data structures used by the utility functions
    // (e.g. CancelThread, WaitForThread etc.) may become invalid at any point. Therefore calling these
    // functions is not safe so simply return.
    if (EXIT_SEC_EXIT == exitType) return;

    // Send a cancel request to the ROLE_CANCELED thread.
    CancelThread(threads[ROLE_CANCELED]);

    // Wait for all threads that are supposed to exit.
    WaitForThread(threads[ROLE_FINISH]);
    WaitForThread(threads[ROLE_EXIT]);

    // The timing of the effect of pthread_cancel is undefined, this could cause the aplication to hang.
    // Currently commented out.
    WaitForThread(threads[ROLE_CANCELED]);
}

// The tool will kill the process, either by the main thread or an internal tool thread.
void volatile doExit(bool appThread)
{
    Print("main thread is in doExit");
    while (1)
    { // wait here until the tool kills the process
        DoYield();
    }
}

static void waitOrExit()
{
    volatile void (*fn)(bool) = &doExit; // This trick prevents GCC 3.4.6 to opt out this function
    switch (exitType)
    {
        case EXIT_TOOL_APP:
            (*fn)(true); // never returns
            break;

        case EXIT_TOOL_INTERNAL:
            (*fn)(false); // never returns
            break;

        case EXIT_SEC_EXIT:
            while (1)
            {
                DoYield(); // wait here for the ROLE_EXIT thread to call exit()
            }
            break;

        case EXIT_MAIN_EXIT:
            Print("main thread is calling exit()"); // FOR DEBUG
            exit(RES_SUCCESS);
            break;

        case EXIT_RETURN:
            return;
    }
}

/**************************************************
 * Main                                           *
 **************************************************/
int main(int argc, const char* argv[])
{
    parseArgs(argc, argv);

    InitLocks();

    // Print("main thread starting the test..."); // FOR DEBUG

    SetTimeout();

    if (!createThreads())
    { // returns true if all threads were created successfully
        ErrorExit(RES_CREATE_FAILED);
    }

    waitForThreads();
    waitOrExit();

    Print("main thread is calling return from main()"); // FOR DEBUG
    return RES_SUCCESS;
}
