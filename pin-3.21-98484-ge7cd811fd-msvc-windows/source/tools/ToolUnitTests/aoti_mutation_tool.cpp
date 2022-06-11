/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
// This tool performs AOTI mutation of instructions.
// It should be used with aoti_mutation_target.c, since it knows about the specific function names
// in that code.
//

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "pin.H"

static REG scratchReg;
static int instrumentationCount = 0;

static BOOL INS_HasImmediateOperand(INS ins)
{
    for (unsigned int i = 0; i < INS_OperandCount(ins); i++)
        if (INS_OperandIsImmediate(ins, i)) return TRUE;

    return FALSE;
}

// Delete the first mov immediate
static VOID deleteMov(RTN rtn)
{
    for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
    {
        if (INS_IsMov(ins) && INS_HasImmediateOperand(ins))
        {
            INS_Delete(ins);
            instrumentationCount++;
            return;
        }
    }
}

// Insert a direct branch over the first mov immediate
static VOID insertJump(RTN rtn)
{
    for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
    {
        if (INS_IsMov(ins) && INS_HasImmediateOperand(ins))
        {
            INS_InsertDirectJump(ins, IPOINT_BEFORE, INS_Address(ins) + INS_Size(ins));
            instrumentationCount++;
            return;
        }
    }
}

static ADDRINT returnValue(ADDRINT arg) { return arg; }

// Insert an indirect branch over the first mov immediate
static VOID insertIndirectJump(RTN rtn)
{
    for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
    {
        if (INS_IsMov(ins) && INS_HasImmediateOperand(ins))
        {
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(returnValue), IARG_ADDRINT, INS_Address(ins) + INS_Size(ins),
                           IARG_RETURN_REGS, scratchReg, IARG_END);

            INS_InsertIndirectJump(ins, IPOINT_BEFORE, scratchReg);
            instrumentationCount++;
            return;
        }
    }
}

static ADDRINT returnValueMinus4(ADDRINT arg)
{
    ADDRINT retVal = arg - 4;

    printf("returnValueMinus4 returns %x\n", retVal);
    return (retVal);
}

VOID AddrValueA(ADDRINT address) { printf("AddrValueA is %x\n", address); }

VOID AddrValueB(ADDRINT address) { printf("AddrValueB is %x\n", address); }

// Offset the addressing of the first "or" instruction back by 4 bytes.
static VOID modifyAddressing(RTN rtn)
{
    for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
    {
        if (INS_Opcode(ins) == XED_ICLASS_OR)
        {
            printf("Rewriting address of ins\n%x: %s\n", INS_Address(ins), INS_Disassemble(ins).c_str());

            // pass the original memory address accessed by the app instruction (i.e. before the rewrite) to AddrValueA
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(AddrValueA), IARG_MEMORYOP_EA, 0, IARG_END);

            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(returnValueMinus4), IARG_MEMORYOP_EA, 0, IARG_RETURN_REGS, scratchReg,
                           IARG_END);

            INS_RewriteMemoryOperand(ins, 0, scratchReg);

            // pass the original memory address accessed by the app instruction (i.e. before the rewrite) to AddrValueB
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(AddrValueB), IARG_MEMORYOP_EA, 0, IARG_END);

            instrumentationCount++;
            return;
        }
    }
}

static struct
{
    const char* rtnName;
    void (*instrumentFunction)(RTN);
} functionInstrumentation[] = {{"deleteMov", deleteMov},
                               {"insertJump", insertJump},
                               {"insertIndirectJump", insertIndirectJump},
                               {"modifyAddressing", modifyAddressing}};

VOID ImageLoad(IMG img, VOID* v)
{
    for (UINT32 i = 0; i < sizeof(functionInstrumentation) / sizeof(functionInstrumentation[0]); i++)
    {
        RTN rtn = RTN_FindByName(img, functionInstrumentation[i].rtnName);

        if (RTN_Valid(rtn))
        {
            fprintf(stderr, "Instrumenting %s\n", functionInstrumentation[i].rtnName);
            RTN_Open(rtn);
            functionInstrumentation[i].instrumentFunction(rtn);
            RTN_Close(rtn);
        }
    }
}

VOID Fini(INT32 code, VOID* v)
{
    // Check that we ran all the tests we expected!
    if (instrumentationCount != sizeof(functionInstrumentation) / sizeof(functionInstrumentation[0]))
    {
        exit(1);
    }
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // prepare for image instrumentation mode
    PIN_InitSymbols();

    // Initialize pin
    if (PIN_Init(argc, argv)) return -1;

    scratchReg = PIN_ClaimToolRegister();

    // Register ImageLoad to be called when an image is loaded
    IMG_AddInstrumentFunction(ImageLoad, 0);
    // And our Fini function to check that we did all we expected to.
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
