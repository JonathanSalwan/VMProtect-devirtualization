/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This application only makes sense when run with the "intercept-tool" Pin tool.  It's
 * part of a test for PIN_InterceptDebuggingEvent().
 */

#include <iostream>

typedef int (*FPTR_INTRET)();
typedef void (*FPTR_VOIDRET)();

extern "C" int Checkpoint();
extern "C" void Breakpoint1();
extern "C" int Breakpoint2();

int main()
{
    // Call these functions through volatile pointers to prevent the compiler from
    // in-lining them.  The tool places instrumentation points in them, and we want
    // to make sure the application calls the out-of-line versions.
    //
    volatile FPTR_INTRET checkpoint   = Checkpoint;
    volatile FPTR_VOIDRET breakpoint1 = Breakpoint1;
    volatile FPTR_INTRET breakpoint2  = Breakpoint2;

    // The Pin tool takes a snapshot in the Checkpoint() function and causes the
    // application to restart from this snapshot, however Checkpoint() returns
    // a different value for each restart.
    //
    int checkpointVal = checkpoint();

    if (checkpointVal == 0)
    {
        // Control gets here after the first return from Checkpoint().  The debugger
        // places a breakpoint in Breakpoint1().  However, the tool squashes the
        // breakpoint event and causes the application to resume from the Checkpoint()
        // snapshot.
        //
        breakpoint1();

        // Control should not get here because the tool intercepts the breakpoit above.
        //
        std::cout << "FAILURE: Tool did not intercept breakpoint" << std::endl;
        return 1;
    }

    if (checkpointVal == 1)
    {
        // Control gets here after the second return from Checkpoint().
        //
        std::cout << "SUCCESS: Tool intercepted and redirected breakpoint #1" << std::endl;

        // The debugger places a breakpoint in Breakpoint2().  However, the tool
        // intercepts the breakpoint event and changes the return value from Breakpoint2()
        // to be "1".  The body of the "if" should execute when the debugger resumes from
        // the breakpoint.
        //
        if (breakpoint2() == 1)
        {
            std::cout << "SUCCESS: Tool intercepted breakpoint #2" << std::endl;
            return 0;
        }

        // If control gets here, either the tool did not intercept the breakpoint, or
        // the tool didn't change the return value from Breakpoint2().
        //
        std::cout << "FAILURE: Tool did not intercept breakpoint #2" << std::endl;
        return 1;
    }

    std::cout << "FAILURE: Should not get here" << std::endl;
    return 1;
}

extern "C" int Checkpoint() { return 0; }

extern "C" void Breakpoint1() {}
