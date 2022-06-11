/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <string.h>
#include "tool_macros.h"
using std::cerr;
using std::cout;
using std::endl;

#ifdef TARGET_MAC
#define LIBC_LIB "libSystem.B.dylib"
#define LIBC_INIT C_MANGLE("libSystem_initializer")
#define PTHREAD_LIB "libsystem_pthread.dylib"
#define PTHREAD_INIT C_MANGLE("__pthread_init")
#else
#define LIBC_LIB "libc.so"
#define LIBC_INIT C_MANGLE("_init")
#define LIBC_INIT_FIRST C_MANGLE("_init_first")
#define PTHREAD_LIB "libpthread.so"
#define PTHREAD_INIT C_MANGLE("_init")
#endif

INT32 Usage()
{
    cerr << "This pin tool tests probe replacement.\n"
            "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

typedef VOID (*INITFUNC)(INT32 argc, CHAR** argv, CHAR** env);

// Save original init functions in global vars
INITFUNC origLibcInit;
INITFUNC origPthreadInit;

// libcInitialized is TRUE when libc from user application completed initialization
BOOL libcInitialized = FALSE;
// libpthreadInitialized is TRUE when libc from user application completed initialization
BOOL libpthreadInitialized = FALSE;

// Wrapper for initialization function of libc
// all standard init functions receive 3 parameters - argc, argv and env
extern "C" VOID AlternativeLibcInit(INT32 argc, CHAR** argv, CHAR** env)
{
    // call to original init function
    (*origLibcInit)(argc, argv, env);
    // switch-on libcInitialized
    libcInitialized = TRUE;
    cout << "libc initialized" << endl;
}

// Wrapper for initialization function of libpthread
extern "C" VOID AlternativeLibpthreadInit(INT32 argc, CHAR** argv, CHAR** env)
{
    // call to original init function
    (*origPthreadInit)(argc, argv, env);
    // switch-on libpthreadInitialized
    libpthreadInitialized = TRUE;
    cout << "libpthread initialized" << endl;
}

extern "C" VOID OtherImageInit(INT32 argc, CHAR** argv, CHAR** env, INITFUNC origInit, UINT32 imgId)
{
    (*origInit)(argc, argv, env);
    IMG img = IMG_FindImgById(imgId);
    cout << "Initialized " << IMG_Name(img) << endl;
}

/* ===================================================================== */

// Called every time a new image is loaded
// Add notification before library initialization

VOID ImageLoad(IMG img, VOID* v)
{
    const char* name = IMG_Name(img).c_str();
    if (strstr(name, LIBC_LIB))
    {
        RTN initRtn = RTN_FindByName(img, LIBC_INIT);
#ifdef TARGET_LINUX
        if (!RTN_Valid(initRtn)) initRtn = RTN_FindByName(img, LIBC_INIT_FIRST);
#endif
        if (RTN_Valid(initRtn))
        {
            PROTO protoLibcInit = PROTO_Allocate(PIN_PARG(VOID), CALLINGSTD_DEFAULT, "AlternativeLibcInit", PIN_PARG(INT32),
                                                 PIN_PARG(CHAR**), PIN_PARG(CHAR**), PIN_PARG_END());

            origLibcInit = (INITFUNC)RTN_ReplaceSignatureProbed(
                initRtn, AFUNPTR(AlternativeLibcInit), IARG_PROTOTYPE, protoLibcInit, IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_FUNCARG_ENTRYPOINT_VALUE, 2, IARG_END);
        }
        else
        {
            // the image doesn't have .init section
            cout << "initialization code of libc is not found" << endl;
        }
    }
    else if (strstr(name, PTHREAD_LIB))
    {
        RTN initRtn = RTN_FindByName(img, PTHREAD_INIT);
        if (RTN_Valid(initRtn))
        {
            PROTO protoPthreadInit = PROTO_Allocate(PIN_PARG(VOID), CALLINGSTD_DEFAULT, "AlternativeLibpthreadInit",
                                                    PIN_PARG(INT32), PIN_PARG(CHAR**), PIN_PARG(CHAR**), PIN_PARG_END());

            origPthreadInit = (INITFUNC)RTN_ReplaceSignatureProbed(
                initRtn, AFUNPTR(AlternativeLibpthreadInit), IARG_PROTOTYPE, protoPthreadInit, IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_FUNCARG_ENTRYPOINT_VALUE, 2, IARG_END);
        }
        else
        {
            cout << "initialization code of libpthread is not found" << endl;
        }
    }
    else
    {
        RTN initRtn = RTN_FindByName(img, "_init");
        if (RTN_Valid(initRtn))
        {
            PROTO protoInit =
                PROTO_Allocate(PIN_PARG(VOID), CALLINGSTD_DEFAULT, "OtherImageInit", PIN_PARG(INT32), PIN_PARG(CHAR**),
                               PIN_PARG(CHAR**), PIN_PARG(AFUNPTR), PIN_PARG(UINT32), PIN_PARG_END());

            RTN_ReplaceSignatureProbed(initRtn, AFUNPTR(OtherImageInit), IARG_PROTOTYPE, protoInit, IARG_FUNCARG_ENTRYPOINT_VALUE,
                                       0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_FUNCARG_ENTRYPOINT_VALUE, 2, IARG_ORIG_FUNCPTR,
                                       IARG_UINT32, IMG_Id(img), IARG_END);
        }
    }
}

/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_StartProgramProbed();

    return 0;
}
