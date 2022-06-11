/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"

VOID delete_int3(INS ins, VOID* v)
{
    if (INS_Opcode(ins) == XED_ICLASS_INT3)
    {
        INS_Delete(ins);
    }
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    INS_AddInstrumentFunction(delete_int3, 0);
    PIN_StartProgram(); // Never returns
    return 0;
}
