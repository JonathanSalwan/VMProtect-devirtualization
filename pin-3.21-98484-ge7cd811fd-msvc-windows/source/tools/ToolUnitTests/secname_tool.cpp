/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>
#include <string>
#include "pin.H"

using std::endl;

/* ===================================================================== */
/* Global Variables and Definitions */
/* ===================================================================== */

std::ofstream trace;

/* ===================================================================== */

VOID ImageLoad(IMG img, VOID* v)
{
    // Looking for sections only in main image
    IMG_TYPE imgType = IMG_Type(img);
    if (imgType == IMG_TYPE_STATIC || imgType == IMG_TYPE_SHARED)
    {
        for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
        {
            trace << "Section " << SEC_Name(sec) << " found" << endl;
        }
    }
}

VOID Fini(INT32 code, VOID*) { trace.close(); }

int main(INT32 argc, CHAR** argv)
{
    trace.open("secname.out");

    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
