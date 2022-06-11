/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file */

#include <iostream>
#include <sys/time.h>
#include "pin.H"
using std::cout;
using std::endl;
using std::string;

string rtn_name;

KNOB< string > KnobFunctionNameToInstrument(KNOB_MODE_WRITEONCE, "pintool", "function_name", "gettimeofday",
                                            "function name to instrument");

int NewImplementation(const char* ptr1, const char* ptr2)
{
    cout << "New implementation!" << endl;
    return 0;
}

void BeforeResolverFunction(void* img_name, ADDRINT rtn_addr) { cout << "Hello once (resolver)" << endl; }

VOID ImageLoad(IMG img, VOID* v)
{
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            if (RTN_Name(rtn).compare(rtn_name) == 0 || RTN_Name(rtn).compare("__" + rtn_name) == 0)
            {
                if (!SYM_IFuncResolver(RTN_Sym(rtn))) continue;

                cout << "Found " << RTN_Name(rtn).c_str() << " in " << IMG_Name(img);
                RTN resolver = rtn;
                RTN impl     = RTN_IFuncImplementation(rtn);

                cout << "...  Replacing" << endl;

                ASSERTX(RTN_Valid(resolver));
                ASSERTX(RTN_Valid(impl));

                RTN_Open(impl);
                RTN_InsertCall(impl, IPOINT_BEFORE, AFUNPTR(NewImplementation), IARG_END);
                RTN_Close(impl);

                // Instrumenting the resolver function, should be called once
                RTN_Open(resolver);
                RTN_InsertCall(resolver, IPOINT_BEFORE, AFUNPTR(BeforeResolverFunction), IARG_PTR, IMG_Name(img).c_str(),
                               IARG_ADDRINT, RTN_Address(rtn), IARG_END);
                RTN_Close(resolver);
            }
        }
    }
}

int main(INT32 argc, CHAR* argv[])
{
    // Initialize pin
    //
    if (PIN_Init(argc, argv)) return 0;

    // Initialize symbol processing
    //
    PIN_InitSymbolsAlt(IFUNC_SYMBOLS);

    //Initialize global variables
    rtn_name = KnobFunctionNameToInstrument.Value();
    cout << "rtn_name : " << rtn_name << endl;

    // Register ImageLoad to be called when an image is loaded
    //
    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Start the program, never returns
    //
    PIN_StartProgram();

    return 0;
}
