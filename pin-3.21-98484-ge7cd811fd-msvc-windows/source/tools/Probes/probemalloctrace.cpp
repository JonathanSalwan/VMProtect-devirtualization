/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */

/* ===================================================================== */
/*! @file
  Generates a trace of malloc/free calls
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include "tool_macros.h"
using std::cerr;
using std::endl;
using std::flush;
using std::hex;
using std::ios;
using std::ofstream;
using std::string;

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */
typedef VOID* (*FUNCPTR_MALLOC)(size_t);
typedef VOID (*FUNCPTR_FREE)(void*);
typedef VOID (*FUNCPTR_EXIT)(int);

VOID* Probe_Malloc_IA32(FUNCPTR_MALLOC orgFuncptr, size_t arg0);
VOID Probe_Free_IA32(FUNCPTR_FREE orgFuncptr, void* arg0);
VOID Probe_Exit_IA32(FUNCPTR_EXIT orgFuncptr, int code);

ofstream TraceFile;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "probemalloctrace.outfile", "specify trace file name");

/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool collects an instruction trace for debugging\n"
            "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

/* ===================================================================== */
// Called every time a new image is loaded.
// Look for routines that we want to replace.
VOID ImageLoad(IMG img, VOID* v)
{
    RTN mallocRtn = RTN_FindByName(img, C_MANGLE("malloc"));

    if (RTN_Valid(mallocRtn) && RTN_IsSafeForProbedReplacement(mallocRtn))
    {
        PROTO proto_malloc = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_DEFAULT, "malloc", PIN_PARG(size_t), PIN_PARG_END());

        RTN_ReplaceSignatureProbed(mallocRtn, AFUNPTR(Probe_Malloc_IA32), IARG_PROTOTYPE, proto_malloc, IARG_ORIG_FUNCPTR,
                                   IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
        TraceFile << "Replaced malloc() in:" << IMG_Name(img) << endl;
    }

    RTN freeRtn = RTN_FindByName(img, C_MANGLE("free"));

    if (RTN_Valid(freeRtn) && RTN_IsSafeForProbedReplacement(freeRtn))
    {
        PROTO proto_free = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "free", PIN_PARG(void*), PIN_PARG_END());

        RTN_ReplaceSignatureProbed(freeRtn, AFUNPTR(Probe_Free_IA32), IARG_PROTOTYPE, proto_free, IARG_ORIG_FUNCPTR,
                                   IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
        TraceFile << "Replaced free() in:" << IMG_Name(img) << endl;
    }

    RTN exitRtn = RTN_FindByName(img, C_MANGLE("exit"));

    if (RTN_Valid(exitRtn) && RTN_IsSafeForProbedReplacement(exitRtn))
    {
        PROTO proto_exit = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "exit", PIN_PARG(int), PIN_PARG_END());

        RTN_ReplaceSignatureProbed(exitRtn, AFUNPTR(Probe_Exit_IA32), IARG_PROTOTYPE, proto_exit, IARG_ORIG_FUNCPTR,
                                   IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
        TraceFile << "Replaced exit() in:" << IMG_Name(img) << endl;
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
    TraceFile << "Probe mode" << endl;

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_StartProgramProbed();

    return 0;
}

/* ===================================================================== */

VOID* Probe_Malloc_IA32(FUNCPTR_MALLOC orgFuncptr, size_t size)
{
    VOID* v = orgFuncptr(size);

    TraceFile << "malloc(" << size << ") returns " << v << endl;

    return v;
}

/* ===================================================================== */

VOID Probe_Free_IA32(FUNCPTR_FREE orgFuncptr, void* ptr)
{
    orgFuncptr(ptr);

    TraceFile << "free(" << ptr << ")" << endl;
}

/* ===================================================================== */

VOID Probe_Exit_IA32(FUNCPTR_EXIT orgFuncptr, int code)
{
    if (TraceFile.is_open())
    {
        TraceFile << "## eof" << endl << flush;
        TraceFile.close();
    }

    orgFuncptr(code);
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
