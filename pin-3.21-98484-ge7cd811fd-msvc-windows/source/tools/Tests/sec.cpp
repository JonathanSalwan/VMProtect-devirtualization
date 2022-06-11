/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Print data on each SEC.
 */

#include <iostream>
#include <fstream>
#include "pin.H"
using std::endl;
using std::ofstream;
using std::string;

ofstream out;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "sec.out", "specify output file name");

VOID ImageLoad(IMG img, VOID* v)
{
    out << "Tool loading " << IMG_Name(img) << " at " << IMG_LoadOffset(img) << endl;
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        out << "  sec " << SEC_Name(sec) << " " << SEC_Address(sec) << ":" << SEC_Size(sec) << endl;
        string pos(" ");
        string neg(" not ");
        out << "  This sec is" << (SEC_IsReadable(sec) ? pos : neg) << "readable, ";
        out << "is" << (SEC_IsWriteable(sec) ? pos : neg) << "writeable, ";
        out << "is" << (SEC_IsExecutable(sec) ? pos : neg) << "executable, ";
        out << "and is" << (SEC_Mapped(sec) ? pos : neg) << "mapped." << endl;
    }
}

int main(INT32 argc, CHAR** argv)
{
    out.open(KnobOutputFile.Value().c_str());

    PIN_InitSymbols();
    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
