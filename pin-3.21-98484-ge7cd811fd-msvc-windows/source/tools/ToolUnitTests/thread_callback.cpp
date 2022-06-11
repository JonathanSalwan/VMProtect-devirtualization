/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//  Sample usage:
//    pin -mt -t thread_callback -- thread_wait

#include <stdio.h>
#include <iostream>
#include "pin.H"

FILE* out;
PIN_LOCK pinLock;
int numThreads;
typedef int (*FUNCPTR_ADD)(int, int);
AFUNPTR pf_Add = 0;

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    if (pf_Add == 0x0) return;

    fprintf(out, "thread begin %d\n", threadid);
    numThreads++;

    int sum = 0;
    int one = 1;
    int two = 2;

    PIN_CallApplicationFunction(ctxt, threadid, CALLINGSTD_DEFAULT, (AFUNPTR)pf_Add, NULL, PIN_PARG(int), &sum, PIN_PARG(int),
                                one, PIN_PARG(int), two, PIN_PARG_END());

    fprintf(out, "%d+%d=%d\n", one, two, sum);
    fflush(out);
}

VOID ThreadFini(THREADID threadid, const CONTEXT* ctxt, INT32 code, VOID* v)
{
    if (pf_Add == 0x0) return;

    fprintf(out, "thread end %d code %d\n", (int)threadid, (int)code);

    int sum   = 0;
    int three = 3;
    int four  = 4;

    PIN_CallApplicationFunction(const_cast< CONTEXT* >(ctxt), threadid, CALLINGSTD_DEFAULT, (AFUNPTR)pf_Add, NULL, PIN_PARG(int),
                                &sum, PIN_PARG(int), three, PIN_PARG(int), four, PIN_PARG_END());

    fprintf(out, "%d+%d=%d\n", three, four, sum);
}

VOID Fini(INT32 code, VOID* v)
{
    fprintf(out, "Fini: code %d\n", code);
    fclose(out);
}

VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        std::cout << "Processing image " << IMG_Name(img) << std::endl;

        RTN rtn = RTN_FindByName(img, "Add2");
        if (RTN_Valid(rtn))
        {
            pf_Add = (AFUNPTR)RTN_Address(rtn);

            fprintf(out, "address of application function = %p\n", (void*)pf_Add);
            fflush(out);
        }
        else
            std::cout << "Cannot find Add2" << std::endl;
    }
}

int main(INT32 argc, CHAR** argv)
{
    PIN_InitLock(&pinLock);

    out        = fopen("thread_callback.out", "w");
    numThreads = 1;

    PIN_InitSymbols();

    PIN_Init(argc, argv);

    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);
    PIN_AddFiniFunction(Fini, 0);
    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
