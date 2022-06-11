/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Emulate instruction that implicitly touch the stack: 
 *     push, pop, leave.
 *     call and return
 */
#include <stdio.h>
#include <iostream>
#include <set>
#include "pin.H"
using std::endl;
using std::hex;
using std::string;

KNOB< UINT32 > KnobDetach(KNOB_MODE_WRITEONCE, "pintool", "d", "-1", "stop tracing at this point");
KNOB< BOOL > KnobLog(KNOB_MODE_WRITEONCE, "pintool", "l", "0", "log all instructions");

static UINT32 scount = 0;
static REG scratchReg;
static std::set< ADDRINT > branchTakenIns;
FILE* out = 0;

VOID PrintIns(void* p, const char* s)
{
    if (!out) return;

    fprintf(out, "%6d: %p %s\n", scount, p, s);
    fflush(out);
}

ADDRINT EmuIndJmp(ADDRINT tgtip) { return tgtip; }

VOID BranchBefore(THREADID tid, ADDRINT pc)
{
    if (0 != tid) return;
    branchTakenIns.insert(pc);
}

VOID BranchTaken(THREADID tid, ADDRINT pc, ADDRINT expected)
{
    if (0 != tid) return;
    ASSERT(pc == expected, "pc = " + hexstr(pc) + "INS_Address() = " + hexstr(expected));
    branchTakenIns.erase(pc);
}

VOID Ins(INS ins, VOID* v)
{
    if (KnobDetach > 0 && scount > KnobDetach) return;

    if (KnobLog)
    {
        void* addr    = Addrint2VoidStar(INS_Address(ins));
        string disasm = INS_Disassemble(ins);
        PrintIns(addr, disasm.c_str());
    }

    scount++;

    // call and return need also stack manipulation (see emu_stack.cpp)
    // conditional jumps need handling the condition (not supported yet)
    if (INS_IsCall(ins) || INS_IsRet(ins) || INS_Category(ins) == XED_CATEGORY_COND_BR) return;

    if (INS_IsIndirectControlFlow(ins))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(EmuIndJmp), IARG_BRANCH_TARGET_ADDR, IARG_RETURN_REGS, scratchReg, IARG_END);

        INS_InsertIndirectJump(ins, IPOINT_AFTER, scratchReg);
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)BranchBefore, IARG_THREAD_ID, IARG_INST_PTR, IARG_END);
        INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)BranchTaken, IARG_THREAD_ID, IARG_INST_PTR, IARG_ADDRINT,
                       INS_Address(ins), IARG_END);

        INS_Delete(ins);
    }
    else if (INS_IsDirectControlFlow(ins))
    {
        ADDRINT tgt = INS_DirectControlFlowTargetAddress(ins);

        INS_InsertDirectJump(ins, IPOINT_AFTER, tgt);
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)BranchBefore, IARG_THREAD_ID, IARG_INST_PTR, IARG_END);
        INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)BranchTaken, IARG_THREAD_ID, IARG_INST_PTR, IARG_ADDRINT,
                       INS_Address(ins), IARG_END);

        INS_Delete(ins);
    }
}

VOID Fini(INT32 code, VOID* v)
{
    if (!branchTakenIns.empty())
    {
        for (std::set< ADDRINT >::iterator it = branchTakenIns.begin(); it != branchTakenIns.end(); it++)
        {
            std::cerr << "Instrumentation for IPOINT_TAKEN_BRANCH for instruction at " << hex << *it << " wasn't executed"
                      << endl;
        }
        ASSERTX(FALSE);
    }
    if (out) fclose(out);
}

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    // Initialize pin
    PIN_Init(argc, argv);

    if (KnobLog)
    {
        out = fopen("emu_jumps.txt", "w");
        if (!out) fprintf(stderr, "Can't open log file emu_jumps.txt\n");
    }

    scratchReg = PIN_ClaimToolRegister();
    if (!REG_valid(scratchReg))
    {
        if (out) fprintf(out, "Cannot allocate a scratch register.\n");
        fprintf(stderr, "Cannot allocate a scratch register.\n");
        return 1;
    }

    // Register instruction instrumentation callback.
    INS_AddInstrumentFunction(Ins, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
