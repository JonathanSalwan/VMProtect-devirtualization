/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file
 * 
 * This file shows how functions with floating point arguments can be wrapped.
 * FP wrappers are not supported in Pin yet.
*/

/* ===================================================================== */
#include "pin.H"
#include <iostream>
#include <string.h>
#include <stdio.h>

/* ===================================================================== */

struct DOUBLE_STRUCT
{
    int _d0;
    int _d1;
};

/*
 * A wrapper for
 *  int func(int, float, int)
 */
int fp_func3_wrapper(CONTEXT* ctxt, AFUNPTR fp_func3_ptr, int i1, float f2, int i3)
{
    int res;
#ifdef TARGET_IA32
    printf("fp_func3_wrapper parameters: %d, %f, %d\n", i1, f2, i3);
    fflush(stdout);

    /*
     * parameter of type "float" is on stack and located in 4 bytes like integer
    */
    int f2_int = *(int*)(&f2);
    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, fp_func3_ptr, NULL, PIN_PARG(int), &res, PIN_PARG(int),
                                i1, PIN_PARG(int), f2_int, PIN_PARG(int), i3, PIN_PARG_END());
#else

    /*
    * all fp parameters are in xmm registers
    * take them from the context
    */
    FPSTATE fpState;
    PIN_GetContextFPState(ctxt, &fpState);
    memcpy(&f2, &fpState.fxsave_legacy._xmms[0], sizeof(f2));
    printf("fp_func3_wrapper parameters: %d, %f, %d\n", i1, f2, i3);
    fflush(stdout);

    // all fp args are in xmm regs in context - do not pass any args to the app function
    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, fp_func3_ptr, NULL, PIN_PARG(int), &res, PIN_PARG(int),
                                i1, PIN_PARG(int), i3, PIN_PARG_END());
#endif

    return res;
}

/*
 * A wrapper for
 *  float func(double, float, int, double)
 */
float fp_func4_wrapper(CONTEXT* ctxt, AFUNPTR fp_ptr, double d1, float f2, int i3, double d4)
{
    float resf;

#ifdef TARGET_IA32
    printf("fp_func4_wrapper parameters: %e, %f, %d, %e\n", d1, f2, i3, d4);
    fflush(stdout);

    /*
     * parameter of type "double" is on stack and located in 8 bytes
    */
    DOUBLE_STRUCT* d_param1 = (DOUBLE_STRUCT*)&d1;
    DOUBLE_STRUCT* d_param4 = (DOUBLE_STRUCT*)&d4;

    /*
    * parameter of type "float" is on stack and located in 4 bytes like integer
    */
    int f2_int = *(int*)(&f2);
    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, fp_ptr, NULL, PIN_PARG(float), &resf, PIN_PARG(int),
                                d_param1->_d0,                                                          // lower part of d1
                                PIN_PARG(int), d_param1->_d1,                                           // upper part of d1
                                PIN_PARG(int), f2_int, PIN_PARG(int), i3, PIN_PARG(int), d_param4->_d0, // lower part of d4
                                PIN_PARG(int), d_param4->_d1,                                           // upper part of d4
                                PIN_PARG_END());

#else
    FPSTATE fpState;

    /*
    * all fp parameters are in xmm registers
    * take them from the context
    */
    PIN_GetContextFPState(ctxt, &fpState);
    memcpy(&d1, &fpState.fxsave_legacy._xmms[0], sizeof(d1));
    memcpy(&f2, &fpState.fxsave_legacy._xmms[1], sizeof(f2));
    memcpy(&d4, &fpState.fxsave_legacy._xmms[2], sizeof(d4));

    printf("fp_func4_wrapper parameters: %e, %f, %d, %e\n", d1, f2, i3, d4);
    fflush(stdout);

    // all fp args are in xmm regs in context - only pass the int args to the app function
    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, fp_ptr, NULL, PIN_PARG(float), &resf, PIN_PARG(int), i3,
                                PIN_PARG_END());

#endif
    printf("fp_func4_wrapper result: %f\n", resf);
    fflush(stdout);

    return resf;
}

double fp_func1_wrapper(CONTEXT* ctxt, AFUNPTR fp_ptr, double d1)
{
    double resd;

#ifdef TARGET_IA32
    printf("fp_func1_wrapper parameters: %e\n", d1);
    fflush(stdout);

    /*
     * parameter of type "double" is on stack and located in 8 bytes
    */
    DOUBLE_STRUCT* d_param1 = (DOUBLE_STRUCT*)&d1;

    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, fp_ptr, NULL, PIN_PARG(double), &resd, PIN_PARG(int),
                                d_param1->_d0, PIN_PARG(int), d_param1->_d1, PIN_PARG_END());

#else
    FPSTATE fpState;

    /*
    * all fp parameters are in xmm registers
    * take them from the context
    */

    PIN_GetContextFPState(ctxt, &fpState);
    memcpy(&d1, &fpState.fxsave_legacy._xmms[0], sizeof(d1));
    printf("fp_func1_wrapper parameters: %e\n", d1);
    fflush(stdout);
    // all fp args are in xmm regs in context - do not pass any args to the app function
    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, fp_ptr, NULL, PIN_PARG(double), &resd, PIN_PARG_END());

#endif

    printf("fp_func1_wrapper result: %e\n", resd);
    fflush(stdout);

    return resd;
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    /*
    * Instrument function
    *  int func3(double, float, int, double)
    */
    RTN rtn = RTN_FindByName(img, "fp_func3");
    if (RTN_Valid(rtn))
    {
        //printf("Replace fp_func3\n");
#ifdef TARGET_IA32
        PROTO proto_func3 = PROTO_Allocate(PIN_PARG(int), CALLINGSTD_DEFAULT, "fp_func3", PIN_PARG(int), PIN_PARG(int),
                                           PIN_PARG(int), PIN_PARG_END());

        RTN_ReplaceSignature(rtn, AFUNPTR(fp_func3_wrapper), IARG_PROTOTYPE, proto_func3, IARG_CONTEXT, IARG_ORIG_FUNCPTR,
                             IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
                             IARG_END);
#else // TARGET_IA32E
        PROTO proto_func3 =
            PROTO_Allocate(PIN_PARG(int), CALLINGSTD_DEFAULT, "fp_func3", PIN_PARG(int), PIN_PARG(int), PIN_PARG_END());

        RTN_ReplaceSignature(rtn, AFUNPTR(fp_func3_wrapper), IARG_PROTOTYPE, proto_func3, IARG_CONTEXT, IARG_ORIG_FUNCPTR,
                             IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_END);

#endif

        PROTO_Free(proto_func3);
    }

    rtn = RTN_FindByName(img, "fp_func4");
    if (RTN_Valid(rtn))
    {
        //printf("Replace fp_func4\n");
        //
        // float fp_func4(double d1, float f2, int i3, double d4)
        //
#ifdef TARGET_IA32
        PROTO proto_func4 = PROTO_Allocate(PIN_PARG(float), CALLINGSTD_DEFAULT, "fp_func4",
                                           PIN_PARG(int), // double p1
                                           PIN_PARG(int), // double p2
                                           PIN_PARG(int), // float
                                           PIN_PARG(int), // int
                                           PIN_PARG(int), // double p1
                                           PIN_PARG(int), // double p2

                                           PIN_PARG_END());

        RTN_ReplaceSignature(rtn, AFUNPTR(fp_func4_wrapper), IARG_PROTOTYPE, proto_func4, IARG_CONTEXT, IARG_ORIG_FUNCPTR,
                             IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
                             IARG_FUNCARG_ENTRYPOINT_VALUE, 3, IARG_FUNCARG_ENTRYPOINT_VALUE, 4, IARG_FUNCARG_ENTRYPOINT_VALUE, 5,
                             IARG_END);
#else
        // only i3 is in register, other parameters in xmm
        PROTO proto_func4 = PROTO_Allocate(PIN_PARG(float), CALLINGSTD_DEFAULT, "fp_func4", PIN_PARG(int), PIN_PARG_END());

        RTN_ReplaceSignature(rtn, AFUNPTR(fp_func4_wrapper), IARG_PROTOTYPE, proto_func4, IARG_CONTEXT, IARG_ORIG_FUNCPTR,
                             IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);

#endif

        PROTO_Free(proto_func4);
    }

    rtn = RTN_FindByName(img, "fp_func1");
    if (RTN_Valid(rtn))
    {
        //printf("Replace fp_func1\n");
        //
        // double fp_func1(double d1)
        //
#ifdef TARGET_IA32

        // d1 on stack, 8 bytes
        // passed as 2 integers
        PROTO proto_func1 = PROTO_Allocate(PIN_PARG(double), CALLINGSTD_DEFAULT, "fp_func1",
                                           PIN_PARG(int), // d1 lower
                                           PIN_PARG(int), // d1 upper
                                           PIN_PARG_END());

        RTN_ReplaceSignature(rtn, AFUNPTR(fp_func1_wrapper), IARG_PROTOTYPE, proto_func1, IARG_CONTEXT, IARG_ORIG_FUNCPTR,
                             IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_END);
#else
        // all parameters in xmm, will be taken from context; nothing to be passed
        PROTO proto_func1 = PROTO_Allocate(PIN_PARG(double), CALLINGSTD_DEFAULT, "fp_func1", PIN_PARG_END());

        RTN_ReplaceSignature(rtn, AFUNPTR(fp_func1_wrapper), IARG_PROTOTYPE, proto_func1, IARG_CONTEXT, IARG_ORIG_FUNCPTR,
                             IARG_END);

#endif

        PROTO_Free(proto_func1);
    }
}

/* ===================================================================== */
int main(INT32 argc, CHAR* argv[])
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
