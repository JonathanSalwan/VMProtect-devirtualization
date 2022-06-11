/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <stdlib.h>

#include "pin.H"
using std::cerr;
using std::endl;

// Add the flags you want to set to FLAGSET.
// Note: Modifying the eflags will very likely break your application.
//       Use at your own risk.

#define FLAGSET 0x00000000
//#define FLAGSET 0x00000010 // set AF (less likely to break the application)

ADDRINT SetFlags(ADDRINT app_flags)
{
    //cout << "SetFlags: app_flags = " << hexstr(app_flags) << endl;
    return (app_flags | FLAGSET);
}

VOID CheckFlags(ADDRINT app_flags)
{
    if ((app_flags & FLAGSET) != FLAGSET)
    {
        cerr << "inline failed: app_flags= " << hexstr(app_flags) << endl;
        exit(-1);
    }
}

int a[10];
int n = 10;

ADDRINT SetFlagsNoInline(ADDRINT app_flags)
{
    for (int i = 0; i < n; i++)
    {
        a[i] = i;
    }
    return (app_flags | FLAGSET);
}

VOID CheckFlagsNoInline(ADDRINT app_flags)
{
    if ((app_flags & FLAGSET) != FLAGSET)
    {
        cerr << "Noinline failed: app_flags= " << hexstr(app_flags) << endl;
        exit(-1);
    }
}

VOID Instruction(INS ins, VOID* v)
{
    static INT32 count = 0;

    switch (count)
    {
        case 0:
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(SetFlags), IARG_REG_VALUE, REG_GFLAGS, IARG_RETURN_REGS, REG_GFLAGS,
                           IARG_END);
            break;

        case 1:
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(CheckFlags), IARG_REG_VALUE, REG_GFLAGS, IARG_END);
            break;

        case 2:
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(SetFlagsNoInline), IARG_REG_VALUE, REG_GFLAGS, IARG_RETURN_REGS,
                           REG_GFLAGS, IARG_END);
            break;

        case 3:
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(CheckFlagsNoInline), IARG_REG_VALUE, REG_GFLAGS, IARG_END);
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
