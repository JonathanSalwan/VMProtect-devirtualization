/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include "pin.H"
using std::cerr;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::string;

//This tool define a new handler for the exception by using PIN_AddContextChangeFunction
//to test if the tool receive the exception that raised during executing protection_test_app.c

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "protection_test_tool.log", "specify tool log file name");

// When PIN_AddFetchFunction() is used Pin allows fetch & execute of non-executable code.
// This knob is used to check this flow.
KNOB< BOOL > KnobFetchCodeCallBack(KNOB_MODE_WRITEONCE, "pintool", "fetch_cb", "0", "Use fetch callback");

ofstream OutFile;

//exception handler
static void OnException(THREADID threadIndex, CONTEXT_CHANGE_REASON reason, const CONTEXT* ctxtFrom, CONTEXT* ctxtTo, INT32 info,
                        VOID* v)
{
    if (reason == CONTEXT_CHANGE_REASON_EXCEPTION || reason == CONTEXT_CHANGE_REASON_SIGNAL)
    {
        OutFile << "Tool: Start handling exception." << endl;
        OutFile.flush();
        UINT32 exceptionCode = info;
        ADDRINT exceptAddr   = PIN_GetContextReg(ctxtFrom, REG_INST_PTR);
        OutFile << "Tool: Exception code " << std::hex << exceptionCode << "."
                << " Context IP " << std::hex << exceptAddr << "." << endl;
        OutFile.flush();
    }
}

VOID Fini(INT32 code, VOID* v)
{
    OutFile << "PinFiniFunction" << endl;
    OutFile.flush();
    OutFile.close();
}

static size_t fetchIns(void* buf, ADDRINT addr, size_t size, EXCEPTION_INFO* pExceptInfo, VOID*)
{
    size_t bytesRead = PIN_SafeCopyEx(static_cast< UINT8* >(buf), reinterpret_cast< UINT8* >(addr), size, pExceptInfo);

    return bytesRead;
}

int main(INT32 argc, CHAR** argv)
{
    PIN_Init(argc, argv);
    OutFile.open(KnobOutputFile.Value().c_str());
    OutFile << std::hex;
    OutFile.setf(std::ios::showbase);

    if (KnobFetchCodeCallBack)
    {
        // When PIN_AddFetchFunction() is used Pin allows fetch & execute of non-executable code.
        // checking this flow works as expected
        PIN_AddFetchFunction(fetchIns, 0);
    }
    PIN_AddContextChangeFunction(OnException, 0);
    PIN_AddFiniFunction(Fini, 0);
    // Never returns
    PIN_StartProgram();

    return 0;
}
