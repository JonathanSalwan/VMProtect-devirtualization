/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/// @file alignchk.cpp

#include "pin.H"
#include <iostream>
#include <iomanip>
#include <stdlib.h>

#include "alignchk.H"
using std::cerr;
using std::endl;
void usage()
{
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
}

INSTLIB::ALIGN_CHECK align_check;
int main(int argc, char** argv)
{
    if (PIN_Init(argc, argv))
    {
        usage();
        return 1;
    }

    align_check.Activate();

    // Never returns
    PIN_StartProgram();

    // NOTREACHED
    return 0;
}
