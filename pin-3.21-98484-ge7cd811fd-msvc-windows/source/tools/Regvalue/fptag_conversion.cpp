/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include "pin.H"
using std::cerr;
using std::endl;

/* ===================================================================== */
/* Checks FPTAG conversion functions                                     */
/* ===================================================================== */

VOID CheckFptagConversion(char* fnstenv_area, const CONTEXT* ctxt)
{
    // According to Intel Software Developer's Manual Vol 1 8.1.10, FPU's 16b tag word
    // is stored in fnstenv area in bytes 9 and 8.
    const UINT16 fullTag = (((unsigned int)fnstenv_area[9]) << 8) + (UINT8)fnstenv_area[8];

    //get fpstate from context
    unsigned char fpContextSpaceForXsave[sizeof(FPSTATE) + FPSTATE_ALIGNMENT];
    FPSTATE* fpState = reinterpret_cast< FPSTATE* >(
        (reinterpret_cast< ADDRINT >(fpContextSpaceForXsave) + (FPSTATE_ALIGNMENT - 1)) & (-1 * FPSTATE_ALIGNMENT));
    PIN_GetContextFPState(ctxt, fpState);
    const UINT8 abridgedTag = fpState->fxsave_legacy._ftw;

    //Checking conversion from 16b to 8b
    if (abridgedTag != REG_ConvertX87FullTagToAbridged(fullTag))
    {
        cerr << "Error in conversion from 16b to 8b" << endl;
        exit(-1);
    }
    //Checking full conversion from 8b to 16b
    if (fullTag != REG_ConvertX87AbridgedTagToFull(&fpState->fxsave_legacy))
    {
        cerr << "Error in full conversion from 8b to 16b" << endl;
        exit(-2);
    }
}
/* ===================================================================== */
/* Adds instrumentation                                                  */
/* ===================================================================== */

VOID Instruction(INS ins, VOID* v)
{
    if (INS_Opcode(ins) == XED_ICLASS_FNSTENV)
    {
        INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)CheckFptagConversion, IARG_MEMORYOP_EA, 0, IARG_CONST_CONTEXT, IARG_END);
    }
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool checks FPTAG conversion" << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
/*   argc, argv are the entire command line: pin -t <toolname> -- ...    */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();
    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);
    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
