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
#include <tool_macros.h>
using std::cerr;
using std::endl;
using std::hex;
using std::ios;
using std::ofstream;
using std::string;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "attach_tool.out", "specify file name");

ofstream TraceFile;
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool tests attach to single-threaded application JIT mode.\n"
            "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

BOOL PinAttached() { return TRUE; }

VOID ImageLoad(IMG img, void* v)
{
    RTN rtn = RTN_FindByName(img, C_MANGLE("PinAttached"));
    if (RTN_Valid(rtn))
    {
        TraceFile << "Replacing PinAttached" << endl;
        RTN_Replace(rtn, AFUNPTR(PinAttached));
    }
}

void Fini(INT32 code, VOID* v) { TraceFile << "Fini" << endl; }

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
    PIN_AddFiniFunction(Fini, 0);
    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
