/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file */

#include <iostream>
#include "pin.H"
using std::cout;
using std::endl;
using std::string;

static int ifunc_resolver_count = 0;

void Before_ifunc_resolver()
{
    ifunc_resolver_count++;
    cout << "Before_ifunc_resolver" << endl;
}

void Before_ifunc_exist() { cout << "Before_ifunc_exist" << endl; }

void Before_ifunc_not_exist() { cout << "Before_ifunc_not_exist" << endl; }

VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_Name(img).find("ifunc_complex_resolver_lib_app") == string::npos) return;
    RTN ifunc           = RTN_FindByName(img, "ifunc");
    RTN ifunc_exist     = RTN_FindByName(img, "ifunc_exist");
    RTN ifunc_not_exist = RTN_FindByName(img, "ifunc_not_exist");
    ASSERTX(RTN_Valid(ifunc));
    ASSERTX(RTN_Valid(ifunc_exist));
    ASSERTX(RTN_Valid(ifunc_not_exist));

    RTN_Open(ifunc);
    RTN_InsertCall(ifunc, IPOINT_BEFORE, AFUNPTR(Before_ifunc_resolver), IARG_END);
    RTN_Close(ifunc);

    RTN_Open(ifunc_exist);
    RTN_InsertCall(ifunc_exist, IPOINT_BEFORE, AFUNPTR(Before_ifunc_exist), IARG_END);
    RTN_Close(ifunc_exist);

    RTN_Open(ifunc_not_exist);
    RTN_InsertCall(ifunc_not_exist, IPOINT_BEFORE, AFUNPTR(Before_ifunc_not_exist), IARG_END);
    RTN_Close(ifunc_not_exist);
}

VOID Fini(INT32 code, VOID* v) { ASSERTX(1 == ifunc_resolver_count); }

int main(INT32 argc, CHAR* argv[])
{
    // Initialize pin
    //
    if (PIN_Init(argc, argv)) return 0;

    // Initialize symbol processing
    //
    PIN_InitSymbolsAlt(IFUNC_SYMBOLS);

    // Register ImageLoad to be called when an image is loaded
    //
    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    //
    PIN_StartProgram();

    return 0;
}
