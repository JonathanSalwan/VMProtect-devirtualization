/*
 * Copyright (C) 2018-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*!@ file
 * Testing TRACE_Address ( ) API
 * Finds tstfunc1,tstfunc2,tstfunc3 by name and makes sure their addresses match a trace address
 *
 * Test app prints addresses of function pointers, of these 3 functions. Comparison is done in make rule.
 *
 * This test tests TRACE_Address only for traces at the beginning of a function.
 * It does not check traces that begin in the middle of a function.
 */

#include <cassert>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <map>
#include <unistd.h>
#include <cstdlib>
#include "pin.H"

using std::cerr;
using std::cout;
using std::hex;
using std::ios;
using std::ofstream;
using std::ostream;
using std::pair;
using std::string;
//=========================================================
//Global Variables:

//This map holds all Routine Addresses of routines that include "tstfunc" in their name,
//and a boolean value stating if it was reached by Trace_Address.
std::map< ADDRINT, bool > rtnAdds;

ostream* TraceFile = NULL;

KNOB< string > KnobOutputFile(
    KNOB_MODE_WRITEONCE, "pintool", "o", "",
    "Specify file name for the tool's output. If no filename is specified, the output will be directed to stdout.");

//=========================================================
//instrumentation functions:

VOID Image(IMG img, VOID* v)
{
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            if (string::npos != RTN_Name(rtn).find("tstfunc"))
            {
                rtnAdds.insert(pair< ADDRINT, bool >(RTN_Address(rtn), false));
            }
        }
    }
}

VOID InstrumentTrace(TRACE trace, VOID* v)
{
    const ADDRINT addr = TRACE_Address(trace);
    assert(BBL_Address(TRACE_BblHead(trace)) == addr);
    if (rtnAdds.find(addr) != rtnAdds.end())
    {
        rtnAdds.find(addr)->second = true;
    }
}

VOID Fini(INT32 code, VOID* v)
{
    for (std::map< ADDRINT, bool >::iterator it = rtnAdds.begin(); it != rtnAdds.end(); it++)
    {
        assert(it->second == true);
        *TraceFile << it->first << "\t";
    }

    if (&cout != TraceFile)
    {
        delete TraceFile;
    }
}

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    TraceFile = KnobOutputFile.Value().empty() ? &cerr : new ofstream(KnobOutputFile.Value().c_str());
    *TraceFile << hex;
    TraceFile->setf(ios::showbase);
    TraceFile->setf(ios::uppercase);

    IMG_AddInstrumentFunction(Image, 0);

    TRACE_AddInstrumentFunction(InstrumentTrace, 0);
    PIN_AddFiniFunction(Fini, 0);

    PIN_StartProgram();

    //Should never reach this point
    return 1;
}
