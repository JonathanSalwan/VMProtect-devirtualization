/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Check that we get the correct effective addresses for pop ops where the stack pointer
 * is part of the address.
 * It is *not* sufficient to use lea on the address, because the stack pointer is adjusted
 * *before* the address is calculated.
 *
 * That's a special case, hence a special test...
 *
 * This is intended to be used with the "far" test, or smallpush since they already have suitable
 * instructions in them.
 */

#include <stdio.h>
#include <stdlib.h>
#include "pin.H"
using std::string;

KNOB< BOOL > KnobTrace(KNOB_MODE_WRITEONCE, "pintool", "t", "0", "trace memory addresses of interest");
KNOB< string > KnobOutput(KNOB_MODE_WRITEONCE, "pintool", "o", "popea_verifier.out", "Name for log file");

static FILE* trace = NULL;

static UINT32 fails = 0;
static UINT32 tests = 0;

/* Check the address we are given matches that we calculate from the ctx 
 * 
 * We're only looking at pop offset(%esp) instructions.
 */

static VOID validateWriteAddress(ADDRINT ip, ADDRINT writeAddr, ADDRINT esp, ADDRDELTA offset, UINT32 operandSize)
{
    ADDRINT expectedAddress = esp + operandSize + offset; /* ESP is incremented *BEFORE* it is used in addressing */

    if (writeAddr != expectedAddress)
    {
        fprintf(trace, "%p: EA %p should be %p\n", (void*)ip, (void*)writeAddr, (void*)expectedAddress);
        fails++;
    }
    else if (KnobTrace)
    {
        fprintf(trace, "%p: EA %p OK\n", (void*)ip, (void*)writeAddr);
    }
    tests++;
}

static VOID RewriteIns(INS ins, VOID*)
{
    if (INS_Opcode(ins) != XED_ICLASS_POP) return;

    if (!INS_IsMemoryWrite(ins)) return;

    if (INS_MemoryBaseReg(ins) != REG_STACK_PTR) return;

    UINT32 readSize = 0;
    for (UINT32 opIdx = 0; opIdx < INS_MemoryOperandCount(ins); opIdx++)
    {
        if (INS_MemoryOperandIsRead(ins, opIdx))
        {
            readSize = INS_MemoryOperandSize(ins, opIdx);
            break;
        }
    }

    // If we get here we have a pop into a stack pointer relative address.
    ADDRDELTA offset = INS_MemoryDisplacement(ins);
    INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(validateWriteAddress), IARG_INST_PTR, IARG_MEMORYWRITE_EA, IARG_REG_VALUE,
                   REG_STACK_PTR, IARG_ADDRINT, offset, IARG_UINT32, readSize, IARG_END);
}

void AtEnd(INT32 code, VOID* arg)
{
    fprintf(trace, "Tested: %d, failed: %d\n", tests, fails);
    fclose(trace);
    exit(fails);
}

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    trace = fopen(KnobOutput.Value().c_str(), "w");
    if (!trace)
    {
        perror("fopen");
        return 1;
    }

    INS_AddInstrumentFunction(RewriteIns, 0);
    PIN_AddFiniFunction(AtEnd, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
