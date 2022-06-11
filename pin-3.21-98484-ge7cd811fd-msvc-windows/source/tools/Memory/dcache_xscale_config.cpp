/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This file contains an ISA-portable PIN tool for functional simulation of
 *  the Intel XScale(R) processor L1 data cache
 */

#include <iostream>

#include "pin.H"

typedef UINT64 CACHE_STATS; // type of cache hit/miss counters

#include "pin_cache.H"

KNOB< UINT32 > KnobCacheSize(KNOB_MODE_WRITEONCE, "pintool", "-s1", "32768", "l1 cache size in bytes");

KNOB< UINT32 > KnobCacheAssociativity(KNOB_MODE_WRITEONCE, "pintool", "-a1", "32", "l1 cache associativity");

KNOB< UINT32 > KnobCacheLineSize(KNOB_MODE_WRITEONCE, "pintool", "-l1", "32", "l1 cache size in bytes");

namespace DL1
{
const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_NO_ALLOCATE;

const UINT32 max_sets = 128;

const UINT32 max_associativity = 32;

typedef CACHE_ROUND_ROBIN(max_sets, max_associativity, allocation) CACHE;
} // namespace DL1

static DL1::CACHE* dl1;

static VOID Fini(int code, VOID* v) { std::cerr << *dl1; }

static VOID MemRefSingle(CACHE_BASE::ACCESS_TYPE accessType, ADDRINT addr)
{
    // first level D-cache: single cache-line access
    dl1->AccessSingleLine(addr, accessType);
}

static VOID MemRefMulti(CACHE_BASE::ACCESS_TYPE accessType, ADDRINT addr, UINT32 size)
{
    // first level D-cache: potentially multiple cache-line access
    dl1->Access(addr, size, accessType);
}

static VOID Instruction(INS ins, void* v)
{
    if (!INS_IsStandardMemop(ins)) return;
    if (INS_MemoryOperandCount(ins) == 0) return;

    UINT32 readSize = 0, writeSize = 0;
    UINT32 readOperandCount = 0, writeOperandCount = 0;

    for (UINT32 opIdx = 0; opIdx < INS_MemoryOperandCount(ins); opIdx++)
    {
        if (INS_MemoryOperandIsRead(ins, opIdx))
        {
            readSize = INS_MemoryOperandSize(ins, opIdx);
            readOperandCount++;
        }
        if (INS_MemoryOperandIsWritten(ins, opIdx))
        {
            writeSize = INS_MemoryOperandSize(ins, opIdx);
            writeOperandCount++;
        }
    }

    if (readOperandCount > 0)
    {
        // we assume accesses <= 4 bytes stay in the same cache line
        // to speed up cache access lookups
        const AFUNPTR countFun = (readSize <= 4 ? (AFUNPTR)MemRefSingle : (AFUNPTR)MemRefMulti);

        // only predicated-on memory instructions access D-cache
        INS_InsertPredicatedCall(ins, IPOINT_BEFORE, countFun, IARG_UINT32, CACHE_BASE::ACCESS_TYPE_LOAD, IARG_MEMORYREAD_EA,
                                 IARG_MEMORYREAD_SIZE, IARG_END);
    }

    if (writeOperandCount > 0)
    {
        const AFUNPTR countFun = (writeSize <= 4 ? (AFUNPTR)MemRefSingle : (AFUNPTR)MemRefMulti);

        // only predicated-on memory instructions access D-cache
        INS_InsertPredicatedCall(ins, IPOINT_BEFORE, countFun, IARG_UINT32, CACHE_BASE::ACCESS_TYPE_STORE, IARG_MEMORYWRITE_EA,
                                 IARG_MEMORYWRITE_SIZE, IARG_END);
    }
}

extern int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    const UINT32 size          = KnobCacheSize.Value();
    const UINT32 linesize      = KnobCacheLineSize.Value();
    const UINT32 associativity = KnobCacheAssociativity.Value();

    ASSERTX(associativity <= DL1::max_associativity);
    ASSERTX(size / (associativity * linesize) <= DL1::max_sets);

    // create the cache object
    dl1 = new DL1::CACHE("L1 Data Cache", size, linesize, associativity);

    // Never returns
    PIN_StartProgram();

    return 0; // make compiler happy
}
