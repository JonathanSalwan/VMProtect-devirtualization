/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <iostream>
#include <string.h>
#include "pin.H"
using std::cout;
using std::endl;

KNOB< BOOL > RunInProbeMode(KNOB_MODE_WRITEONCE, "pintool", "probe_mode", "0", "Run Pin in probe mode");

VOID Strcmp(const char* str1, const char* str2) { cout << "strcmp called" << endl; }

VOID IfuncStrcmp() { cout << "ifunc strcmp called" << endl; }

VOID Routine(RTN rtn)
{
    if (!RunInProbeMode) RTN_Open(rtn);

    // In some libc implementations, the bcmp and strcmp symbols have the same address.
    // Since Pin only creates one RTN per start address, the RTN name will be eithor bcmp or strcmp.
    bool isStrcmp = strcmp(RTN_Name(rtn).c_str(), "strcmp") == 0;
    bool isSbcmp  = strcmp(RTN_Name(rtn).c_str(), "bcmp") == 0;

    if (isStrcmp || isSbcmp)
    {
        if (SYM_IFuncResolver(RTN_Sym(rtn)))
        {
            cout << "Found IFUNC strcmp" << endl;
            if (RunInProbeMode)
            {
                RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(IfuncStrcmp), IARG_END);
            }
            else
            {
                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(IfuncStrcmp), IARG_END);
            }
        }
        else
        {
            cout << "Found non IFUNC strcmp" << endl;
            if (RunInProbeMode)
            {
                RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(Strcmp), IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                                     IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_END);
            }
            else
            {
                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(Strcmp), IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                               IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_END);
            }
        }
    }
    if (!RunInProbeMode) RTN_Close(rtn);
}

VOID Image(IMG img, void* v)
{
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        if (SEC_IsExecutable(sec))
        {
            for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
                Routine(rtn);
        }
    }
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    PIN_InitSymbolsAlt(SYMBOL_INFO_MODE(UINT32(IFUNC_SYMBOLS) | UINT32(DEBUG_OR_EXPORT_SYMBOLS)));

    IMG_AddInstrumentFunction(Image, 0);

    // Never returns
    if (RunInProbeMode)
    {
        cout << "Testing the Probe mode." << endl;
        PIN_StartProgramProbed();
    }
    else
    {
        cout << "Testing the JIT mode." << endl;
        PIN_StartProgram();
    }

    return 0;
}
