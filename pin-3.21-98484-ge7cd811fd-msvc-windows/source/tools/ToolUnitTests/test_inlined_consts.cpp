/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>
#include "pin.H"

const UINT32 val2  = 0xbaadf00d;
const ADDRINT val3 = 0xacdcacdcacdcacdc;

extern "C" ADDRINT uint32Glob  = 0;
extern "C" ADDRINT addrIntGlob = 0;
int first                      = TRUE;

using std::cout;
using std::endl;
using std::hex;

extern "C" VOID PIN_FAST_ANALYSIS_CALL GetConsts(UINT32 uint32, ADDRINT addrint, ADDRINT whereToSave);
/*
{ 
    uint32Glob = uint32; 
    addrIntGlob = addrint;
}
*/

VOID Trace(TRACE trace, VOID* v)
{
    if (first)
    {
        BBL_InsertCall(TRACE_BblHead(trace), IPOINT_BEFORE, AFUNPTR(GetConsts), IARG_FAST_ANALYSIS_CALL, IARG_UINT32, val2,
                       IARG_ADDRINT, val3, IARG_END);
        first = FALSE;
    }
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v)
{
    cout << hex << " uint32Glob " << uint32Glob << " addrIntGlob " << addrIntGlob << endl;
    if (uint32Glob != val2)
    {
        cout << "***ERROR uint32Glob" << endl;
        exit(-1);
    }
    if (addrIntGlob != val3)
    {
        cout << "***ERROR addrIntGlob" << endl;
        exit(-1);
    }
}

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    // Initialize pin
    PIN_Init(argc, argv);

    // Register Instruction to be called to instrument instructions
    TRACE_AddInstrumentFunction(Trace, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
