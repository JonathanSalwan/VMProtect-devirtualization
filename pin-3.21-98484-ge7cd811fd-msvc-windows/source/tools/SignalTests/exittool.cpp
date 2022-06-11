/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <fstream>
#include "pin.H"
using std::endl;
using std::ofstream;
using std::string;

static void OnExit(INT32, VOID*);

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "exittool.out", "specify output file name");

ofstream OutFile;

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    OutFile.open(KnobOutputFile.Value().c_str());

    PIN_AddFiniFunction(OnExit, 0);

    PIN_StartProgram();
    return 0;
}

static void OnExit(INT32 code, VOID* v) { OutFile << "Tool sees exit" << endl; }
