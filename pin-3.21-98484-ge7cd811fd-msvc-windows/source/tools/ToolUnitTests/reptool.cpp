/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 *  Record the memory read and written by variable size instructions
 */

#include <stdio.h>
#include "pin.H"
using std::string;

/* Introduce ambiguity into the definition of UINT32.
 * Now we have two: LEVEL_BASE::UINT32 and breakUINT32::UINT32.
 * Since there are no references to UINT32 in this code, it should be fine
 * to do this, but this provides a check for Mantis 2344, which was
 * that IARG_END used a naked UINT32, rather than LEVEL_BASE::UINT32.
 */

namespace breakUINT32
{
typedef void UINT32;
}

using namespace breakUINT32;

KNOB< string > KnobOutput(KNOB_MODE_WRITEONCE, "pintool", "o", "reptool.out", "output file");

FILE* out    = 0;
UINT8 oldVal = 0;
UINT8* ea;

// Change the first byte of string
VOID Change(UINT8* addr)
{
    ea = addr;

    oldVal = *ea;
    *ea    = '1';
}

// Restore first byte of string
VOID Restore() { *ea = oldVal; }

VOID SaveEa(UINT8* addr) { ea = addr; }

VOID RecordMemory(VOID* ip, ADDRINT testNum, char* type, UINT8* addr, INT32 size)
{
    // We pass addr == 0 at IPOINT_AFTER, and pick up the address we saved at IPOINT_BEFORE.
    if (addr == 0)
    {
        addr = ea;
    }
    else
    {
        ea = addr;
    }

    fprintf(out, "Test %lu ", (unsigned long)testNum);
    fprintf(out, "%s size %d: ", type, size);
    for (INT32 i = 0; i < size; i++)
    {
        fprintf(out, "%c", addr[i]);
    }

    fprintf(out, "\n");
}

// Is called for every instruction and instruments reads and writes
VOID Trace(TRACE trace, VOID* v)
{
    // Ignore everything that is not in the main function
    RTN rtn = TRACE_Rtn(trace);
    if (!RTN_Valid(rtn)) return;

    if (RTN_Name(rtn) != "main") return;

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            if (!INS_HasRealRep(ins)) continue;

            if (INS_IsMemoryRead(ins))
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)Change, IARG_MEMORYREAD_EA, IARG_END);

                INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordMemory, IARG_INST_PTR, IARG_REG_VALUE, REG_EDX,
                                         IARG_PTR, "read1", IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE, IARG_END);

                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)Restore, IARG_END);

                INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordMemory, IARG_INST_PTR, IARG_REG_VALUE, REG_EDX,
                                         IARG_PTR, "read1", IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE, IARG_END);
            }

            if (INS_HasMemoryRead2(ins))
            {
                INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordMemory, IARG_INST_PTR, IARG_REG_VALUE, REG_EDX,
                                         IARG_PTR, "read2", IARG_MEMORYREAD2_EA, IARG_MEMORYREAD_SIZE,
                                         // IARG_CONTEXT requires a bridge and tests liveness of
                                         // helper virtual registers
                                         IARG_CONTEXT, IARG_END);
            }

            if (INS_IsMemoryWrite(ins))
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)SaveEa, IARG_MEMORYWRITE_EA, IARG_END);

                INS_InsertPredicatedCall(ins, IPOINT_AFTER, (AFUNPTR)RecordMemory, IARG_INST_PTR, IARG_REG_VALUE, REG_EDX,
                                         IARG_PTR, "write", IARG_ADDRINT, ADDRINT(0), IARG_MEMORYWRITE_SIZE, IARG_END);
            }
        }
    }
}

VOID Fini(INT32 code, VOID* v)
{
    if (out != stdout) fclose(out);
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    out = fopen(KnobOutput.Value().c_str(), "w");
    if (!out)
    {
        fprintf(stderr, "Can't open output file... using stdout\n");
        out = stdout;
    }

    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
