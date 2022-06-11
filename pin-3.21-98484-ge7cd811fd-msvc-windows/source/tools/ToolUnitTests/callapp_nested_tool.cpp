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

static KNOB< string > KnobOutput(KNOB_MODE_WRITEONCE, "pintool", "o", "callapp_nested_tool.out", "output file");
static ofstream out;
int numTimesReplacedOriginal1 = 0;
int numTimesReplacedOriginal2 = 0;
/* ===================================================================== */

/* ===================================================================== */
long Original2Replacement(CONTEXT* ctxt, AFUNPTR origFunc, long param1, long param2)
{
    long res;

    numTimesReplacedOriginal2++;
    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, origFunc, NULL, PIN_PARG(long), &res, PIN_PARG(long),
                                param1, PIN_PARG(long), param2, PIN_PARG_END());

    return res;
}

long Original1Replacement(CONTEXT* ctxt, AFUNPTR origFunc, long param1, long param2)
{
    long res;

    numTimesReplacedOriginal1++;
    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, origFunc, NULL, PIN_PARG(long), &res, PIN_PARG(long),
                                param1, PIN_PARG(long), param2, PIN_PARG_END());

    return res;
}

/* ===================================================================== */
VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
#if !defined(TARGET_MAC)
        const char* orig1Name = "Original1";
        const char* orig2Name = "Original2";
#else
        const char* orig1Name = "_Original1";
        const char* orig2Name = "_Original2";
#endif
        PROTO proto =
            PROTO_Allocate(PIN_PARG(long), CALLINGSTD_DEFAULT, "Original1Proto", PIN_PARG(long), PIN_PARG(long), PIN_PARG_END());

        VOID* pf_Original;
        RTN rtn = RTN_FindByName(img, orig1Name);
        if (RTN_Valid(rtn))
        {
            pf_Original = reinterpret_cast< VOID* >(RTN_Address(rtn));
            out << "Replacing " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;
            RTN_ReplaceSignature(rtn, AFUNPTR(Original1Replacement), IARG_PROTOTYPE, proto, IARG_CONTEXT, IARG_PTR, pf_Original,
                                 IARG_ADDRINT, 1, IARG_ADDRINT, 2, IARG_END);
        }
        else
        {
            out << "Original cannot be found." << endl;
            exit(1);
        }

        PROTO_Free(proto);

        proto =
            PROTO_Allocate(PIN_PARG(long), CALLINGSTD_DEFAULT, "Original2Proto", PIN_PARG(long), PIN_PARG(long), PIN_PARG_END());

        rtn = RTN_FindByName(img, orig2Name);
        if (RTN_Valid(rtn))
        {
            pf_Original = reinterpret_cast< VOID* >(RTN_Address(rtn));
            out << "Replacing " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;
            RTN_ReplaceSignature(rtn, AFUNPTR(Original2Replacement), IARG_PROTOTYPE, proto, IARG_CONTEXT, IARG_PTR, pf_Original,
                                 IARG_ADDRINT, 1, IARG_ADDRINT, 2, IARG_END);
        }
        else
        {
            out << "Original2 cannot be found." << endl;
            exit(1);
        }

        PROTO_Free(proto);
    }
}

VOID Fini(INT32 code, VOID* v)
{
    if (10 != numTimesReplacedOriginal1)
    {
        out << "***ERROR numTimesReplacedOriginal1 " << numTimesReplacedOriginal1 << "is unexpected\n";
        exit(-1);
    }
    if (10 != numTimesReplacedOriginal2)
    {
        out << "***ERROR numTimesPreOriginalReplaced " << numTimesReplacedOriginal2 << " is unexpected\n";
        exit(-1);
    }
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "Nested application call from tool." << endl;
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
