/*
 * Copyright (C) 2017-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include "pin.H"

VOID DoASSERT() { ASSERTX(FALSE); }

VOID Bar() { DoASSERT(); }

VOID Foo() { Bar(); }

VOID Instruction(INS ins, void* v) { Foo(); }

/* ================================================================== */
/*
 Initialize and begin program execution under the control of Pin
*/
int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    PIN_InitSymbols();

    INS_AddInstrumentFunction(Instruction, 0);

    PIN_StartProgram();

    return 0;
}
