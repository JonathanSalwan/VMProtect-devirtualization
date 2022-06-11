/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <fstream>
#include <iostream>
#include <set>
#include <utility>
#include <cstdlib>
using std::cerr;
using std::endl;
using std::hex;
using std::ofstream;
using std::pair;
using std::set;
using std::string;

#ifdef TARGET_MAC
#define NAME(x) "_" x
#else
#define NAME(x) x
#endif

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "iarg_explicit_memory_ea.out", "specify output file name");

static ofstream* out = NULL;
static set< pair< ADDRINT, ADDRINT > > memOpAddresses;

THREADID myThread = INVALID_THREADID;

ADDRINT IfMyThread(THREADID threadId) { return threadId == myThread; }

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    if (myThread == INVALID_THREADID)
    {
        myThread = threadid;
    }
}

/* =====================================================================
 * Called upon bad command line argument
 * ===================================================================== */
INT32 Usage()
{
    cerr << "This pin tool instruments memory instructions with explicit operand, "
         << " so the address the're refering to will be printed\n"
            "\n";

    cerr << KNOB_BASE::StringKnobSummary();

    cerr << endl;

    return -1;
}

/* =====================================================================
 * Called upon program finish
 * ===================================================================== */
VOID Fini(int, VOID* v) { *out << "Fini" << endl; }

/* =====================================================================
 * The analysis routine that is instrumented before any memory operand instruction
 * ===================================================================== */
VOID MemOpAnalysis(ADDRINT pc, ADDRINT addr)
{
    memOpAddresses.insert(pair< ADDRINT, ADDRINT >(pc, addr));
    *out << hex << "At PC=" << pc << " Memory operation with address " << addr << endl;
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
            if (INS_HasExplicitMemoryReference(ins))
            {
                INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
                INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(MemOpAnalysis), IARG_INST_PTR, IARG_EXPLICIT_MEMORY_EA, IARG_END);
            }
        }
    }
}

/* =====================================================================
 * This function should replace the checkVar function of the application
 * ===================================================================== */
void CheckVarReplaced(const char* name, void* pc, void* value)
{
    *out << "CheckVar called for " << name << ", value " << value << " at address " << pc << endl;
    pair< ADDRINT, ADDRINT > p((ADDRINT)pc, (ADDRINT)value);
    if (memOpAddresses.end() == memOpAddresses.find(p))
    {
        *out << "Instruction for " << name << " at " << pc << " with operand " << value << " wasn't caught in instrumentation"
             << endl;
        out->flush();
        exit(4);
    }
}

/* =====================================================================
 * Called upon image load to instrument the function checkVar
 * ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        RTN rtn = RTN_FindByName(img, NAME("checkVar"));
        if (!RTN_Valid(rtn))
        {
            *out << "Cannot find routine " << NAME("checkVar") << " in main image" << endl;
            out->flush();
            exit(3);
        }
        RTN_Replace(rtn, AFUNPTR(CheckVarReplaced));
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
    IMG_AddInstrumentFunction(ImageLoad, NULL);
    TRACE_AddInstrumentFunction(Trace, NULL);

    PIN_AddThreadStartFunction(ThreadStart, NULL);

    PIN_AddFiniFunction(Fini, NULL);

    // Never returns
    PIN_StartProgram();
    return 1;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
