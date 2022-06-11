/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
using std::cerr;
using std::dec;
using std::endl;
using std::hex;
using std::ios;
using std::ofstream;
using std::string;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "block_realTime_signals.outfile", "specify file name");

ofstream TraceFile;
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool tests blocking the real time signals.\n"
            "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

UINT32 threadCounter = 0;

VOID AttachedThreadStart(VOID* sigmask, VOID* v)
{
    /* Assuming that 40 is real time signal in both glibc and pincrt */
    int REALTIMESIGNAL = 40;
    TraceFile << "Thread counter is updated to " << dec << ++threadCounter << endl;
    if ((sigismember((sigset_t*)sigmask, REALTIMESIGNAL)) && (sigismember((sigset_t*)sigmask, REALTIMESIGNAL + 1) == false))
        TraceFile << "signal Blocked OK  " << REALTIMESIGNAL << " and signal " << REALTIMESIGNAL + 1 << "is not Blocked" << endl;
    else
        PIN_ExitProcess(-1);
}

int PinReady(unsigned int numOfThreads) { return (threadCounter == numOfThreads) ? 1 : 0; }

/* ===================================================================== */
// Called every time a new image is loaded
// Look for routines that we want to probe
VOID ImageLoad(IMG img, VOID* v)
{
    RTN rtn = RTN_FindByName(img, "ThreadsReady");
    if (RTN_Valid(rtn))
    {
        if (!RTN_IsSafeForProbedReplacement(rtn))
        {
            fprintf(stderr, "Can't replace ThreadsReady\n");
            PIN_ExitProcess(-1);
        }
        RTN_ReplaceProbed(rtn, AFUNPTR(PinReady));
    }
}

/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    TraceFile.open(KnobOutputFile.Value().c_str());
    TraceFile << hex;
    TraceFile.setf(ios::showbase);

    IMG_AddInstrumentFunction(ImageLoad, 0);
    PIN_AddThreadAttachProbedFunction(AttachedThreadStart, 0);
    PIN_StartProgramProbed();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
