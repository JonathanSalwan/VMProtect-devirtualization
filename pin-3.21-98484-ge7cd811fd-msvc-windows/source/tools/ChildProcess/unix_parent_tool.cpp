/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * NOTE: Due to the structure of tests that use this tool, its output file is opened in append mode.
 * If the test is run more than once without removing the output file for the tool, the output will
 * be concatenated with the output from the previous run. To prevent it, always delete this tool's
 * output file before running.
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include "arglist.h"
#include "tool_macros.h"

/* ===================================================================== */
/* Command line Switches */
/* ===================================================================== */

//Pin command line
KNOB< string > KnobPin(KNOB_MODE_WRITEONCE, "pintool", "pin", "", "pin full path");

//Parent configuration - Application name
KNOB< string > KnobApplication(KNOB_MODE_WRITEONCE, "pintool", "app", "", "application name");

KNOB< BOOL > KnobToolProbeMode(KNOB_MODE_WRITEONCE, "pintool", "probe", "0", "invoke tool in probe mode");

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "unix_parent_tool.out", "specify output file name");

ofstream OutFile;

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cout << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/*
 * FollowChild(CHILD_PROCESS cProcess, VOID * userData) - child process configuration
 * Sets pin command line for a child that should be created
 */
BOOL FollowChild(CHILD_PROCESS cProcess, VOID* userData)
{
    INT appArgc;
    CHAR const* const* appArgv;

    CHILD_PROCESS_GetCommandLine(cProcess, &appArgc, &appArgv);
    ARGUMENTS_LIST appCmd(appArgc, appArgv);
    string childApp(appArgv[0]);

    if (KnobApplication.Value() == "ALL" || KnobApplication.Value() == childApp)
    {
        // Change pin command line if defined, otherwise just follow child
        if (!KnobPin.Value().empty())
        {
            ARGUMENTS_LIST newPinCmd;
            newPinCmd.Add(KnobPin.Value());
            newPinCmd.Add("--");

            CHILD_PROCESS_SetPinCommandLine(cProcess, newPinCmd.Argc(), newPinCmd.Argv());
            OutFile << "Process to execute: " << newPinCmd.String() << endl;
        }
        else
        {
            OutFile << "Pin command line remains unchanged" << endl;
            OutFile << "Application to execute: " << appCmd.String() << endl;
        }
        return TRUE;
    }
    OutFile << "knob val " << KnobApplication.Value() << ", app " << childApp << endl;
    OutFile << "Do not run Pin under the child process" << endl;
    return FALSE;
}

/* ===================================================================== */
VOID Fini(INT32 code, VOID* v)
{
    OutFile << "In unix_parent_tool PinTool" << endl;
    OutFile.close();
}

typedef VOID (*EXITFUNCPTR)(INT code);
EXITFUNCPTR origExit;

VOID ExitInProbeMode(INT code)
{
    Fini(code, 0);
    (*origExit)(code);
}

/* ===================================================================== */

VOID ImageLoad(IMG img, VOID* v)
{
    RTN exitRtn = RTN_FindByName(img, C_MANGLE("_exit"));
    if (RTN_Valid(exitRtn) && RTN_IsSafeForProbedReplacement(exitRtn))
    {
        origExit = (EXITFUNCPTR)RTN_ReplaceProbed(exitRtn, AFUNPTR(ExitInProbeMode));
    }
}
/* ===================================================================== */

int main(INT32 argc, CHAR** argv)
{
    if (PIN_Init(argc, argv)) return Usage();
    PIN_InitSymbols();

    // Can't just open for writing because the child process' Pintool may overwrite
    // the parent process' Pintool file (when the -o parameter doesn't change).
    // Opening in append mode instead.
    OutFile.open(KnobOutputFile.Value().c_str(), ofstream::app);

    PIN_AddFollowChildProcessFunction(FollowChild, 0);

    // Never returns
    if (KnobToolProbeMode)
    {
        IMG_AddInstrumentFunction(ImageLoad, 0);
        PIN_StartProgramProbed();
    }
    else
    {
        PIN_AddFiniFunction(Fini, 0);
        PIN_StartProgram();
    }
    return 0;
}
