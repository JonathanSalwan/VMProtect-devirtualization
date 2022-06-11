/*
 * Copyright (C) 2018-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/**
 * @file
 * A tool which test the Pin_Detach() flow.
 * It calls Pin_Detach after a native operation over an XMM register. The test itself tests that the extended state is restored
 * correctly when moving to native code
 */
#include <stdio.h>
#include "pin.H"
#include <iostream>
#include <fstream>

using namespace std;

// This tool shows how to detach Pin from an
// application that is under Pin's control.

KNOB< BOOL > KnobChangeBit(KNOB_MODE_WRITEONCE, "pintool", "change_bit", "0", "Change bit.... ");

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "instrument.out", "specify output file name");

UINT64 icount = 0;
ofstream OutFile;
static bool addssInstrumented = false;

VOID inject(PIN_REGISTER* val)
{
    icount++;

    // icount controls the instruction to inject to
    if (icount == 1)
    {
        //change bit 12
        if (KnobChangeBit)
        {
            val->byte[1] = (val->byte[1] ^ (1U << 4));
        }
        PIN_Detach();
    }
    // We shouln't get here the second time after PIN_Detach() was called
    ASSERTX(icount < 2);
}

VOID Instruction(INS ins, VOID* v)
{
    // instrument only the main exec, if valid checks go through
    RTN Rtn = INS_Rtn(ins);
    if (!RTN_Valid(Rtn)) return;
    SEC Sec = RTN_Sec(Rtn);
    if (!SEC_Valid(Sec)) return;
    IMG Img = SEC_Img(Sec);
    if (!IMG_Valid(Img)) return;
    if (!IMG_IsMainExecutable(Img)) return;

    // ADDSS in linux or macOS*
    if (INS_Mnemonic(ins).find("ADDSS") != std::string::npos)
    {
        addssInstrumented = true;
        OutFile << INS_Disassemble(ins) << "\n";
        INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)inject, IARG_REG_REFERENCE, INS_RegW(ins, 0), IARG_END);
    }
}

VOID DetachCb(VOID* v)
{
    std::cerr << "PIN DETACHED" << std::endl;
    OutFile.close();
}

VOID Fini(INT32 code, VOID* v)
{
    OutFile.close();
    ASSERT(!addssInstrumented, "Detach didn't happen\n");
}

int main(int argc, char* argv[])
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    OutFile.open(KnobOutputFile.Value().c_str());
    // Callback function to invoke for every
    // execution of an instruction
    INS_AddInstrumentFunction(Instruction, 0);

    // Callback functions to invoke before
    // Pin releases control of the application
    PIN_AddDetachFunction(DetachCb, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
