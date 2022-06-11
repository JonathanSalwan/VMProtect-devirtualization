/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool preforms various test cases which are related to PIN's behavior
 * when it tries to instrument instructions with non-standard memory operand
 * such as XSAVE/GATHER/SCATER.
  */
#include "pin.H"
#include <fstream>
#include <iostream>
#include <utility>
#include <cstdlib>
using std::cerr;
using std::endl;
using std::hex;
using std::ofstream;
using std::string;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "standard-memop.out", "specify output file name");

// The knob below specifies the testcase number to perform.
// Available test cases:
// 0: Instrument xsave typed instructions with IARG_MEMORYWRITE_EA.
// 1: Instrument xsave typed instructions with IARG_MEMORYWRITE_SIZE.
KNOB< int > KnobTestCase(KNOB_MODE_WRITEONCE, "pintool", "c", "0",
                         "specify the test case number to perform (0/IARG_MEMORYWRITE_EA 1/IARG_MEMORYWRITE_SIZE)");

// The knob below specifies the instrumentation point to perform.
// Available points cases:
// 0: Instrument before the instruction.
// 1: Instrument after the instruction.
KNOB< int > KnobIpoint(KNOB_MODE_WRITEONCE, "pintool", "i", "0", "specify the instrumentation point (0/before 1/after)");

// The knob below specifies if it is regular instrumentation or if/then instrumentation.
// 0: regular instrumentation.
// 1: if/then intrumentation.
KNOB< int > KnobIfThen(KNOB_MODE_WRITEONCE, "pintool", "f", "0",
                       "specify if to use if/then instrumentation point (0/regular 1/if-then)");

const static IARG_TYPE TESTCASE_IARG_WRITE[] = {
    IARG_MEMORYWRITE_EA,
    IARG_MEMORYWRITE_SIZE,
};

const static IARG_TYPE TESTCASE_IARG_READ[] = {
    IARG_MEMORYREAD_EA,
    IARG_MEMORYREAD_SIZE,
};

const static IPOINT TESTCASE_IPOINT[] = {
    IPOINT_BEFORE,
    IPOINT_AFTER,
};

static ofstream* out = NULL;

/* =====================================================================
 * Called upon bad command line argument
 * ===================================================================== */
INT32 Usage()
{
    cerr << "This tool preforms various test cases which are related to PIN's behavior" << endl
         << "when it tries to instrument XSAVE family instructions which has non-standard memory operand" << endl;

    cerr << KNOB_BASE::StringKnobSummary();

    cerr << endl;

    return -1;
}

/* =====================================================================
 * Called upon program finish
 * ===================================================================== */
VOID Fini(int, VOID* v)
{
    *out << "Fini" << endl;
    out->close();
    out = NULL;
}

/* =====================================================================
 * The analysis routine that is instrumented before/after XSAVE instruction
 * ===================================================================== */
VOID MemOpAnalysisXSAVE_EA(ADDRINT addr) { *out << hex << "XSAVE on 0x" << addr << endl; }

VOID MemOpAnalysisXSAVE_Size(UINT32 addr) { *out << hex << "XSAVE of size 0x" << addr << endl; }

/* =====================================================================
 * The analysis routine that is instrumented before/after XSAVEOPT instruction
 * ===================================================================== */
VOID MemOpAnalysisXSAVEOPT_EA(ADDRINT addr) { *out << hex << "XSAVEOPT on 0x" << addr << endl; }

VOID MemOpAnalysisXSAVEOPT_Size(UINT32 addr) { *out << hex << "XSAVEOPT of size 0x" << addr << endl; }

/* =====================================================================
 * The analysis routine that is instrumented before/after XRSTOR instruction
 * ===================================================================== */
VOID MemOpAnalysisXRSTOR_EA(ADDRINT addr) { *out << hex << "XRSTOR on 0x" << addr << endl; }

VOID MemOpAnalysisXRSTOR_Size(UINT32 addr) { *out << hex << "XRSTOR of size 0x" << addr << endl; }

BOOL AlwaysTrue() { return TRUE; }
/* =====================================================================
 * Iterate over a trace and instrument its memory related instructions
 * ===================================================================== */
VOID Trace(TRACE trace, VOID* v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            if (INS_IsMemoryWrite(ins) && INS_Category(ins) == XED_CATEGORY_XSAVE)
            {
                INS_InsertCall(ins, TESTCASE_IPOINT[KnobIpoint.Value()],
                               KnobTestCase.Value() ? AFUNPTR(MemOpAnalysisXSAVE_Size) : AFUNPTR(MemOpAnalysisXSAVE_EA),
                               TESTCASE_IARG_WRITE[KnobTestCase.Value()], IARG_END);
            }
            if (INS_IsMemoryWrite(ins) && INS_Category(ins) == XED_CATEGORY_XSAVEOPT)
            {
                INS_InsertCall(ins, TESTCASE_IPOINT[KnobIpoint.Value()],
                               KnobTestCase.Value() ? AFUNPTR(MemOpAnalysisXSAVEOPT_Size) : AFUNPTR(MemOpAnalysisXSAVEOPT_EA),
                               TESTCASE_IARG_WRITE[KnobTestCase.Value()], IARG_END);
            }
            if (INS_IsMemoryRead(ins) && INS_Opcode(ins) == XED_ICLASS_XRSTOR)
            {
                INS_InsertCall(ins, TESTCASE_IPOINT[KnobIpoint.Value()],
                               KnobTestCase.Value() ? AFUNPTR(MemOpAnalysisXRSTOR_Size) : AFUNPTR(MemOpAnalysisXRSTOR_EA),
                               TESTCASE_IARG_READ[KnobTestCase.Value()], IARG_END);
            }
        }
    }
}

/* ================================================================================================
 * Iterate over a trace and instrument its memory related instructions with if/then instrumentation
 * =================================================================================================
 */
VOID TraceIfThen(TRACE trace, VOID* v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            if (INS_IsMemoryWrite(ins) && INS_Category(ins) == XED_CATEGORY_XSAVE)
            {
                INS_InsertIfCall(ins, TESTCASE_IPOINT[KnobIpoint.Value()], AFUNPTR(AlwaysTrue), IARG_END);
                INS_InsertThenCall(ins, TESTCASE_IPOINT[KnobIpoint.Value()],
                                   KnobTestCase.Value() ? AFUNPTR(MemOpAnalysisXSAVE_Size) : AFUNPTR(MemOpAnalysisXSAVE_EA),
                                   TESTCASE_IARG_WRITE[KnobTestCase.Value()], IARG_END);
            }
            if (INS_IsMemoryWrite(ins) && INS_Category(ins) == XED_CATEGORY_XSAVEOPT)
            {
                INS_InsertIfCall(ins, TESTCASE_IPOINT[KnobIpoint.Value()], AFUNPTR(AlwaysTrue), IARG_END);
                INS_InsertThenCall(ins, TESTCASE_IPOINT[KnobIpoint.Value()],
                                   KnobTestCase.Value() ? AFUNPTR(MemOpAnalysisXSAVEOPT_Size) : AFUNPTR(MemOpAnalysisXSAVEOPT_EA),
                                   TESTCASE_IARG_WRITE[KnobTestCase.Value()], IARG_END);
            }
            if (INS_IsMemoryRead(ins) && INS_Opcode(ins) == XED_ICLASS_XRSTOR)
            {
                INS_InsertIfCall(ins, TESTCASE_IPOINT[KnobIpoint.Value()], AFUNPTR(AlwaysTrue), IARG_END);
                INS_InsertThenCall(ins, TESTCASE_IPOINT[KnobIpoint.Value()],
                                   KnobTestCase.Value() ? AFUNPTR(MemOpAnalysisXRSTOR_Size) : AFUNPTR(MemOpAnalysisXRSTOR_EA),
                                   TESTCASE_IARG_READ[KnobTestCase.Value()], IARG_END);
            }
        }
    }
}

/* =====================================================================
 * Entry point for the tool
 * ===================================================================== */
int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }
    out = new std::ofstream(KnobOutputFile.Value().c_str());
    if (KnobTestCase < 0 || KnobTestCase > (int)(sizeof(TESTCASE_IARG_WRITE) / sizeof(TESTCASE_IARG_WRITE[0]) - 1))
    {
        *out << "Bad test case number specified on command line: " << KnobTestCase << endl;
        return 4;
    }
    if (KnobIpoint < 0 || KnobIpoint > (int)(sizeof(TESTCASE_IPOINT) / sizeof(TESTCASE_IPOINT[0]) - 1))
    {
        *out << "Bad test case number specified on command line: " << KnobIpoint << endl;
        return 4;
    }

    if (KnobIfThen)
    {
        TRACE_AddInstrumentFunction(TraceIfThen, 0);
    }
    else
    {
        TRACE_AddInstrumentFunction(Trace, 0);
    }

    // Never returns
    PIN_StartProgram();
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
