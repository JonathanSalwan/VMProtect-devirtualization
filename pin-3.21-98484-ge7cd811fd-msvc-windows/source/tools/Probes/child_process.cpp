/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//Child process application
#include <stdio.h>
#include <string.h>
#include <string>
#include <iostream>
using std::cout;
using std::endl;

int main(int argc, char* argv[])
{
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
