/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool inserts instrumentation before every instruction
 *
 * When the instrumentation is not inlined, it forces the xmm registers to
 * be spilled. This is used to test construction of a context during a
 * synchronous signal when xmm registers are spilled
 *
 */

#include <pin.H>

int n = 0;

void Spill() {}

void Ins(INS ins, VOID*) { INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(Spill), IARG_END); }

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Ins, 0);

    PIN_StartProgram();
    return 0;
}
