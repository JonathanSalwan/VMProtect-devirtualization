/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This tool is used by the memory_allocation_from_tool_access_protection.test.
// It checks the correctness of the APIs: PIN_CheckReadAccessProtection and PIN_CheckWriteAccessProtectionaccess
// when the mmap function is called by the tool.

#include "pin.H"
#include <os-apis.h>
#include <iostream>
#include <fstream>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
using std::dec;
using std::endl;
using std::hex;
using std::ios;
using std::string;

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

std::ofstream TraceFile;
BOOL AllocatedOnce;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "tool_memory_access_protection_tool.out",
                              "specify trace file name");

/* ================================================================== */
// Utilities
/* ================================================================== */

const char* MmapNoMemoryAccess()
{
    char* ptr = NULL;
    OS_AllocateMemory(NATIVE_PID_CURRENT, OS_PAGE_PROTECTION_TYPE_NOACCESS, getpagesize(), OS_MEMORY_FLAGS_PRIVATE, (void**)&ptr);
    return ptr;
}

const char* MmapWithMemoryAccess()
{
    char* ptr = NULL;
    OS_AllocateMemory(NATIVE_PID_CURRENT, OS_PAGE_PROTECTION_TYPE_READ | OS_PAGE_PROTECTION_TYPE_WRITE, getpagesize(),
                      OS_MEMORY_FLAGS_PRIVATE, (void**)&ptr);
    return ptr;
}

static VOID ToolMmap()
{
    if (!AllocatedOnce)
    {
        AllocatedOnce              = true;
        const char* pageFrameStart = MmapNoMemoryAccess();
        bool ans                   = PIN_CheckReadAccess((void*)pageFrameStart);
        TraceFile << dec << ans;
        ans = PIN_CheckWriteAccess((void*)pageFrameStart);
        TraceFile << dec << ans;
        pageFrameStart = MmapWithMemoryAccess();
        ans            = PIN_CheckReadAccess((void*)pageFrameStart);
        TraceFile << dec << ans;
        ans = PIN_CheckWriteAccess((void*)pageFrameStart);
        TraceFile << dec << ans;
        TraceFile << endl;
    }
}

static VOID InstrumentTrace(TRACE t, VOID* v) { TRACE_InsertCall(t, IPOINT_BEFORE, (AFUNPTR)ToolMmap, IARG_END); }

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    TraceFile.open(KnobOutputFile.Value().c_str());
    TraceFile << hex;
    TraceFile.setf(ios::showbase);

    AllocatedOnce = false;

    // Register Image to be called to instrument functions.
    TRACE_AddInstrumentFunction(InstrumentTrace, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
