/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <string.h>
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

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "symbolnames.outfile", "specify trace file name");

/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool collects an instruction trace for debugging\n"
            "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

VOID ImageLoad(IMG img, VOID* v)
{
    TraceFile << "Processing " << IMG_Name(img) << endl;
    cout << "Processing " << IMG_Name(img) << endl;

    for (SYM sym = IMG_RegsymHead(img); SYM_Valid(sym); sym = SYM_Next(sym))
    {
        TraceFile << "   " << SYM_Name(sym) << endl;
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
