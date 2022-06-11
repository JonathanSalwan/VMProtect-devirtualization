/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <string>

extern "C" __declspec(dllimport) int Something();

using std::cout;
using std::endl;
using std::flush;
//this application has static linking with dll which terminate the process in it's DllMain(PROCESS_ATTACH)
int main()
{
    //should never get here
    cout << "should never get here" << endl << flush;
    volatile int i = Something();
    exit(-1);
    return 0;
}
