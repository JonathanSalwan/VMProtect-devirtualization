/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 */

#include "pin.H"
#include "tool_macros.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <signal.h>
#include <sched.h>
using std::cerr;
using std::endl;

BOOL changeSigmask = FALSE;

INT32 Usage()
{
    cerr << "This pin tool examines the correctness of the retrieve and alteration of the thread sigmask"
            "when the tool registers a THREAD_ATTACH_PROBED_CALLBACK callback.\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

// Notify the application when the sigmask has changed.
BOOL Replacement_waitChangeSigmask()
{
    while (changeSigmask == 0)
        sched_yield();
    return TRUE;
}

VOID AttachedThreadStart(void* sigmask, VOID* v)
{
    sigset_t* sigset = (sigset_t*)sigmask;

    /* 
     *    change the sigmask of the thread whose sigmask contains SIGUSR2 and doen't contain SIGUSR1
     */
    if (0 != sigismember(sigset, SIGUSR2) && (0 == sigismember(sigset, SIGUSR1)))
    {
        sigdelset(sigset, SIGUSR2);
        sigaddset(sigset, SIGUSR1);
    }

    changeSigmask = TRUE;
}

// Image load callback for the first Pin session
VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        RTN rtn = RTN_FindByName(img, C_MANGLE("WaitChangeSigmask"));

        // Relevant only in the attach scenario.
        if (RTN_Valid(rtn))
        {
            if (RTN_IsSafeForProbedReplacement(rtn)) RTN_ReplaceProbed(rtn, AFUNPTR(Replacement_waitChangeSigmask));
        }
    }
}

/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }
    PIN_InitSymbols();
    IMG_AddInstrumentFunction(ImageLoad, 0);
    PIN_AddThreadAttachProbedFunction(AttachedThreadStart, 0);
    PIN_StartProgramProbed();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
