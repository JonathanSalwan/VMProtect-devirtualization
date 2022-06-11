/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * A tool that has a custom command that changes the application's
 * register state.  We use this to test the "invalidate registers" API.
 */

#include "pin.H"

static BOOL OnCommand(THREADID, CONTEXT*, const std::string&, std::string*, VOID*);

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    PIN_AddDebugInterpreter(OnCommand, 0);
    PIN_StartProgram();
    return 0;
}

static BOOL OnCommand(THREADID, CONTEXT* ctxt, const std::string& cmd, std::string* reply, VOID*)
{
    if (cmd == "clear-eflags")
    {
        PIN_SetContextReg(ctxt, REG_GFLAGS, 0);
        *reply = "Changed $EFLAGS to 0\n";
        return TRUE;
    }
    return FALSE;
}
