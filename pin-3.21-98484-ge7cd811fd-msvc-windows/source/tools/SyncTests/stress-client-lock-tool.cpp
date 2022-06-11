/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <sched.h>
#include "pin.H"

using std::cerr;
using std::cout;
using std::endl;
using std::flush;

typedef void (*AppFunSig)(unsigned int, pid_t);
AppFunSig OrigThreadInit = NULL;
AppFunSig OrigThreadFini = NULL;

static RTN GetProbableRtn(IMG img, const char* name)
{
    RTN rtn = RTN_FindByName(img, name);
    if (!RTN_Valid(rtn))
    {
        cerr << "TOOL ERROR: Unable to find the function \"" << name << "\"" << endl;
        PIN_ExitProcess(101);
    }
    if (!RTN_IsSafeForProbedInsertion(rtn))
    {
        cerr << "TOOL ERROR: The function \"" << name << "\" is not safe for probe insertion" << endl;
        PIN_ExitProcess(102);
    }
    return rtn;
}

static RTN GetReplacableRtn(IMG img, const char* name)
{
    RTN rtn = RTN_FindByName(img, name);
    if (!RTN_Valid(rtn))
    {
        cerr << "TOOL ERROR: Unable to find the function \"" << name << "\"" << endl;
        PIN_ExitProcess(103);
    }
    if (!RTN_IsSafeForProbedReplacement(rtn))
    {
        cerr << "TOOL ERROR: The function \"" << name << "\" is not safe for replacement" << endl;
        PIN_ExitProcess(104);
    }
    return rtn;
}

static void OnSecondaryThreadInit(unsigned int threadNum, pid_t tid)
{
    PIN_LockClient();
    if (NULL == OrigThreadInit)
    {
        cerr << "TOOL ERROR: Attempting to call SecondaryThreadInit but it is a NULL pointer" << endl;
        PIN_ExitProcess(105);
    }
    OrigThreadInit(threadNum, tid);
    PIN_UnlockClient();
}

static void OnSecondaryThreadFini(unsigned int threadNum, pid_t tid)
{
    PIN_LockClient();
    if (NULL == OrigThreadFini)
    {
        cerr << "TOOL ERROR: Attempting to call SecondaryThreadFini but it is a NULL pointer" << endl;
        PIN_ExitProcess(106);
    }
    OrigThreadFini(threadNum, tid);
    PIN_UnlockClient();
}

static void OnSecondaryThreadWork()
{
    PIN_LockClient();
    cout << "TOOL: (" << PIN_GetTid() << ") executing OnSecondaryThreadWork" << endl << flush;
    sched_yield();
    PIN_UnlockClient();
}

static void OnReleaseThreads(ADDRINT doRelease)
{
    PIN_LockClient();
    *((bool*)doRelease) = true;
    cout << "TOOL: Released the threads, now waiting a few seconds for them to reach the lock." << endl;
    PIN_Sleep(5 * 1000);
    PIN_UnlockClient();
}

static VOID Image(IMG img, VOID* v)
{
    if (!IMG_IsMainExecutable(img)) return;

    RTN secondaryThreadInitRtn = GetReplacableRtn(img, "SecondaryThreadInit");
    OrigThreadInit             = (AppFunSig)RTN_ReplaceProbed(secondaryThreadInitRtn, AFUNPTR(OnSecondaryThreadInit));

    RTN secondaryThreadFiniRtn = GetReplacableRtn(img, "SecondaryThreadFini");
    OrigThreadFini             = (AppFunSig)RTN_ReplaceProbed(secondaryThreadFiniRtn, AFUNPTR(OnSecondaryThreadFini));

    RTN secondaryThreadWorkRtn = GetProbableRtn(img, "SecondaryThreadWork");
    RTN_InsertCallProbed(secondaryThreadWorkRtn, IPOINT_BEFORE, AFUNPTR(OnSecondaryThreadWork), IARG_END);

    RTN releaseThreadsRtn = GetProbableRtn(img, "ReleaseThreads");
    RTN_InsertCallProbed(releaseThreadsRtn, IPOINT_BEFORE, AFUNPTR(OnReleaseThreads), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    IMG_AddInstrumentFunction(Image, 0);

    PIN_StartProgram(); // never returns
    return 0;
}
