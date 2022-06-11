/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <stdlib.h>
#include "pin.H"
using std::cerr;
using std::endl;

ADDRINT SetVal() { return 999; }

VOID CheckVal(INT32 val)
{
    if (val != 999)
    {
        cerr << "inline failed, sent 999, received " << val << endl;
        exit(-1);
    }
}

int a[10];
int n = 10;

ADDRINT SetValNoInline()
{
    for (int i = 0; i < n; i++)
    {
        a[i] = i;
    }

    return 666;
}

VOID CheckValNoInline(INT32 val)
{
    if (val != 666)
    {
        cerr << "no inline failed, sent 666, received " << val << endl;
        exit(-1);
    }
}

VOID Instruction(INS ins, VOID* v)
{
    static INT32 count = 0;

    switch (count)
    {
        case 0:
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(SetVal), IARG_RETURN_REGS, REG_INST_G0, IARG_END);
            break;

        case 1:
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(CheckVal), IARG_REG_VALUE, REG_INST_G0, IARG_END);
            break;

        case 2:
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(SetValNoInline), IARG_RETURN_REGS, REG_INST_G1, IARG_END);
            break;

        case 3:
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(CheckValNoInline), IARG_REG_VALUE, REG_INST_G1, IARG_END);
            break;

        default:
            break;
    }

    count++;
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
