/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>

namespace WIND
{
#include <windows.h>
}
using std::cout;
using std::endl;
using std::string;

/* ===================================================================== */
/* Command line Switches */
/* ===================================================================== */

// General configuration

// Pin full path
KNOB< string > KnobPinFullPath(KNOB_MODE_WRITEONCE, "pintool", "pin_path", "", "pin full path");
// Tool full path
KNOB< string > KnobToolsFullPath(KNOB_MODE_WRITEONCE, "pintool", "tools_path", "", "grand parent tool full path");

// Parent configuration

// Application name
KNOB< string > KnobParentApplicationName(KNOB_MODE_WRITEONCE, "pintool", "parent_app_name", "win_parent_process",
                                         "parent application name");
// PinTool name
KNOB< string > KnobParentToolName(KNOB_MODE_WRITEONCE, "pintool", "parent_tool_name", "parent_tool", "parent tool full path");
// Current process id received by grand_parent tool
KNOB< OS_PROCESS_ID > KnobCurrentProcessId(KNOB_MODE_WRITEONCE, "pintool", "process_id", "0", "current process id");
// Whether to check current process id received in KnobCurrentProcessId
KNOB< BOOL > KnobCheckCurrentProcessId(KNOB_MODE_WRITEONCE, "pintool", "check_process_id", "0", "current process id");

// Child configuration

// Application name
KNOB< string > KnobChildApplicationName(KNOB_MODE_WRITEONCE, "pintool", "child_app_name", "win_child_process",
                                        "child application name");
// PinTool name
KNOB< string > KnobChildToolName(KNOB_MODE_WRITEONCE, "pintool", "child_tool_name", "follow_child_3gen_tool",
                                 "child tool full path");
// Whether to probe the child
KNOB< BOOL > KnobProbeChild(KNOB_MODE_WRITEONCE, "pintool", "probe_child", "0", "probe the child process");

/* ===================================================================== */

BOOL FollowChild(CHILD_PROCESS cProcess, VOID* userData)
{
    BOOL res;
    INT appArgc;
    CHAR const* const* appArgv;
    OS_PROCESS_ID pid = CHILD_PROCESS_GetId(cProcess);

    CHILD_PROCESS_GetCommandLine(cProcess, &appArgc, &appArgv);

    //Inject only if the KnobChildApplicationName value is current child process application
    string childAppToInject = KnobChildApplicationName.Value();
    string childApp(appArgv[0]);
    string::size_type index = childApp.find(childAppToInject);
    if (index == string::npos)
    {
        return FALSE;
    }

    //Set Pin's command line for child process
    INT pinArgc          = 0;
    const INT pinArgcMax = 6;
    CHAR const* pinArgv[pinArgcMax];

    string pin         = KnobPinFullPath.Value() + "pin";
    pinArgv[pinArgc++] = pin.c_str();
    pinArgv[pinArgc++] = "-follow_execv";
    if (KnobProbeChild)
    {
        pinArgv[pinArgc++] = "-probe"; // pin in probe mode
    }
    pinArgv[pinArgc++] = "-t";
    string tool        = KnobToolsFullPath.Value() + KnobChildToolName.Value();
    pinArgv[pinArgc++] = tool.c_str();
    pinArgv[pinArgc++] = "--";

    ASSERTX(pinArgc <= pinArgcMax);

    CHILD_PROCESS_SetPinCommandLine(cProcess, pinArgc, pinArgv);

    return TRUE;
}

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    cout << "In parent_tool PinTool is probed " << decstr(PIN_IsProbeMode()) << endl;

    PIN_AddFollowChildProcessFunction(FollowChild, 0);

    //Verify KnobCurrentProcessId is accurate
    if (KnobCheckCurrentProcessId == TRUE)
    {
        if (WIND::GetCurrentProcessId() != KnobCurrentProcessId)
        {
            cout << "Got wrong process id in KnobCurrentProcessId" << endl;
            exit(0);
        }
    }

    // Never returns
    if (PIN_IsProbeMode())
    {
        PIN_StartProgramProbed();
    }
    else
    {
        PIN_StartProgram();
    }

    return 0;
}
