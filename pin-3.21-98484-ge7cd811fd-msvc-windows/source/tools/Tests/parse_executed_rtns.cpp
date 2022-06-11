/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* This tool shows how to examine only RTNs that are actually executed */

#include <stdio.h>
#include "pin.H"

int numRtnsParsed = 0;
static VOID Trace(TRACE trace, VOID* v)
{
    INS ins = BBL_InsHead(TRACE_BblHead(trace));
    RTN rtn = INS_Rtn(ins);

    if (!RTN_Valid(rtn))
    {
        return;
    }

    if (INS_Address(ins) == RTN_Address(rtn))
    {
        /* The first ins of an RTN that will be executed - it is possible at this point to examine all the INSs 
           of the RTN that Pin can statically identify (using whatever standard symbol information is available).
           A tool may wish to parse each such RTN only once, if so it will need to record and identify which RTNs 
           have already been parsed
        */
        numRtnsParsed++;
        printf("\nfound the RTN %s   RTN instructions:\n", RTN_Name(rtn).c_str());
        RTN_Open(rtn);
        for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
        {
            printf("%p %s\n", reinterpret_cast< void* >(INS_Address(ins)), INS_Disassemble(ins).c_str());
        }
        RTN_Close(rtn);
    }
}

VOID Fini(INT32 code, VOID* v) { ASSERTX(numRtnsParsed != 0); }

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddFiniFunction(Fini, 0);
    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
