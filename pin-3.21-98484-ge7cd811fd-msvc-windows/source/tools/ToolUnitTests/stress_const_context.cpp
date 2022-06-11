/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "pin.H"
/*
Test to assure no stack leak in IARG_CONST_CONTEXT. Must be run with the stress_const_context_app
*/

#ifdef TARGET_MAC
#define STRESSTEST_FN_NAME "_StressTestConstContextAppFunc"
#else
#define STRESSTEST_FN_NAME "StressTestConstContextAppFunc"
#endif

UINT32 numCallsToStressTestConstContextToolFunc = 0;
VOID StressTestConstContextToolFunc(ADDRINT* fer0, ADDRINT* fer1, ADDRINT* fer2, ADDRINT* fer3, ADDRINT* fer4, ADDRINT* fer5,
                                    ADDRINT returnIP, ADDRINT stackPointer, ADDRINT threadID, CONTEXT* ctxt)
{
    numCallsToStressTestConstContextToolFunc++;
    if ((numCallsToStressTestConstContextToolFunc % 100000) == 0)
    {
        printf("%d calls to StressTestConstContextToolFunc\n", numCallsToStressTestConstContextToolFunc);
        fflush(stdout);
    }
}

// When an image is loaded, check for a MyAlloc function
VOID Image(IMG img, VOID* v)
{
    //fprintf(stderr, "Loading %s\n",IMG_name(img));

    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        //fprintf(stderr, "  sec %s\n", SEC_name(sec).c_str());
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            //fprintf(stderr, "    rtn %s\n", RTN_Name(rtn).c_str());

            if (RTN_Name(rtn) == STRESSTEST_FN_NAME)
            {
                RTN_Open(rtn);

                RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)StressTestConstContextToolFunc, IARG_FUNCARG_ENTRYPOINT_REFERENCE, 0,
                               IARG_FUNCARG_ENTRYPOINT_REFERENCE, 1, IARG_FUNCARG_ENTRYPOINT_REFERENCE, 2,
                               IARG_FUNCARG_ENTRYPOINT_REFERENCE, 3, IARG_FUNCARG_ENTRYPOINT_REFERENCE, 4,
                               IARG_FUNCARG_ENTRYPOINT_REFERENCE, 5,
                               IARG_RETURN_IP, // address of inst after caller
                               IARG_REG_VALUE, REG_STACK_PTR, IARG_THREAD_ID, IARG_CONST_CONTEXT, IARG_END);

                RTN_Close(rtn);
            }
        }
    }
}

VOID Fini(INT32 code, VOID* v)
{
    if (numCallsToStressTestConstContextToolFunc < 4000000)
    {
        printf("expetced 4000000 calls to StressTestConstContextToolFunc, had only %u\n",
               numCallsToStressTestConstContextToolFunc);
        fflush(stdout);
        exit(-1);
    }
}

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(Image, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
