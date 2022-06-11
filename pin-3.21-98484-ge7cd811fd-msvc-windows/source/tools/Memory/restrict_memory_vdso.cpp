/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This simple PIN tool checks that the memory reservation (given on -restrict_memory
 * or -reserve_memory on PIN's command line) was either succeeded or failed according
 * to the knob value that it gets
 */
#include <stdio.h>
#include <stdlib.h>
#include "pin.H"

/* ===================================================================== 
 * Commandline Switches 
 * ===================================================================== */

KNOB< BOOL > KnobReservationSuccess(KNOB_MODE_WRITEONCE, "pintool", "success", "FALSE", "Expect memory reservation to succeed");

/* ===================================================================== 
 * bool2status(BOOL)
 * Translates TRUE to the string "success" and FALSE to the string "failure"
 * ===================================================================== */

const char* bool2status(BOOL val) { return val ? "success" : "failure"; }

/* =====================================================================
 * main
 * ===================================================================== */

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    ASSERT(KnobReservationSuccess.Value() == PIN_WasMemoryReservedInLoadTime(),
           "Expected: reservation " + bool2status(KnobReservationSuccess.Value()) + " but actually got " + "reservation " +
               bool2status(PIN_WasMemoryReservedInLoadTime()));

    // Never returns
    PIN_StartProgram();

    return 0;
}
