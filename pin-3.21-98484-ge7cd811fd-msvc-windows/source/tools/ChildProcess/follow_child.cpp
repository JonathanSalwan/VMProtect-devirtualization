/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>
#include <fstream>
using std::cout;
using std::endl;
using std::ofstream;
using std::string;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "inscount.out", "specify output file name");

ofstream OutFile;

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cout << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
VOID Fini(INT32 code, VOID* v)
{
    OutFile << "In follow_child PinTool" << endl;
    OutFile.close();
}

/* ===================================================================== */

int main(INT32 argc, CHAR** argv)
{
    if (PIN_Init(argc, argv)) return Usage();

    // If the file is not appended to, every instance of the pintool will overwrite it with its own output.
    OutFile.open(KnobOutputFile.Value().c_str(), ofstream::app);

    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
