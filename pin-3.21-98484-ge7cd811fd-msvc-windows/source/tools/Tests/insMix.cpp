/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Basic tests for some INS_* APIs.
 */

#include "pin_tests_util.H"

UINT32 fullRegRContainCount = 0;
UINT32 interruptCount       = 0;
UINT32 rdtscCount           = 0;
UINT32 sysretCount          = 0;
UINT32 xchgCount            = 0;
UINT32 directFarJumpCount   = 0;
UINT32 memoryIndexRegCount  = 0;

VOID CountsUpdate(INS ins)
{
    if (INS_FullRegRContain(ins, REG_EAX) && XED_CATEGORY_WIDENOP != INS_Category(ins)) fullRegRContainCount++;
    if (INS_IsInterrupt(ins)) interruptCount++;
    if (INS_IsRDTSC(ins)) rdtscCount++;
    if (INS_IsSysret(ins)) sysretCount++;
    if (INS_IsXchg(ins)) xchgCount++;

    if (INS_IsDirectFarJump(ins))
    {
        directFarJumpCount++;

        UINT32 displacement;
        UINT16 segment;
        INS_GetFarPointer(ins, segment, displacement);
        TEST(segment == 0xabcd && displacement == 0x14, "INS_GetFarPointer failed");
    }
    if (INS_MemoryIndexReg(ins) != REG_INVALID() && XED_CATEGORY_WIDENOP != INS_Category(ins)) memoryIndexRegCount++;
}

VOID Rtn(RTN rtn, VOID* v)
{
    string name = RTN_Name(rtn);
    if (name == "test1")
    {
        RTN_Open(rtn);
        for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
        {
            CountsUpdate(ins);
        }
        RTN_Close(rtn);
    }
}

VOID Fini(INT32 code, VOID* v)
{
    // "xor eax, eax" & "cmp eax, 0x0" are also counted in fullRegRContainCount
    TEST(fullRegRContainCount == 3, "INS_FullRegRContain failed");
    TEST(interruptCount == 1, "INS_IsInterrupt failed");
    TEST(rdtscCount == 1, "INS_IsRDTSC failed");
    TEST(sysretCount == 1, "INS_IsSysret failed");
    TEST(xchgCount == 1, "INS_IsXchg failed");
    TEST(directFarJumpCount == 1, "INS_GetFarPointer failed");
    TEST(memoryIndexRegCount == 1, "INS_MemoryIndexReg failed");
}

int main(INT32 argc, CHAR** argv)
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    RTN_AddInstrumentFunction(Rtn, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
