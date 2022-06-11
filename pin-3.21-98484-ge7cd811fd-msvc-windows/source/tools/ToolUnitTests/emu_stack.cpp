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
#include "pin.H"
using std::string;

KNOB< UINT32 > KnobDetach(KNOB_MODE_WRITEONCE, "pintool", "d", "-1", "stop tracing at this point");
KNOB< BOOL > KnobLog(KNOB_MODE_WRITEONCE, "pintool", "l", "0", "log all instructions");

static UINT32 scount = 0;
FILE* out            = 0;
static REG scratchReg;

VOID PrintIns(void* p, const char* s)
{
    if (!out) return;

    fprintf(out, "%6d: %p %s\n", scount, p, s);
    fflush(out);
}

ADDRINT EmuPushValue(ADDRINT rsp, ADDRINT value)
{
    rsp          = rsp - sizeof(ADDRINT);
    ADDRINT* psp = (ADDRINT*)rsp;
    *psp         = value;
    return rsp;
}

ADDRINT EmuPushMem(ADDRINT rsp, ADDRINT* ea)
{
    rsp          = rsp - sizeof(ADDRINT);
    ADDRINT* psp = (ADDRINT*)rsp;
    *psp         = *ea;
    return rsp;
}

ADDRINT EmuPopReg(ADDRINT rsp, ADDRINT* reg)
{
    ADDRINT* psp = (ADDRINT*)rsp;
    *reg         = *psp;
    return rsp + sizeof(ADDRINT);
}

ADDRINT EmuPopMem(ADDRINT rsp, ADDRINT* ea)
{
    ADDRINT* psp = (ADDRINT*)rsp;
    *ea          = *psp;
    return rsp + sizeof(ADDRINT);
}

ADDRINT EmuLeave(ADDRINT rsp, ADDRINT* rbp)
{
    rsp          = *rbp;
    ADDRINT* psp = (ADDRINT*)rsp;
    *rbp         = *psp;
    return rsp + sizeof(ADDRINT);
}

ADDRINT EmuCall(ADDRINT nextip, ADDRINT tgtip, ADDRINT* rsp)
{
    //*rsp = EmuPushValue(*rsp, nextip);

    ADDRINT rspVal = *rsp;

    rspVal       = rspVal - sizeof(ADDRINT);
    ADDRINT* psp = (ADDRINT*)rspVal;
    *psp         = nextip;
    *rsp         = rspVal;

    return tgtip;
}

ADDRINT EmuRet(ADDRINT* rsp, UINT32 framesize)
{
    ADDRINT retval;

    //*rsp = EmuPopMem(*rsp, &retval);

    ADDRINT rspVal = *rsp;
    ADDRINT* psp   = (ADDRINT*)rspVal;
    retval         = *psp;
    *rsp           = rspVal + sizeof(ADDRINT);

    *rsp += framesize;
    return retval;
}

ADDRINT EmuIndJmp(ADDRINT tgtip) { return tgtip; }

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

    if (INS_Opcode(ins) == XED_ICLASS_PUSH)
    {
        if (INS_OperandIsImmediate(ins, 0))
        {
            ADDRINT value = INS_OperandImmediate(ins, 0);
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(EmuPushValue), IARG_REG_VALUE, REG_STACK_PTR, IARG_ADDRINT, value,
                           IARG_RETURN_REGS, REG_STACK_PTR, IARG_END);

            INS_Delete(ins);
        }
        else if (INS_OperandIsReg(ins, 0))
        {
            REG reg = INS_OperandReg(ins, 0);
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(EmuPushValue), IARG_REG_VALUE, REG_STACK_PTR, IARG_REG_VALUE, reg,
                           IARG_RETURN_REGS, REG_STACK_PTR, IARG_END);

            INS_Delete(ins);
        }
        else if (INS_OperandIsMemory(ins, 0))
        {
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(EmuPushMem), IARG_REG_VALUE, REG_STACK_PTR, IARG_MEMORYREAD_EA,
                           IARG_RETURN_REGS, REG_STACK_PTR, IARG_END);

            INS_Delete(ins);
        }
        else
        {
            fprintf(stderr, "EmuPush: unsupported operand type (%p:'%s')\n", Addrint2VoidStar(INS_Address(ins)),
                    INS_Disassemble(ins).c_str());
        }
    }
    else if (INS_Opcode(ins) == XED_ICLASS_POP)
    {
        if (INS_OperandIsReg(ins, 0))
        {
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(EmuPopReg), IARG_REG_VALUE, REG_STACK_PTR, IARG_REG_REFERENCE,
                           INS_OperandReg(ins, 0), IARG_RETURN_REGS, REG_STACK_PTR, IARG_END);

            INS_Delete(ins);
        }
        else if (INS_OperandIsMemory(ins, 0))
        {
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(EmuPopMem), IARG_REG_VALUE, REG_STACK_PTR, IARG_MEMORYWRITE_EA,
                           IARG_RETURN_REGS, REG_STACK_PTR, IARG_END);

            INS_Delete(ins);
        }
        else
        {
            fprintf(stderr, "EmuPop: unsupported operand type (%p:'%s')\n", Addrint2VoidStar(INS_Address(ins)),
                    INS_Disassemble(ins).c_str());
        }
    }
    else if (INS_Opcode(ins) == XED_ICLASS_LEAVE)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(EmuLeave), IARG_REG_VALUE, REG_STACK_PTR, IARG_REG_REFERENCE, REG_GBP,
                       IARG_RETURN_REGS, REG_STACK_PTR, IARG_END);

        INS_Delete(ins);
    }
    else if (INS_IsCall(ins))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(EmuCall), IARG_ADDRINT, INS_NextAddress(ins), IARG_BRANCH_TARGET_ADDR,
                       IARG_REG_REFERENCE, REG_STACK_PTR, IARG_RETURN_REGS, scratchReg, IARG_END);

        INS_InsertIndirectJump(ins, IPOINT_AFTER, scratchReg);

        INS_Delete(ins);
    }
    else if (INS_IsRet(ins))
    {
        UINT64 imm = 0;
        if (INS_OperandCount(ins) > 0 && INS_OperandIsImmediate(ins, 0))
        {
            imm = INS_OperandImmediate(ins, 0);
        }

        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(EmuRet), IARG_CALL_ORDER, CALL_ORDER_FIRST, IARG_REG_REFERENCE, REG_STACK_PTR,
                       IARG_ADDRINT, (ADDRINT)imm, IARG_RETURN_REGS, scratchReg, IARG_END);

        INS_InsertIndirectJump(ins, IPOINT_AFTER, scratchReg);

        INS_Delete(ins);
    }
    else if (INS_IsIndirectControlFlow(ins))
    {
        // This is not a call (it was checked before) so this is indirect jump
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(EmuIndJmp), IARG_BRANCH_TARGET_ADDR, IARG_RETURN_REGS, scratchReg, IARG_END);

        INS_InsertIndirectJump(ins, IPOINT_AFTER, scratchReg);

        INS_Delete(ins);
    }
}

VOID Fini(INT32 code, VOID* v)
{
    if (out) fclose(out);
}

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    // Initialize pin
    PIN_Init(argc, argv);

    if (KnobLog)
    {
        out = fopen("emu_stack.txt", "w");
        if (!out) fprintf(stderr, "Can't open log file emu_stack.txt\n");
    }

    scratchReg = PIN_ClaimToolRegister();
    if (!REG_valid(scratchReg))
    {
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
