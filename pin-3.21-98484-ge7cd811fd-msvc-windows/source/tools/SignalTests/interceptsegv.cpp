/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <signal.h>
#include <stdlib.h>
#include <fstream>
#include "pin.H"
using std::string;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "interceptsegv.out", "output file");
std::ofstream Out;

static BOOL SigFunc(THREADID, INT32, CONTEXT*, BOOL, const EXCEPTION_INFO*, void*);

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    Out.open(KnobOutputFile.Value().c_str());

    PIN_InterceptSignal(SIGSEGV, SigFunc, 0);
    PIN_StartProgram();
    return 0;
}

static BOOL SigFunc(THREADID tid, INT32 sig, CONTEXT* ctxt, BOOL hasHandler, const EXCEPTION_INFO* exception, void*)
{
    Out << "Thread " << std::dec << tid << ": Tool got signal ";
    if (sig == SIGSEGV)
        Out << "SIGSEGV";
    else
        Out << "<signal " << std::dec << sig << ">";
    Out << " at PC 0x" << std::hex << PIN_GetContextReg(ctxt, REG_INST_PTR) << std::endl;

    if (exception) Out << "Signal is an exception" << std::endl;
    if (hasHandler) Out << "Application has a handler for this signal" << std::endl;

    return TRUE;
}
