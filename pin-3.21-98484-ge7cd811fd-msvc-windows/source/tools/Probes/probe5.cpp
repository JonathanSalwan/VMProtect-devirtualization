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
#include "tool_macros.h"
using std::cerr;
using std::cout;
using std::endl;
using std::hex;
using std::ios;
using std::ofstream;
using std::string;

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

ofstream TraceFile;
static void (*pf_dn)();
static void (*pf_nd)();

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "probe5.outfile", "specify file name");

/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool tests probe replacement.\n"
            "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

void Call_Function()
{
    if (pf_dn)
    {
        (*pf_dn)();

        TraceFile << "Doing nothing." << endl;
    }
}

void Nothing_Doing()
{
    if (pf_nd)
    {
        (*pf_nd)();

        TraceFile << "Nothing doing." << endl;
    }
}

/* ===================================================================== */
// Called every time a new image is loaded
// Look for routines that we want to probe
VOID ImageLoad(IMG img, VOID* v)
{
    cout << "Processing " << IMG_Name(img) << endl;

    RTN rtn = RTN_FindByName(img, C_MANGLE("call_nothing"));
    if (RTN_Valid(rtn))
    {
        if (!RTN_IsSafeForProbedReplacement(rtn))
        {
            TraceFile << "Cannot replace " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;
            exit(1);
        }

        pf_dn = (void (*)())RTN_ReplaceProbed(rtn, AFUNPTR(Call_Function));

        TraceFile << "Inserted probe for call_nothing:" << IMG_Name(img) << endl;
    }

    rtn = RTN_FindByName(img, C_MANGLE("nothing_doing"));
    if (RTN_Valid(rtn))
    {
        if (!RTN_IsSafeForProbedReplacement(rtn))
        {
            TraceFile << "Cannot replace " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;
            exit(1);
        }

        pf_nd = (void (*)())RTN_ReplaceProbed(rtn, AFUNPTR(Nothing_Doing));

        TraceFile << "Inserted probe for nothing_doing:" << IMG_Name(img) << endl;
    }

    cout << "Completed " << IMG_Name(img) << endl;
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

    PIN_StartProgramProbed();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
