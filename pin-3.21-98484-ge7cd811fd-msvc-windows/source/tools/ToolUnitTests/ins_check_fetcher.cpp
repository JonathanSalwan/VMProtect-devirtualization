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

static UINT64 insCount = 0;

static size_t fetchIns(void* buf, ADDRINT addr, size_t size, EXCEPTION_INFO* pExceptInfo, VOID*)
{
    /* Validate that the fetch is in the range we expect and abort if not.
     */
    size_t bytesRead = PIN_SafeCopyEx(static_cast< UINT8* >(buf), reinterpret_cast< UINT8* >(addr), size, pExceptInfo);

    return bytesRead;
}

// This is small enough to be inlined, which stresses the code fetcher from the tool
// slightly more.
static VOID incCount() { insCount++; }

static VOID fini(INT32, VOID*) { cerr << insCount << " instructions in the main image executed" << endl; }

static VOID Trace(TRACE trace, VOID* arg)
{
    UINT8 bytes[16];
    size_t len = 0;

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)(incCount), IARG_END);
            len = PIN_FetchCode(bytes, (VOID*)INS_Address(ins), INS_Size(ins), 0);
            if (len != INS_Size(ins))
            {
                fprintf(stderr, "Error in fetcher callback call\n");
                exit(1);
            }
        }
    }
}

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    PIN_InitSymbols();

    // Initialize pin
    PIN_Init(argc, argv);

    PIN_AddFetchFunction(fetchIns, 0);
    TRACE_AddInstrumentFunction(Trace, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
