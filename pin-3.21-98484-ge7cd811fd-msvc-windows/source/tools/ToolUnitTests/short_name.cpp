/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
  *  Test for the shortest symbol name at an address.
 */

#include "pin.H"
#include <iostream>
#include <fstream>
using std::cerr;
using std::cout;
using std::endl;
using std::hex;
using std::ios;
using std::ofstream;
using std::string;

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

ofstream TraceFile;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "short_name.outfile", "specify profile file name");

/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool prints the shortest name for each address\n"
            "\n";

    cerr << KNOB_BASE::StringKnobSummary();

    cerr << endl;

    return -1;
}

string invalid = "invalid_rtn";
/* ===================================================================== */
const string* Target2String(ADDRINT target)
{
    string name = RTN_FindNameByAddress(target);
    if (name == "")
        return &invalid;
    else
        return new string(name);
}

/* ===================================================================== */

VOID ImageLoad(IMG img, VOID* v)
{
    cout << "Processing " << IMG_Name(img) << endl;

    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            ADDRINT rtnAddr       = RTN_Address(rtn);
            const string& rtnName = RTN_FindNameByAddress(rtnAddr);

            TraceFile << rtnName << ": " << (unsigned long)rtnAddr;

            if (SYM_Dynamic(RTN_Sym(rtn)))
                TraceFile << ", dynamic";
            else
                TraceFile << ", static";

            TraceFile << endl;
        }
    }

    cout << "Completed " << IMG_Name(img) << endl;
}

/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    TraceFile.open(KnobOutputFile.Value().c_str());
    TraceFile << hex;
    TraceFile.setf(ios::showbase);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Never returns

    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
