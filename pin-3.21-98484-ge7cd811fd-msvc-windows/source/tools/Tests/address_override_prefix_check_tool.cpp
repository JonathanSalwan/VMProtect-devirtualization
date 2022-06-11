/*
 * Copyright (C) 2021-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Description
 * ===========
 * This test verifies the fix for Mantis #4795.
 * When instrumenting the operand of an indirect call with address override and replacing the operand
 * with a register, the new instruction should not include the address override prefix.
 *
 * Valid platforms on which to run the test:
 * ===============================
 * On 32B Arch the test requires to be able to allocate a page in the lower 64KB.
 * This is not possible on PIN's currently supported OSs:
 * On Windows, the lower 64KB is reserved for catching access violations.
 * On Linux the user will have to change the value of "/proc/sys/vm/mmap_min_addr".
 *
 * On 64B Arch, the test requires to be able to allocate a page in the lower 64GB (0 .. (1<<32)).
 * This is not possible on OSX as the OS reserves the lower 4GB for the kernel.
 *
 * So the only valid combinations of OS\Arch are Windows\64B and Linux\64B.
 *
 * Test flow:
 * ==========
 * (1) Tool : Instrument the "HookAnalyze" method.
 * (2) Tool : Instrument each instruction and look for the instruction address.
 * (3) App  : Allocate a page in the lower 32b range.
 * (4) App  : Copy a function that indirectly call an address in page+delta
 * (5) App  : Call HookAnalyze with the instruction address and the displacement address
 * (6) Tool : Analyze the "HookAnalyze" method and save the instruction address and
 *            the displacement address.
 * (7) App  : Call the function on the allocated page.
 * (8) Tool : Recognize the checked instruction through the instruction address.
 * (9) Tool : Replace the memory operand in the instruction with the address of the displacement
 *            address.
 * (10) App : Successfully jump to a method that prints "Success."
 */

#include "pin.H"
#include <iostream>
#include <sstream>

struct Data
{
    Data() : instructionAddress(0), displacementAddress(0), hookMethod("ArgsHook") {}
    ADDRINT instructionAddress;
    ADDRINT displacementAddress;
    CHAR const* const hookMethod;
} static data;

VOID HookAnalyze(ADDRINT instructionAddress, ADDRINT displacementAddress)
{
    data.instructionAddress  = instructionAddress;
    data.displacementAddress = displacementAddress;
}

VOID Image(IMG img, VOID *v)
{
    if (!IMG_IsMainExecutable(img)) return;
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            if (RTN_Name(rtn) == data.hookMethod)
            {
                RTN_Open(rtn);
                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(HookAnalyze),
                               IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                               IARG_FUNCARG_ENTRYPOINT_VALUE, 1 ,
                               IARG_END);
                RTN_Close(rtn);
                return;
            }
        }
    }
    ASSERT(false, "\nCouldn't locate the method: " + data.hookMethod);
}

static UINT64 MoveImmediateToRegister() { return data.displacementAddress; }

static void analyzeInsturction(INS ins, VOID* val)
{
    if (INS_Address(ins) == data.instructionAddress)
    {
        ASSERTX(INS_IsCall(ins));
        ASSERTX(INS_IsIndirectControlFlow(ins));
        ASSERTX(INS_AddressSizePrefix(ins));
        ASSERTX(INS_MemoryOperandIsRead(ins, 0));

        REG reg = PIN_ClaimToolRegister();

        INS_InsertCall( ins, IPOINT_BEFORE, AFUNPTR(MoveImmediateToRegister),
                        IARG_RETURN_REGS, reg, IARG_END);

        INS_RewriteMemoryOperand(ins, 0, reg);
    }
}

int main(int argc, char *argv[])
{
    PIN_InitSymbols();
    ASSERTX(!PIN_Init(argc, argv));

    INS_AddInstrumentFunction(analyzeInsturction, NULL);
    IMG_AddInstrumentFunction(Image, NULL);

    PIN_StartProgram();
    return 0;
}

