/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <stdlib.h>
using std::endl;
using std::hex;
using std::ios;

#if defined(TARGET_MAC)
#define FUNC_PREFIX "_"
#else
#define FUNC_PREFIX
#endif

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

std::ofstream TraceFile;

/* ===================================================================== */

VOID MmapArgs(ADDRINT ra, ADDRINT arg1, ADDRINT arg2, ADDRINT arg3, ADDRINT arg4, ADDRINT arg5, ADDRINT arg6)
{
    TraceFile << "mmap(" << arg1 << "," << arg2 << "," << arg3 << "," << arg4 << "," << arg5 << "," << arg6 << ")" << endl;
    TraceFile << "  called from " << ra << endl;
}

VOID CallArgs(ADDRINT arg1) { TraceFile << "Call(" << arg1 << ")" << endl; }

VOID FoobarArgs(ADDRINT arg1, ADDRINT arg2)
{
    TraceFile << "Foobar(" << arg1 << "," << arg2 << ")" << endl;
    if (arg1 != 0x0eadbeef || arg2 != 0x0eedfeed)
    {
        fprintf(stderr, "Error in arguments\n");
        abort();
    }
}

VOID BazArg(ADDRINT* arg1, ADDRINT* arg2, ADDRINT* arg3)
{
    TraceFile << "Baz(" << *arg1 << "," << *arg2 << "," << *arg3 << ")" << endl;
    *arg1 = 4;
    *arg2 = 5;
    *arg3 = 6;
}

static ADDRINT foobarAddress = 0;

/* ===================================================================== */

VOID Ins(INS ins, VOID* v)
{
    if (!INS_IsCall(ins)) return;

    if (foobarAddress != 0 && INS_IsDirectControlFlow(ins) && INS_DirectControlFlowTargetAddress(ins) == foobarAddress)
    {
        TraceFile << "Instrument call to foobar" << endl;
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(FoobarArgs), IARG_FUNCARG_CALLSITE_VALUE, 0, IARG_FUNCARG_CALLSITE_VALUE, 1,
                       IARG_END);
    }

    static BOOL first = true;

    if (!first) return;

    first = false;

    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(CallArgs), IARG_FUNCARG_CALLSITE_VALUE, 0, IARG_END);
}

/* ===================================================================== */

VOID Image(IMG img, VOID* v)
{
    RTN mmapRtn = RTN_FindByName(img, FUNC_PREFIX "mmap");
    if (RTN_Valid(mmapRtn))
    {
        RTN_Open(mmapRtn);
        RTN_InsertCall(mmapRtn, IPOINT_BEFORE, AFUNPTR(MmapArgs), IARG_RETURN_IP, IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_FUNCARG_ENTRYPOINT_VALUE, 2, IARG_FUNCARG_ENTRYPOINT_VALUE, 3,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 4, IARG_FUNCARG_ENTRYPOINT_VALUE, 5, IARG_END);
        RTN_Close(mmapRtn);
    }
    RTN foobarRtn = RTN_FindByName(img, FUNC_PREFIX "foobar");
    if (RTN_Valid(foobarRtn))
    {
        foobarAddress = RTN_Address(foobarRtn);

        RTN_Open(foobarRtn);
        RTN_InsertCall(foobarRtn, IPOINT_BEFORE, AFUNPTR(FoobarArgs), IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_END);
        RTN_Close(foobarRtn);
    }

    RTN bazRtn = RTN_FindByName(img, FUNC_PREFIX "baz");
    if (RTN_Valid(bazRtn))
    {
        RTN_Open(bazRtn);
        RTN_InsertCall(bazRtn, IPOINT_BEFORE, AFUNPTR(BazArg), IARG_FUNCARG_ENTRYPOINT_REFERENCE, 0,
                       IARG_FUNCARG_ENTRYPOINT_REFERENCE, 1, IARG_FUNCARG_ENTRYPOINT_REFERENCE, 2, IARG_END);
        RTN_Close(bazRtn);
    }
}

/* ===================================================================== */

VOID Fini(INT32 code, VOID* v) { TraceFile.close(); }

/* ===================================================================== */

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    TraceFile.open("args.out");

    TraceFile << hex;
    TraceFile.setf(ios::showbase);

    IMG_AddInstrumentFunction(Image, 0);
    INS_AddInstrumentFunction(Ins, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
