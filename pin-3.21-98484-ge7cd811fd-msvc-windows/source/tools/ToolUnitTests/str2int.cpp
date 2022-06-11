/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file
  This is a negative test.
*/

/* ===================================================================== */
#include "pin.H"
#include <iostream>
using std::cout;
using std::endl;
using std::string;

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        string s = "-333";
        INT32 i  = Int32FromString(s);
        cout << s << "=" << i << endl;

        s        = "0xa";
        UINT32 j = Uint32FromString(s);
        cout << s << "=" << j << endl;

        s = "0Xb";
        j = Uint32FromString(s);
        cout << s << "=" << j << endl;

        // this should report an error
        s = "a";
        j = Uint32FromString(s);
        cout << s << "=" << j << endl;
    }
}

/* ===================================================================== */
int main(INT32 argc, CHAR* argv[])
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
