/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * This test checks callback insertion with IARG_FUNCARG_ENTRYPOINT_VALUE argument.
 * The size of argument passed to original function is specified in prototype.
 * Pin should remove garbage from stack is original parameter is smaller than ADDRINT.
 */

#include <iostream>
#include <fstream>
#include "pin.H"
using std::dec;
using std::hex;
using std::ofstream;
using std::string;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "funcarg_ep.out", "specify file name");
ofstream TraceFile;

static VOID OnImage(IMG, VOID*);
static VOID Check(ADDRINT, ADDRINT, ADDRINT, ADDRINT, ADDRINT, ADDRINT, ADDRINT, ADDRINT, ADDRINT, ADDRINT, ADDRINT, ADDRINT,
                  ADDRINT);
static VOID CheckUnsigned(ADDRINT, ADDRINT, ADDRINT, ADDRINT, ADDRINT, ADDRINT, ADDRINT, ADDRINT, ADDRINT, ADDRINT, ADDRINT,
                          ADDRINT, ADDRINT);
static VOID FooReplacement(CONTEXT* ctxt, AFUNPTR orgFoo1, long, long, long, long, long, long, long, long, long, long, long, long,
                           long);

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    TraceFile.open(KnobOutputFile.Value().c_str());

    IMG_AddInstrumentFunction(OnImage, 0);

    PIN_StartProgram();
    return 0;
}

static VOID OnImage(IMG img, VOID*)
{
    PROTO protoUnsigned =
        PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "Foo2", PIN_PARG(unsigned int), PIN_PARG(unsigned short),
                       PIN_PARG(unsigned char), PIN_PARG(unsigned int), PIN_PARG(unsigned int), PIN_PARG(unsigned int),
                       PIN_PARG(unsigned int), PIN_PARG(unsigned int), PIN_PARG(unsigned int), PIN_PARG(unsigned int),
                       PIN_PARG(unsigned short), PIN_PARG(unsigned int), PIN_PARG(unsigned char), PIN_PARG_END());
    PROTO protoSigned = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "Foo", PIN_PARG(int), PIN_PARG(short), PIN_PARG(char),
                                       PIN_PARG(int), PIN_PARG(int), PIN_PARG(int), PIN_PARG(int), PIN_PARG(int), PIN_PARG(int),
                                       PIN_PARG(int), PIN_PARG(short), PIN_PARG(int), PIN_PARG(char), PIN_PARG_END());
    RTN rtn           = RTN_FindByName(img, "Foo");
    if (RTN_Valid(rtn))
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(Check), IARG_PROTOTYPE, protoSigned, IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_FUNCARG_ENTRYPOINT_VALUE, 2, IARG_FUNCARG_ENTRYPOINT_VALUE, 3,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 4, IARG_FUNCARG_ENTRYPOINT_VALUE, 5, IARG_FUNCARG_ENTRYPOINT_VALUE, 6,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 7, IARG_FUNCARG_ENTRYPOINT_VALUE, 8, IARG_FUNCARG_ENTRYPOINT_VALUE, 9,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 10, IARG_FUNCARG_ENTRYPOINT_VALUE, 11, IARG_FUNCARG_ENTRYPOINT_VALUE, 12,
                       IARG_END);
        RTN_Close(rtn);
    }
    rtn = RTN_FindByName(img, "Foo1");
    if (RTN_Valid(rtn))
    {
        RTN_ReplaceSignature(rtn, AFUNPTR(FooReplacement), IARG_PROTOTYPE, protoSigned, IARG_CONTEXT, IARG_ORIG_FUNCPTR,
                             IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
                             IARG_FUNCARG_ENTRYPOINT_VALUE, 3, IARG_FUNCARG_ENTRYPOINT_VALUE, 4, IARG_FUNCARG_ENTRYPOINT_VALUE, 5,
                             IARG_FUNCARG_ENTRYPOINT_VALUE, 6, IARG_FUNCARG_ENTRYPOINT_VALUE, 7, IARG_FUNCARG_ENTRYPOINT_VALUE, 8,
                             IARG_FUNCARG_ENTRYPOINT_VALUE, 9, IARG_FUNCARG_ENTRYPOINT_VALUE, 10, IARG_FUNCARG_ENTRYPOINT_VALUE,
                             11, IARG_FUNCARG_ENTRYPOINT_VALUE, 12, IARG_END);
    }
    rtn = RTN_FindByName(img, "Foo2");
    if (RTN_Valid(rtn))
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(CheckUnsigned), IARG_PROTOTYPE, protoUnsigned, IARG_FUNCARG_ENTRYPOINT_VALUE,
                       0, IARG_FUNCARG_ENTRYPOINT_VALUE, 1, IARG_FUNCARG_ENTRYPOINT_VALUE, 2, IARG_FUNCARG_ENTRYPOINT_VALUE, 3,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 4, IARG_FUNCARG_ENTRYPOINT_VALUE, 5, IARG_FUNCARG_ENTRYPOINT_VALUE, 6,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 7, IARG_FUNCARG_ENTRYPOINT_VALUE, 8, IARG_FUNCARG_ENTRYPOINT_VALUE, 9,
                       IARG_FUNCARG_ENTRYPOINT_VALUE, 10, IARG_FUNCARG_ENTRYPOINT_VALUE, 11, IARG_FUNCARG_ENTRYPOINT_VALUE, 12,
                       IARG_END);
        RTN_Close(rtn);
    }

    PROTO_Free(protoSigned);
    PROTO_Free(protoUnsigned);
}

static VOID Check(ADDRINT x1, ADDRINT x2, ADDRINT x3, ADDRINT x4, ADDRINT x5, ADDRINT x6, ADDRINT x7, ADDRINT x8, ADDRINT x9,
                  ADDRINT x10, ADDRINT x11, ADDRINT x12, ADDRINT x13)
{
    TraceFile << "In callback (signed): " << std::endl;
    TraceFile << "Tool x1 " << hex << " 0x" << x1 << std::endl;
    TraceFile << "Tool x2 " << hex << " 0x" << x2 << std::endl;
    TraceFile << "Tool x3 " << hex << " 0x" << x3 << std::endl;
    TraceFile << "Tool x4 " << hex << " 0x" << x4 << std::endl;
    TraceFile << "Tool x5 " << hex << " 0x" << x5 << std::endl;
    TraceFile << "Tool x6 " << hex << " 0x" << x6 << std::endl;
    TraceFile << "Tool x7 " << hex << " 0x" << x7 << std::endl;
    TraceFile << "Tool x8 " << hex << " 0x" << x8 << std::endl;
    TraceFile << "Tool x9 " << hex << " 0x" << x9 << std::endl;
    TraceFile << "Tool x10 " << hex << " 0x" << x10 << std::endl;
    TraceFile << "Tool x11 " << hex << " 0x" << x11 << std::endl;
    TraceFile << "Tool x12 " << hex << " 0x" << x12 << std::endl;
    TraceFile << "Tool x13 " << hex << " 0x" << x13 << std::endl;
}

static VOID CheckUnsigned(ADDRINT x1, ADDRINT x2, ADDRINT x3, ADDRINT x4, ADDRINT x5, ADDRINT x6, ADDRINT x7, ADDRINT x8,
                          ADDRINT x9, ADDRINT x10, ADDRINT x11, ADDRINT x12, ADDRINT x13)
{
    TraceFile << "In callback (unsigned): " << std::endl;
    TraceFile << "Tool x1 " << hex << " 0x" << x1 << std::endl;
    TraceFile << "Tool x2 " << hex << " 0x" << x2 << std::endl;
    TraceFile << "Tool x3 " << hex << " 0x" << x3 << std::endl;
    TraceFile << "Tool x4 " << hex << " 0x" << x4 << std::endl;
    TraceFile << "Tool x5 " << hex << " 0x" << x5 << std::endl;
    TraceFile << "Tool x6 " << hex << " 0x" << x6 << std::endl;
    TraceFile << "Tool x7 " << hex << " 0x" << x7 << std::endl;
    TraceFile << "Tool x8 " << hex << " 0x" << x8 << std::endl;
    TraceFile << "Tool x9 " << hex << " 0x" << x9 << std::endl;
    TraceFile << "Tool x10 " << hex << " 0x" << x10 << std::endl;
    TraceFile << "Tool x11 " << hex << " 0x" << x11 << std::endl;
    TraceFile << "Tool x12 " << hex << " 0x" << x12 << std::endl;
    TraceFile << "Tool x13 " << hex << " 0x" << x13 << std::endl;
}

static VOID FooReplacement(CONTEXT* ctxt, AFUNPTR orgFoo1, long x1, long x2, long x3, long x4, long x5, long x6, long x7, long x8,
                           long x9, long x10, long x11, long x12, long x13)
{
    TraceFile << hex << "In replacement (signed): " << std::endl;
    TraceFile << "Tool x1 " << dec << x1 << std::endl;
    TraceFile << "Tool x2 " << dec << x2 << std::endl;
    TraceFile << "Tool x3 " << dec << x3 << std::endl;
    TraceFile << "Tool x4 " << dec << x4 << std::endl;
    TraceFile << "Tool x5 " << dec << x5 << std::endl;
    TraceFile << "Tool x6 " << dec << x6 << std::endl;
    TraceFile << "Tool x7 " << dec << x7 << std::endl;
    TraceFile << "Tool x8 " << dec << x8 << std::endl;
    TraceFile << "Tool x9 " << dec << x9 << std::endl;
    TraceFile << "Tool x10 " << dec << x10 << std::endl;
    TraceFile << "Tool x11 " << dec << x11 << std::endl;
    TraceFile << "Tool x12 " << dec << x12 << std::endl;
    TraceFile << "Tool x13 " << dec << x13 << std::endl;

    ADDRINT res;

    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, orgFoo1, NULL, PIN_PARG(long), &res, PIN_PARG(int), x1,
                                PIN_PARG(short), x2, PIN_PARG(char), x3, PIN_PARG(int), x4, PIN_PARG(int), x5, PIN_PARG(int), x6,
                                PIN_PARG(int), x7, PIN_PARG(int), x8, PIN_PARG(int), x9, PIN_PARG(int), x10, PIN_PARG(short), x11,
                                PIN_PARG(int), x12, PIN_PARG(char), x13, PIN_PARG_END());
}
