/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>

using std::cerr;
using std::endl;

static void pinException() { cerr << "APP: in pinException" << endl; }

static void toolException() { cerr << "APP: in toolException" << endl; }

static int appException()
{
    cerr << "APP: in appException" << endl;
    int zero = 0; // This is to avoid getting a compiler warning of division by zero.
    return 1 / zero;
}

int main()
{
    // Cause a Pin exception via PIN_SafeCopyEx.
    pinException();

    // Cause a tool exception.
    toolException();

    // Cause an application exception (SIGFPE) - divide by zero.
    appException();

    return 0;
}
