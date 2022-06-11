/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>
#include "pin.H"
using std::cerr;
using std::endl;
using std::hex;
using std::ios;
using std::ofstream;
using std::string;

ofstream OutFile;

REG dummy_reg;

ADDRINT PIN_FAST_ANALYSIS_CALL returnMemoryFunc(ADDRINT memea_orig, ADDRINT memea_callback, ADDRINT ip, string* dis)
{
    OutFile << "returnMemoryFunc: " << hex << ip << " " << *dis << endl;

    return memea_orig;
}

ADDRINT PIN_FAST_ANALYSIS_CALL memoryCallback(PIN_MEM_TRANS_INFO* memTransInfo, VOID* v) { return (memTransInfo->addr); }

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID* v)
{
    string* disptr = new string(INS_Disassemble(ins));

    UINT32 memOperands = INS_MemoryOperandCount(ins);
    if (!INS_IsVgather(ins) && memOperands)
    {
        // OPs
        for (UINT32 memOp = 0; memOp < memOperands; memOp++)
        {
            if (INS_MemoryOperandIsRead(ins, memOp) || INS_MemoryOperandIsWritten(ins, memOp))
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)returnMemoryFunc, IARG_FAST_ANALYSIS_CALL, IARG_MEMORYOP_EA, memOp,
                               IARG_MEMORYOP_PTR, memOp, IARG_INST_PTR, IARG_PTR, disptr, IARG_RETURN_REGS,
#if defined(TARGET_IA32E)
                               REG_RSP,
#else
                               REG_ESP,
#endif
                               IARG_END);
            }
        }
    }
}

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "memcallerr.out", "specify output file name");

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v)
{
    // Write to a file since cout and cerr maybe closed by the application
    OutFile.setf(ios::showbase);
    OutFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool tests memory address translation callback assert" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    OutFile.open(KnobOutputFile.Value().c_str());

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register memory callback
    PIN_AddMemoryAddressTransFunction(memoryCallback, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
