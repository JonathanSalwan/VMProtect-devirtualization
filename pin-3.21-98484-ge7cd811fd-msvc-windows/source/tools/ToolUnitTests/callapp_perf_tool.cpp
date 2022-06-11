/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file
  Replace an original function with a custom function defined in the tool. The
  new function can have either the same or different signature from that of its
  original function.
*/

/* ===================================================================== */
#include "pin.H"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
using std::cerr;
using std::endl;
using std::ofstream;
using std::string;

static KNOB< string > KnobOutput(KNOB_MODE_WRITEONCE, "pintool", "o", "countreps.out", "output file");
static ofstream out;
int numTimesPreOriginalReplaced = 0;
int numTimesOriginalReplaced    = 0;
/* ===================================================================== */

/* ===================================================================== */
long PreOriginalReplacement(CONTEXT* ctxt, AFUNPTR origFunc, long one, long two)
{
    long res;

    numTimesPreOriginalReplaced++;
    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, origFunc, NULL, PIN_PARG(long), &res, PIN_PARG(long),
                                one, PIN_PARG(long), two, PIN_PARG_END());

    return res;
}

long OriginalReplacement(CONTEXT* ctxt, AFUNPTR origFunc, long one, long two)
{
    long res;

    numTimesOriginalReplaced++;
    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, origFunc, NULL, PIN_PARG(long), &res, PIN_PARG(long),
                                one, PIN_PARG(long), two, PIN_PARG_END());

    return res;
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
#if !defined(TARGET_MAC)
        const char* origName    = "Original";
        const char* preOrigName = "PreOriginal";
#else
        const char* origName    = "_Original";
        const char* preOrigName = "_PreOriginal";
#endif
        PROTO proto =
            PROTO_Allocate(PIN_PARG(long), CALLINGSTD_DEFAULT, "OriginalProto", PIN_PARG(long), PIN_PARG(long), PIN_PARG_END());

        VOID* pf_Original;
        RTN rtn = RTN_FindByName(img, origName);
        if (RTN_Valid(rtn))
        {
            pf_Original = reinterpret_cast< VOID* >(RTN_Address(rtn));
            out << "Replacing " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;
            RTN_ReplaceSignature(rtn, AFUNPTR(OriginalReplacement), IARG_PROTOTYPE, proto, IARG_CONTEXT, IARG_PTR, pf_Original,
                                 IARG_ADDRINT, 1, IARG_ADDRINT, 2, IARG_END);
        }
        else
        {
            out << "Original cannot be found." << endl;
            exit(1);
        }

        PROTO_Free(proto);

        proto = PROTO_Allocate(PIN_PARG(long), CALLINGSTD_DEFAULT, "PreOriginalProto", PIN_PARG(long), PIN_PARG(long),
                               PIN_PARG_END());

        rtn = RTN_FindByName(img, preOrigName);
        if (RTN_Valid(rtn))
        {
            pf_Original = reinterpret_cast< VOID* >(RTN_Address(rtn));
            out << "Replacing " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;
            RTN_ReplaceSignature(rtn, AFUNPTR(PreOriginalReplacement), IARG_PROTOTYPE, proto, IARG_CONTEXT, IARG_PTR, pf_Original,
                                 IARG_ADDRINT, 1, IARG_ADDRINT, 2, IARG_END);
        }
        else
        {
            out << "PreOriginal cannot be found." << endl;
            exit(1);
        }

        PROTO_Free(proto);
    }
}

VOID Fini(INT32 code, VOID* v)
{
    if (500000 != numTimesOriginalReplaced)
    {
        out << "***ERROR numTimesOriginalReplaced " << numTimesOriginalReplaced << " is unexpected\n";
        exit(-1);
    }
    if (1 != numTimesPreOriginalReplaced)
    {
        out << "***ERROR numTimesPreOriginalReplaced " << numTimesPreOriginalReplaced << " is unexpected\n";
        exit(-1);
    }
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "Test call app function performance." << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
int main(INT32 argc, CHAR* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv)) return Usage();

    out.open(KnobOutput.Value().c_str());

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_AddFiniFunction(Fini, 0);

    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
