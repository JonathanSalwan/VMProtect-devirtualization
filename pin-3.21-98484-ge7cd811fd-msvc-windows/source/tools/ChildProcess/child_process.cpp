/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * In addition to what this test checks, we also check these:
 * - In case 'ParentEnv' environment variable was set by the application (after Pin tool over)
 *   which executed the current application we print it here so we can check it from the makefile (by using GREP)
 */

//Child process application
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <string>
#include <iostream>
using std::cout;
using std::endl;
using std::string;

int main(int argc, char* argv[], char* envp[])
{
    int i = 0;

    // In case 'ParentEnv' environment variable was set by the application (after Pin tool over)
    // which executed the current application we print it here so we can check it from the makefile (by using GREP)
    while (envp[i] != 0)
    {
        if (string(envp[i]).compare(string("ParentEnv=1")) == 0)
        {
            cout << envp[i] << endl;
        }
        i++;
    }

    if (argc != 3)
    {
        cout << "Child report: expected 2 parameters, received " << argc - 1 << endl;
        return -1;
    }
    if (strcmp(argv[1], "param1 param2") || strcmp(argv[2], "param3"))
    {
        cout << "Child report: wrong parameters: " << argv[1] << " " << argv[2] << endl;
        return -1;
    }
    cout << "Child report: The process works correctly!" << endl;
    return 0;
}
