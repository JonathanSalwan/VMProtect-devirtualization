/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool excersizes the fetch_rtn_ins code.
 */
#include <fstream>
#include <iostream>
#include <iomanip>

#include <string.h>
#include "pin.H"
using std::endl;
using std::hex;
using std::ofstream;
using std::string;

#ifdef TARGET_MAC
#define NAME(fun) "_" fun
#else
#define NAME(fun) fun
#endif

KNOB< string > KnobOutput(KNOB_MODE_WRITEONCE, "pintool", "o", "ins_fetch_overrun.out", "Name for log file");

static ofstream out;

VOID AtRtn(VOID* addr) { out << hex << "Executing the function in address 0x" << reinterpret_cast< ADDRINT >(addr) << endl; }

VOID Image(IMG img, VOID* v)
{
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            if (RTN_Name(rtn) != NAME("foo") && RTN_Name(rtn) != NAME("bar"))
            {
                continue;
            }
            BOOL canBeProbed = RTN_IsSafeForProbedInsertion(rtn);
            out << RTN_Name(rtn) << ": can be probed? " << canBeProbed << endl;
            if (canBeProbed)
            {
                RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(AtRtn), IARG_PTR, RTN_Address(rtn), IARG_END);
            }
        }
    }
}

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    out.open(KnobOutput.Value().c_str());

    IMG_AddInstrumentFunction(Image, 0);

    // Never returns
    PIN_StartProgramProbed();

    return 0;
}
