/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include "pin.H"
using std::cerr;
using std::endl;

static int* replacement_ptr = NULL;
static KNOB< bool > KnobVerbose(KNOB_MODE_WRITEONCE, "pintool", "verbose", "0", "Verbose output");

static int* replaceMemoryReadFunc(int* readAddr, void* pc)
{
    if (KnobVerbose)
    {
        cerr << pc << ") replaceMemoryReadFunc called with " << (void*)readAddr << endl;
    }
    if (*readAddr != 0xbadc0de) return readAddr;
    if (KnobVerbose)
    {
        cerr << pc << ") replacing " << (void*)readAddr << " to " << (void*)replacement_ptr << endl;
    }
    return replacement_ptr;
}

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID* v)
{
    UINT32 memOperands = INS_MemoryOperandCount(ins);
    if (INS_IsMov(ins) && 1 == memOperands && INS_MemoryOperandIsRead(ins, 0) &&
        REG_Width(INS_MemoryBaseReg(ins)) == REGWIDTH_32 && INS_MemoryIndexReg(ins) == REG_INVALID() &&
        INS_MemoryDisplacement(ins) == 0)
    {
        if (KnobVerbose)
        {
            cerr << "found " << INS_Disassemble(ins) << endl;
        }
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)replaceMemoryReadFunc, IARG_MEMORYOP_EA, 0, IARG_REG_VALUE, REG_INST_PTR,
                       IARG_RETURN_REGS, REG_INST_G0, IARG_END);
        INS_RewriteMemoryOperand(ins, 0, REG_INST_G0);
    }
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool tests rewrite memory operand prefixed with segment register" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/*
 * Allocate an address that falls outside the lower 32 bit address space
 * This ensures that we get an address cannot be accessed by memory operand
 * with only 32 bit base register
 */
void* allocateMemoryAbove32bitAddressSpace()
{
    void* startAddr = (void*)0x100000000;
    size_t memSize  = getpagesize();
    for (void* curAddr = startAddr; curAddr >= startAddr; curAddr = (void*)((ADDRINT)curAddr + memSize))
    {
        void* mem = mmap(curAddr, memSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
        if (NULL == mem)
            continue;
        else if (mem >= startAddr)
            return mem;

        // We got memory in the low 32 bit address space.
        // Try to allocate again in a different region
        munmap(mem, memSize);
    }
    return NULL;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    replacement_ptr = (int*)allocateMemoryAbove32bitAddressSpace();
    if (NULL == replacement_ptr)
    {
        cerr << "Failed to allocate memory above the 32 bit address space" << endl;
        exit(2);
    }
    if (KnobVerbose)
    {
        cerr << "replacement_ptr = " << (void*)replacement_ptr << endl;
    }
    *replacement_ptr = 0xdeadbee;

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
