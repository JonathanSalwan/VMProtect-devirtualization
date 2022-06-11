/*
 * Copyright (C) 2017-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Call the iret assembler stubs.
 */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#define __USE_GNU
#include <ucontext.h>

typedef unsigned int UINT32;

extern int iretTest();

#define registerSegvHandler() ((void)0)

int main(int argc, char** argv)
{
    int result;
    int ok    = 0;
    int tests = 0;

    registerSegvHandler();

    tests++;
    fprintf(stderr, "Calling iret\n");
    result = iretTest();
    fprintf(stderr, "iretd result = %d %s\n", result, result == -1 ? "OK" : "***ERROR***");

    ok += (result == -1);

    return (ok == tests) ? 0 : -1;
}
