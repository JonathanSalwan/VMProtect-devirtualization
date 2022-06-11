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
        string s = "0.001";
        FLT64 f  = FLT64FromString(s);
        cout << s << "=" << f << endl;

        s = "-0.1e-2";
        f = FLT64FromString(s);
        cout << s << "=" << f << endl;

        s = "100.";
        f = FLT64FromString(s);
        cout << s << "=" << f << endl;

        s = "1.0E2";
        f = FLT64FromString(s);
        cout << s << "=" << f << endl;

        s = "+1";
        f = FLT64FromString(s);
        cout << s << "=" << f << endl;

        // this should report an error
        s = "1.00.00";
        f = FLT64FromString(s);
        cout << s << "=" << f << endl;
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
