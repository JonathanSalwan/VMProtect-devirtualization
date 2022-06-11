/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>

/* ===================================================================== */
/* Command line Switches */
/* ===================================================================== */

KNOB< BOOL > KnobToolProbeMode(KNOB_MODE_WRITEONCE, "pintool", "probe", "0", "invoke tool in probe mode");

/* ===================================================================== */

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);

    // Never returns
    if (KnobToolProbeMode)
    {
        PIN_StartProgramProbed();
    }
    else
    {
        PIN_StartProgram();
    }
    return 0;
}
