/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <cassert>
#include <stdlib.h>
#include "pin.H"
using std::endl;

KNOB< std::string > KnobOutfile(KNOB_MODE_WRITEONCE, "pintool", "o", "indirect_call_branch_target_addr.out", "Output file name");

std::ofstream Out;

BOOL haveError = FALSE;

BOOL justExecutedIndirectThruStackCall                   = FALSE;
ADDRINT savedBranchTargetAddr                            = 0;
BOOL calledIpointBeforeIndirectThruStackCall             = FALSE;
BOOL calledIpointBeforeIndirectThruStackCall1            = FALSE;
BOOL calledIpointBeforeIndirectThruStackCall2            = FALSE;
BOOL calledIpointTakenBranchIndirectThruStackCall        = FALSE;
BOOL calledIpointTakenBranchIndirectThruStackCall1       = FALSE;
BOOL calledIpointTakenBranchIndirectThruStackCall2       = FALSE;
BOOL calledPreviousInsExecutedWasAnIndirectCallThruStack = FALSE;

static void IpointBeforeIndirectThruStackCall(ADDRINT branchTargetAddr, ADDRINT rsp, ADDRDELTA displacementSigned, ADDRINT rip,
                                              CONTEXT* ctxt)
{
    calledIpointBeforeIndirectThruStackCall = TRUE;
    ADDRINT* ptr1                           = (ADDRINT*)(rsp + displacementSigned);
    justExecutedIndirectThruStackCall       = TRUE;
    Out << "IpointBeforeIndirectThruStackCall at rip " << std::hex << rip << " branchTargetAddr " << std::hex << branchTargetAddr
        << "  rsp " << std::hex << rsp << " context rsp " << PIN_GetContextReg(ctxt, REG_STACK_PTR) << " [rsp+" << std::hex
        << displacementSigned << "]==(" << std::hex << (rsp + displacementSigned) << ") contains " << std::hex << *ptr1 << endl;
    if (*ptr1 != branchTargetAddr)
    {
        Out << "***Error expected *ptr1 == branchTargetAddr" << endl;
        exit(-1);
    }
    savedBranchTargetAddr = branchTargetAddr;
}

static void IpointBeforeIndirectThruStackCall1(ADDRINT branchTargetAddr, ADDRINT rsp, ADDRDELTA displacementSigned, ADDRINT rip,
                                               CONTEXT* ictxt)
{
    calledIpointBeforeIndirectThruStackCall1 = TRUE;

    ADDRINT* ptr1                     = (ADDRINT*)(rsp + displacementSigned);
    justExecutedIndirectThruStackCall = TRUE;
    Out << "IpointBeforeIndirectThruStackCall1 at rip " << std::hex << rip << " branchTargetAddr " << std::hex << branchTargetAddr
        << "  rsp " << std::hex << rsp << " [rsp+" << std::hex << displacementSigned << "]==(" << std::hex
        << (rsp + displacementSigned) << ") contains " << std::hex << *ptr1 << endl;
    if (*ptr1 != branchTargetAddr)
    {
        Out << "***Error expected *ptr1 == branchTargetAddr" << endl;
        exit(-1);
    }
    if (savedBranchTargetAddr != branchTargetAddr)
    {
        Out << "***Error expected savedBranchTargetAddr == branchTargetAddr" << endl;
        exit(-1);
    }
}

static void IpointBeforeIndirectThruStackCall2(ADDRINT branchTargetAddr, ADDRINT rsp, ADDRDELTA displacementSigned, ADDRINT rip)
{
    calledIpointBeforeIndirectThruStackCall2 = TRUE;

    ADDRINT* ptr1                     = (ADDRINT*)(rsp + displacementSigned);
    justExecutedIndirectThruStackCall = TRUE;
    Out << "IpointBeforeIndirectThruStackCall2 at rip " << std::hex << rip << " branchTargetAddr " << std::hex << branchTargetAddr
        << "  rsp " << std::hex << rsp << " [rsp+" << std::hex << displacementSigned << "]==(" << std::hex
        << (rsp + displacementSigned) << ") contains " << std::hex << *ptr1 << endl;
    if (*ptr1 != branchTargetAddr)
    {
        Out << "***Error expected *ptr1 == branchTargetAddr" << endl;
        exit(-1);
    }
    if (savedBranchTargetAddr != branchTargetAddr)
    {
        Out << "***Error expected savedBranchTargetAddr == branchTargetAddr" << endl;
        exit(-1);
    }
}

static void IpointTakenBranchIndirectThruStackCall(ADDRINT branchTargetAddr, ADDRINT rip, CONTEXT* ictxt)
{
    calledIpointTakenBranchIndirectThruStackCall = TRUE;

    justExecutedIndirectThruStackCall = TRUE;
    Out << "IpointTakenBranchIndirectThruStackCall at rip " << std::hex << rip << " branchTargetAddr " << std::hex
        << branchTargetAddr << endl;
    if (savedBranchTargetAddr != branchTargetAddr)
    {
        Out << "***Error expected savedBranchTargetAddr == branchTargetAddr" << endl;
        exit(-1);
    }
}

static void IpointTakenBranchIndirectThruStackCall1(ADDRINT branchTargetAddr, ADDRINT rip, CONTEXT* ictxt)
{
    calledIpointTakenBranchIndirectThruStackCall1 = TRUE;

    justExecutedIndirectThruStackCall = TRUE;
    Out << "IpointTakenBranchIndirectThruStackCall1 at rip " << std::hex << rip << " branchTargetAddr " << std::hex
        << branchTargetAddr << endl;
    if (savedBranchTargetAddr != branchTargetAddr)
    {
        Out << "***Error expected savedBranchTargetAddr == branchTargetAddr" << endl;
        exit(-1);
    }
}

static void IpointTakenBranchIndirectThruStackCall2(ADDRINT branchTargetAddr, ADDRINT rip)
{
    calledIpointTakenBranchIndirectThruStackCall2 = TRUE;

    justExecutedIndirectThruStackCall = TRUE;
    Out << "IpointTakenBranchIndirectThruStackCall2 at rip " << std::hex << rip << " branchTargetAddr " << std::hex
        << branchTargetAddr << endl;
    if (savedBranchTargetAddr != branchTargetAddr)
    {
        Out << "***Error expected savedBranchTargetAddr == branchTargetAddr" << endl;
        exit(-1);
    }
}

static BOOL WasPreviousInsExecutedAnIndirectCallThruStack()
{
    BOOL valToReturn                  = justExecutedIndirectThruStackCall;
    justExecutedIndirectThruStackCall = FALSE;
    return (valToReturn);
}
static void PreviousInsExecutedWasAnIndirectCallThruStack(ADDRINT rip)
{
    calledPreviousInsExecutedWasAnIndirectCallThruStack = TRUE;
    Out << "IndirectThruStackCall target reached at rip " << std::hex << rip << endl;
    if (rip != savedBranchTargetAddr)
    {
        Out << "***Error expected rip == savedBranchTargetAddr" << endl;
        exit(-1);
    }
    savedBranchTargetAddr = 0;
}

static void InstrumentIndirects(INS ins, VOID*)
{
    if (INS_IsIndirectControlFlow(ins) && INS_IsCall(ins) && INS_IsMemoryRead(ins) && INS_OperandIsMemory(ins, 0) &&
        INS_OperandMemoryBaseReg(ins, 0) == REG_STACK_PTR)
    {
        Out << "Instrumenting Indirect Thru Stack Call at 0x" << std::hex << INS_Address(ins) << " " << INS_Disassemble(ins)
            << std::endl;

        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(IpointBeforeIndirectThruStackCall), IARG_BRANCH_TARGET_ADDR, IARG_REG_VALUE,
                       REG_STACK_PTR, IARG_ADDRINT, INS_MemoryDisplacement(ins), IARG_INST_PTR, IARG_CONTEXT, IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(IpointBeforeIndirectThruStackCall1), IARG_BRANCH_TARGET_ADDR, IARG_REG_VALUE,
                       REG_STACK_PTR, IARG_ADDRINT, INS_MemoryDisplacement(ins), IARG_INST_PTR, IARG_CONST_CONTEXT, IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(IpointBeforeIndirectThruStackCall2), IARG_BRANCH_TARGET_ADDR, IARG_REG_VALUE,
                       REG_STACK_PTR, IARG_ADDRINT, INS_MemoryDisplacement(ins), IARG_INST_PTR, IARG_END);
        INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, AFUNPTR(IpointTakenBranchIndirectThruStackCall), IARG_BRANCH_TARGET_ADDR,
                       IARG_INST_PTR, IARG_CONTEXT, IARG_END);
        INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, AFUNPTR(IpointTakenBranchIndirectThruStackCall1), IARG_BRANCH_TARGET_ADDR,
                       IARG_INST_PTR, IARG_CONST_CONTEXT, IARG_END);
        INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, AFUNPTR(IpointTakenBranchIndirectThruStackCall2), IARG_BRANCH_TARGET_ADDR,
                       IARG_INST_PTR, IARG_END);
    }
    else
    {
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)WasPreviousInsExecutedAnIndirectCallThruStack, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)PreviousInsExecutedWasAnIndirectCallThruStack, IARG_INST_PTR, IARG_END);
    }
}

VOID Fini(INT32 code, VOID* v)
{
    if (!calledIpointBeforeIndirectThruStackCall || !calledIpointBeforeIndirectThruStackCall1 ||
        !calledIpointBeforeIndirectThruStackCall2 || !calledIpointTakenBranchIndirectThruStackCall ||
        !calledIpointTakenBranchIndirectThruStackCall1 || !calledIpointTakenBranchIndirectThruStackCall2 ||
        !calledPreviousInsExecutedWasAnIndirectCallThruStack)
    {
        Out << "***Error not all expected tests executed" << endl;
        exit(-1);
    }
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    Out.open(KnobOutfile.Value().c_str());
    if (!Out.good())
    {
        std::cerr << "Unable to open '" << KnobOutfile.Value() << "'" << std::endl;
        return 1;
    }

    INS_AddInstrumentFunction(InstrumentIndirects, 0);
    PIN_AddFiniFunction(Fini, 0);

    PIN_StartProgram();
    return 0;
}
