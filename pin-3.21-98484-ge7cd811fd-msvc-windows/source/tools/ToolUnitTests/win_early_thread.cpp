/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <string>
using std::cout;
using std::endl;
using std::flush;
extern "C" __declspec(dllimport) __declspec(noinline) int Something();

//this application has static linking with dll which creates thread in it's DllMain(PROCESS_ATTACH)
int main()
{
    cout << "in main()" << endl << flush;
    volatile int i = Something();
    return 0;
}
