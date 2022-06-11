/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*!
 * @file
 * This pin tool domonstrate the difference between PIN_CallApplicationFunction() and
 * PIN_CallApplicationFunctionNatively().
 * It was designed to instrument the application call_function_natively_app and
 * instrument the call from helloX() to world().
 * Right before the main() function returns, this tool gain control and call
 * the helloX() function using both PIN_CallApplicationFunction() and
 * PIN_CallApplicationFunctionNatively().
 * We expect that when calling helloX() using PIN_CallApplicationFunctionNatively()
 * the instrumentation routines and the analysis routines that were instrumented
 * ahead of time will not be call on the executed application code.
 * But, when calling helloX() using PIN_CallApplicationFunction()
 * The routines mentioned above should be called.
*/
#include "pin.H"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
using std::cerr;
using std::endl;
using std::ofstream;
using std::string;

#ifdef TARGET_MAC
#define NAME(x) "_" x
#else
#define NAME(x) x
#endif

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "call_function_natively.out", "specify file name");

// Tool's output file
ofstream TraceFile;
/* ===================================================================== */

// address of helloX() function in the application
static ADDRINT helloXAddr = 0;
// address of helloX() function's end in the application
static ADDRINT helloXEndAddr = 0;
// address of world() function in the application
static ADDRINT worldAddr = 0;

// depth of the call to PIN_CallApplicationFunction*()
static int callingDepth = 0;

static INT32 Usage()
{
    cerr << "This pin tool domonstrate the difference between PIN_CallApplicationFunction() and "
            "PIN_CallApplicationFunctionNatively."
         << endl;
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

// Return the identation for text printing
// The identation is calculated according to the call depth
static string depthString()
{
    string s;
    for (int i = 0; i < callingDepth; i++)
        s += "\t";
    return s;
}

// A call to this function is instrumented in the app right before the call from helloX() to world()
static void HandleCallInHello(char* instType)
{
    TraceFile << depthString() << "Calling world from helloX (instrumented " << instType << ")" << endl;
}

static void callApplicationFunction(THREADID tid, CONTEXT* ctxt, BOOL natively)
{
    char buf[128] = {0};
    char* ret;

    TraceFile << depthString() << "Calling helloX from tool " << (natively ? "natively" : "regularly") << endl;
    callingDepth++;
    CALL_APPLICATION_FUNCTION_PARAM param;
    memset(&param, 0, sizeof(param));
    param.native = natively;

    PIN_CallApplicationFunction(ctxt, tid, CALLINGSTD_DEFAULT, AFUNPTR(helloXAddr), &param, PIN_PARG(char*), &ret,
                                PIN_PARG(char*), buf, PIN_PARG_END());

    callingDepth--;
    TraceFile << depthString() << "Returned from helloX in tool with '" << ret << "'" << endl;
}

// This function should be called at the end of the application's main()
static void AfterMain(THREADID tid, CONTEXT* ctxt)
{
    callApplicationFunction(tid, ctxt, TRUE);
    callApplicationFunction(tid, ctxt, FALSE);
    callApplicationFunction(tid, ctxt, TRUE);
}

// This is the instrumentation routine
// It Should locate the single instruction in the application which is
// the call from helloX() to world(), and instrument it
static void Instruction(INS ins, VOID* v)
{
    ADDRINT addr = INS_Address(ins);
    if (addr < helloXAddr && addr >= helloXEndAddr)
    {
        // We're not instrumenting the body of helloX()
        return;
    }

    // Find the instruction which is the call to world()
    if (INS_IsCall(ins) && INS_IsDirectControlFlow(ins) && INS_DirectControlFlowTargetAddress(ins) == worldAddr)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(HandleCallInHello), IARG_ADDRINT, v, IARG_END);
    }
}

// Utility function to get the address of an RTN using its name
static ADDRINT FindAddressOfRtn(IMG img, const string& rtnName)
{
    RTN r = RTN_FindByName(img, rtnName.c_str());
    ASSERT(RTN_Valid(r), "Failed to find RTN " + rtnName);
    return RTN_Address(r);
}

// Called opon new image load
static void Image(IMG img, VOID* v)
{
    if (!IMG_IsMainExecutable(img))
    {
        return;
    }

    // Find the relevant function addresses
    helloXAddr = FindAddressOfRtn(img, NAME("helloX"));
    worldAddr  = FindAddressOfRtn(img, NAME("world"));

    RTN r = RTN_FindByAddress(helloXAddr);
    ASSERTX(RTN_Valid(r));
    RTN_Open(r);
    helloXEndAddr = helloXAddr + RTN_Size(r);

    // Instrument helloX() ahead of time
    for (INS i = RTN_InsHead(r); INS_Valid(i); i = INS_Next(i))
    {
        Instruction(i, (VOID*)"AOTI");
    }
    RTN_Close(r);

    // Instrument our function to the end of the application's main()
    RTN mainRtn = RTN_FindByName(img, NAME("main"));
    ASSERTX(RTN_Valid(mainRtn));

    RTN_Open(mainRtn);
    RTN_InsertCall(mainRtn, IPOINT_AFTER, AFUNPTR(AfterMain), IARG_THREAD_ID, IARG_CONTEXT, IARG_END);
    RTN_Close(mainRtn);
}

int main(int argc, CHAR* argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }
    PIN_InitSymbols();

    TraceFile.open(KnobOutputFile.Value().c_str());

    IMG_AddInstrumentFunction(Image, 0);

    INS_AddInstrumentFunction(Instruction, (VOID*)"JIT");

    // Never return
    PIN_StartProgram();
}
