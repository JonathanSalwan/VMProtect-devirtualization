/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file
 *  This file tests calling malloc from the tool and if the memoray 
 *  allocation statistic are reported correctly.
 *
 *  For each trace of the application is allocates 128 bytes.
 *  In the fini it reports the amount of allocated memory.
 *  the test is run as:
 *      pin -t tool_malloc -o tool_malloc.out -xyzzy -statistic -- cp makefile <new-file>
 *
 *  The test then compare between the amount in tool_malloc.out and pintool.log
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <stdlib.h>
using std::cerr;
using std::endl;
using std::ofstream;
using std::string;

const INT BUF_SIZE = 100;

struct REC
{
    CHAR data[BUF_SIZE];
    struct REC* next;
};

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

struct REC* head   = 0;
UINT64 alloc_count = 0;
ofstream TraceFile;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "tool_malloc.out", "specify output file name");

/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool allocate a record for each trace, and report the number of allocated bytes"
            "\n";

    cerr << KNOB_BASE::StringKnobSummary();

    cerr << endl;

    return -1;
}

/* ===================================================================== */

VOID Trace(TRACE trace, VOID* v)
{
    struct REC* rec = (struct REC*)malloc(sizeof(struct REC));
    alloc_count += sizeof(struct REC);
    rec->next = head;
    head      = rec;
}

/* ===================================================================== */

VOID Fini(INT32 code, VOID* v)
{
    TraceFile.open(KnobOutputFile.Value().c_str());
    TraceFile << alloc_count << endl;
    TraceFile.close();
}

/* ===================================================================== */

int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
