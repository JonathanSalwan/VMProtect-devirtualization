/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <fstream>
#include <cstring>
#include <cassert>
#include "context_utils.h"
using std::endl;
using std::flush;

using std::ofstream;

/////////////////////
// GLOBAL VARIABLES
/////////////////////

// A knob for defining which context to test. One of:
// 1. default   - regular CONTEXT passed to the analysis routine using IARG_CONTEXT.
// 2. const     - const CONTEXT passed to the analysis routine using IARG_CONST_CONTEXT.
// 2. partial   - partial CONTEXT passed to the analysis routine using IARG_PARTIAL_CONTEXT.
KNOB< string > KnobTestContext(KNOB_MODE_WRITEONCE, "pintool", "testcontext", "default",
                               "specify which context to test. One of default|const|partial.");

// A knob for defining the output file name
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "context_regvalue.out", "specify output file name");

// ofstream object for handling the output
ofstream OutFile;

/////////////////////
// ANALYSIS FUNCTIONS
/////////////////////

static void PrintsBefore(CONTEXT* ctxt)
{
    OutFile << "Context values before ChangeRegs functions" << endl << flush;
    StoreContext(ctxt);
    PrintStoredRegisters(OutFile);
}

static void ChecksAfter(CONTEXT* ctxt)
{
    OutFile << "Context values after ChangeRegs functions" << endl << flush;
    StoreContext(ctxt);
    PrintStoredRegisters(OutFile);
    if (!CheckAllExpectedValues(OutFile))
    {
        OutFile << "ERROR: values mismatch" << endl << flush;
        PIN_ExitApplication(1); // never returns
    }
}

/////////////////////
// CALLBACKS
/////////////////////

static VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        RTN changeRegsRtn = RTN_FindByName(img, "ChangeRegs");
        assert(RTN_Valid(changeRegsRtn));
        RTN_Open(changeRegsRtn);
        if (KnobTestContext.Value() == "default")
        {
            RTN_InsertCall(changeRegsRtn, IPOINT_BEFORE, (AFUNPTR)PrintsBefore, IARG_CONTEXT, IARG_END);
            RTN_InsertCall(changeRegsRtn, IPOINT_AFTER, (AFUNPTR)ChecksAfter, IARG_CONTEXT, IARG_END);
        }
        else if (KnobTestContext.Value() == "const")
        {
            RTN_InsertCall(changeRegsRtn, IPOINT_BEFORE, (AFUNPTR)PrintsBefore, IARG_CONST_CONTEXT, IARG_END);
            RTN_InsertCall(changeRegsRtn, IPOINT_AFTER, (AFUNPTR)ChecksAfter, IARG_CONST_CONTEXT, IARG_END);
        }
        else if (KnobTestContext.Value() == "partial")
        {
            REGSET regsin  = GetTestRegset();
            REGSET regsout = GetTestRegset();
            RTN_InsertCall(changeRegsRtn, IPOINT_BEFORE, (AFUNPTR)PrintsBefore, IARG_PARTIAL_CONTEXT, &regsin, &regsout,
                           IARG_END);
            RTN_InsertCall(changeRegsRtn, IPOINT_AFTER, (AFUNPTR)ChecksAfter, IARG_PARTIAL_CONTEXT, &regsin, &regsout, IARG_END);
        }
        else
        {
            OutFile << "ERROR: Unknown context requested for testing: " << KnobTestContext.Value() << endl;
            PIN_ExitApplication(2); // never returns
        }
        RTN_Close(changeRegsRtn);
    }
}

static VOID Fini(INT32 code, VOID* v) { OutFile.close(); }

/////////////////////
// MAIN FUNCTION
/////////////////////

int main(int argc, char* argv[])
{
    // Initialize Pin
    PIN_InitSymbolsAlt(EXPORT_SYMBOLS);
    PIN_Init(argc, argv);

    // Open the output file
    OutFile.open(KnobOutputFile.Value().c_str());

    // Add instrumentation
    IMG_AddInstrumentFunction(ImageLoad, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Start running the application
    PIN_StartProgram(); // never returns

    return 0;
}
