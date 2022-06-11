/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 *  Record the memory read and written by variable size instructions. This test checks for the bug
 *  that caused xor reg1, reg2   to be changed to   mov  reg1, 0    even when reg1 != reg2.
 *  The tool is run with the align_access_app
 */

#include "pin.H"

#include <stdlib.h>
#include <stdio.h>
#include <vector>
using std::string;
using std::vector;

KNOB< string > KnobOutput(KNOB_MODE_WRITEONCE, "pintool", "o", "align_check.out", "output file");

UINT32 minAlign = 0xFFFFFFFF;

struct INS_INFO
{
    ADDRINT addr;
    UINT32 align;
    UINT32 xorVal;

    INS_INFO(ADDRINT ip) : addr(ip), align(minAlign) {}
    INS_INFO() : addr(0), align(minAlign) {}
};

typedef vector< INS_INFO* > INFO_ARRAY;

INFO_ARRAY infoArray;

ADDRINT x;
VOID CalcAlignment(INS_INFO* info, ADDRINT ea)
{
    ADDRINT align = ea ^ (ea - 1);
    info->xorVal  = align;
    info->align &= align;
}

// Is called for every instruction and instruments reads and writes
VOID Trace(TRACE trace, VOID* v)
{
    // Ignore everything that is not in the main function
    RTN rtn = TRACE_Rtn(trace);
    if (!RTN_Valid(rtn))
    {
        printf("!RTN_Valid(rtn)\n");
        return;
    }

    if (RTN_Name(rtn) != "set_value" && RTN_Name(rtn) != "_set_value")
    {
        return;
    }

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            if (INS_IsMemoryRead(ins) && !INS_IsStackRead(ins))
            {
                INS_INFO* info = new INS_INFO(INS_Address(ins));
                infoArray.push_back(info);
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)CalcAlignment, IARG_PTR, info, IARG_MEMORYREAD_EA, IARG_END);
            }

            if (INS_IsMemoryWrite(ins) && !INS_IsStackWrite(ins))
            {
                INS_INFO* info = new INS_INFO(INS_Address(ins));
                infoArray.push_back(info);
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)CalcAlignment, IARG_PTR, info, IARG_MEMORYWRITE_EA, IARG_END);
            }
        }
    }
}

VOID Fini(INT32 code, VOID* v)
{
    FILE* out = fopen(KnobOutput.Value().c_str(), "w");
    if (!out)
    {
        fprintf(stderr, "Can't open output file... using stdout\n");
        out = stdout;
    }

    vector< INS_INFO* >::iterator iter = infoArray.begin();
    while (iter != infoArray.end())
    {
        INS_INFO* info = *iter;
        UINT32 align   = (info->align + 1) >> 1;
        fprintf(out, "Minimal alignment for ip %p is %d\n", (void*)(info->addr), align);
        if (align == 0)
        {
            fprintf(out, "  ***Error - got alignment of 0\n");
            printf("  ***Error - got alignment of 0\n");
            exit(-1);
        }
        if (info->xorVal == 0)
        {
            fprintf(out, "  ***Error - got xor is 0\n");
            printf("  ***Error - got xor is 0\n");
            exit(-1);
        }
        iter++;
    }

    if (out != stdout)
    {
        fclose(out);
    }
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
