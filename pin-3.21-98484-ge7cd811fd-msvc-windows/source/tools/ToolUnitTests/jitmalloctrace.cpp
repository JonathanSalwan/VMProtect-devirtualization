/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
  Generates a trace of malloc/free calls
 */

#include "pin.H"
#include <iostream>
#include <fstream>
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

VOID ReplaceJitted(RTN rtn, PROTO proto_malloc);
VOID* Jit_Malloc_IA32(CONTEXT* context, AFUNPTR orgFuncptr, size_t arg0);
VOID Jit_Free_IA32(CONTEXT* context, AFUNPTR orgFuncptr, void* arg0);
VOID Jit_Exit_IA32(CONTEXT* context, AFUNPTR orgFuncptr, int code);

ofstream TraceFile;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "jitmalloctrace.outfile", "specify trace file name");

/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool collects an instruction trace for debugging\n"
            "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

#ifdef TARGET_MAC
#define MALLOC_NAME "_malloc"
#define FREE_NAME "_free"
#define EXIT_NAME "_exit"
#else
#define MALLOC_NAME "malloc"
#define FREE_NAME "free"
#define EXIT_NAME "exit"
#endif

/* ===================================================================== */
// Called every time a new image is loaded.
// Look for routines that we want to replace.
VOID ImageLoad(IMG img, VOID* v)
{
    RTN mallocRtn = RTN_FindByName(img, MALLOC_NAME);
    if (RTN_Valid(mallocRtn))
    {
        PROTO proto_malloc = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_DEFAULT, MALLOC_NAME, PIN_PARG(size_t), PIN_PARG_END());

        RTN_ReplaceSignature(mallocRtn, AFUNPTR(Jit_Malloc_IA32), IARG_PROTOTYPE, proto_malloc, IARG_CONTEXT, IARG_ORIG_FUNCPTR,
                             IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);

        TraceFile << "Replaced malloc() in:" << IMG_Name(img) << endl;
    }

    RTN freeRtn = RTN_FindByName(img, FREE_NAME);
    if (RTN_Valid(freeRtn))
    {
        PROTO proto_free = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, FREE_NAME, PIN_PARG(void*), PIN_PARG_END());

        RTN_ReplaceSignature(freeRtn, AFUNPTR(Jit_Free_IA32), IARG_PROTOTYPE, proto_free, IARG_CONTEXT, IARG_ORIG_FUNCPTR,
                             IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);

        TraceFile << "Replaced free() in:" << IMG_Name(img) << endl;
    }

    RTN exitRtn = RTN_FindByName(img, EXIT_NAME);
    if (RTN_Valid(exitRtn))
    {
        PROTO proto_exit = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, EXIT_NAME, PIN_PARG(int), PIN_PARG_END());

        RTN_ReplaceSignature(exitRtn, AFUNPTR(Jit_Exit_IA32), IARG_PROTOTYPE, proto_exit, IARG_CONTEXT, IARG_ORIG_FUNCPTR,
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
    TraceFile << "JIT mode" << endl;

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */

VOID* Jit_Malloc_IA32(CONTEXT* context, AFUNPTR orgFuncptr, size_t size)
{
    VOID* ret;

    PIN_CallApplicationFunction(context, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFuncptr, NULL, PIN_PARG(void*), &ret,
                                PIN_PARG(size_t), size, PIN_PARG_END());

    TraceFile << "malloc(" << size << ") returns " << ret << endl;
    return ret;
}

/* ===================================================================== */

VOID Jit_Free_IA32(CONTEXT* context, AFUNPTR orgFuncptr, void* ptr)
{
    PIN_CallApplicationFunction(context, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFuncptr, NULL, PIN_PARG(void), PIN_PARG(void*),
                                ptr, PIN_PARG_END());

    TraceFile << "free(" << ptr << ")" << endl;
}

/* ===================================================================== */

VOID Jit_Exit_IA32(CONTEXT* context, AFUNPTR orgFuncptr, int code)
{
    if (TraceFile.is_open())
    {
        TraceFile << "## eof" << endl << flush;
        TraceFile.close();
    }

    PIN_CallApplicationFunction(context, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFuncptr, NULL, PIN_PARG(void), PIN_PARG(int),
                                code, PIN_PARG_END());
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
