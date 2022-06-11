/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * In addition to what this test checks, we also check these:
 * - Checking if an environment variable which is set after Pin took over the application is being passed to
 *   the current application which was executed by the previous application
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <iostream>
using std::string;

int main(int argc, char** argv, char* envp[])
{
    printf("child:%u-%u\n", getppid(), getpid());

    int i = 0;

    // Verifying that 'ParentEnv' environment variable which was set by the application (after Pin tool over)
    // which executed the current application was passed as expected
    bool parentEnvPassed = false;
    while (envp[i] != 0)
    {
        if (string(envp[i]).compare(string("ParentEnv=1")) == 0)
        {
            parentEnvPassed = true;
        }
        i++;
    }

    assert(parentEnvPassed);

    return 0;
}
