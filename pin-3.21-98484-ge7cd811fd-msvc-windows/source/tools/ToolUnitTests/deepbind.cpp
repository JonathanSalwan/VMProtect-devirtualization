/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test checks the usage of the -selfcontained_tool knob. This knob loads the
 * tool with the RTLD_DEEPBIND flag, making it a self-contained library. The test
 * verifies that the usage does not impact Pin or the application. We override two
 * libc functions, "sched_yield" and "sleep", which are used by the application and
 * Pin. Both functions will cause the process to exit with a non-zero return value.
 * "sched_yield" is implemented in this file, while "sleep" is implemented in
 * deepbind_syscalls.o which is linked to the tool.
 */

#include "pin.H"
#include <cstdio>
#include <cstdlib>

extern "C" int sched_yield()
{
    fprintf(stderr, "ERROR: In deepbind.cpp implementation of sched_yield\n");
    exit(2);
}

int main(int argc, char** argv)
{
    PIN_InitSymbols();

    if (!PIN_Init(argc, argv))
    {
        PIN_StartProgram();
    }

    return (1);
}
