/*
 * Copyright (C) 2016-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This Pin tool is dynamically linked with the backtrace_dll library
 * and calls the function foo() that is implemented in that library.
 * The library backtrace_dll is linked against libunwind and should produce
 * the correct backtrace of the calling stack.
 */
#include <stdlib.h>
#include <iostream>
#include "pin.H"
using std::cerr;
using std::endl;

// Implemented in backtrace_dll
extern "C" void foo();

void InstImage(IMG img, void* v)
{
    if (IMG_IsMainExecutable(img))
    {
        foo();
    }
}

int main(int argc, char** argv)
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        cerr << "usage..." << endl;
        return EXIT_FAILURE;
    }

    IMG_AddInstrumentFunction(InstImage, 0);

    PIN_StartProgram();
    return EXIT_FAILURE;
}
