/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 */

#include "pin.H"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
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

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "probe2.outfile", "specify file name");

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
    if (pf_dn)
    {
        (*pf_dn)();

        TraceFile << "Doing nothing." << endl;
    }
}

/* ===================================================================== */
// Called every time a new image is loaded
// Look for routines that we want to probe
VOID ImageLoad(IMG img, VOID* v)
{
    cout << "Processing " << IMG_Name(img) << endl;

    RTN rtn = RTN_FindByName(img, C_MANGLE("do_nothing"));
    if (RTN_Valid(rtn))
    {
        if (!RTN_IsSafeForProbedReplacement(rtn))
        {
            TraceFile << "Cannot replace " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;
        }

        pf_dn = (void (*)())RTN_ReplaceProbed(rtn, AFUNPTR(Do_Nothing));

        TraceFile << "Inserted probe for do_nothing:" << IMG_Name(img) << endl;
    }

    cout << "Completed " << IMG_Name(img) << endl;
}

/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    char* s = getenv("LD_ASSUME_KERNEL");
    if (s == 0)
        printf("LD_ASSUME_KERNEL not set\n");
    else if (strcmp(s, "2.4.1") != 0)
        printf("LD_ASSUME_KERNEL is wrong: %s\n", s);

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
