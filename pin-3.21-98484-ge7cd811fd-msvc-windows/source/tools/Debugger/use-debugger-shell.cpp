/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This is a test of the "debugger-shell" tool extension.  See
 * "InstLib/debugger-shell.H" for a description of the functionality.
 */

#include "debugger-shell.H"

int main(int argc, char** argv)
{
    if (PIN_Init(argc, argv)) return 1;

    DEBUGGER_SHELL::ISHELL* shell = DEBUGGER_SHELL::CreateShell();
    DEBUGGER_SHELL::STARTUP_ARGUMENTS args;
    args._enableIcountBreakpoints = TRUE;
    if (!shell->Enable(args)) return 1;

    PIN_StartProgram();
}
