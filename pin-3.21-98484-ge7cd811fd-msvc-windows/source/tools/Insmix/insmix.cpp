/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This file contains a static and dynamic instruction mix profiler
 */

#include <vector>
#include <map>
#include <algorithm> // for sort
#include <iostream>
#include <iomanip>
#include <fstream>
#include <unistd.h>
#include "pin.H"
#include "control_manager.H"

using namespace CONTROLLER;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "insmix.out", "specify file name for insmix profile ");
KNOB< string > KnobOutput2File(KNOB_MODE_WRITEONCE, "pintool", "o2", "bblcnt.out", "specify file name for bblcnt profile");
KNOB< BOOL > KnobPid(KNOB_MODE_WRITEONCE, "pintool", "i", "0", "append pid to output");
KNOB< BOOL > KnobProfilePredicated(KNOB_MODE_WRITEONCE, "pintool", "p", "0",
                                   "enable accurate profiling for predicated instructions");
KNOB< BOOL > KnobProfileRoutines(KNOB_MODE_WRITEONCE, "pintool", "r", "1", "enable per routine profiling");
KNOB< BOOL > KnobProfileStaticOnly(KNOB_MODE_WRITEONCE, "pintool", "s", "0",
                                   "terminate after collection of static profile for main image");
KNOB< BOOL > KnobProfileDynamicOnly(KNOB_MODE_WRITEONCE, "pintool", "d", "1", "Only collect dynamic profile");
KNOB< BOOL > KnobNoSharedLibs(KNOB_MODE_WRITEONCE, "pintool", "no_shared_libs", "0", "do not instrument shared libraries");
KNOB< UINT32 > KnobNumInstructions(KNOB_MODE_WRITEONCE, "pintool", "num_instructions", "0",
                                   "Maximum instructions before detach (zero means no limit)");

static string longstr(int rtn_no, const char* name)
{
    return string("rtn[") + decstr(rtn_no) + string(",") + string(name) + string("]");
}

// The running count of instructions is kept here
// make it static to help the compiler optimize count_instructions
static UINT64 inscount = 0;

// This function is called before every block
VOID count_instructions(UINT32 c) { inscount += c; }

/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool computes a static and dynamic instruction mix profile\n"
            "\n";

    cerr << KNOB_BASE::StringKnobSummary();

    cerr << endl;

    return -1;
}

/* ===================================================================== */
/* INDEX HELPERS */
/* ===================================================================== */

const UINT32 MAX_INDEX     = 4096;
const UINT32 INDEX_SPECIAL = 3000;
const UINT32 MAX_MEM_SIZE  = 520;

const UINT32 INDEX_TOTAL          = INDEX_SPECIAL + 0;
const UINT32 INDEX_MEM_ATOMIC     = INDEX_SPECIAL + 1;
const UINT32 INDEX_STACK_READ     = INDEX_SPECIAL + 2;
const UINT32 INDEX_STACK_WRITE    = INDEX_SPECIAL + 3;
const UINT32 INDEX_IPREL_READ     = INDEX_SPECIAL + 4;
const UINT32 INDEX_IPREL_WRITE    = INDEX_SPECIAL + 5;
const UINT32 INDEX_MEM_READ_SIZE  = INDEX_SPECIAL + 6;
const UINT32 INDEX_MEM_WRITE_SIZE = INDEX_SPECIAL + 6 + MAX_MEM_SIZE;
const UINT32 INDEX_SPECIAL_END    = INDEX_SPECIAL + 6 + MAX_MEM_SIZE + MAX_MEM_SIZE;

BOOL IsMemReadIndex(UINT32 i) { return (INDEX_MEM_READ_SIZE <= i && i < INDEX_MEM_READ_SIZE + MAX_MEM_SIZE); }

BOOL IsMemWriteIndex(UINT32 i) { return (INDEX_MEM_WRITE_SIZE <= i && i < INDEX_MEM_WRITE_SIZE + MAX_MEM_SIZE); }

static UINT32 IndexStringLength(BBL bbl, BOOL memory_acess_profile)
{
    UINT32 count = 0;

    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
    {
        if (!INS_IsStandardMemop(ins)) continue;

        count++;
        if (memory_acess_profile)
        {
            if (INS_IsMemoryRead(ins)) count++; // for size

            if (INS_IsStackRead(ins)) count++;

            if (INS_IsIpRelRead(ins)) count++;

            if (INS_IsMemoryWrite(ins)) count++; // for size

            if (INS_IsStackWrite(ins)) count++;

            if (INS_IsIpRelWrite(ins)) count++;

            if (INS_IsAtomicUpdate(ins)) count++;
        }
    }

    return count;
}

/* ===================================================================== */
static UINT32 INS_MemsizeIndex(INS ins, BOOL write)
{
    UINT32 size = 0;
    for (UINT32 opIdx = 0; opIdx < INS_MemoryOperandCount(ins); opIdx++)
    {
        UINT32 opSize = (UINT32)INS_MemoryOperandSize(ins, opIdx);
        if (write && INS_MemoryOperandIsWritten(ins, opIdx))
        {
            size = opSize;
            break;
        }
        else if (!write && INS_MemoryOperandIsRead(ins, opIdx))
        {
            size = opSize;
            break;
        }
    }

    ASSERT(size < MAX_MEM_SIZE && size > 0,
           "write=" + decstr(write) + " found bad size " + decstr(size) + " in " + INS_Disassemble(ins));
    return (write ? INDEX_MEM_WRITE_SIZE : INDEX_MEM_READ_SIZE) + size;
}

/* ===================================================================== */
static UINT16* INS_GenerateIndexString(INS ins, UINT16* stats, BOOL memory_acess_profile)
{
    const UINT32 offset = 0;

    *stats++ = offset + INS_Opcode(ins);

    if (memory_acess_profile)
    {
        if (INS_IsMemoryRead(ins)) *stats++ = INS_MemsizeIndex(ins, 0);
        if (INS_IsMemoryWrite(ins)) *stats++ = INS_MemsizeIndex(ins, 1);

        if (INS_IsAtomicUpdate(ins)) *stats++ = INDEX_MEM_ATOMIC;

        if (INS_IsStackRead(ins)) *stats++ = INDEX_STACK_READ;
        if (INS_IsStackWrite(ins)) *stats++ = INDEX_STACK_WRITE;

        if (INS_IsIpRelRead(ins)) *stats++ = INDEX_IPREL_READ;
        if (INS_IsIpRelWrite(ins)) *stats++ = INDEX_IPREL_WRITE;
    }

    return stats;
}

/* ===================================================================== */

static string IndexToOpcodeString(UINT32 index)
{
    if (INDEX_SPECIAL <= index && index < INDEX_SPECIAL_END)
    {
        if (index == INDEX_TOTAL)
            return "*total";
        else if (IsMemReadIndex(index))
            return "*mem-read-" + decstr(index - INDEX_MEM_READ_SIZE);
        else if (IsMemWriteIndex(index))
            return "*mem-write-" + decstr(index - INDEX_MEM_WRITE_SIZE);
        else if (index == INDEX_MEM_ATOMIC)
            return "*mem-atomic";
        else if (index == INDEX_STACK_READ)
            return "*stack-read";
        else if (index == INDEX_STACK_WRITE)
            return "*stack-write";
        else if (index == INDEX_IPREL_READ)
            return "*iprel-read";
        else if (index == INDEX_IPREL_WRITE)
            return "*iprel-write";

        else
        {
            ASSERTX(0);
            return "";
        }
    }
    else
    {
        return OPCODE_StringShort(index);
    }
}

/* ===================================================================== */

/* ===================================================================== */

typedef UINT64 COUNTER;

/* zero initialized */

class STATS
{
  public:
    COUNTER unpredicated[MAX_INDEX];
    COUNTER predicated[MAX_INDEX];
    COUNTER predicated_true[MAX_INDEX];

    VOID Clear()
    {
        for (UINT32 i = 0; i < MAX_INDEX; i++)
        {
            unpredicated[i]    = 0;
            predicated[i]      = 0;
            predicated_true[i] = 0;
        }
    }
};

STATS GlobalStatsStatic;
STATS GlobalStatsDynamic;

class RTN_TABLE_ENTRY
{
  public:
    ADDRINT _address;
    const char* _name;

    RTN_TABLE_ENTRY(ADDRINT address, const char* name) : _address(address), _name(name) {}
};

map< UINT32, RTN_TABLE_ENTRY* > rtn_table;

class BBLSTATS
{
  public:
    COUNTER _counter;
    const UINT16* const _stats;
    const ADDRINT _addr;
    const UINT32 _rtn_num;
    const UINT32 _size;
    const UINT32 _numins;

  public:
    BBLSTATS(UINT16* stats, ADDRINT addr, UINT32 rtn_num, UINT32 size, UINT32 numins)
        : _counter(0), _stats(stats), _addr(addr), _rtn_num(rtn_num), _size(size), _numins(numins) {};
};

static BOOL CompareLess(const BBLSTATS* const& s1, const BBLSTATS* const& s2)
{
    return ((rtn_table[s1->_rtn_num]->_address) < (rtn_table[s2->_rtn_num]->_address));
}

static vector< const BBLSTATS* > statsList;

/* ===================================================================== */

static UINT32 enabled = 0;

static VOID Handler(EVENT_TYPE ev, VOID*, CONTEXT*, VOID*, THREADID, bool bast)
{
    switch (ev)
    {
        case EVENT_START:
            enabled = 1;
            break;

        case EVENT_STOP:
            enabled = 0;
            break;

        default:
            ASSERTX(false);
    }
}

static CONTROL_MANAGER control;

/* ===================================================================== */
VOID PIN_FAST_ANALYSIS_CALL docount(COUNTER* counter) { (*counter) += enabled; }

/* ===================================================================== */

VOID Trace(TRACE trace, VOID* v)
{
    if (KnobNoSharedLibs.Value() && IMG_Type(SEC_Img(RTN_Sec(TRACE_Rtn(trace)))) == IMG_TYPE_SHAREDLIB) return;

    if (KnobNumInstructions.Value() > 0 && inscount > KnobNumInstructions.Value()) PIN_Detach();

    RTN rtn = TRACE_Rtn(trace);
    ADDRINT rtn_address;
    const char* rtn_name;
    UINT32 rtn_num;
    if (!RTN_Valid(rtn))
    {
        //cerr << "Cannot find valid RTN for trace at address" << TRACE_Address(trace);
        rtn_address = 0;
        rtn_name    = "UNKNOWN";
        rtn_num     = 0;
    }
    else
    {
        rtn_num     = RTN_Id(rtn);
        rtn_address = RTN_Address(rtn);
        rtn_name    = RTN_Name(rtn).c_str();
    }
    map< UINT32, RTN_TABLE_ENTRY* >::const_iterator it = rtn_table.find(rtn_num);
    if (it == rtn_table.end())
    {
        char* str = new char[strlen(rtn_name) + 1];
        strcpy(str, rtn_name);
        RTN_TABLE_ENTRY* rtn_table_entry = new RTN_TABLE_ENTRY(rtn_address, str);
        rtn_table[rtn_num]               = rtn_table_entry;
    }
    const BOOL accurate_handling_of_predicates = KnobProfilePredicated.Value();

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        // Insert a call to count_instructions before every bbl, passing the number of instructions
        BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)count_instructions, IARG_UINT32, BBL_NumIns(bbl), IARG_END);

        // Summarize the stats for the bbl in a 0 terminated list
        // This is done at instrumentation time
        const UINT32 n = IndexStringLength(bbl, 1);

        UINT16* const stats     = new UINT16[n + 1];
        UINT16* const stats_end = stats + (n + 1);
        UINT16* curr            = stats;

        UINT32 numins = 0;
        UINT32 size   = 0;

        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            if ((INS_IsMemoryRead(ins) || INS_IsMemoryWrite(ins)) && !INS_IsStandardMemop(ins)) continue;

            numins += 1;
            size += INS_Size(ins);

            // Count the number of times a predicated instruction is actually executed
            // this is expensive and hence disabled by default
            if (INS_IsPredicated(ins) && accurate_handling_of_predicates)
            {
                INS_InsertPredicatedCall(ins, IPOINT_BEFORE, AFUNPTR(docount), IARG_FAST_ANALYSIS_CALL, IARG_PTR,
                                         &(GlobalStatsDynamic.predicated_true[INS_Opcode(ins)]), IARG_END);
            }

            curr = INS_GenerateIndexString(ins, curr, 1);
        }

        // string terminator
        *curr++ = 0;

        ASSERTX(curr == stats_end);

        // Insert instrumentation to count the number of times the bbl is executed
        BBLSTATS* bblstats = new BBLSTATS(stats, INS_Address(BBL_InsHead(bbl)), rtn_num, size, numins);
        INS_InsertCall(BBL_InsHead(bbl), IPOINT_BEFORE, AFUNPTR(docount), IARG_FAST_ANALYSIS_CALL, IARG_PTR,
                       &(bblstats->_counter), IARG_END);

        // Remember the counter and stats so we can compute a summary at the end
        statsList.push_back(bblstats);
    }
}

/* ===================================================================== */
static VOID DumpStats(ofstream& out, STATS& stats, BOOL predicated_true, BOOL print_zeros, const string& title)
{
    out << "#\n"
           "# "
        << title
        << "\n"
           "#\n"
           "#     opcode       count-unpredicated    count-predicated";

    if (predicated_true) out << "    count-predicated-true";

    out << "\n#\n";

    for (UINT32 i = 0; i < INDEX_SPECIAL; i++)
    {
        stats.unpredicated[INDEX_TOTAL] += stats.unpredicated[i];
        stats.predicated[INDEX_TOTAL] += stats.predicated[i];
        stats.predicated_true[INDEX_TOTAL] += stats.predicated_true[i];
    }

    for (UINT32 i = 0; i < MAX_INDEX; i++)
    {
        if (!print_zeros && stats.unpredicated[i] == 0 && stats.predicated[i] == 0) continue;

        out << setw(4) << i << " " << ljstr(IndexToOpcodeString(i), 20) << " " << setw(16) << stats.unpredicated[i] << " "
            << setw(16) << stats.predicated[i];
        if (predicated_true) out << " " << setw(16) << stats.predicated_true[i];
        out << endl;
    }
}

/* ===================================================================== */

VOID PrintDynamicCounts(ofstream& out)
{
    sort(statsList.begin(), statsList.end(), CompareLess);
    statsList.push_back(0); // add terminator marker

    STATS DynamicRtn;
    UINT32 rtn_num = 0;

    for (vector< const BBLSTATS* >::iterator bi = statsList.begin(); bi != statsList.end(); bi++)
    {
        const BBLSTATS* b = (*bi);

        if (b == 0 || rtn_num != b->_rtn_num)
        {
            if (rtn_num > 0 && KnobProfileRoutines)
            {
                DumpStats(out, DynamicRtn, false, 0,
                          "$rtn-counts " + longstr(rtn_num, rtn_table[rtn_num]->_name) + " at " +
                              hexstr(rtn_table[rtn_num]->_address));
                out << "#" << endl;
            }

            if (b != 0)
            {
                rtn_num = b->_rtn_num;
                DynamicRtn.Clear();
            }
            else
            {
                break;
            }
        }

        for (const UINT16* stats = b->_stats; *stats; stats++)
        {
            ASSERT(*stats < MAX_INDEX, "bad index " + decstr(*stats) + " at " + hexstr(b->_addr) + "\n");
            DynamicRtn.unpredicated[*stats] += b->_counter;
            GlobalStatsDynamic.unpredicated[*stats] += b->_counter;
        }
    }

    DumpStats(out, GlobalStatsDynamic, KnobProfilePredicated, 0, "$dynamic-counts");

    out << "# $eof" << endl;
}

/* ===================================================================== */

VOID PrintBblCount()
{
    string filename;
    std::ofstream out;

    filename = KnobOutput2File.Value();

    if (KnobPid)
    {
        filename += "." + decstr(getpid());
    }

    out.open(filename.c_str());

    out << "BBLCOUNT        1.0         0\n";

    for (vector< const BBLSTATS* >::iterator bi = statsList.begin(); bi != statsList.end(); bi++)
    {
        const BBLSTATS* b = (*bi);
        if (b == 0) break; // sentinel

        out << "0x" << hex << b->_addr << " " << dec << b->_counter << " " << b->_numins << " " << b->_size << endl;
    }

    out << "# $eof" << endl;

    out.close();
}

/* ===================================================================== */

VOID PrintOutput()
{
    string filename;
    std::ofstream out;

    // dump insmix profile

    filename = KnobOutputFile.Value();

    if (KnobPid)
    {
        filename += "." + decstr(getpid());
    }
    out.open(filename.c_str());

    out << "INSMIX        1.0         0\n";

    DumpStats(out, GlobalStatsStatic, false, 0, "$static-counts");

    out << endl;

    PrintDynamicCounts(out);

    out.close();

    PrintBblCount();
}

/* ===================================================================== */

VOID Fini(int, VOID* v) { PrintOutput(); }

/* ===================================================================== */

VOID Detach(VOID* v) { PrintOutput(); }

/* ===================================================================== */

VOID Image(IMG img, VOID* v)
{
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            // Prepare for processing of RTN, an  RTN is not broken up into BBLs,
            // it is merely a sequence of INSs
            RTN_Open(rtn);

            for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
            {
                UINT16 array[128];
                UINT16* end = INS_GenerateIndexString(ins, array, 1);

                if (INS_IsPredicated(ins))
                {
                    for (UINT16* start = array; start < end; start++)
                        GlobalStatsStatic.predicated[*start]++;
                }
                else
                {
                    for (UINT16* start = array; start < end; start++)
                        GlobalStatsStatic.unpredicated[*start]++;
                }
            }

            // to preserve space, release data associated with RTN after we have processed it
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

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    control.RegisterHandler(Handler, 0, FALSE);
    control.Activate();
    TRACE_AddInstrumentFunction(Trace, 0);

    PIN_AddFiniFunction(Fini, 0);
    PIN_AddDetachFunction(Detach, 0);

    if (!KnobProfileDynamicOnly.Value()) IMG_AddInstrumentFunction(Image, 0);

    // Never returns

    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
