/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Pin Tool for testing that the same action value is passed to "INS_InsertIfCall" and its corresponding "INS_InsertThenCall".
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <string.h>
using std::cerr;
using std::endl;
using std::string;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

static std::ofstream out;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "insert_if_then_call2.out", "Output file");

/* ===================================================================== */

static ADDRINT IfFunction() { return TRUE; }

static VOID ThenFunction(BOOL first)
{
    //do nothing
}

/* ===================================================================== */

VOID Instruction(INS ins, VOID* v)
{
    /* Should cause an error since a different action value is passed to "INS_InsertIfCall" and its 
     * corresponding "INS_InsertThenCall".
     */
    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)IfFunction, IARG_END);
    INS_InsertThenCall(ins, IPOINT_AFTER, (AFUNPTR)ThenFunction, IARG_END);
}

/* ===================================================================== */
/* Utilities                                                             */
/* ===================================================================== */

/*!
*  Print out help message.
*/

INT32 Usage()
{
    /* Knobs automate the parsing and management of command line switches. 
     * A command line contains switches for Pin, the tool, and the application. 
     * The knobs parsing code understands how to separate them. 
     */
    cerr << KNOB_BASE::StringKnobSummary() << endl; //   Print out a summary of all the knobs declare

    return -1;
}

/* ===================================================================== */

int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    INS_AddInstrumentFunction(Instruction, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
