/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include "pin.H"

VOID Instruction(INS ins, VOID* v)
{
    if (INS_IsMemoryRead(ins)) cout << " read";
    if (INS_IsMemoryWrite(ins)) cout << " write";
    cout << INS_StringShort(ins) << endl;
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
