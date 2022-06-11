/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "pin.H"
#include <iostream>
#include <fstream>
using std::cerr;
using std::endl;

std::ofstream* out = 0;

// Move from memory to register
ADDRINT DoLoad(REG reg, ADDRINT* addr)
{
    *out << "Emulate loading from addr " << addr << " to " << REG_StringShort(reg) << endl;
    ADDRINT value;
    PIN_SafeCopy(&value, addr, sizeof(ADDRINT));
    return value;
}

VOID EmulateLoad(INS ins, VOID* v)
{
    if (INS_Opcode(ins) == XED_ICLASS_MOV && INS_IsMemoryRead(ins) && INS_OperandIsReg(ins, 0) && INS_OperandIsMemory(ins, 1))
    {
        // op0 <- *op1
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(DoLoad), IARG_UINT32, REG(INS_OperandReg(ins, 0)), IARG_MEMORYREAD_EA,
                       IARG_RETURN_REGS, INS_OperandReg(ins, 0), IARG_END);

        INS_Delete(ins);
    }
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool emulates loads" << endl;
    cerr << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    out = new std::ofstream("emuload.out");
    if (PIN_Init(argc, argv)) return Usage();
    INS_AddInstrumentFunction(EmulateLoad, 0);
    PIN_StartProgram(); // Never returns
    return 0;
}
