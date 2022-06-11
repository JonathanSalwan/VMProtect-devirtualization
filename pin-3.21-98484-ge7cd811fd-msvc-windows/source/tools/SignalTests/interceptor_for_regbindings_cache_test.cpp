/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <signal.h>
#include <stdlib.h>
#include <iostream>
#include "pin.H"
#include "num_segvs.h"
using std::cout;
using std::dec;
using std::endl;
using std::hex;

int numSignalsReceived = 0;
BOOL SigFunc(THREADID tid, INT32 sig, CONTEXT* ctxt, BOOL hasHandler, const EXCEPTION_INFO* pExceptInfo, void* dummy)
{
    ADDRINT address = PIN_GetContextReg(ctxt, REG_INST_PTR);
    cout << "Thread " << tid << ": Tool got signal " << sig << " at PC " << hex << address << dec << "\n";
    numSignalsReceived++;
    if (numSignalsReceived == (NUM_SEGVS / 2))
    {
        // Invalidate this instruction in code cache so it will be reinstrumented
        cout << "invalidating after " << numSignalsReceived << endl;
        PIN_RemoveInstrumentationInRange(address, address + 20);
    }
    return (TRUE); // skip to next instruction
}

// argc, argv are the entire command line, including pin -t <toolname> -- ...
int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    PIN_InterceptSignal(SIGSEGV, SigFunc, 0);

    PIN_StartProgram();
    return 0;
}
