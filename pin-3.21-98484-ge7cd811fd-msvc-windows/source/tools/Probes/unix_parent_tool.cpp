/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <ctype.h>
#include <fstream>
#include <algorithm>
#include "arglist.h"
#include "tool_macros.h"

static const string ExecVELib =
#ifdef TARGET_MAC
    "libsystem_kernel.dylib";
#else
    "libc.so";
#endif

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
    OutFile << endl << KNOB_BASE::StringKnobSummary() << endl;
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

    if (KnobApplication.Value() == childApp)
    {
        // Change pin command line if defined, otherwise just follow child
        if (!KnobPin.Value().empty())
        {
            ARGUMENTS_LIST newPinCmd;
            newPinCmd.Add(KnobPin.Value());
            newPinCmd.Add("--");
            newPinCmd.Add(appCmd.String());

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
    OutFile << "knob val " << KnobApplication.Value() << "app " << childApp << endl;
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

int (*fptrexecve)(const char*, char* const*, char* const*);

int myexecve(const char* __path, char* const* __argv, char* const* __envp)
{
    OutFile << "myexecve called " << endl;
    int res = fptrexecve(__path, __argv, __envp);

    return res;
}

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
    string imageName = IMG_Name(img);
    std::transform(imageName.begin(), imageName.end(), imageName.begin(), ::tolower);
    if ((IMG_Name(img).find(ExecVELib) != string::npos))
    { // check that tool can also probe execve successfully
        RTN rtnexecve = RTN_FindByName(img, C_MANGLE("execve"));
        if (RTN_Valid(rtnexecve))
        {
            if (RTN_IsSafeForProbedReplacement(rtnexecve))
            {
                OutFile << "Inserting probe for execve at " << hex << RTN_Address(rtnexecve) << endl;
                AFUNPTR fptr = (RTN_ReplaceProbed(rtnexecve, AFUNPTR(myexecve)));
                fptrexecve   = (int (*)(__const char*, char* __const*, char* __const*))fptr;
            }
            else
            {
                OutFile << "Unsafe to probe execve" << endl;
            }
        }
    }
}
/* ===================================================================== */

int main(INT32 argc, CHAR** argv)
{
    if (PIN_Init(argc, argv)) return Usage();
    PIN_InitSymbols();

    // Can't just open for writing because child_process' Pintool may overwrite
    // the parent_process' Pintool file (when the -o parameter doesn't change).
    // Opening in append mode instead.
    OutFile.open(KnobOutputFile.Value().c_str(), ofstream::app);

    PIN_AddFollowChildProcessFunction(FollowChild, 0);

    // Never returns
    IMG_AddInstrumentFunction(ImageLoad, 0);
    PIN_StartProgramProbed();

    return 0;
}
