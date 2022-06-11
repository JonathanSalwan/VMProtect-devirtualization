/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <fstream>

#include "pin.H"
#include "instlib.H"

using namespace INSTLIB;

FOLLOW_CHILD follow;

INT32 Usage()
{
    cerr << "This pin tool demonstrates use of FOLLOW_CHILD to inject pin in programs that call exec\n"
            "\n";

    cerr << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    follow.Activate();

    // Use the same prefix as our command line
    follow.SetPrefix(argv);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
