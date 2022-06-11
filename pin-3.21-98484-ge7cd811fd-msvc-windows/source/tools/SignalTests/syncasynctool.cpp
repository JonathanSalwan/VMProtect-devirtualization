/*
 * Copyright (C) 2005-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * See "syncasyncapp.c" for a description of this test.
 */

#include <iostream>
#include <stdlib.h>
#include "pin.H"
using std::cerr;

static void InstrumentImage(IMG, VOID*);
static void AtSegv();
static void AtApplicationEnd(INT32, VOID*);

static int ExecuteCount = 0;

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    PIN_InitSymbols();
    IMG_AddInstrumentFunction(InstrumentImage, 0);
    PIN_AddFiniFunction(AtApplicationEnd, 0);
    PIN_StartProgram();
    return 0;
}

static void InstrumentImage(IMG img, VOID* dummy)
{
#if defined(TARGET_MAC)
    RTN rtn = RTN_FindByName(img, "_MakeSegv");
#else
    RTN rtn = RTN_FindByName(img, "MakeSegv");
#endif
    if (RTN_Valid(rtn))
    {
        RTN_Open(rtn);
        INS first = RTN_InsHead(rtn);
        if (INS_Valid(first)) INS_InsertCall(first, IPOINT_BEFORE, (AFUNPTR)AtSegv, IARG_END);
        RTN_Close(rtn);
    }
}

static void AtSegv()
{
    // Sanity check to make sure the tool really instruments something.
    //
    ExecuteCount++;

    // Just eat up time here.  Our goal is to delay long enough to ensure that the
    // application's VTALRM signal get delivered.
    //
    unsigned long val = 123456789;
    for (unsigned long i = 1; i < 100000000; i++)
        val = val / i + i;

    volatile unsigned long useResult __attribute__((unused)) = val;
}

static void AtApplicationEnd(INT32 code, VOID* dummy)
{
    if (ExecuteCount != 1)
    {
        cerr << "Test did not find MakeSegv() (count = " << ExecuteCount << ")\n";
        exit(1);
    }
}
