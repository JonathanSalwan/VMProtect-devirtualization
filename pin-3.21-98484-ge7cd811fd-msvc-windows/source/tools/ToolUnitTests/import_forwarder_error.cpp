/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This Pin tool is created to validate that import forwarded to
 *  unknown DLL (other than ntdll.dll and kernel32.dll)
 *  will not be resolved by Pin loader.
 *  The tool DLL will not be loaded.
 */

#include "pin.H"
#include <iostream>
using std::cerr;
using std::flush;

// Linker requires definition of target forwarded function
extern "C" int bar() { return 0; }

// Specify export directive for linker to define forwarder.
// Forwarder DLL does not exist.
// Note that name of export is mangled.
#if defined(TARGET_IA32)
#pragma comment(linker, "/EXPORT:_foo=nodll.bar")
#else
#pragma comment(linker, "/EXPORT:foo=nodll.bar")
#endif

// Declare imported function.
// Its target will be forwarder.
extern "C" __declspec(dllimport) int foo();

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return 1;
    }

    // Call imported function.
    // Linker will resolve the reference using import table.
    cerr << foo() << flush;

    // Never returns
    PIN_StartProgramProbed();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
