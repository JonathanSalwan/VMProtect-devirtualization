/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This file contains a test for the implementation of thenpredicated 
 * instrumentation.
 * It tests the condition reported in Mantis 1862 which causes a Pin
 * assertion on this code. 
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <string.h>
using std::cerr;
using std::endl;
using std::string;

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

static std::ofstream out;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "thenpredicated.out", "Output file");

/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool tests thenpredicated instrumentation" << endl;
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;

    return -1;
}

/* ===================================================================== */
static ADDRINT returnArg(ADDRINT arg) { return arg; }

// Not thread safe, but that's not the point of this test.
static UINT32 predicatedTrueCount = 0;

static VOID countInst() { predicatedTrueCount++; }

/* ===================================================================== */

VOID Instruction(INS ins, VOID* v)
{
    // Find predicated instructions only.
    if (INS_IsPredicated(ins))
    {
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)returnArg, IARG_ADDRINT, 1, IARG_END);
        INS_InsertThenPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)countInst, IARG_END);
    }
}

/* ===================================================================== */

VOID Fini(INT32 code, VOID* v)
{
    out << "Predicated true instructions " << predicatedTrueCount << endl;
    out.close();
}

/* ===================================================================== */

int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    string filename = KnobOutputFile.Value();

    // Do this before we activate controllers
    out.open(filename.c_str());

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
