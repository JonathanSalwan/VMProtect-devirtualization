/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <fstream>
#include "pin.H"

KNOB< std::string > KnobOut(KNOB_MODE_WRITEONCE, "pintool", "o", "mt-exit-tool.out", "Output file");

std::ofstream Out;

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    Out.open(KnobOut.Value().c_str());
    Out << std::dec << PIN_GetPid() << std::endl;
    Out.close();

    PIN_StartProgram();
    return 0;
}
