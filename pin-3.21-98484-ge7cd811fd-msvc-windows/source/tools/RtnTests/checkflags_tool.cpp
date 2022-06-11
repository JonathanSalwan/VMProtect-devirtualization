/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include "pin.H"
#include "tool_macros.h"

using std::cerr;
using std::cout;
using std::endl;

const char* checkFlagsFuncName = C_MANGLE("CheckFlags");
const char* toolRtnName        = C_MANGLE("ToolRtn");

void PushfAnalysis() { cout << "TOOL INFO: In PushfAnalysis." << endl; }

VOID Image(IMG img, VOID* v)
{
    if (!IMG_IsMainExecutable(img)) return;
    const RTN rtn = RTN_FindByName(img, checkFlagsFuncName);
    if (!RTN_Valid(rtn))
    {
        cerr << "TOOL ERROR: Unable to find " << checkFlagsFuncName << "." << endl;
        PIN_ExitProcess(11);
    }
    unsigned int numOfPushfFound = 0;
    bool instrumentationComplete = false;
    RTN_Open(rtn);
    for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
    {
        const OPCODE opcode = INS_Opcode(ins);
        if (XED_ICLASS_PUSHF == opcode || XED_ICLASS_PUSHFD == opcode || XED_ICLASS_PUSHFQ == opcode)
        {
            ++numOfPushfFound;
            if (1 == numOfPushfFound) continue;
            if (2 < numOfPushfFound)
            {
                cerr << "TOOL ERROR: Unexpected number of pushf instructions found - " << numOfPushfFound << "." << endl;
                PIN_ExitProcess(12);
            }
            RTN_Close(rtn);
            const RTN toolRtn = RTN_CreateAt(INS_Address(ins), toolRtnName);
            if (!RTN_Valid(toolRtn))
            {
                cerr << "TOOL ERROR: Unable to create " << toolRtnName << "." << endl;
                PIN_ExitProcess(13);
            }
            RTN_InsertCallProbed(toolRtn, IPOINT_BEFORE, PushfAnalysis, IARG_END);
            RTN_Open(rtn);
            instrumentationComplete = true;
        }
    }
    RTN_Close(rtn);
    if (!instrumentationComplete)
    {
        cerr << "TOOL ERROR: Failed to add instrumentation." << endl;
        PIN_ExitProcess(14);
    }
}

int main(int argc, char* argv[])
{
    // Initialization.
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    // Add instrumentation.
    IMG_AddInstrumentFunction(Image, 0);

    // Start the application.
    PIN_StartProgramProbed(); // never returns
    return 0;
}
