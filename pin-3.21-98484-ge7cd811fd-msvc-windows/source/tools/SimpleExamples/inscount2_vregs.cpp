/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This tool demonstrates how to use a "vitual register" to hold
// thread-private tool data.  We implement an MT-safe tool that
// counts dynamic instructions, by accumulating each thread's
// instruction count into a thread-private virtual register.
// To allow the possibility of layering this tool on top of
// other Pin tools we use PIN_ClaimToolRegister to allocate the
// register we use.

#include <iostream>
#include <fstream>
#include "pin.H"
using std::cerr;
using std::endl;
using std::ofstream;
using std::string;

ofstream OutFile;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "inscount2_vregs.out", "specify output file name");

PIN_LOCK Lock;
UINT64 TotalCount = 0;
REG ScratchReg;

// First parameter is the number of instructions in this basic block.
// Second parameter is the current dynamic instruction count
// Return the new count
//
ADDRINT PIN_FAST_ANALYSIS_CALL DoCount(ADDRINT numInsts, ADDRINT count) { return count + numInsts; }

VOID ThreadStart(THREADID tid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    // When the thread starts, zero the virtual register that holds the
    // dynamic instruction count.
    //
    PIN_SetContextReg(ctxt, ScratchReg, 0);
}

VOID ThreadFini(THREADID tid, const CONTEXT* ctxt, INT32 code, VOID* v)
{
    // When the thread exits, accumulate the thread's dynamic instruction
    // count into the total.
    PIN_GetLock(&Lock, tid + 1);
    TotalCount += PIN_GetContextReg(ctxt, ScratchReg);
    PIN_ReleaseLock(&Lock);
}

VOID Trace(TRACE trace, VOID* v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        // The virtual register ScratchReg holds the dynamic instruction
        // count for each thread.  DoCount returns the sum of the basic
        // block instruction count and G0, we write the result back to G0
        BBL_InsertCall(bbl, IPOINT_ANYWHERE, AFUNPTR(DoCount), IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, BBL_NumIns(bbl),
                       IARG_REG_VALUE, ScratchReg, IARG_RETURN_REGS, ScratchReg, IARG_END);
    }
}

VOID Fini(INT32 code, VOID* v)
{
    OutFile << "Count= " << TotalCount << endl;
    OutFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This Pintool counts the number of dynamic instructions executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // We accumlate counts into a register, make sure it is 64 bits to
    // avoid overflow
    ASSERTX(sizeof(ADDRINT) == sizeof(UINT64));

    if (PIN_Init(argc, argv)) return Usage();

    OutFile.open(KnobOutputFile.Value().c_str());

    PIN_InitLock(&Lock);

    ScratchReg = PIN_ClaimToolRegister();
    if (!REG_valid(ScratchReg))
    {
        std::cerr << "Cannot allocate a scratch register.\n";
        std::cerr << std::flush;
        return 1;
    }

    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);
    PIN_AddFiniFunction(Fini, 0);

    TRACE_AddInstrumentFunction(Trace, 0);

    PIN_StartProgram();
    return 0;
}
