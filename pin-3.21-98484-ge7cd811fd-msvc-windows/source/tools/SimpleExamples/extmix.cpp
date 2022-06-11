/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*
  Tweaked Muth's catmix.C to make extmix.C. An ISA extension profiler
*/

/* ===================================================================== */
/*! @file
 *  This file contains a static and dynamic instruction extension mix profiler
 */

#include "pin.H"
#include <list>
#include <iostream>
#include <fstream>
#include <stdlib.h>
using std::cerr;
using std::endl;
using std::list;
using std::ofstream;
using std::string;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "extmix.out", "specify profile file name");
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
    cerr << "This pin tool computes static and  dynamic instruction family (=extension) mix profile\n"
            "\n";

    cerr << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

const UINT32 MAX_INDEX = 64;

typedef UINT64 COUNTER;

/* zero initialized */

typedef struct
{
    COUNTER unpredicated[MAX_INDEX];
    COUNTER predicated[MAX_INDEX];
    COUNTER predicated_true[MAX_INDEX];
} STATS;

STATS GlobalStatsStatic;
STATS GlobalStatsDynamic;

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
            GlobalStatsDynamic.unpredicated[*stats] += (*bi)->_counter;
        }
    }
}

/* ===================================================================== */

UINT16 INS_GetStatsIndex(INS ins)
{
    if (INS_IsPredicated(ins))
        return MAX_INDEX + INS_Extension(ins);
    else
        return INS_Extension(ins);
}

/* ===================================================================== */

VOID docount(COUNTER* counter) { (*counter)++; }

/* ===================================================================== */

VOID Trace(TRACE trace, VOID* v)
{
    if (KnobNoSharedLibs.Value() && IMG_Type(SEC_Img(RTN_Sec(TRACE_Rtn(trace)))) == IMG_TYPE_SHAREDLIB) return;

    const BOOL accurate_handling_of_predicates = KnobProfilePredicated.Value();

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        // Summarize the stats for the bbl in a 0 terminated list
        // This is done at instrumentation time
        UINT16* stats = new UINT16[BBL_NumIns(bbl) + 1];

        INT32 index = 0;
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            // Count the number of times a predicated instruction is actually executed
            // this is expensive and hence disabled by default
            if (INS_IsPredicated(ins) && accurate_handling_of_predicates)
            {
                INS_InsertPredicatedCall(ins, IPOINT_BEFORE, AFUNPTR(docount), IARG_PTR,
                                         &(GlobalStatsDynamic.predicated_true[INS_Extension(ins)]), IARG_END);
            }

            stats[index++] = INS_GetStatsIndex(ins);
        }
        stats[index] = 0;

        // Insert instrumentation to count the number of times the bbl is executed
        BBLSTATS* bblstats = new BBLSTATS(stats);
        INS_InsertCall(BBL_InsHead(bbl), IPOINT_BEFORE, AFUNPTR(docount), IARG_PTR, &(bblstats->_counter), IARG_END);

        // Remember the counter and stats so we can compute a summary at the end
        statsList.push_back(bblstats);
    }
}

/* ===================================================================== */
VOID DumpStats(ofstream& out, STATS& stats, const string& title)
{
    out << "#\n"
           "# "
        << title
        << "\n"
           "#\n"
           "#num extension   count-unpredicated count-predicated count-predicated-true\n"
           "#\n";

    for (UINT32 i = 0; i < MAX_INDEX; i++)
    {
        if (stats.unpredicated[i] == 0 && stats.predicated[i] == 0) continue;

        out << decstr(i, 3) << " " << ljstr(EXTENSION_StringShort(i), 15) << decstr(stats.unpredicated[i], 12)
            << decstr(stats.predicated[i], 12) << decstr(stats.predicated_true[i], 12) << endl;
    }
}

/* ===================================================================== */
static std::ofstream* out = 0;

VOID Fini(int, VOID* v)
{
    DumpStats(*out, GlobalStatsStatic, "static counts");

    *out << endl;

    ComputeGlobalStats();

    DumpStats(*out, GlobalStatsDynamic, "dynamic counts");

    *out << "# eof" << endl;

    out->close();
}

/* ===================================================================== */

VOID Image(IMG img, VOID* v)
{
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            RTN_Open(rtn);

            for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
            {
                if (INS_IsPredicated(ins))
                    GlobalStatsStatic.predicated[INS_Extension(ins)]++;
                else
                    GlobalStatsStatic.unpredicated[INS_Extension(ins)]++;
            }

            RTN_Close(rtn);
        }
    }

    if (KnobProfileStaticOnly.Value())
    {
        Fini(0, 0);
        exit(0);
    }
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

    IMG_AddInstrumentFunction(Image, 0);

    // Never returns

    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
