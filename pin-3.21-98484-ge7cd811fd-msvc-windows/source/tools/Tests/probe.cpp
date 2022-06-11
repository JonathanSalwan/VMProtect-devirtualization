/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pin.H"

#if defined(TARGET_MAC)
// macOS*
#define LIBC "libSystem.B.dylib"
#else
// Linux
#define LIBC "libc.so"
#endif

FILE* trace;

BOOL DoneLoad(IMG img)
{
    if (IMG_Type(img) == IMG_TYPE_STATIC) return true;

    // Give up after libc.so is loaded
    if (strstr(IMG_Name(img).c_str(), LIBC)) return true;

    return false;
}

INT32 readcount = 0;

VOID Mem(ADDRINT a, ADDRINT s) {}

VOID ImageLoad(IMG img, VOID* v)
{
    fprintf(trace, "Loading %s\n", IMG_Name(img).c_str());
    fflush(trace);

    // Scan the instructions to test image parsing code
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
#if 0
            fprintf(trace, "Routine name %s %x\n",
                  RTN_Name(rtn).c_str(),
                  RTN_Address(rtn));
            fflush(trace);
#endif
            RTN_Open(rtn);

            for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
            {
                if (INS_HasMemoryRead2(ins))
                {
                    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(Mem), IARG_MEMORYREAD2_EA, IARG_MEMORYREAD_SIZE, IARG_END);
                }

                if (INS_IsMemoryRead(ins))
                {
                    //fprintf(trace, "RSize %d %s\n",INS_MemoryReadSize(ins), INS_Disassemble(ins).c_str());

                    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(Mem), IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE, IARG_END);
                }

                if (INS_IsMemoryWrite(ins))
                {
                    //fprintf(trace, "WSize %d %x %s\n",INS_MemoryWriteSize(ins), INS_Address(ins), INS_Disassemble(ins).c_str());

                    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(Mem), IARG_MEMORYWRITE_EA, IARG_MEMORYWRITE_SIZE, IARG_END);
                }
            }

            RTN_Close(rtn);
        }
    }

    if (DoneLoad(img))
    {
        fprintf(trace, "Finished\n");
        fclose(trace);
        exit(0);
    }
}

void Trace(TRACE trace, void* v)
{
    static bool first = true;

    if (first)
    {
        ASSERTX(RTN_Valid(TRACE_Rtn(trace)));
    }

    first = false;
}

int main(INT32 argc, CHAR** argv)
{
    // On macOS*, ImageLoad() works only after we call PIN_InitSymbols().
    // This is not necessary on Linux, but doing it doesn't hurt.
    PIN_InitSymbols();

    trace = fopen("probe.out", "w");

    if (PIN_Init(argc, argv))
    {
        PIN_ERROR("bad commandline\n");
    }

    IMG_AddInstrumentFunction(ImageLoad, 0);

    TRACE_AddInstrumentFunction(Trace, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
