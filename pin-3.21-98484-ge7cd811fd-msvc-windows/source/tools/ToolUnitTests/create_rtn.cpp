/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Tool that tests the RTN_CreateAt function.
 * RTN_CreateAt() may be used in Probe mode and in JIT mode for
 * ahead-of-time (aoti) instrumentation
 *
 * We test the both Probe and JIT mode. We create a routine with
 * RTN_CreateAt and then (1) replace it and (2) insert a call before
 * the routine
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "pin.H"

using std::cout;
using std::endl;
using std::hex;
using std::string;

void MyReplacement() { cout << "The newly created routine was replaced successfully" << endl; }

void MyInsertedCall() { cout << "The call was inserted successfully" << endl; }

/* ===================================================================== */

VOID ImageLoad(IMG img, VOID* v)
{
    BOOL found = FALSE;
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        if (SEC_Name(sec) == ".fTable")
        {
            // .fTable section holds 2 addresses of 2 routines
            // The both routines exist, but the symbols should not
            // be in symbol table
            ADDRINT offset = 0;
            // When instrumenting a PIE file, IMG_LoadOffset will contain the value that we need to find
            // the actual address of the routines we want to create in memory. When we instrument a non-PIE file,
            // the load offset is 0 on linux. But at least in Windows, IMG_LoadOffset does not return 0 even though
            // the file isn't a PIE. Added a check so we only get the load offset if the executable is a PIE.
#ifndef TARGET_MAC
            if (IMG_Type(img) == IMG_TYPE_SHAREDLIB)
#else
            if (IMG_Type(img) == IMG_TYPE_SHARED)
#endif
            {
                offset = IMG_LoadOffset(img);
            }
            ADDRINT* secAddr  = reinterpret_cast< ADDRINT* >(SEC_Address(sec));
            ADDRINT proc1Addr = secAddr[0] + offset;
            ADDRINT proc2Addr = secAddr[1] + offset;

            // 1. Replace Proc1
            RTN rtn = RTN_FindByAddress(proc1Addr);
            if (RTN_Address(rtn) != proc1Addr)
            {
                rtn = RTN_CreateAt(proc1Addr, "Proc1NewRoutine");
                if (!RTN_Valid(rtn))
                {
                    cout << "Can't create a routine " << endl;
                    exit(-1);
                }
                // The routine was created successfully

                PROTO proto = PROTO_Allocate(PIN_PARG(int), CALLINGSTD_DEFAULT, "MyReplacement", PIN_PARG_END());
                if (PIN_IsProbeMode())
                {
                    if (RTN_IsSafeForProbedReplacement(rtn))
                    {
                        RTN_ReplaceSignatureProbed(rtn, AFUNPTR(MyReplacement), IARG_PROTOTYPE, proto, IARG_END);
                    }
                }
                else
                {
                    RTN_ReplaceSignature(rtn, AFUNPTR(MyReplacement), IARG_PROTOTYPE, proto, IARG_END);
                }
            }
            else
            {
                cout << "Proc1: Existing routine has been found at the given address, 0x" << hex << proc1Addr
                     << ". The new routine will not be created" << endl;
                exit(-1);
            }

            // 2. Insert a call before Proc2

            rtn = RTN_FindByAddress(proc2Addr);
            if (RTN_Address(rtn) != proc2Addr)
            {
                rtn = RTN_CreateAt(proc2Addr, "Proc2NewRoutine");
                if (!RTN_Valid(rtn))
                {
                    cout << "Can't create a routine " << endl;
                    exit(-1);
                }
                // The routine was created successfully

                PROTO proto = PROTO_Allocate(PIN_PARG(int), CALLINGSTD_DEFAULT, "MyInsertedCall", PIN_PARG_END());
                if (PIN_IsProbeMode())
                {
                    if (RTN_IsSafeForProbedInsertion(rtn))
                    {
                        RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(MyInsertedCall), IARG_PROTOTYPE, proto, IARG_END);
                    }
                }
                else
                {
                    RTN_Open(rtn);
                    RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(MyInsertedCall), IARG_PROTOTYPE, proto, IARG_END);
                    RTN_Close(rtn);
                }
            }
            else
            {
                cout << "Proc2: Existing routine has been found at the given address, 0x" << hex << proc2Addr
                     << ". The new routine will not be created" << endl;
                exit(-1);
            }

            found = TRUE;
            break;
        }
    }
    // Ensure that the new routine can be found inside image

    if (found)
    {
        if (!RTN_Valid(RTN_FindByName(img, "Proc1NewRoutine")))
        {
            cout << "The newly created routine is not correctly inserted into the image." << endl;
            exit(-1);
        }
    }
}

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);
    // Ignore debug symbols
    PIN_InitSymbolsAlt(EXPORT_SYMBOLS);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Never returns
    if (PIN_IsProbeMode())
    {
        cout << "Testing the Probe mode." << endl;
        PIN_StartProgramProbed();
    }
    else
    {
        cout << "Testing the JIT mode." << endl;
        PIN_StartProgram();
    }

    return 0;
}
