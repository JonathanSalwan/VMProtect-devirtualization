/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file
 *  This file contains a dynamic register/memory operand pattern profiler
 */

#include "pin.H"
#include <list>
#include <iostream>
#include <cassert>
#include <iomanip>
#include <fstream>
#include <stdlib.h>
#include <string.h>
using std::cerr;
using std::endl;
using std::list;
using std::ostream;
using std::showpoint;
using std::string;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "ldstmix.out", "specify profile file name");
KNOB< BOOL > KnobNoSharedLibs(KNOB_MODE_WRITEONCE, "pintool", "no_shared_libs", "0", "do not instrument shared libraries");
KNOB< UINT32 > KnobMaxThreads(KNOB_MODE_WRITEONCE, "pintool", "threads", "100", "Maximum number of threads");

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool computes a dynamic register/memory pattern mix profile\n"
            "\n";

    cerr << KNOB_BASE::StringKnobSummary();

    cerr << endl;

    return -1;
}

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */
UINT32 MaxNumThreads = 1;

typedef UINT64 COUNTER;

typedef enum
{
    PATTERN_INVALID,
    PATTERN_MEM_RW,
    PATTERN_MEM_R,
    PATTERN_MEM_W,
    PATTERN_NO_MEM,
    PATTERN_NO_MEM_LIES,
    PATTERN_LAST
} pattern_t;

char const* pattern_t2str(pattern_t x)
{
    switch (x)
    {
        case PATTERN_INVALID:
            return "INVALID";
        case PATTERN_MEM_RW:
            return "MEM_RW";
        case PATTERN_MEM_R:
            return "MEM_R";
        case PATTERN_MEM_W:
            return "MEM_W";
        case PATTERN_NO_MEM:
            return "NO_MEM";
        case PATTERN_NO_MEM_LIES:
            return "NO_MEM_LIES";
        case PATTERN_LAST:
            return "LAST";
    }
    assert(0);
    /* NOTREACHED */
    return 0;
}

typedef struct
{
    COUNTER pattern[PATTERN_LAST];
} STATS;

STATS GlobalStats;

class BBLSTATS
{
  public:
    BBLSTATS(UINT16* stats) : _stats(stats)
    {
        _counter = new COUNTER[MaxNumThreads];
        memset(_counter, 0, sizeof(COUNTER) * MaxNumThreads);
    };

    //array of uint16, one per instr in the block, 0 terminated
    const UINT16* _stats;

    // one ctr per thread to avoid runtime locking at the expense of memory
    COUNTER* _counter;
};

list< const BBLSTATS* > statsList;

//////////////////////////////////////////////////////////

PIN_LOCK pinLock;
UINT32 numThreads = 0;

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    PIN_GetLock(&pinLock, threadid + 1);
    numThreads++;
    PIN_ReleaseLock(&pinLock);

    ASSERT(numThreads <= MaxNumThreads, "Maximum number of threads exceeded\n");
}

/* ===================================================================== */

VOID ComputeGlobalStats()
{
    for (UINT32 i = 0; i < PATTERN_LAST; i++)
        GlobalStats.pattern[i] = 0;

    // We have the count for each bbl and its stats, compute the summary
    for (list< const BBLSTATS* >::iterator bi = statsList.begin(); bi != statsList.end(); bi++)
        for (const UINT16* stats = (*bi)->_stats; *stats; stats++)
            for (UINT32 thd = 0; thd < numThreads; thd++)
                GlobalStats.pattern[*stats] += (*bi)->_counter[thd];
}

/* ===================================================================== */

/* ===================================================================== */

VOID docount(COUNTER* counter, THREADID tid) { counter[tid]++; }

INT32 RecordRegisters(BBL bbl, UINT16* stats, UINT32 max_stats)
{
    UINT32 count = 0;

    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
    {
        if (count >= max_stats)
        {
            cerr << "Too many stats in this block" << endl;
            exit(1);
        }
        bool rmem   = INS_IsMemoryRead(ins) || INS_HasMemoryRead2(ins);
        bool wmem   = INS_IsMemoryWrite(ins);
        bool rw_mem = rmem & wmem;
        if (rw_mem)
            stats[count++] = PATTERN_MEM_RW;
        else if (rmem)
            stats[count++] = PATTERN_MEM_R;
        else if (wmem)
            stats[count++] = PATTERN_MEM_W;
        else if (INS_SegmentRegPrefix(ins) != REG_INVALID())
            stats[count++] = PATTERN_NO_MEM_LIES;
        else
            stats[count++] = PATTERN_NO_MEM;
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
#define MAX_STATS_PER_BLOCK (128 * 1024)
        UINT16 buffer[MAX_STATS_PER_BLOCK];
        INT32 count = RecordRegisters(bbl, buffer, MAX_STATS_PER_BLOCK);
        ASSERTX(count < MAX_STATS_PER_BLOCK);

        // Summarize the stats for the bbl in a 0 terminated list
        // This is done at instrumentation time
        UINT16* stats = new UINT16[count];

        memcpy(stats, buffer, count * sizeof(UINT16));

        // Insert instrumentation to count the number of times the bbl is executed
        BBLSTATS* bblstats = new BBLSTATS(stats);
        INS_InsertCall(BBL_InsHead(bbl), IPOINT_BEFORE, AFUNPTR(docount), IARG_PTR, bblstats->_counter, IARG_THREAD_ID, IARG_END);

        // Remember the counter and stats so we can compute a summary at the end
        statsList.push_back(bblstats);
    }
}

/* ===================================================================== */
VOID EmitPerThreadStats(ostream* out)
{
    *out << std::setprecision(4) << showpoint;

    for (UINT32 thd = 0; thd < numThreads; thd++)
    {
        STATS ThreadStats;
        for (UINT32 i = 0; i < PATTERN_LAST; i++)
            ThreadStats.pattern[i] = 0;

        for (list< const BBLSTATS* >::iterator bi = statsList.begin(); bi != statsList.end(); bi++)
            for (const UINT16* stats = (*bi)->_stats; *stats; stats++)
                ThreadStats.pattern[*stats] += (*bi)->_counter[thd];

        COUNTER total = 0;
        for (int i = PATTERN_INVALID + 1; i < PATTERN_LAST; i++)
            total += ThreadStats.pattern[i];

        *out << "Thread " << thd << endl;
        for (int i = PATTERN_INVALID + 1; i < PATTERN_LAST; i++)
            *out << ljstr(pattern_t2str(static_cast< pattern_t >(i)), 15) << decstr(ThreadStats.pattern[i], 12) << "\t"
                 << std::setw(10) << 100.0 * ThreadStats.pattern[i] / total << std::endl;
        *out << endl;
    }
}

static std::ofstream* out = 0;

VOID Fini(int, VOID* v)
{
    ComputeGlobalStats();

    *out << "#\n"
            "#pattern-type count percent\n"
            "#\n";

    *out << "All Threads" << endl;
    COUNTER total = 0;
    for (int i = PATTERN_INVALID + 1; i < PATTERN_LAST; i++)
        total += GlobalStats.pattern[i];

    *out << std::setprecision(4) << showpoint;
    for (int i = PATTERN_INVALID + 1; i < PATTERN_LAST; i++)
        *out << ljstr(pattern_t2str(static_cast< pattern_t >(i)), 15) << decstr(GlobalStats.pattern[i], 12) << "\t"
             << std::setw(10) << 100.0 * GlobalStats.pattern[i] / total << std::endl;

    *out << endl;

    EmitPerThreadStats(out);

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

    PIN_InitLock(&pinLock);

    MaxNumThreads = KnobMaxThreads.Value();
    out           = new std::ofstream(KnobOutputFile.Value().c_str());

    PIN_AddThreadStartFunction(ThreadStart, 0);

    TRACE_AddInstrumentFunction(Trace, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Never returns

    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
