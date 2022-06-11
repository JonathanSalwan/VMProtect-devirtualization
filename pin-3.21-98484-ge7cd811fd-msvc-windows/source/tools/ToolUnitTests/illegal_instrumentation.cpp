/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * This test tool verifies that PIN asserts that trace re-generation is equal
 * to the original trace generation.
 * When PIN generates a trace in the code cache, it does not generate the 
 * meta-data for that trace (App instructions vs. instrumentation code vs. spill/fill code). 
 * This meta-data is created only in case of an exception, when PIN needs to figure out what 
 * code caused the exception, by re-generating the trace. If the instrumentation scheme changes 
 * between the first and second trace generation, the results of the exception analysis are bogus.
 */

#include "pin.H"

//=================================================================================================
/*!
 * Global variables
 */
volatile void* ptr = NULL;

//=================================================================================================

/*!
 * INS analysis routines.
 */
VOID OnIns()
{
    // Generate exception in inlined analysis routine.
    *(int*)ptr = 11111;
}

/*!
 * INS instrumentation routine.
 * This is an inconsistent instrumentation function as it instruments only once.
 * In case of trace re-generation, the original instrumented instruction won't be
 * instrumented again. 
 */
VOID Instruction(INS ins, VOID* v)
{
    static bool isFirst = true;
    if (isFirst)
    {
        isFirst = false;
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)OnIns, IARG_END);
    }
}

//=================================================================================================
/*!
 * The main procedure of the tool.
 */
int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);

    PIN_StartProgram();
    return 0;
}
