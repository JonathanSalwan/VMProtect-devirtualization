/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <stdlib.h>
#include "pin.H"
using std::endl;

UINT64 icount = 0;
UINT64 error  = 0;

VOID* pc       = 0;
VOID* next_pc  = 0;
BOOL isSkipped = TRUE; // always skip checking the first inst

VOID CountError()
{
    error++;
    if (error > 100)
    {
        std::cerr << "Too many errors, giving up" << endl;
        exit(error);
    }
}

VOID CheckPc(VOID* iaddr)
{
    if (next_pc != iaddr && !isSkipped)
    {
        std::cerr << "error: at PC " << pc << " computed target " << next_pc << " but executing at " << iaddr << endl;
        CountError();
    }

    isSkipped = FALSE;

    icount++;
    if ((icount % 1000) == 0)
    {
        std::cerr << icount << " BBLs checked" << endl;
    }
}

// record PC of following instruction
VOID RecordPc(VOID* iaddr, VOID* target, BOOL taken)
{
    if (!taken) return;

    pc      = iaddr;
    next_pc = target;
}

VOID Skip() { isSkipped = TRUE; }

VOID Trace(TRACE trace, VOID* v)
{
    static BOOL programStart = TRUE;

    if (programStart)
    {
        programStart = FALSE;
        next_pc      = (void*)INS_Address(BBL_InsHead(TRACE_BblHead(trace)));
    }

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        // check BBL entry PC
        INS_InsertCall(BBL_InsHead(bbl), IPOINT_BEFORE, (AFUNPTR)CheckPc, IARG_INST_PTR, IARG_END);

        INS tail = BBL_InsTail(bbl);

        if (INS_IsControlFlow(tail))
        {
            // record taken branch targets
            INS_InsertCall(tail, IPOINT_BEFORE, AFUNPTR(RecordPc), IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN,
                           IARG_END);
        }

        if (INS_IsValidForIpointAfter(tail))
        {
            // record fall-through
            INS_InsertCall(tail, IPOINT_AFTER, (AFUNPTR)RecordPc, IARG_INST_PTR, IARG_FALLTHROUGH_ADDR, IARG_BOOL, TRUE,
                           IARG_END);
        }

#if defined(TARGET_IA32) || defined(TARGET_IA32E)
        if (INS_IsSysenter(tail) || INS_HasRealRep(tail))
        { // sysenter on x86 has some funny control flow that we can't correctly verify for now
            // Genuinely REP prefixed instructions are also odd, they appear to stutter.
            INS_InsertCall(tail, IPOINT_BEFORE, (AFUNPTR)Skip, IARG_END);
        }
#endif
    }
}

VOID SyscallEntry(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v) { Skip(); }

VOID Fini(INT32 code, VOID* v)
{
    std::cerr << error << " errors (" << icount << " BBLs checked)" << endl;

    if (code) exit(code);

    if (error > 0) exit(error);
}

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);
    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddSyscallEntryFunction(SyscallEntry, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
