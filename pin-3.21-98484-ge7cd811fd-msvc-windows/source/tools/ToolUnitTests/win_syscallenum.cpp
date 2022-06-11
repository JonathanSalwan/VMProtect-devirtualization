/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * Check that we can obtain the windows system call numbers from the 
 * system call keys, and convert back from the number to the key.
 */

#include <iostream>

#include "pin.H"

using std::cerr;
using std::endl;

static KNOB< BOOL > KnobVerbose(KNOB_MODE_WRITEONCE, "pintool", "v", "0", "enable verbose output");

static int checkSyscalls(BOOL verbose)
{
    int successes = 0;

    if (verbose)
    {
        cerr << "Key   Call" << endl;
    }

    for (UINT32 key = SYSCALL_KEY_FIRST; key < SYSCALL_KEY_END; key++)
    {
        UINT32 syscallNumber = PIN_GetWindowsSyscallFromKey(SYSCALL_KEY(key));

        if (verbose)
        {
            cerr << decstr(key, 3) << "   " << hexstr(syscallNumber, 4) << endl;
        }

        // Some system calls are not available on some versions of the OS,
        // e.g.
        //    NtCreateUserProcess is only available on Vista (and later)
        //    NtCreateThreadEx    is only available on Vista (and later)
        if (syscallNumber == SYSCALL_NUMBER_INVALID)
        {
            continue;
        }

        SYSCALL_KEY secondKey = PIN_GetKeyFromWindowsSyscall(syscallNumber);

        if (secondKey != key)
        {
            continue;
        }

        successes++;
    }

    return successes;
}

int main(INT32 argc, CHAR* argv[])
{
    PIN_Init(argc, argv);

    int successes = checkSyscalls(KnobVerbose);

    cerr << successes << " known system calls found" << endl;
    if (successes == 0)
    {
        exit(1); /* Failed to find any, something really is wrong. */
    }
    exit(0);
}
