/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This tool is used to test passing the IARG_TSC argument to an analysis function in probe mode.

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include "pin.H"
using std::cout;
using std::dec;
using std::endl;
using std::hex;
using std::string;

#if defined(TARGET_MAC)
const char* FACTORIAL_NAME = "_factorial";
#else
const char* FACTORIAL_NAME = "factorial";
#endif

std::ostream* OutFile = 0;

/* ===================================================================== */
/* Command line switches                                                 */
/* ===================================================================== */

KNOB< string > KnobOutputFile(
    KNOB_MODE_WRITEONCE, "pintool", "o", "",
    "Specify file name for the tool's output. If no filename is specified, the output will be directed to stdout.");

VOID AtRtn(VOID* name, ADDRINT arg1, UINT64 tsc)
{
    *OutFile << reinterpret_cast< CHAR* >(name) << "(" << arg1 << ")"
             << "\tTime Stamp Counter is: " << hex << tsc << dec << endl;
}

VOID Image(IMG img, VOID* v)
{
    if (!IMG_IsMainExecutable(img))
    {
        return;
    }

    RTN rtn = RTN_FindByName(img, FACTORIAL_NAME);
    if (RTN_Valid(rtn))
    {
        if (!RTN_IsSafeForProbedInsertion(rtn))
        {
            *OutFile << "It is not safe to insert a call before " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;
            return;
        }

        RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(AtRtn), IARG_PTR, RTN_Name(rtn).c_str(), IARG_FUNCARG_ENTRYPOINT_VALUE,
                             0, IARG_TSC, IARG_END);
    }
    else
    {
        *OutFile << FACTORIAL_NAME << " wasn't found." << endl;
    }
}

int main(int argc, char** argv)
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    OutFile = KnobOutputFile.Value().empty() ? &cout : new std::ofstream(KnobOutputFile.Value().c_str());

    IMG_AddInstrumentFunction(Image, 0);

    PIN_StartProgramProbed();
    return 0;
}
