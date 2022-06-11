/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
namespace WIND
{
#include <windows.h>
}

using std::cerr;
using std::endl;
using std::hex;
using std::ios;
using std::ofstream;
using std::string;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "ssewin.outfile", "specify trace file name");

/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool replaces SetSystemError().\n"
            "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    cerr.flush();
    return -1;
}

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

ofstream TraceFile;

typedef void (*FUNCPTR_SET_SYSTEM_ERROR)(WIND::DWORD);

static void (*fp_setSystemError)(WIND::DWORD);

ADDRINT mainImgEntry = 0;

/* ===================================================================== */

void ReplacedSetSystemErrorProbed(FUNCPTR_SET_SYSTEM_ERROR origFunc, WIND::DWORD errorCode)
{
    // exercise heavy use of stack inside PinTool
    char useTheStack[0x8765];
    useTheStack[0] = 'a';

    fprintf(stderr, "SetSystemError(%d)\n", errorCode);

    origFunc(errorCode);
}

void ReplacedSetSystemErrorJit(CONTEXT* context, AFUNPTR origFunc, WIND::DWORD errorCode)
{
    fprintf(stderr, "SetSystemError(%d)\n", errorCode);

    PIN_CallApplicationFunction(context, PIN_ThreadId(), CALLINGSTD_DEFAULT, origFunc, NULL, PIN_PARG(void), PIN_PARG(int),
                                errorCode, PIN_PARG_END());
}

VOID ImageLoad(IMG img, VOID* v)
{
    TraceFile << "Processing " << IMG_Name(img) << endl;
    TraceFile.flush();

    const char* name    = "SetSystemError";
    PROTO proto_funcptr = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, name, PIN_PARG(WIND::DWORD), PIN_PARG_END());

    // Get the function pointer for the executable SetSystemError()
    RTN rtnSse = RTN_FindByName(img, "SetSystemError");
    if (RTN_Valid(rtnSse))
    {
        fprintf(stderr, "Found SetSystemError RTN\n");

        if (PIN_IsProbeMode())
            RTN_ReplaceSignatureProbed(rtnSse, AFUNPTR(ReplacedSetSystemErrorProbed), IARG_PROTOTYPE, proto_funcptr,
                                       IARG_ORIG_FUNCPTR, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
        else
            RTN_ReplaceSignature(rtnSse, AFUNPTR(ReplacedSetSystemErrorJit), IARG_PROTOTYPE, proto_funcptr, IARG_CONTEXT,
                                 IARG_ORIG_FUNCPTR, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
    }
}

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

    if (PIN_IsProbeMode())
    {
        PIN_StartProgramProbed();
    }
    else
    {
        PIN_StartProgram();
    }

    return 0;
}
