/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Chek the prefixes APIs.
 */

#include "pin_tests_util.H"
#include <fstream>

UINT32 addressSizePrefixCount    = 0;
UINT32 branchNotTakenPrefixCount = 0;
UINT32 branchTakenPrefixCount    = 0;
UINT32 lockPrefixCount           = 0;
UINT32 operandSizePrefixCount    = 0;
UINT32 repPrefixCount            = 0;
UINT32 repnePrefixCount          = 0;
UINT32 segmentPrefixCount        = 0;

bool seen_nop = false;

VOID CountsUpdate(INS ins)
{
    if (INS_AddressSizePrefix(ins)) addressSizePrefixCount++;
    if (INS_BranchNotTakenPrefix(ins)) branchNotTakenPrefixCount++;
    if (INS_BranchTakenPrefix(ins)) branchTakenPrefixCount++;
    if (INS_LockPrefix(ins)) lockPrefixCount++;
    if (INS_OperandSizePrefix(ins)) operandSizePrefixCount++;
    if (INS_RepPrefix(ins)) repPrefixCount++;
    if (INS_RepnePrefix(ins)) repnePrefixCount++;
    if (INS_SegmentPrefix(ins)) segmentPrefixCount++;
}

VOID Rtn(RTN rtn, VOID* v)
{
    seen_nop    = false;
    string name = RTN_Name(rtn);
    if ((name == "test1") || (name == "test2"))
    {
        RTN_Open(rtn);
        for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
        {
            if (INS_IsNop(ins))
            {
                seen_nop = true;
                continue;
            }
            ASSERT(!seen_nop, "assertion failed: non-NOP instruction after NOP");
            CountsUpdate(ins);
        }
        RTN_Close(rtn);
    }
}

VOID Fini(INT32 code, VOID* v)
{
    TEST(addressSizePrefixCount == 1, "INS_AddressSizePrefix failed");
    TEST(branchNotTakenPrefixCount == 0, "INS_BranchNotTakenPrefix failed");
    TEST(branchTakenPrefixCount == 0, "INS_BranchTakenPrefix failed");
    TEST(lockPrefixCount == 1, "INS_LockPrefix failed");
    TEST(operandSizePrefixCount == 1, "INS_OperandSizePrefix failed");
    TEST(repPrefixCount == 1, "INS_RepPrefix failed");
    TEST(repnePrefixCount == 1, "INS_RepnePrefix failed");
    TEST(segmentPrefixCount == 1, "INS_SegmentPrefix failed");
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
