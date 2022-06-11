/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Print the first INS according to all syntax options.
 */

#include <iostream>
#include <fstream>
#include "pin.H"
using std::cout;
using std::endl;

BOOL test = true;

VOID Inst(INS ins, VOID* v)
{
    if (!test) return;
    test = false;

    cout << "DEFAULT: " << INS_Disassemble(ins) << endl;
    PIN_SetSyntaxATT();
    cout << "ATT: " << INS_Disassemble(ins) << endl;
    PIN_SetSyntaxIntel();
    cout << "INTEL: " << INS_Disassemble(ins) << endl;
    PIN_SetSyntaxXED();
    cout << "XED: " << INS_Disassemble(ins) << endl;
}

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Inst, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
