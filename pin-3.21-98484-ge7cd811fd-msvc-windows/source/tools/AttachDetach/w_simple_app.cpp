/*
 * Copyright (C) 2016-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <iostream>

using std::cerr;
using std::endl;

extern "C" __declspec(noinline, dllexport) int PinIsAttached() { return 0; }

extern "C" __declspec(noinline, dllexport) int PinIsDetached() { return 1; }

extern "C" __declspec(noinline, dllexport) void FirstProbeInvoked()
{
    cerr << "FirstProbeInvoked shouldn't be called" << endl;
    abort();
}

extern "C" __declspec(noinline, dllexport) void SecondProbeInvoked()
{
    cerr << "SecondProbeInvoked shouldn't be called" << endl;
    abort();
}

int main()
{
    while (!PinIsAttached())
        SwitchToThread();
    FirstProbeInvoked();
    while (!PinIsDetached())
        SwitchToThread();
    while (!PinIsAttached())
        SwitchToThread();
    SecondProbeInvoked();
    while (!PinIsDetached())
        SwitchToThread();
    while (!PinIsAttached())
        SwitchToThread();

    cerr << "Test passed!" << endl;

    return 0;
}
