/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>
#include "pin.H"
using std::string;

const ADDRINT BaseValue = 0x80808000;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "reg_inst_gx.out", "trace file");

std::ofstream Out;
PIN_LOCK Lock;

VOID Error(std::string where, THREADID tid, UINT32 r, ADDRINT expect, ADDRINT val)
{
    PIN_GetLock(&Lock, 1);
    Out << "Mismatch " << where << ": tid=" << std::dec << tid << " (G" << r << ")"
        << ", Expect " << std::hex << expect << ", Got " << std::hex << val << std::endl;
    PIN_ReleaseLock(&Lock);
}

VOID ThreadStart(THREADID tid, CONTEXT* ctxt, int flags, VOID* v)
{
    for (UINT32 r = 0; r <= 9; r++)
        PIN_SetContextReg(ctxt, REG(REG_INST_G0 + r), BaseValue + tid + r);
}

VOID ThreadFini(THREADID tid, const CONTEXT* ctxt, INT32 code, VOID* v)
{
    for (UINT32 r = 0; r <= 9; r++)
    {
        ADDRINT val    = PIN_GetContextReg(ctxt, REG(REG_INST_G0 + r));
        ADDRINT expect = BaseValue + tid + r;
        if (expect != val) Error("at thread exit", tid, r, expect, val);
    }
}

VOID OnIns(THREADID tid,
#if defined(TARGET_IA32) || defined(TARGET_IA32E)
           CONTEXT* ctxt,
#endif
           ADDRINT g0, ADDRINT g1, ADDRINT g2, ADDRINT g3, ADDRINT g4, ADDRINT g5, ADDRINT g6, ADDRINT g7, ADDRINT g8, ADDRINT g9)
{
    ADDRINT gx[10];
    gx[0] = g0;
    gx[1] = g1;
    gx[2] = g2;
    gx[3] = g3;
    gx[4] = g4;
    gx[5] = g5;
    gx[6] = g6;
    gx[7] = g7;
    gx[8] = g8;
    gx[9] = g9;

    for (UINT32 r = 0; r <= 9; r++)
    {
        ADDRINT expect = BaseValue + tid + r;

        if (expect != gx[r]) Error("on IARG_REG_VALUE", tid, r, expect, gx[r]);

#if defined(TARGET_IA32) || defined(TARGET_IA32E)
        ADDRINT val = PIN_GetContextReg(ctxt, REG(REG_INST_G0 + r));
        if (expect != val) Error("on IARG_CONTEXT", tid, r, expect, val);
#endif
    }
}

VOID Instruction(INS ins, void* unused)
{
    // Instrumenting every instruction is too slow.
    //
    static UINT32 Count = 0;
    if (++Count < 100) return;
    Count = 0;

    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)OnIns, IARG_THREAD_ID,
#if defined(TARGET_IA32) || defined(TARGET_IA32E)
                   IARG_CONTEXT,
#endif
                   IARG_REG_VALUE, REG_INST_G0, IARG_REG_VALUE, REG_INST_G1, IARG_REG_VALUE, REG_INST_G2, IARG_REG_VALUE,
                   REG_INST_G3, IARG_REG_VALUE, REG_INST_G4, IARG_REG_VALUE, REG_INST_G5, IARG_REG_VALUE, REG_INST_G6,
                   IARG_REG_VALUE, REG_INST_G7, IARG_REG_VALUE, REG_INST_G8, IARG_REG_VALUE, REG_INST_G9, IARG_END);
}

VOID Fini(INT32 code, VOID* v)
{
    Out << "(eof)" << std::endl;
    Out.close();
}

int main(INT32 argc, CHAR** argv)
{
    PIN_InitLock(&Lock);
    PIN_Init(argc, argv);

    PIN_InitLock(&Lock);

    Out.open(KnobOutputFile.Value().c_str());

    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
