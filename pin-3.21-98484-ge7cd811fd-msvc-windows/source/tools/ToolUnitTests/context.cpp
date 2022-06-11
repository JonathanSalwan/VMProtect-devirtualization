/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <string.h>
using std::endl;
using std::hex;
using std::ios;
using std::ofstream;
using std::string;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "context.out", "specify output file name");

ofstream out;

ADDRINT BeforeIP = 0x0;

int failure = 0;

THREADID myThread = INVALID_THREADID;

ADDRINT IfMyThread(THREADID threadId) { return threadId == myThread; }

#if defined(TARGET_IA32) || defined(TARGET_IA32E)

VOID PrintContext(const CONTEXT* ctxt)
{
    out << "ip:    " << PIN_GetContextReg(ctxt, REG_INST_PTR) << endl;
    out << "gax:   " << PIN_GetContextReg(ctxt, REG_GAX) << endl;
    out << "gbx:   " << PIN_GetContextReg(ctxt, REG_GBX) << endl;
    out << "gcx:   " << PIN_GetContextReg(ctxt, REG_GCX) << endl;
    out << "gdx:   " << PIN_GetContextReg(ctxt, REG_GDX) << endl;
    out << "gsi:   " << PIN_GetContextReg(ctxt, REG_GSI) << endl;
    out << "gdi:   " << PIN_GetContextReg(ctxt, REG_GDI) << endl;
    out << "gbp:   " << PIN_GetContextReg(ctxt, REG_GBP) << endl;
    out << "sp:    " << PIN_GetContextReg(ctxt, REG_ESP) << endl;

    out << "ss:    " << PIN_GetContextReg(ctxt, REG_SEG_SS) << endl;
    out << "cs:    " << PIN_GetContextReg(ctxt, REG_SEG_CS) << endl;
    out << "ds:    " << PIN_GetContextReg(ctxt, REG_SEG_DS) << endl;
    out << "es:    " << PIN_GetContextReg(ctxt, REG_SEG_ES) << endl;
    out << "fs:    " << PIN_GetContextReg(ctxt, REG_SEG_FS) << endl;
    out << "gs:    " << PIN_GetContextReg(ctxt, REG_SEG_GS) << endl;
    out << "gflags:" << PIN_GetContextReg(ctxt, REG_GFLAGS) << endl;
    out << endl;
}

VOID CheckFpState(CONTEXT* ctxt)
{
    FPSTATE fpState;
    FPSTATE fpStateCopy;
    // need to clear the fp state as Get/Set FP state will not copy over all the state
    // e.g. in case we are running on machine without AVX/AVX512 support
    memset(&fpState, 0, FPSTATE_SIZE);
    memset(&fpStateCopy, 0, FPSTATE_SIZE);
    PIN_GetContextFPState(ctxt, &fpState);
    PIN_SetContextFPState(ctxt, &fpState);
    PIN_GetContextFPState(ctxt, &fpStateCopy);
    if (memcmp(&fpState, &fpStateCopy, FPSTATE_SIZE) != 0)
    {
        failure++;
        out << "Failure!! Mismatch in FP state" << endl;
    }
}

#else

VOID PrintContext(const CONTEXT* ctxt) {}
VOID CheckFpState(CONTEXT* ctxt) {}

#endif

VOID SetBeforeContext(CONTEXT* ctxt)
{
    BeforeIP = (ADDRINT)PIN_GetContextReg(ctxt, REG_INST_PTR);
    CheckFpState(ctxt);
}

VOID ShowAfterContext(const CONTEXT* ctxt)
{
    ADDRINT AfterIP = (ADDRINT)PIN_GetContextReg(ctxt, REG_INST_PTR);

    if (BeforeIP == AfterIP)
    {
        failure++;
        out << "Failure!! Before Context IP = " << BeforeIP << ", After Context IP = " << AfterIP << endl;
    }
    else
    {
        //        out << "Success!! Before Context IP = " << BeforeIP << ", After Context IP = " << AfterIP << endl;
    }
}

VOID ShowTakenBrContext(const CONTEXT* ctxt)
{
    ADDRINT TakenIP = (ADDRINT)PIN_GetContextReg(ctxt, REG_INST_PTR);

    if (BeforeIP == TakenIP)
    {
        failure++;
        out << "Failure!! Before Context IP = " << BeforeIP << ", Branch Taken Context IP = " << TakenIP << endl;
    }
    else
    {
        //        out << "Success!! Before Context IP = " << BeforeIP << ", Branch Taken Context IP = " << TakenIP << endl;
    }
}

VOID ShowTakenIndirBrContext(const CONTEXT* ctxt)
{
    ADDRINT TakenIP = (ADDRINT)PIN_GetContextReg(ctxt, REG_INST_PTR);

    if (BeforeIP == TakenIP)
    {
        failure++;
        out << "Failure!! Before Context IP = " << BeforeIP << ", Indirect Branch Taken Context IP = "
            << "unknown" << endl;
    }
    else
    {
        //        out << "Success!! Before Context IP = " << BeforeIP << ", Indirect Branch Taken Context IP = " << "unknown" << endl;
    }
}

VOID Instruction(INS ins, VOID* v)
{
    if (INS_IsControlFlow(ins))
    {
        INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, AFUNPTR(SetBeforeContext), IARG_CONTEXT, IARG_END);

        if (INS_IsBranch(ins) && INS_IsValidForIpointAfter(ins))
        {
            INS_InsertIfCall(ins, IPOINT_AFTER, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
            INS_InsertThenCall(ins, IPOINT_AFTER, AFUNPTR(ShowAfterContext), IARG_CONTEXT, IARG_END);
        }

        if (INS_IsDirectControlFlow(ins) && INS_IsValidForIpointTakenBranch(ins))
        {
            INS_InsertIfCall(ins, IPOINT_TAKEN_BRANCH, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
            INS_InsertThenCall(ins, IPOINT_TAKEN_BRANCH, AFUNPTR(ShowTakenBrContext), IARG_CONTEXT, IARG_END);
        }

        if (INS_IsIndirectControlFlow(ins) && INS_IsValidForIpointTakenBranch(ins))
        {
            INS_InsertIfCall(ins, IPOINT_TAKEN_BRANCH, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
            INS_InsertThenCall(ins, IPOINT_TAKEN_BRANCH, AFUNPTR(ShowTakenIndirBrContext), IARG_CONTEXT, IARG_END);
        }
    }
}

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    if (myThread == INVALID_THREADID)
    {
        myThread = threadid;
    }
}

VOID Fini(INT32 code, VOID* v)
{
    if (failure > 0)
        out << "Test detected " << failure << " failures." << endl;
    else
        out << "Success!! Test passed!" << endl;
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    out.open(KnobOutputFile.Value().c_str());
    out << hex;
    out.setf(ios::showbase);

    PIN_AddThreadStartFunction(ThreadStart, NULL);
    INS_AddInstrumentFunction(Instruction, NULL);
    PIN_AddFiniFunction(Fini, NULL);

    // Never returns
    PIN_StartProgram();

    return 0;
}
