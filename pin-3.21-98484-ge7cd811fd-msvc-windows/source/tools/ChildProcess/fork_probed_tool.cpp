/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
using std::cerr;
using std::endl;
using std::ios_base;
using std::ofstream;
using std::string;

/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "forktool.out", "specify file name");

ofstream Out;
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool tests probe replacement.\n"
            "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

pid_t activeProcessId = 0;
pid_t parentPid       = 0;

void BeforeFork(UINT32 childPid, void* data)
{
    parentPid = PIN_GetPid();
    Out << "TOOL: Before fork.." << endl;
}

void AfterForkInParent(UINT32 childPid, void* data)
{
    activeProcessId = PIN_GetPid();
    Out << "TOOL: After fork in parent." << endl;
}

void AfterForkInChild(UINT32 childPid, void* data)
{
    activeProcessId = PIN_GetPid();
    Out << "TOOL: After fork in child." << endl;
}

BOOL FollowChild(CHILD_PROCESS cProcess, VOID* userData)
{
    if (PIN_GetPid() == parentPid)
    {
        Out << "TOOL: At follow child callback in parent process." << endl;
    }
    else
    {
        Out << "TOOL: At follow child callback in child process." << endl;
    }
    // Pin replaces vfork with fork. In this case the global variable
    // activeProcessId will receive the right value
    if (activeProcessId != PIN_GetPid())
    {
        fprintf(stderr, "vfork works incorrectly with -follow_execv\n");
        exit(-1);
    }
    return TRUE;
}

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    string outFileName = KnobOutputFile.Value() + string("_") + decstr(PIN_GetPid());
    Out.open(outFileName.c_str(), ios_base::app);
    if (!Out.is_open())
    {
        cerr << "Can't open file " << outFileName << endl;
        exit(-1);
    }
    cerr << "Open file " << outFileName << endl;

    PIN_AddForkFunctionProbed(FPOINT_BEFORE, BeforeFork, 0);
    PIN_AddForkFunctionProbed(FPOINT_AFTER_IN_CHILD, AfterForkInChild, 0);
    PIN_AddForkFunctionProbed(FPOINT_AFTER_IN_PARENT, AfterForkInParent, 0);
    PIN_AddFollowChildProcessFunction(FollowChild, 0);

    PIN_StartProgramProbed();

    return 0;
}
