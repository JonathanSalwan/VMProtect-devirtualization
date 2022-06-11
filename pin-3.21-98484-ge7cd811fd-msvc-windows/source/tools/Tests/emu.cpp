/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <cstdio>
#include <iostream>
#include "pin.H"

UINT32 icount = 0;

static KNOB< UINT32 > KnobCount(KNOB_MODE_WRITEONCE, "pintool", "count", "100000000",
                                "Explicitly save and restore the flags in inlined analysis routines");
VOID Emulate2Address(OPCODE opcode, INS ins, ADDRINT (*OpRM)(ADDRINT, ADDRINT*), ADDRINT (*OpRV)(ADDRINT, ADDRINT),
                     VOID (*OpMV)(ADDRINT*, ADDRINT))
{
    if (INS_Opcode(ins) != opcode) return;
    if (INS_OperandIsMemory(ins, 0)
        // This will filter out segment overrides
        && INS_IsMemoryWrite(ins))
    {
        if (INS_OperandIsReg(ins, 1) && REG_is_gr(INS_OperandReg(ins, 1)))
        {
            // Source register, dst memory
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(OpMV), IARG_MEMORYWRITE_EA, IARG_REG_VALUE, INS_OperandReg(ins, 1),
                           IARG_END);
            INS_Delete(ins);
        }
        else
        {
            ASSERTX(!INS_OperandIsMemory(ins, 1));
        }
    }
    else if (INS_OperandIsReg(ins, 0))
    {
        REG dst = INS_OperandReg(ins, 0);
        if ((dst == REG_SEG_GS) || (dst == REG_SEG_FS)) return;
        if (INS_OperandIsReg(ins, 1))
        {
            // Source register, dst register
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(OpRV), IARG_REG_VALUE, INS_OperandReg(ins, 0), IARG_REG_VALUE,
                           INS_OperandReg(ins, 1), IARG_RETURN_REGS, INS_OperandReg(ins, 0), IARG_END);
            INS_Delete(ins);
        }
        else if (INS_OperandIsMemory(ins, 1)
                 // This will filter out segment overrides
                 && INS_IsMemoryRead(ins))
        {
            // Source register, dst register
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(OpRM), IARG_REG_VALUE, INS_OperandReg(ins, 0), IARG_MEMORYREAD_EA,
                           IARG_RETURN_REGS, INS_OperandReg(ins, 0), IARG_END);
            INS_Delete(ins);
        }
    }

#if 0
    if (KnobCount == icount)
        fprintf(stderr,"Last one %s\n",INS_Disassemble(ins).c_str());
    else if (icount > KnobCount)
        return;
    icount++;
#endif
}

// Move a register or literal to memory
VOID MovMV(ADDRINT* op0, ADDRINT op1) { PIN_SafeCopy(op0, &op1, sizeof(ADDRINT)); }

// Move a literal or register to a register
ADDRINT MovRV(ADDRINT op0, ADDRINT op1) { return op1; }

// Move from memory to register
ADDRINT MovRM(ADDRINT op0, ADDRINT* op1)
{
    ADDRINT value;
    PIN_SafeCopy(&value, op1, sizeof(ADDRINT));
    return value;
}

VOID Instruction(INS ins, VOID* v) { Emulate2Address(XED_ICLASS_MOV, ins, MovRM, MovRV, MovMV); }

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
