/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This file contains a static and dynamic register mix profiler
 */

#include "pin.H"
#include <list>
#include <iostream>
#include <fstream>
using std::cerr;
using std::endl;
using std::list;
using std::string;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "regmix.out", "specify profile file name");
KNOB< BOOL > KnobProfilePredicated(KNOB_MODE_WRITEONCE, "pintool", "p", "0",
                                   "enable accurate profiling for predicated instructions");
KNOB< BOOL > KnobProfileStaticOnly(KNOB_MODE_WRITEONCE, "pintool", "s", "0",
                                   "terminate after collection of static profile for main image");
KNOB< BOOL > KnobNoSharedLibs(KNOB_MODE_WRITEONCE, "pintool", "no_shared_libs", "0", "do not instrument shared libraries");

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool computes a dynamic register usage mix profile\n"
            "\n";

    cerr << KNOB_BASE::StringKnobSummary();

    cerr << endl;

    return -1;
}

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

const UINT16 MAX_REG = 4096;

typedef UINT64 COUNTER;

/* zero initialized */

struct GLOBALSTATS
{
    COUNTER reg_r[MAX_REG];
    COUNTER reg_w[MAX_REG];
} GlobalStats;

class BBLSTATS
{
  public:
    BBLSTATS(UINT16* stats) : _stats(stats), _counter(0) {};

    const UINT16* _stats;
    COUNTER _counter;
};

list< const BBLSTATS* > statsList;

/* ===================================================================== */

VOID ComputeGlobalStats()
{
    // We have the count for each bbl and its stats, compute the summary
    for (list< const BBLSTATS* >::iterator bi = statsList.begin(); bi != statsList.end(); bi++)
    {
        for (const UINT16* stats = (*bi)->_stats; *stats; stats++)
        {
            GlobalStats.reg_r[*stats] += (*bi)->_counter;
        }
    }
}

/* ===================================================================== */

UINT16 REG_GetStatsIndex(REG reg, BOOL is_write)
{
    if (is_write)
        return MAX_REG + reg;
    else
        return reg;
}

/* ===================================================================== */

VOID docount(COUNTER* counter) { (*counter)++; }

INT32 RecordRegisters(BBL bbl, UINT16* stats)
{
    INT32 count = 0;

    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
    {
        const UINT32 max_r = INS_MaxNumRRegs(ins);

        for (UINT32 i = 0; i < max_r; i++)
        {
            const REG reg = INS_RegR(ins, i);
            if (REG_is_gr(reg))
            {
                stats[count++] = REG_GetStatsIndex(reg, FALSE);
            }
#if 0
            // This is for arm
            else if( REG_is_aggregate(reg) )
            {
                REGSET regset = INS_RegAggregateR(ins);
                for( REG reg =  REGSET_PopNext(regset); REG_valid(reg); reg  =  REGSET_PopNext(regset) )
                {
                    stats[count++] = REG_GetStatsIndex(reg,FALSE);
                }
            }
#endif
        }

        const UINT32 max_w = INS_MaxNumWRegs(ins);

        for (UINT32 i = 0; i < max_w; i++)
        {
            const REG reg = INS_RegW(ins, i);
            if (REG_is_gr(reg))
            {
                stats[count++] = REG_GetStatsIndex(reg, TRUE);
            }
#if 0
            else if( REG_is_aggregate(reg) )
            {
                REGSET regset = INS_RegAggregateW(ins);
                for( REG reg =  REGSET_PopNext(regset); REG_valid(reg); reg  =  REGSET_PopNext(regset) )
                {
                    stats[count++] = REG_GetStatsIndex(reg,TRUE);
                }
            }
#endif
        }
    }

    stats[count++] = 0;

    return count;
}

/* ===================================================================== */

VOID Trace(TRACE trace, VOID* v)
{
    const RTN rtn = TRACE_Rtn(trace);

    if (!RTN_Valid(rtn)) return;

    const SEC sec = RTN_Sec(rtn);
    ASSERTX(SEC_Valid(sec));

    const IMG img = SEC_Img(sec);
    if (!IMG_Valid(img)) return;

    if (KnobNoSharedLibs.Value() && IMG_Type(img) == IMG_TYPE_SHAREDLIB) return;

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        // Record the registers into a dummy buffer so we can count them
        UINT16 buffer[128 * 1024];
        INT32 count = RecordRegisters(bbl, buffer);
        ASSERTX(count < 128 * 1024);

        // Summarize the stats for the bbl in a 0 terminated list
        // This is done at instrumentation time
        UINT16* stats = new UINT16[count];

        RecordRegisters(bbl, stats);

        // Insert instrumentation to count the number of times the bbl is executed
        BBLSTATS* bblstats = new BBLSTATS(stats);
        INS_InsertCall(BBL_InsHead(bbl), IPOINT_BEFORE, AFUNPTR(docount), IARG_PTR, &(bblstats->_counter), IARG_END);

        // Remember the counter and stats so we can compute a summary at the end
        statsList.push_back(bblstats);
    }
}

/* ===================================================================== */
static std::ofstream* out = 0;
VOID Fini(int, VOID* v)
{
    ComputeGlobalStats();

    *out << "#\n"
            "#num reg  count-read  count-written\n"
            "#\n";

    for (UINT32 i = 0; i < MAX_REG; i++)
    {
        if (GlobalStats.reg_w[i] == 0 && GlobalStats.reg_r[i] == 0) continue;

        *out << decstr(i, 3) << " " << ljstr(REG_StringShort(REG(i)), 15) << decstr(GlobalStats.reg_r[i], 12)
             << decstr(GlobalStats.reg_w[i], 12) << endl;
    }

    *out << "# eof" << endl;

    out->close();
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }
    out = new std::ofstream(KnobOutputFile.Value().c_str());

    TRACE_AddInstrumentFunction(Trace, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Never returns

    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
