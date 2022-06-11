/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>

using std::cout;
using std::endl;
using std::string;

/* ===================================================================== */
/* Command line Switches */
/* ===================================================================== */

//General configuration - Pin and Tool full path
KNOB< string > KnobPinFullPath(KNOB_MODE_WRITEONCE, "pintool", "pin_path", "", "pin full path");

KNOB< string > KnobToolsFullPath(KNOB_MODE_WRITEONCE, "pintool", "tools_path", "", "grand parent tool full path");

//Parent configuration - Application and PinTool name
KNOB< string > KnobParentApplicationName(KNOB_MODE_WRITEONCE, "pintool", "parent_app_name", "win_parent_process",
                                         "parent application name");

KNOB< string > KnobParentToolName(KNOB_MODE_WRITEONCE, "pintool", "parent_tool_name", "parent_tool", "parent tool full path");

//Child configuration - Application and PinTool name
KNOB< string > KnobChildApplicationName(KNOB_MODE_WRITEONCE, "pintool", "child_app_name", "win_child_process",
                                        "child application name");

KNOB< string > KnobChildToolName(KNOB_MODE_WRITEONCE, "pintool", "child_tool_name", "follow_child_3gen_tool",
                                 "child tool full path");

// Whether to probe the child
KNOB< BOOL > KnobProbeChild(KNOB_MODE_WRITEONCE, "pintool", "probe_child", "0", "probe the child process");

// Whether to probe the grand child
KNOB< BOOL > KnobProbeGrandChild(KNOB_MODE_WRITEONCE, "pintool", "probe_grand_child", "0", "probe the grand child process");

/* ===================================================================== */
//Helper function
string Int64ToDecimalString(INT64 number)
{
    const UINT32 BUFFER_SIZE = 64;
    CHAR buffer[BUFFER_SIZE] = {0};

    buffer[BUFFER_SIZE - 1] = '\0';
    INT32 index;
    for (index = BUFFER_SIZE - 2; (number != 0) && (index >= 0); index--, number /= 10)
    {
        buffer[index] = '0' + number % 10;
    }
    return string(&buffer[index + 1]);
}

BOOL FollowChild(CHILD_PROCESS cProcess, VOID* userData)
{
    BOOL res;
    INT appArgc;
    CHAR const* const* appArgv;
    OS_PROCESS_ID pid = CHILD_PROCESS_GetId(cProcess);

    CHILD_PROCESS_GetCommandLine(cProcess, &appArgc, &appArgv);

    //Inject only if the KnobParentApplicationName value is current child process application
    string childAppToInject = KnobParentApplicationName.Value();
    string childApp(appArgv[0]);
    string::size_type index = childApp.find(childAppToInject);
    if (index == string::npos)
    {
        return FALSE;
    }

    //Set Pin's command line for child process
    INT pinArgc = 0;
    CHAR const* pinArgv[20];

    string pin         = KnobPinFullPath.Value() + "pin";
    pinArgv[pinArgc++] = pin.c_str();
    pinArgv[pinArgc++] = "-follow_execv";
    if (KnobProbeChild)
    {
        pinArgv[pinArgc++] = "-probe"; // pin in probe mode
    }
    pinArgv[pinArgc++] = "-t";
    string tool        = KnobToolsFullPath.Value() + KnobParentToolName.Value();
    pinArgv[pinArgc++] = tool.c_str();
    if (KnobProbeGrandChild)
    {
        pinArgv[pinArgc++] = "-probe_child";
        pinArgv[pinArgc++] = "1";
    }
    pinArgv[pinArgc++] = "-pin_path";
    pinArgv[pinArgc++] = KnobPinFullPath.Value().c_str();
    pinArgv[pinArgc++] = "-tools_path";
    pinArgv[pinArgc++] = KnobToolsFullPath.Value().c_str();
    pinArgv[pinArgc++] = "-check_process_id";
    pinArgv[pinArgc++] = "1";
    pinArgv[pinArgc++] = "-process_id";
    string decPidStr   = Int64ToDecimalString(pid).c_str();
    pinArgv[pinArgc++] = decPidStr.c_str();
    pinArgv[pinArgc++] = "--";

    CHILD_PROCESS_SetPinCommandLine(cProcess, pinArgc, pinArgv);

    return TRUE;
}

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    cout << "In grand_parent_tool PinTool is probed " << decstr(PIN_IsProbeMode()) << endl;

    PIN_AddFollowChildProcessFunction(FollowChild, 0);

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
