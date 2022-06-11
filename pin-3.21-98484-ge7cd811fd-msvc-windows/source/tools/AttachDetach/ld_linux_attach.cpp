/*
 * Copyright (C) 2016-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>
using std::cout;
using std::endl;

/* ===================================================================== */

BOOL MyPinAttached() { return TRUE; }

/* ===================================================================== */

VOID ImageLoad(IMG img, VOID* v)
{
    RTN rtn = RTN_FindByName(img, "PinAttached");
    if (RTN_Valid(rtn))
    {
        RTN_Replace(rtn, (AFUNPTR)MyPinAttached);
    }

    if (IMG_IsMainExecutable(img) || IMG_IsVDSO(img))
    {
        return;
    }

    //If str dosn't include "/" then str.substr(idx + 1) returns full str.
    INT idx = IMG_Name(img).find_last_of("/");
    cout << IMG_Name(img).substr(idx + 1) << endl;
}

/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, NULL);

    PIN_StartProgram();

    return 1;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
