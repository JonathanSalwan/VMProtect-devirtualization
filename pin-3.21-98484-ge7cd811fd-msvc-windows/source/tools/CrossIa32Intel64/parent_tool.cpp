/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>

BOOL FollowChild(CHILD_PROCESS cProcess, VOID* userData)
{
    cout << "At follow child callback << endl;
        return TRUE;
}

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    cout << "In parent_tool PinTool is probed " << decstr(PIN_IsProbeMode()) << endl;

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
