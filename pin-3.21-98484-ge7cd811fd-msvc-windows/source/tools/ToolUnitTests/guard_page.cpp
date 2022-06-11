/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * This test tool verifies that Pin on Windows correctly updates the 
 * TEB.NtTib.StackLimit value when PIN_SafeCopy() touches the guard page 
 * of the application stack.
 * This tool must be run with the "guard_page_app" test application.
 */

#include "pin.H"

/*!
 * Touch (read) the specified memory address.
 */
VOID TouchMemory(CHAR* addr)
{
    CHAR value;
    PIN_SafeCopy(&value, addr, sizeof(CHAR));
}

/*!
 * Instruction instrumentation routine.
 */
VOID Instruction(INS ins, VOID* v)
{
    if (INS_IsMemoryRead(ins))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(TouchMemory), IARG_MEMORYREAD_EA, IARG_END);
    }
}

/*!
 * The main procedure of the tool.
 */
int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_StartProgram(); // Never returns
    return 0;
}
