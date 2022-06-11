/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool performs IARG_MEMORYXX_SIZE check on fxsave and fxrstor
 */
#include <fstream>
#include <iostream>
#include <utility>
#include <cstdlib>
#include "pin.H"
using std::cerr;
using std::endl;
using std::ofstream;
using std::ostream;
using std::string;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "", "specify output file name");

const UINT32 FXSAVE_AREA_SIZE = 512;
static ostream* out           = NULL;

#define DELETE_OUT \
    if (&cerr != out) delete out

/* =====================================================================
 * Called upon bad command line argument
 * ===================================================================== */
INT32 Usage()
{
    cerr << "This tool performs IARG_MEMORYXX_SIZE check on fxsave and fxrstor" << endl;

    cerr << KNOB_BASE::StringKnobSummary() << endl;

    return 1;
}

/* =====================================================================
 * Called upon program finish
 * ===================================================================== */
VOID Fini(int, VOID* v)
{
    *out << "Fini" << endl;
    DELETE_OUT;
}

/* =====================================================================
 * The analysis routine that is instrumented before FXSAVE instruction
 * ===================================================================== */
VOID MemOpAnalysisFXSAVE(const UINT32 size)
{
    if (FXSAVE_AREA_SIZE != size)
    {
        *out << "mismatch of fxsave size. exiting..." << endl;
        DELETE_OUT;
        PIN_ExitProcess(1);
    }
    *out << "FXSAVE of size " << size << endl;
}

/* =====================================================================
 * The analysis routine that is instrumented before FXRSTOR instruction
 * ===================================================================== */
VOID MemOpAnalysisFXRSTOR(const UINT32 size)
{
    if (FXSAVE_AREA_SIZE != size)
    {
        *out << "mismatch of fxsave size. exiting..." << endl;
        DELETE_OUT;
        PIN_ExitProcess(1);
    }
    *out << "FXRSTOR of size " << size << endl;
}

/* =====================================================================
 * Iterate over a trace and instrument its memory related instructions
 * ===================================================================== */
VOID Trace(TRACE trace, VOID* v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            OPCODE oc = INS_Opcode(ins);
            if (INS_IsMemoryWrite(ins) && (XED_ICLASS_FXSAVE == oc || XED_ICLASS_FXSAVE64 == oc))
            {
                INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(MemOpAnalysisFXSAVE), IARG_MEMORYWRITE_SIZE, IARG_END);
            }
            if (INS_IsMemoryRead(ins) && (XED_ICLASS_FXRSTOR == oc || XED_ICLASS_FXRSTOR64 == oc))
            {
                INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(MemOpAnalysisFXRSTOR), IARG_MEMORYREAD_SIZE, IARG_END);
            }
        }
    }
}

/* =====================================================================
 * Entry point for the tool
 * ===================================================================== */
int main(int argc, CHAR* argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    // Initialize the output stream.
    const string fileName = KnobOutputFile.Value();
    out                   = (fileName.empty()) ? &cerr : new ofstream(fileName.c_str());
    if (NULL == out || out->fail())
    {
        cerr << "TOOL ERROR: Unable to open " << fileName << " for writing." << endl;
        return 1;
    }

    TRACE_AddInstrumentFunction(Trace, 0);

    // Never returns
    PIN_StartProgram();
    return 1;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
