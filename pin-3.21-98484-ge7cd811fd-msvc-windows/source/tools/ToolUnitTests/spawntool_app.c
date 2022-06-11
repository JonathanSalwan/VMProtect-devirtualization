/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * A trivial application that does nothing, but contains function that loops forever
 * which is invoked by Pin's aplication thread creation routine.
 */

#include <stdlib.h>
#include <unistd.h>
#include <sched.h>

void doNothing()
{
    volatile int loopCount;

    for (;;)
    {
        sched_yield();
        loopCount++;
    }
}

void (*funcPtr)();

int main(int argc, char** argv)
{
    // Ensure that the compiler thinks there is a reference to doNothing.
    funcPtr = doNothing;
    sleep(10);
    exit(0);
}
