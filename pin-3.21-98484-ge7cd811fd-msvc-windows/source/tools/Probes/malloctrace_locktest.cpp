/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies that we can call PIN_LockClient / PIN_UnlockClient
 * from a probe mode tool.  Note, this is just a unit test.  There is no real
 * need to call PIN_LockClient / PIN_UnlockClient in this tool.
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include "tool_macros.h"
using std::ofstream;
using std::string;

ofstream TraceFile;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "malloctrace_locktest.out", "specify trace file name");

static VOID ImageLoad(IMG img, VOID* v);
static void* MallocProbe(void* (*origMalloc)(size_t), UINT32 mallocNum, size_t size, ADDRINT appTP);
static void FreeProbe(void (*origFree)(void*), UINT32 freeNum, void* ptr, ADDRINT appTP);

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();
    if (PIN_Init(argc, argv)) return 1;

    TraceFile.open(KnobOutputFile.Value().c_str());

    IMG_AddInstrumentFunction(ImageLoad, 0);
    PIN_StartProgramProbed();

    return 0;
}

static VOID ImageLoad(IMG img, VOID* v)
{
    static UINT32 mallocCount = 0;

    PROTO protoMalloc = PROTO_Allocate(PIN_PARG(void*), CALLINGSTD_DEFAULT, "malloc", PIN_PARG(size_t), PIN_PARG_END());

    RTN rtnMalloc = RTN_FindByName(img, C_MANGLE("malloc"));
    if (RTN_Valid(rtnMalloc))
    {
        TraceFile << "probing malloc #" << mallocCount << " in " << IMG_Name(img) << std::endl;

        RTN_ReplaceSignatureProbed(rtnMalloc, AFUNPTR(MallocProbe), IARG_PROTOTYPE, protoMalloc, IARG_ORIG_FUNCPTR, IARG_UINT32,
                                   static_cast< UINT32 >(mallocCount), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_ADDRINT,
                                   static_cast< ADDRINT >(0), IARG_END);

        mallocCount++;
    }

    PROTO_Free(protoMalloc);

    static UINT32 freeCount = 0;

    PROTO protoFree = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "free", PIN_PARG(void*), PIN_PARG_END());

    RTN freeRtn = RTN_FindByName(img, C_MANGLE("free"));
    if (RTN_Valid(freeRtn))
    {
        TraceFile << "probing free #" << freeCount << " in " << IMG_Name(img) << std::endl;

        RTN_ReplaceSignatureProbed(freeRtn, AFUNPTR(FreeProbe), IARG_PROTOTYPE, protoFree, IARG_ORIG_FUNCPTR, IARG_UINT32,
                                   static_cast< UINT32 >(freeCount), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_ADDRINT,
                                   static_cast< ADDRINT >(0), IARG_END);

        freeCount++;
    }

    PROTO_Free(protoFree);
}

static void* MallocProbe(void* (*origMalloc)(size_t), UINT32 mallocNum, size_t size, ADDRINT appTP)
{
    void* ptr = origMalloc(size);
    PIN_LockClient();
    TraceFile << mallocNum << " malloc(" << size << ") returns " << ptr << std::endl;
    PIN_UnlockClient();

    return ptr;
}

static void FreeProbe(void (*origFree)(void*), UINT32 freeNum, void* ptr, ADDRINT appTP)
{
    origFree(ptr);
    PIN_LockClient();
    TraceFile << freeNum << " free(" << ptr << ")" << std::endl;
    PIN_UnlockClient();
}
