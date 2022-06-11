/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Knobs have a name, which is the text in the command-line (after the
 * dash) which is used to pass values to the knob.  Pin does not allow
 * developers to define several knobs with the same exact name.  This
 * test verifies this restriction in Pin tools.
 */

#include "pin.H"
using std::string;

KNOB< string > KnobTest1(KNOB_MODE_WRITEONCE, "pintool", "test", "", "Test knob #1 - checks the operation of 'write once' knobs");

KNOB< string > KnobTest2(KNOB_MODE_WRITEONCE, "pintool", "test", "", "Test knob #2 - checks the operation of 'write once' knobs");

int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv)) return 1;

    PIN_StartProgram();
    return 0;
}
