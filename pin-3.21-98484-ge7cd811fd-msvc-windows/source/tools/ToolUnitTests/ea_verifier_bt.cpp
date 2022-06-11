/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Verify the memory addressing for BT, BTC, BTR, BTS instructions.
 * This tool expects the specific test code in btc_app.cpp, since it
 * assumes that all of the relevant instructions access a single array.
 */
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <string.h>

#include "pin.H"
using std::endl;
using std::ofstream;
using std::string;

KNOB< string > KnobOutput(KNOB_MODE_WRITEONCE, "pintool", "o", "ea_verifier_bt.out", "Name for log file");

static ofstream out;
static ADDRINT baseAddr = 0;
static int errors       = 0;
static int tests        = 0;

static void printHex(ADDRINT p)
{
    UINT32 fieldWidth = (sizeof(ADDRINT) == 4) ? 8 : 12;

    out << std::setw(fieldWidth) << std::hex << p << std::dec;
}

/************************************************************************/
/* We simply allocate space for the dis-assembled instruction strings and 
 * let them leak.
 */
static char const* formatInstruction(INS ins)
{
    ADDRINT ip       = INS_Address(ins);
    string formatted = hexstr(ip) + " " + INS_Disassemble(ins);
    char* res        = new char[formatted.length() + 1];

    strcpy(res, formatted.c_str());

    return res;
}

static int byteOffset(UINT32 size, INT32 bitIndex)
{
    INT32 bitBits = (size == 2) ? 4 : (size == 4) ? 5 : 6;

    return (bitIndex >> bitBits) * size;
}

static VOID ProcessAddress(ADDRINT name, UINT32 size, ADDRINT ea, INT32 bitIndex)
{
    // Work out the base address of the array being accessed
    if (baseAddr == 0)
    {
        baseAddr = ea - byteOffset(size, bitIndex);
        return;
    }

    tests++;
    // Check that the position agrees with what we think it should be.
    if (baseAddr + byteOffset(size, bitIndex) != ea)
    {
        errors++;
        out << (const char*)name << ": Bad EA ";
        printHex(ea);
        out << " Expected ";
        printHex(baseAddr + byteOffset(size, bitIndex));
        out << " Bit offset " << bitIndex << " Access size " << size << endl;
    }
}

static VOID Instrument(INS ins)
{
    UINT32 op = INS_Opcode(ins);

    switch (op)
    {
        case XED_ICLASS_BT:
        case XED_ICLASS_BTC:
        case XED_ICLASS_BTR:
        case XED_ICLASS_BTS:
            // Filter out the BTs we want to look at, we don't expect any in system libraries,
            // but we have seen a bt reg,reg on FC12
            if (INS_IsMemoryRead(ins) && !INS_OperandIsImmediate(ins, 1)) break;
            // Fall through and ignore non mem,reg operations.
        default:
            return;
    }

    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(ProcessAddress), IARG_ADDRINT, ADDRINT(formatInstruction(ins)),
                   IARG_MEMORYREAD_SIZE, IARG_MEMORYOP_EA, 0, IARG_REG_VALUE, INS_OperandReg(ins, 1), IARG_END);
}

static BOOL traceFromExecutable(TRACE trace)
{
    RTN rtn = TRACE_Rtn(trace);
    if (!RTN_Valid(rtn)) return FALSE;
    SEC sec = RTN_Sec(rtn);
    if (!SEC_Valid(sec)) return FALSE;
    IMG img = SEC_Img(sec);
    if (!IMG_Valid(img)) return FALSE;
    return IMG_IsMainExecutable(img);
}

VOID Trace(TRACE trace, VOID* v)
{
    // Only instrument the main executable, we're not interested in anything in dynamic libraries.
    if (!traceFromExecutable(trace)) return;

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            Instrument(ins);
        }
    }
}

void AtEnd(INT32 code, VOID* arg)
{
    out << "Target exited with code : " << code << endl;
    out << errors << " BTx addressing errors in " << tests << " tests" << endl;
    out.close();
    if (tests == 0)
        exit(-1);
    else
        exit(errors);
}

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    out.open(KnobOutput.Value().c_str());

    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddFiniFunction(AtEnd, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
