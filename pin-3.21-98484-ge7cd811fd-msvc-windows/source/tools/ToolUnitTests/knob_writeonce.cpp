/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test checks that knobs, defined as "write once", can be specified
 * more than once in the command line as long as their values are exactly
 * the same.  This scenario occurs, for example, when the user enters the
 * same knob twice by mistake in the command line.
 */

#include "pin.H"
using std::string;

KNOB< string > KnobTest(KNOB_MODE_WRITEONCE, "pintool", "test", "", "Test knob - checks the operation of 'write once' knobs");

int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv)) return 1;

    PIN_StartProgram();
    return 0;
}
