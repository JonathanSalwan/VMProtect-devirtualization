/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * Test detach from an analysis routine (in Jit). Also check detach callback was called
 */
#include <stdio.h>
#include "pin.H"
#include <iostream>
#include <fstream>
using std::cerr;
using std::endl;
using std::ofstream;
using std::string;

UINT64 icount = 0;

UINT32 icountMax       = 10000;
volatile bool detached = false; // True if detach callback was called

ofstream outfile;

KNOB< string > KnobOutFile(KNOB_MODE_WRITEONCE, "pintool", "o", "jit_anls_detach.out",
                           "Specify file name for the tool's output.");

VOID docount()
{
    ASSERT(!detached, "Analysis function was called after detach ended");
    if (++icount == icountMax)
    {
        outfile << "Send detach request form analysis routine after " << icount << " instructions." << endl;
        PIN_Detach();
    }
}

VOID Instruction(INS ins, VOID* v)
{
    ASSERT(!detached, "Instrumentation function was called after detach ended");
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END);
}

VOID Detach(VOID* v)
{
    if (detached) // sanity check
    {
        // This should never be reached because only one detach request should be executed.
        cerr << "TOOL ERROR: jit_instr_detach is executing the Detach callback twice." << endl;
        exit(20); // use exit instead of PIN_ExitProcess because we don't know if it is available at this point.
    }
    detached = true;
    outfile << "Pin detached after " << icount << " executed instructions." << endl;
    outfile.close();
}

VOID Fini(INT32 code, VOID* v)
{
    cerr << "Count: " << icount << endl;
    ASSERT(0, "Error, Fini called although we detached");
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    outfile.open(KnobOutFile.Value().c_str());
    if (!outfile.good())
    {
        cerr << "Failed to open output file " << KnobOutFile.Value().c_str() << "." << endl;
        PIN_ExitProcess(10);
    }

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);
    PIN_AddDetachFunction(Detach, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
