/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "pin.H"

const UINT32 period = 10000;

// The running bbl count is kept here
UINT32 acount = 0;
UINT32 pcount = 0;

THREADID myThread = INVALID_THREADID;

ADDRINT IfMyThread(THREADID threadId) { return threadId == myThread; }

ADDRINT Always(THREADID threadId)
{
    if (!IfMyThread(threadId))
    {
        return false;
    }
    ++acount;
    return acount == period;
}

VOID Rare()
{
    ++pcount;
    acount = 0;
}

UINT32 ccount = 0;
UINT32 rcount = 0;

ADDRINT AlwaysNoinline(THREADID threadId)
{
    if (!IfMyThread(threadId))
    {
        return false;
    }
    ++ccount;
    if (ccount == 1000000) printf("Should not get here\n");
    return ccount == period;
}

VOID RareNoinline()
{
    assert(ccount == period);
    ++rcount;
    if (ccount == 1000000) printf("Should not get here\n");
    ccount = 0;
}

UINT32 bcount = 0;
UINT32 qcount = 0;

VOID Noinline(THREADID threadId)
{
    if (!IfMyThread(threadId))
    {
        return;
    }
    ++bcount;

    if (bcount == period)
    {
        ++qcount;
        bcount = 0;
    }
}

static UINT32 mcount;

ADDRINT ReadAlways(THREADID threadId)
{
    if (!IfMyThread(threadId))
    {
        return false;
    }
    mcount++;
    return mcount == 1000;
}

VOID ReadRare()
{
    if (mcount != 1000)
    {
        printf("Mcount %d\n", mcount);
        exit(1);
    }

    mcount = 0;
}

// Pin calls this function every time a new basic block is encountered
// It inserts a call to docount
VOID Trace(TRACE trace, VOID* v)
{
    // Visit every basic block in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            if (INS_IsMemoryRead(ins))
            {
                INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)ReadAlways, IARG_THREAD_ID, IARG_END);
                INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)ReadRare, IARG_END);
            }
        }

        // Always()->Rare() are partially inlined
        BBL_InsertIfCall(bbl, IPOINT_BEFORE, (AFUNPTR)Always, IARG_THREAD_ID, IARG_END);
        BBL_InsertThenCall(bbl, IPOINT_BEFORE, (AFUNPTR)Rare, IARG_END);

        // Always()->Rare() are partially inlined
        BBL_InsertIfCall(bbl, IPOINT_BEFORE, (AFUNPTR)AlwaysNoinline, IARG_THREAD_ID, IARG_END);
        BBL_InsertThenCall(bbl, IPOINT_BEFORE, (AFUNPTR)RareNoinline, IARG_END);

        // Noinline() is not inlined
        BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)Noinline, IARG_THREAD_ID, IARG_END);
    }
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v)
{
    if (pcount * period + acount != qcount * period + bcount || pcount * period + acount != rcount * period + ccount)
    {
        fprintf(stderr,
                "Counts NOT matched:\n"
                "partial-inline   count=%d (pcount=%d acount=%d),\n"
                "partial-noinline count=%d (rcount=%d ccount=%d),\n"
                "noninline        count=%d (qcount=%d bcount=%d)\n",
                pcount * period + acount, pcount, acount, rcount * period + ccount, rcount, ccount, qcount * period + bcount,
                qcount, bcount);
        exit(1);
    }
}

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    if (INVALID_THREADID == myThread)
    {
        myThread = threadid;
    }
}

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    // Initialize pin
    PIN_Init(argc, argv);

    // Register Instruction to be called to instrument instructions
    TRACE_AddInstrumentFunction(Trace, NULL);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, NULL);

    PIN_AddThreadStartFunction(ThreadStart, NULL);

    // Start the program, never returns
    PIN_StartProgram();

    return 1;
}
