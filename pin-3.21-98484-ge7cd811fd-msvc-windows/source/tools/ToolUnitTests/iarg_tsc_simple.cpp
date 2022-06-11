/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This tool test RTN_InsHead RTN_InsHeadOnly and RTN_InsertCall (before)

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include "pin.H"
using std::endl;
using std::hex;
using std::ofstream;

ofstream OutFile;

/*
 * Sample timestamp
 */
static UINT64 Rdtsc();

/*
 * return true if:
 * (1) tscLow was sampled earlier than tscHigh
 * (2) (tscHigh - tscLow) < TSC_MAX_DELTA cycles.
 */
static bool IsTscValid(UINT64 tscLow, UINT64 tscHigh);

VOID BeforeRtn(VOID* name, UINT64 argTsc)
{
    UINT64 expectedTsc = Rdtsc();

    std::stringstream stream;
    stream << "Method: [" << reinterpret_cast< CHAR* >(name) << "]";
    stream << " *** argTsc: [" << argTsc << "]";
    stream << " *** expectedTsc: [" << expectedTsc << "]";

    if (!IsTscValid(argTsc, expectedTsc))
    {
        stream << "   ***   Invalid timestamp" << endl;
        OutFile << stream.str();
        ASSERTQ(stream.str());
    }

    OutFile << stream.str() << endl;
}

VOID PIN_FAST_ANALYSIS_CALL BeforeRtnFastcall(VOID* name, UINT64 argTsc)
{
    UINT64 expectedTsc = Rdtsc();

    std::stringstream stream;
    stream << "Method: [" << reinterpret_cast< CHAR* >(name) << "]";
    stream << " *** argTsc: [" << argTsc << "]";
    stream << " *** expectedTsc: [" << expectedTsc << "]";

    if (!IsTscValid(argTsc, expectedTsc))
    {
        stream << "   ***   Invalid timestamp" << endl;
        OutFile << stream.str();
        ASSERTQ(stream.str());
    }

    OutFile << stream.str() << endl;
}

VOID Image(IMG img, void* v)
{
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            RTN_Open(rtn);

            RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(BeforeRtn), IARG_PTR, RTN_Name(rtn).c_str(), IARG_TSC, IARG_END);
            RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(BeforeRtnFastcall), IARG_FAST_ANALYSIS_CALL, IARG_PTR,
                           RTN_Name(rtn).c_str(), IARG_TSC, IARG_END);

            RTN_Close(rtn);
        }
    }
}

VOID Fini(INT32 code, VOID* v) { OutFile.close(); }

#ifdef TARGET_WINDOWS
extern "C" unsigned __int64 __rdtsc();
#pragma intrinsic(__rdtsc)
UINT64 RdtscWin() { return __rdtsc(); }
#endif

UINT64 Rdtsc()
{
#ifdef TARGET_WINDOWS

    return RdtscWin();

#else // if Linux

    UINT32 hi, lo;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return (((UINT64)lo) + (((UINT64)hi) << 32));

#endif
}

/*
 * return true if:
 * (1) tscLow was sampled earlier than tscHigh
 * (2) (tscHigh - tscLow) < TSC_MAX_DELTA cycles.
 */
static bool IsTscValid(UINT64 tscLow, UINT64 tscHigh)
{
    UINT64 TSC_MAX_DELTA = (UINT64(1)) << 31; // Max delta between two rdtsc instructions
    return ((tscHigh - tscLow) < TSC_MAX_DELTA) && (tscHigh > (UINT64(1) << UINT64(32)));
}

int main(int argc, char** argv)
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    OutFile.open("iarg_tsc_simple.out");
    OutFile << std::unitbuf;

    IMG_AddInstrumentFunction(Image, 0);
    PIN_AddFiniFunction(Fini, 0);

    PIN_StartProgram();
    return 0;
}
