/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <fstream>

// GCC 4.4, ICC 18.0 on intel64, has more aggressive inliner, when it is inlined in main below
// that alignment of a10 is not the same as from a normal call (according to the calling convensions).
#ifdef __GNUC__
static VOID CheckAlign(ADDRINT a1, ADDRINT a2, ADDRINT a3, ADDRINT a4, ADDRINT a5, ADDRINT a6, ADDRINT a7, ADDRINT a8, ADDRINT a9,
                       ADDRINT a10) __attribute__((noinline));
#endif

#if defined(TARGET_IA32E) && defined(__ICL)
__declspec(noinline) static VOID CheckAlign(ADDRINT a1, ADDRINT a2, ADDRINT a3, ADDRINT a4, ADDRINT a5, ADDRINT a6, ADDRINT a7,
                                            ADDRINT a8, ADDRINT a9, ADDRINT a10);
#endif

// The test validates that Pin retains stack alignment in analysis routine
// according to standard calling conventions.
static VOID CheckAlign(ADDRINT a1, ADDRINT a2, ADDRINT a3, ADDRINT a4, ADDRINT a5, ADDRINT a6, ADDRINT a7, ADDRINT a8, ADDRINT a9,
                       ADDRINT a10)
{
    // a10 is assumed to reside in stack
    static const ADDRINT a10addr = (ADDRINT)&a10;

#if defined(TARGET_IA32E) || defined(TARGET_MAC) || defined(TARGET_BSD)
    // stack alignment should be preserved even if the function is called from instrumented code
    ASSERTX((a10addr & 0xf) == ((ADDRINT)&a10 & 0xf));
#else
    ASSERTX((a10addr % sizeof(ADDRINT)) == 0);
#endif
}

VOID Instruction(INS ins, VOID* v)
{
    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(CheckAlign), IARG_ADDRINT, 1, IARG_ADDRINT, 2, IARG_ADDRINT, 3, IARG_ADDRINT, 4,
                   IARG_ADDRINT, 5, IARG_ADDRINT, 6, IARG_ADDRINT, 7, IARG_ADDRINT, 8, IARG_ADDRINT, 9, IARG_ADDRINT, 10,
                   IARG_END);
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);

    // First call of the analysis routine should be done from regular code.
    CheckAlign(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

    // Never returns
    PIN_StartProgram();

    return 0;
}
