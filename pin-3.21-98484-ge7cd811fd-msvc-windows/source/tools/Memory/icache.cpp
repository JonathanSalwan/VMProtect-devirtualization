/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This file contains an ISA-portable cache simulator
 *  instruction cache hierarchies
 */

#include "pin.H"

#include <iostream>
#include <fstream>
#include <cassert>

#include "cache.H"
#include "pin_profile.H"
using std::cerr;
using std::endl;
using std::string;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "icache.out", "specify icache file name");
KNOB< BOOL > KnobTrackInsts(KNOB_MODE_WRITEONCE, "pintool", "ti", "0",
                            "track individual instructions -- increases profiling time");
KNOB< UINT32 > KnobThresholdHit(KNOB_MODE_WRITEONCE, "pintool", "rh", "100", "only report ops with hit count above threshold");
KNOB< UINT32 > KnobThresholdMiss(KNOB_MODE_WRITEONCE, "pintool", "rm", "100", "only report ops with miss count above threshold");
KNOB< UINT32 > KnobCacheSize(KNOB_MODE_WRITEONCE, "pintool", "c", "32", "cache size in kilobytes");
KNOB< UINT32 > KnobLineSize(KNOB_MODE_WRITEONCE, "pintool", "b", "32", "cache block size in bytes");
KNOB< UINT32 > KnobAssociativity(KNOB_MODE_WRITEONCE, "pintool", "a", "4", "cache associativity (1 for direct mapped)");

/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool represents a cache simulator.\n"
            "\n";

    cerr << KNOB_BASE::StringKnobSummary();

    cerr << endl;

    return -1;
}

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

// wrap configuation constants into their own name space to avoid name clashes
namespace IL1
{
const UINT32 max_sets                          = KILO; // cacheSize / (lineSize * associativity);
const UINT32 max_associativity                 = 256;  // associativity;
const CACHE_ALLOC::STORE_ALLOCATION allocation = CACHE_ALLOC::STORE_ALLOCATE;

typedef CACHE_ROUND_ROBIN(max_sets, max_associativity, allocation) CACHE;
} // namespace IL1

IL1::CACHE* il1 = NULL;

typedef enum
{
    COUNTER_MISS = 0,
    COUNTER_HIT  = 1,
    COUNTER_NUM
} COUNTER;

typedef COUNTER_ARRAY< UINT64, COUNTER_NUM > COUNTER_HIT_MISS;

// holds the counters with misses and hits
// conceptually this is an array indexed by instruction address
COMPRESSOR_COUNTER< ADDRINT, UINT32, COUNTER_HIT_MISS > profile;

/* ===================================================================== */

VOID LoadMulti(ADDRINT addr, UINT32 size, UINT32 instId)
{
    // first level I-cache
    const BOOL il1Hit = il1->Access(addr, size, CACHE_BASE::ACCESS_TYPE_LOAD);

    const COUNTER counter = il1Hit ? COUNTER_HIT : COUNTER_MISS;
    profile[instId][counter]++;
}

/* ===================================================================== */

VOID LoadSingle(ADDRINT addr, UINT32 instId)
{
    // @todo we may access several cache lines for
    // first level I-cache
    const BOOL il1Hit = il1->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_LOAD);

    const COUNTER counter = il1Hit ? COUNTER_HIT : COUNTER_MISS;
    profile[instId][counter]++;
}

/* ===================================================================== */

VOID LoadMultiFast(ADDRINT addr, UINT32 size) { il1->Access(addr, size, CACHE_BASE::ACCESS_TYPE_LOAD); }

/* ===================================================================== */

VOID LoadSingleFast(ADDRINT addr) { il1->AccessSingleLine(addr, CACHE_BASE::ACCESS_TYPE_LOAD); }

/* ===================================================================== */

VOID Instruction(INS ins, void* v)
{
    // map sparse INS addresses to dense IDs
    const ADDRINT iaddr = INS_Address(ins);
    const UINT32 instId = profile.Map(iaddr);

    const UINT32 size = INS_Size(ins);
    const BOOL single = (size <= 4);

    if (KnobTrackInsts)
    {
        if (single)
        {
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)LoadSingle, IARG_ADDRINT, iaddr, IARG_UINT32, instId, IARG_END);
        }
        else
        {
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)LoadMulti, IARG_UINT32, iaddr, IARG_UINT32, size, IARG_UINT32,
                                     instId, IARG_END);
        }
    }
    else
    {
        if (single)
        {
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)LoadSingleFast, IARG_UINT32, iaddr, IARG_END);
        }
        else
        {
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)LoadMultiFast, IARG_UINT32, iaddr, IARG_UINT32, size, IARG_END);
        }
    }
}

/* ===================================================================== */

VOID Fini(int code, VOID* v)
{
    std::ofstream out(KnobOutputFile.Value().c_str());

    // print I-cache profile
    // @todo what does this print

    out << "PIN:MEMLATENCIES 1.0. 0x0\n";

    out << "#\n"
           "# ICACHE stats\n"
           "#\n";

    out << il1->StatsLong("# ", CACHE_BASE::CACHE_TYPE_ICACHE);

    if (KnobTrackInsts)
    {
        out << "#\n"
               "# INST stats\n"
               "#\n";

        out << profile.StringLong();
    }
    out.close();
}

/* ===================================================================== */

int main(int argc, char* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    il1 = new IL1::CACHE("L1 Inst Cache", KnobCacheSize.Value() * KILO, KnobLineSize.Value(), KnobAssociativity.Value());

    profile.SetKeyName("iaddr          ");
    profile.SetCounterName("icache:miss        icache:hit");

    COUNTER_HIT_MISS threshold;

    threshold[COUNTER_HIT]  = KnobThresholdHit.Value();
    threshold[COUNTER_MISS] = KnobThresholdMiss.Value();

    profile.SetThreshold(threshold);

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns

    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
