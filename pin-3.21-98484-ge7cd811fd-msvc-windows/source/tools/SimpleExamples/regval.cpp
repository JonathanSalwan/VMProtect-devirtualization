/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This tool demonstrates the use of the PIN_GetContextRegval API for various types of registers.
// It is used with the regval_app application.

#include <fstream>
#include <cassert>
#include "pin.H"
#include "../Utils/regvalue_utils.h"
using std::endl;
using std::hex;

using std::ofstream;

/////////////////////
// GLOBAL VARIABLES
/////////////////////

// A knob for defining the output file name
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "regval.out", "specify output file name");

// ofstream object for handling the output
ofstream OutFile;

// We don't want to print the registers too many times, so we put placeholders in the application to tell the tool
// when to start and stop printing.
volatile bool printRegsNow = false;

#ifdef TARGET_MAC
const char* startRtnName = "_Start";
const char* stopRtnName  = "_Stop";
#else
const char* startRtnName = "Start";
const char* stopRtnName  = "Stop";
#endif

/////////////////////
// ANALYSIS FUNCTIONS
/////////////////////

// Once this is called, the registers will be printed until EndRoutine is called.
static void StartRoutine() { printRegsNow = true; }

// After this is called, the registers will no longer be printed.
static void StopRoutine() { printRegsNow = false; }

static void PrintRegisters(const CONTEXT* ctxt)
{
    if (!printRegsNow) return;
    static const UINT stRegSize = REG_Size(REG_ST_BASE);
    for (int reg = (int)REG_GR_BASE; reg <= (int)REG_GR_LAST; ++reg)
    {
        // For the integer registers, it is safe to use ADDRINT. But make sure to pass a pointer to it.
        ADDRINT val;
        PIN_GetContextRegval(ctxt, (REG)reg, reinterpret_cast< UINT8* >(&val));
        OutFile << REG_StringShort((REG)reg) << ": 0x" << hex << val << endl;
    }
    for (int reg = (int)REG_ST_BASE; reg <= (int)REG_ST_LAST; ++reg)
    {
        // For the x87 FPU stack registers, using PIN_REGISTER ensures a large enough buffer.
        PIN_REGISTER val;
        PIN_GetContextRegval(ctxt, (REG)reg, reinterpret_cast< UINT8* >(&val));
        OutFile << REG_StringShort((REG)reg) << ": " << Val2Str(&val, stRegSize) << endl;
    }
}

/////////////////////
// INSTRUMENTATION FUNCTIONS
/////////////////////

static VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        RTN StartRtn = RTN_FindByName(img, startRtnName);
        assert(RTN_Valid(StartRtn));
        RTN_Open(StartRtn);
        RTN_InsertCall(StartRtn, IPOINT_BEFORE, (AFUNPTR)StartRoutine, IARG_END);
        RTN_Close(StartRtn);

        RTN StopRtn = RTN_FindByName(img, stopRtnName);
        assert(RTN_Valid(StopRtn));
        RTN_Open(StopRtn);
        RTN_InsertCall(StopRtn, IPOINT_AFTER, (AFUNPTR)StopRoutine, IARG_END);
        RTN_Close(StopRtn);
    }
}

static VOID Trace(TRACE trace, VOID* v)
{
    TRACE_InsertCall(trace, IPOINT_BEFORE, (AFUNPTR)PrintRegisters, IARG_CONST_CONTEXT, IARG_END);
}

static VOID Fini(INT32 code, VOID* v) { OutFile.close(); }

/////////////////////
// MAIN FUNCTION
/////////////////////

int main(int argc, char* argv[])
{
    // Initialize Pin
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    // Open the output file
    OutFile.open(KnobOutputFile.Value().c_str());

    // Add instrumentation
    IMG_AddInstrumentFunction(ImageLoad, 0);
    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Start running the application
    PIN_StartProgram(); // never returns

    return 0;
}
