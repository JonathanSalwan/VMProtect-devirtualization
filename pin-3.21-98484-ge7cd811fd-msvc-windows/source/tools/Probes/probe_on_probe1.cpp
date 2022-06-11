/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
*/

#include "pin.H"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
using std::cerr;
using std::endl;
using std::hex;
using std::ios;
using std::ofstream;
using std::string;

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

ofstream TraceFile;
typedef VOID (*VOIDFUNC)();

static VOIDFUNC origFptrNotify1;
static VOIDFUNC origFptrNotify2;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "probe_on_probe1.outfile", "specify file name");

/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool tests probe replacement.\n"
            "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

void Notify1()
{
    if (origFptrNotify1)
    {
        TraceFile << "Notify 1 before" << endl;
        (*origFptrNotify1)();
        TraceFile << "Notify 1 after." << endl;
    }
}

void Notify2()
{
    if (origFptrNotify2)
    {
        TraceFile << "Notify 2 before" << endl;
        (*origFptrNotify2)();
        TraceFile << "Notify 2 after." << endl;
    }
}

void Notify1Sig(VOIDFUNC origFunc)
{
    TraceFile << "NotifySig 1 before" << endl;
    (*origFunc)();
    TraceFile << "NotifySig 1 after." << endl;
}

void Notify2Sig(VOIDFUNC origFunc)
{
    TraceFile << "NotifySig 2 before" << endl;
    (*origFunc)();
    TraceFile << "NotifySig 2 after." << endl;
}

BOOL FindAndCheckRtn(IMG img, string rtnName, RTN& rtn)
{
    rtn = RTN_FindByName(img, rtnName.c_str());
    if (!RTN_Valid(rtn)) rtn = RTN_FindByName(img, (string("_") + rtnName).c_str());

    if (RTN_Valid(rtn))
    {
        if (!RTN_IsSafeForProbedReplacement(rtn))
        {
            TraceFile << "Cannot replace " << RTN_Name(rtn) << " in " << IMG_Name(img) << endl;
            exit(1);
        }
        return TRUE;
    }
    return FALSE;
}

BOOL ReplaceProbed(IMG img, string rtnName)
{
    RTN rtn;
    if (!FindAndCheckRtn(img, rtnName, rtn)) return FALSE;

    origFptrNotify1 = (void (*)())RTN_ReplaceProbed(rtn, AFUNPTR(Notify1));
    origFptrNotify2 = (void (*)())RTN_ReplaceProbed(rtn, AFUNPTR(Notify2));

    TraceFile << "Inserted probe for " << rtnName << endl;
    return TRUE;
}

BOOL ReplaceSignatureProbed(IMG img, string rtnName)
{
    RTN rtn;
    if (!FindAndCheckRtn(img, rtnName, rtn)) return FALSE;

    PROTO proto1 = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "Notify1Sig", PIN_PARG(AFUNPTR), PIN_PARG_END());

    RTN_ReplaceSignatureProbed(rtn, AFUNPTR(Notify1Sig), IARG_PROTOTYPE, proto1, IARG_ORIG_FUNCPTR, IARG_END);

    PROTO proto2 = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "Notify2Sig", PIN_PARG(AFUNPTR), PIN_PARG_END());

    RTN_ReplaceSignatureProbed(rtn, AFUNPTR(Notify2Sig), IARG_PROTOTYPE, proto2, IARG_ORIG_FUNCPTR, IARG_END);

    TraceFile << "Inserted probe for " << rtnName << endl;
    return TRUE;
}

/* ===================================================================== */
// Called every time a new image is loaded
// Look for routines that we want to probe
VOID ImageLoad(IMG img, VOID* v)
{
    static BOOL replaceProbedDone = FALSE;
    if (!replaceProbedDone)
    {
        replaceProbedDone = ReplaceProbed(img, "do_nothing");
    }

    static BOOL replaceSigProbedDone = FALSE;
    if (!replaceSigProbedDone)
    {
        replaceSigProbedDone = ReplaceSignatureProbed(img, "nothing_doing");
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

    TraceFile.open(KnobOutputFile.Value().c_str());
    TraceFile << hex;
    TraceFile.setf(ios::showbase);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_StartProgramProbed();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
