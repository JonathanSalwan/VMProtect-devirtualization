/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Check that if we register an instruction fetcher it is not used to fetch from 
 * the tool itself. 
 */

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "pin.H"

using std::cerr;
using std::endl;

static UINT64 insCount      = 0;
static ADDRINT toolLowAddr  = 0;
static ADDRINT toolHighAddr = 0;

static void reportError(ADDRINT addr)
{
    fprintf(stderr, "BAD Fetch from 0x%08x\n", addr);
    for (IMG img = APP_ImgHead(); IMG_Valid(img); img = IMG_Next(img))
    {
        fprintf(stderr, "%-30s: 0x%08x:0x%08x\n", IMG_Name(img).c_str(), IMG_LowAddress(img), IMG_HighAddress(img));
    }
    exit(-1);
}

static BOOL addrInTool(ADDRINT addr) { return (toolLowAddr <= addr) && (addr <= toolHighAddr); }

static size_t fetchIns(void* buf, ADDRINT addr, size_t size, EXCEPTION_INFO* pExceptInfo, VOID*)
{
    /* Validate that the fetch is in the range we expect and abort if not.
     */
    size_t bytesRead = PIN_SafeCopyEx(static_cast< UINT8* >(buf), reinterpret_cast< UINT8* >(addr), size, pExceptInfo);

    if (bytesRead && addrInTool(addr))
    {
        reportError(addr);
    }

    return bytesRead;
}

// This is small enough to be inlined, which stresses the code fetcher from the tool
// slightly more.
static VOID incCount() { insCount++; }

static VOID imageLoad(IMG img, VOID* v)
{
    // Just instrument the main image.
    if (!IMG_IsMainExecutable(img)) return;

    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            RTN_Open(rtn);
            for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)incCount, IARG_END);
            }
            RTN_Close(rtn);
        }
    }
}

static VOID fini(INT32, VOID*) { cerr << insCount << " instructions in the main image executed" << endl; }

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    toolLowAddr  = ADDRINT(&reportError);
    toolHighAddr = ADDRINT(&main);

    fprintf(stderr, "toolLowAddr  is 0x%08x\n", toolLowAddr);
    fprintf(stderr, "toolHighAddr is 0x%08x\n", toolHighAddr);

    PIN_InitSymbols();

    // Initialize pin
    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(imageLoad, 0);

    PIN_AddFetchFunction(fetchIns, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
