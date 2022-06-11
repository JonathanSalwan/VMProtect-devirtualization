/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <stdlib.h>
using std::cerr;
using std::dec;
using std::endl;
using std::hex;
using std::ios;
using std::ofstream;
using std::string;

/*
 * This test puts a probe in very short function and checks how Pin
 * moves thread IP, if it fails on probe
 */

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "probe_tool.outfile", "specify file name");

ofstream TraceFile;
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool tests probe replacement.\n"
            "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

void Do_Nothing()
{
    int x = 0;
    while (1)
    {
        x++;
        --x;
    }
}

UINT32 threadCounter = 0;

VOID AttachedThreadStart(VOID* sigmask, VOID* v)
{
    TraceFile << "Thread counter is updated to " << dec << ++threadCounter << endl;
}

int PinReady(unsigned int numOfThreads) { return (threadCounter == numOfThreads) ? 1 : 0; }

/* ===================================================================== */
// Called every time a new image is loaded
// Look for routines that we want to probe
VOID ImageLoad(IMG img, VOID* v)
{
    RTN rtn = RTN_FindByName(img, "ShortFunc");
    if (RTN_Valid(rtn))
    {
        RTN_ReplaceProbed(rtn, AFUNPTR(Do_Nothing));
    }

    rtn = RTN_FindByName(img, "ShortFunc2");
    if (RTN_Valid(rtn))
    {
        RTN_ReplaceProbed(rtn, AFUNPTR(Do_Nothing));
    }

    rtn = RTN_FindByName(img, "ThreadsReady");
    if (RTN_Valid(rtn))
    {
        if (!RTN_IsSafeForProbedReplacement(rtn))
        {
            fprintf(stderr, "Can't replace ThreadsReady\n");
            exit(-1);
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
