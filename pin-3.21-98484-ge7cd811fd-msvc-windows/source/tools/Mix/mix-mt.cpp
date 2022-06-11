/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/* ===================================================================== */
/*! @file This file contains a static and dynamic opcode/ISA extension/ISA
 *  category mix profiler
 *
 * This is derived from mix.cpp. Handles an arbitrary number of threads
 * using TLS for data storage and avoids locking, except during I/O.
 */

#if defined(TARGET_WINDOWS)
#define strdup _strdup
#endif

#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <map>
#include <utility> /* for pair */
#include <vector>
#include <unistd.h>
#include "pin.H"
#include "control_manager.H"
#include "mix-fp-state.H"
using namespace CONTROLLER;

// key for accessing TLS storage in the threads. initialized once in main()
static TLS_KEY tls_key;

typedef UINT32 stat_index_t;

static string disassemble(UINT64 start, UINT64 stop);

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */
KNOB_COMMENT mix_knob_family("pintool:mix", "Mix knobs");
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool:mix", "o", "mix.out", "specify profile file name");
KNOB< UINT32 > KnobTopBlocks(KNOB_MODE_WRITEONCE, "pintool:mix", "top_blocks", "20",
                             "specify a maximal number of top blocks for which icounts are printed");
KNOB< BOOL > KnobShowDisassembly(KNOB_MODE_WRITEONCE, "pintool:mix", "disas", "0", "Show disassembly for top blocks");
KNOB< BOOL > KnobPid(KNOB_MODE_WRITEONCE, "pintool:mix", "i", "0", "append pid to output file name");
KNOB< BOOL > KnobProfilePredicated(KNOB_MODE_WRITEONCE, "pintool:mix", "p", "0",
                                   "enable accurate profiling for predicated instructions");
KNOB< BOOL > KnobProfileStaticOnly(KNOB_MODE_WRITEONCE, "pintool:mix", "s", "0",
                                   "terminate after collection of static profile for main image");
#ifndef TARGET_WINDOWS
KNOB< BOOL > KnobProfileDynamicOnly(KNOB_MODE_WRITEONCE, "pintool:mix", "d", "0", "Only collect dynamic profile");
#else
KNOB< BOOL > KnobProfileDynamicOnly(KNOB_MODE_WRITEONCE, "pintool:mix", "d", "1", "Only collect dynamic profile");
#endif
KNOB< BOOL > KnobNoSharedLibs(KNOB_MODE_WRITEONCE, "pintool:mix", "no_shared_libs", "0", "do not instrument shared libraries");

KNOB< BOOL > KnobInstructionLengthMix(KNOB_MODE_WRITEONCE, "pintool:mix", "ilen", "0", "Compute instruction length mix");
KNOB< BOOL > KnobCategoryMix(KNOB_MODE_WRITEONCE, "pintool:mix", "category", "0", "Compute ISA category mix");
KNOB< BOOL > KnobIformMix(KNOB_MODE_WRITEONCE, "pintool:mix", "iform", "0", "Compute ISA iform mix");
KNOB< BOOL > KnobMapToFile(KNOB_MODE_WRITEONCE, "pintool:mix", "mapaddr", "0", "Map Addresses to File/Line information");
KNOB< BOOL > KnobEarlyOut(KNOB_MODE_WRITEONCE, "pintool:mix", "early_out", "0", "Exit after tracing the first region.");

typedef enum
{
    measure_opcode   = 0,
    measure_category = 1,
    measure_ilen     = 2,
    measure_iform    = 3
} measurement_t;
measurement_t measurement = measure_opcode;

/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool computes a static and dynamic opcode, "
         << "instruction form, instruction length, extension or category mix profile\n\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    cerr << "The default is to do opcode and ISA extension profileing" << endl;
    cerr << "At most one of -iform, -ilen or  -category is allowed" << endl;
    cerr << endl;
    return -1;
}

/* ===================================================================== */
/* INDEX HELPERS */
/* ===================================================================== */

const UINT32 INDEX_SPECIAL = 3000;
const UINT32 MAX_MEM_SIZE  = 520;
const UINT32 MAX_EXTENSION = XED_EXTENSION_LAST + 10;

const UINT32 INDEX_TOTAL          = INDEX_SPECIAL + 0;
const UINT32 INDEX_MEM_ATOMIC     = INDEX_SPECIAL + 1;
const UINT32 INDEX_STACK_READ     = INDEX_SPECIAL + 2;
const UINT32 INDEX_STACK_WRITE    = INDEX_SPECIAL + 3;
const UINT32 INDEX_IPREL_READ     = INDEX_SPECIAL + 4;
const UINT32 INDEX_IPREL_WRITE    = INDEX_SPECIAL + 5;
const UINT32 INDEX_MEM_READ_SIZE  = INDEX_SPECIAL + 6;
const UINT32 INDEX_MEM_WRITE_SIZE = INDEX_SPECIAL + 6 + MAX_MEM_SIZE;

const UINT32 INDEX_EXTENSION = INDEX_SPECIAL + 6 + 2 * MAX_MEM_SIZE;

const UINT32 INDEX_SCALAR_SIMD = INDEX_EXTENSION + MAX_EXTENSION;
const UINT32 INDEX_FMA_BASE    = INDEX_SCALAR_SIMD + 1;
const UINT32 INDEX_FMA         = INDEX_FMA_BASE + 1;
const UINT32 INDEX_FMA_ADD     = INDEX_FMA_BASE + 2;
const UINT32 INDEX_FMA_MUL     = INDEX_FMA_BASE + 3;
const UINT32 INDEX_FMA_S       = INDEX_FMA_BASE + 4;
const UINT32 INDEX_FMA_S_ADD   = INDEX_FMA_BASE + 5; // NOTE: skipped 6. does not matter
const UINT32 INDEX_FMA_S_MUL   = INDEX_FMA_BASE + 7;
const UINT32 INDEX_FMA_D       = INDEX_FMA_BASE + 8;
const UINT32 INDEX_FMA_D_ADD   = INDEX_FMA_BASE + 9;
const UINT32 INDEX_FMA_D_MUL   = INDEX_FMA_BASE + 10;
const UINT32 INDEX_FPMA        = INDEX_FMA_BASE + 11;
const UINT32 INDEX_FPMA_ADD    = INDEX_FMA_BASE + 12;
const UINT32 INDEX_FPMA_MUL    = INDEX_FMA_BASE + 13;
const UINT32 INDEX_FMS         = INDEX_FMA_BASE + 14;
const UINT32 INDEX_FMS_SUB     = INDEX_FMA_BASE + 15;
const UINT32 INDEX_FMS_MUL     = INDEX_FMA_BASE + 16;
const UINT32 INDEX_FMS_S       = INDEX_FMA_BASE + 17;
const UINT32 INDEX_FMS_S_SUB   = INDEX_FMA_BASE + 18;
const UINT32 INDEX_FMS_S_MUL   = INDEX_FMA_BASE + 19;
const UINT32 INDEX_FMS_D       = INDEX_FMA_BASE + 20;
const UINT32 INDEX_FMS_D_SUB   = INDEX_FMA_BASE + 21;
const UINT32 INDEX_FMS_D_MUL   = INDEX_FMA_BASE + 22;
const UINT32 INDEX_FPMS        = INDEX_FMA_BASE + 23;
const UINT32 INDEX_FPMS_SUB    = INDEX_FMA_BASE + 24;
const UINT32 INDEX_FPMS_MUL    = INDEX_FMA_BASE + 25;
const UINT32 INDEX_FNMA        = INDEX_FMA_BASE + 26;
const UINT32 INDEX_FNMA_ADD    = INDEX_FMA_BASE + 27;
const UINT32 INDEX_FNMA_MUL    = INDEX_FMA_BASE + 28;
const UINT32 INDEX_FNMA_S      = INDEX_FMA_BASE + 29;
const UINT32 INDEX_FNMA_S_ADD  = INDEX_FMA_BASE + 30;
const UINT32 INDEX_FNMA_S_MUL  = INDEX_FMA_BASE + 31;
const UINT32 INDEX_FNMA_D      = INDEX_FMA_BASE + 32;
const UINT32 INDEX_FNMA_D_ADD  = INDEX_FMA_BASE + 33;
const UINT32 INDEX_FNMA_D_MUL  = INDEX_FMA_BASE + 34;
const UINT32 INDEX_FPNMA       = INDEX_FMA_BASE + 35;
const UINT32 INDEX_FPNMA_ADD   = INDEX_FMA_BASE + 36;
const UINT32 INDEX_FPNMA_MUL   = INDEX_FMA_BASE + 37;

const UINT32 INDEX_SPECIAL_END = INDEX_FMA_BASE + 38;

BOOL IsMemReadIndex(UINT32 i) { return (INDEX_MEM_READ_SIZE <= i && i < INDEX_MEM_READ_SIZE + MAX_MEM_SIZE); }

BOOL IsMemWriteIndex(UINT32 i) { return (INDEX_MEM_WRITE_SIZE <= i && i < INDEX_MEM_WRITE_SIZE + MAX_MEM_SIZE); }

/* ===================================================================== */
static UINT32 INS_GetIndex(INS ins)
{
    UINT32 index = 0;
    switch (measurement)
    {
        case measure_opcode:
            index = INS_Opcode(ins);
            break;
        case measure_ilen:
            index = INS_Size(ins);
            break;
        case measure_category:
            index = INS_Category(ins);
            break;
        case measure_iform:
        {
            xed_decoded_inst_t* xedd = INS_XedDec(ins);
            xed_iform_enum_t iform   = xed_decoded_inst_get_iform_enum(xedd);
            index                    = static_cast< UINT32 >(iform);
        }
        break;
    }
    return index;
}

/* ===================================================================== */

static bool IsScalarSimd(INS ins)
{
    xed_decoded_inst_t* xedd = INS_XedDec(ins);
    return xed_decoded_inst_get_attribute(xedd, XED_ATTRIBUTE_SIMD_SCALAR);
}

static UINT32 IndexStringLength(BBL bbl, BOOL memory_access_profile)
{
    UINT32 count = 0;

    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
    {
        count++;                                   // one for the ins
        if (measurement != measure_iform) count++; // one for the ISA extension.

        if (measurement == measure_opcode && memory_access_profile)
        {
            if (INS_IsMemoryRead(ins)) count++; // for size

            if (INS_IsStackRead(ins)) count++;

            if (INS_IsIpRelRead(ins)) count++;

            if (INS_IsMemoryWrite(ins)) count++; // for size

            if (INS_IsStackWrite(ins)) count++;

            if (INS_IsIpRelWrite(ins)) count++;

            if (INS_IsAtomicUpdate(ins)) count++;

            if (IsScalarSimd(ins)) count++;
        }
    }

    return count;
}

/* ===================================================================== */
static UINT32 MemsizeToIndex(UINT32 size, BOOL write) { return (write ? INDEX_MEM_WRITE_SIZE : INDEX_MEM_READ_SIZE) + size; }

/* ===================================================================== */
static stat_index_t* INS_GenerateIndexString(INS ins, stat_index_t* stats, BOOL memory_access_profile)
{
    *stats++ = INS_GetIndex(ins);
    if (measurement != measure_iform) *stats++ = INS_Extension(ins) + INDEX_EXTENSION;

    if (measurement == measure_opcode && memory_access_profile)
    {
        UINT32 readSize = 0, writeSize = 0;
        for (UINT32 opIdx = 0; opIdx < INS_MemoryOperandCount(ins); opIdx++)
        {
            if (INS_MemoryOperandIsRead(ins, opIdx))
            {
                readSize = INS_MemoryOperandSize(ins, opIdx);
                break;
            }
            if (INS_MemoryOperandIsWritten(ins, opIdx))
            {
                writeSize = INS_MemoryOperandSize(ins, opIdx);
                break;
            }
        }

        if (INS_IsMemoryRead(ins)) *stats++ = MemsizeToIndex(readSize, FALSE);
        if (INS_IsMemoryWrite(ins)) *stats++ = MemsizeToIndex(writeSize, TRUE);

        if (INS_IsAtomicUpdate(ins)) *stats++ = INDEX_MEM_ATOMIC;

        if (INS_IsStackRead(ins)) *stats++ = INDEX_STACK_READ;
        if (INS_IsStackWrite(ins)) *stats++ = INDEX_STACK_WRITE;

        if (INS_IsIpRelRead(ins)) *stats++ = INDEX_IPREL_READ;
        if (INS_IsIpRelWrite(ins)) *stats++ = INDEX_IPREL_WRITE;

        if (IsScalarSimd(ins)) *stats++ = INDEX_SCALAR_SIMD;
    }

    return stats;
}

/* ===================================================================== */

static string IndexToString(UINT32 index)
{
    if (measurement == measure_iform)
    {
        return xed_iform_enum_t2str(static_cast< xed_iform_enum_t >(index));
    }

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
        else if (index == INDEX_SCALAR_SIMD)
            return "*scalar-simd";
        else if (index >= INDEX_EXTENSION && index < INDEX_EXTENSION + MAX_EXTENSION)
            return "*isa-ext-" + EXTENSION_StringShort(index - INDEX_EXTENSION);

        else if (index == INDEX_FMA)
            return "*FMA";
        else if (index == INDEX_FMA_ADD)
            return "*FMA_ADD";
        else if (index == INDEX_FMA_MUL)
            return "*FMA_MUL";
        else if (index == INDEX_FMA_S)
            return "*FMA_S";
        else if (index == INDEX_FMA_S_ADD)
            return "*FMA_S_ADD";
        else if (index == INDEX_FMA_S_MUL)
            return "*FMA_S_MUL";
        else if (index == INDEX_FMA_D)
            return "*FMA_D";
        else if (index == INDEX_FMA_D_ADD)
            return "*FMA_D_ADD";
        else if (index == INDEX_FMA_D_MUL)
            return "*FMA_D_MUL";
        else if (index == INDEX_FPMA)
            return "*FPMA";
        else if (index == INDEX_FPMA_ADD)
            return "*FPMA_ADD";
        else if (index == INDEX_FPMA_MUL)
            return "*FPMA_MUL";
        else if (index == INDEX_FMS)
            return "*FMS";
        else if (index == INDEX_FMS_SUB)
            return "*FMS_SUB";
        else if (index == INDEX_FMS_MUL)
            return "*FMS_MUL";
        else if (index == INDEX_FMS_S)
            return "*FMS_S";
        else if (index == INDEX_FMS_S_SUB)
            return "*FMS_S_SUB";
        else if (index == INDEX_FMS_S_MUL)
            return "*FMS_S_MUL";
        else if (index == INDEX_FMS_D)
            return "*FMS_D";
        else if (index == INDEX_FMS_D_SUB)
            return "*FMS_D_SUB";
        else if (index == INDEX_FMS_D_MUL)
            return "*FMS_D_MUL";
        else if (index == INDEX_FPMS)
            return "*FPMS";
        else if (index == INDEX_FPMS_SUB)
            return "*FPMS_SUB";
        else if (index == INDEX_FPMS_MUL)
            return "*FPMS_MUL";
        else if (index == INDEX_FNMA)
            return "*FNMA";
        else if (index == INDEX_FNMA_ADD)
            return "*FNMA_ADD";
        else if (index == INDEX_FNMA_MUL)
            return "*FNMA_MUL";
        else if (index == INDEX_FNMA_S)
            return "*FNMA_S";
        else if (index == INDEX_FNMA_S_ADD)
            return "*FNMA_S_ADD";
        else if (index == INDEX_FNMA_S_MUL)
            return "*FNMA_S_MUL";
        else if (index == INDEX_FNMA_D)
            return "*FNMA_D";
        else if (index == INDEX_FNMA_D_ADD)
            return "*FNMA_D_ADD";
        else if (index == INDEX_FNMA_D_MUL)
            return "*FNMA_D_MUL";
        else if (index == INDEX_FPNMA)
            return "*FPNMA";
        else if (index == INDEX_FPNMA_ADD)
            return "*FPNMA_ADD";
        else if (index == INDEX_FPNMA_MUL)
            return "*FPNMA_MUL";

        else
        {
            ASSERTX(0);
            return "";
        }
    }
    else if (measurement == measure_ilen)
    {
        ostringstream s;
        s << "ILEN-" << index;
        return s.str();
    }
    else if (measurement == measure_opcode)
    {
        return OPCODE_StringShort(index);
    }
    else if (measurement == measure_category)
    {
        return CATEGORY_StringShort(index);
    }
    ASSERTX(0);
    return "";
}

/* ===================================================================== */
/* ===================================================================== */
typedef UINT64 COUNTER;

/* zero initialized */

typedef map< UINT32, COUNTER > stat_map_t;

class CSTATS
{
  public:
    CSTATS() { clear(); }

    stat_map_t unpredicated;
    stat_map_t predicated;
    stat_map_t predicated_true;

    VOID clear()
    {
        unpredicated.erase(unpredicated.begin(), unpredicated.end());
        predicated.erase(predicated.begin(), predicated.end());
        predicated_true.erase(predicated_true.begin(), predicated_true.end());
    }
};

class BBL_SORT_STATS
{
  public:
    ADDRINT _pc;
    UINT32 _rtn_num;
    COUNTER _icount;
    COUNTER _executions;
    COUNTER _nbytes;
};

CSTATS GlobalStatsStatic; // summary stats for static analysis

class BBLSTATS
{
    // Our first pass sets up the types of stats we need to update for this
    // block. We have one stat per instruction in the block. The _stats
    // array is null terminated.
  public:
    const stat_index_t* const _stats;
    const ADDRINT _pc;     // start PC of the block
    const UINT32 _ninst;   // # of instructions
    const UINT32 _nbytes;  // # of bytes in the block
    const UINT32 _rtn_num; // index of the routine address/function name information
    BBLSTATS(stat_index_t* stats, ADDRINT pc, UINT32 ninst, UINT32 nbytes, UINT32 rtn_number)
        : _stats(stats), _pc(pc), _ninst(ninst), _nbytes(nbytes), _rtn_num(rtn_number) {};
};

static vector< BBLSTATS* > statsList;

/* ===================================================================== */

#if defined(__GNUC__)
#if defined(TARGET_MAC) || defined(TARGET_WINDOWS)
// macOS* XCODE2.4.1 gcc and Cgywin gcc 3.4.x only allow for 16b
// alignment! So we need to pad!
#define ALIGN_LOCK __attribute__((aligned(16)))
#else
#define ALIGN_LOCK __attribute__((aligned(64)))
#endif
#else
#define ALIGN_LOCK __declspec(align(64))
#endif

typedef struct
{
    char pad0[64];
    PIN_LOCK ALIGN_LOCK lock; /* for mediating output */
    char pad1[64];
    PIN_LOCK ALIGN_LOCK bbl_list_lock; /* for the bbl list */
    char pad2[64];
    PIN_LOCK ALIGN_LOCK rtn_table_lock; /* for the rtn table */
    char pad3[64];
} ALIGN_LOCK locks_t;

locks_t locks;

static std::ofstream* out;

class thread_data_t
{
  public:
    thread_data_t() : enabled(0) {}
    CSTATS cstats;
    vector< CSTATS > stats_per_function; // indexed by rtn_num
    UINT32 enabled;

    vector< COUNTER > block_counts;

    UINT32 size()
    {
        UINT32 limit;
        limit = block_counts.size();
        return limit;
    }

    void resize(UINT32 n)
    {
        if (size() < n) block_counts.resize(2 * n);
    }
};

thread_data_t* get_tls(THREADID tid)
{
    thread_data_t* tdata = static_cast< thread_data_t* >(PIN_GetThreadData(tls_key, tid));
    return tdata;
}

VOID activate_counting(THREADID tid)
{
    thread_data_t* tdata = get_tls(tid);
    tdata->enabled       = 1;
}
VOID deactivate_counting(THREADID tid)
{
    thread_data_t* tdata = get_tls(tid);
    tdata->enabled       = 0;
}

UINT32 numThreads = 0;

VOID ThreadStart(THREADID tid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    // This function is locked no need for a Pin Lock here
    numThreads++;
    PIN_GetLock(&locks.lock, tid + 2); // for output
    *out << "# Starting tid " << tid << endl;
    PIN_ReleaseLock(&locks.lock);

    thread_data_t* tdata = new thread_data_t;
    // remember my pointer for later
    PIN_SetThreadData(tls_key, tdata, tid);

    // make sure the thread is counting stuff.

    // FIXME: The controller should start all threads if no trigger
    // conditions are specified, but currently it only starts
    // TID0. Starting here is wrong if the controller has a nontrivial
    // starting condition, but this is what most people want. They can
    // always stop the controller and zero the stats using markers as a
    // workaround.

    if (tid) activate_counting(tid);
}

VOID emit_stats(THREADID tid);    //forward prototype
VOID emit_pc_stats(THREADID tid); //forward prototype
VOID zero_stats(THREADID tid);    //forward prototype
VOID Fini(int, VOID* v);
VOID emit_bbl_stats_sorted(THREADID tid);
static CONTROL_MANAGER control;

static VOID Handler(EVENT_TYPE ev, VOID* val, CONTEXT* ctxt, VOID* ip, THREADID tid, bool bcast)
{
    switch (ev)
    {
        case EVENT_START:
            PIN_GetLock(&locks.lock, tid + 2); // for output
            *out << "# Start counting for tid " << tid << endl;
            PIN_ReleaseLock(&locks.lock);
            activate_counting(tid);
            break;
        case EVENT_STOP:
            PIN_GetLock(&locks.lock, tid + 2); // for output
            *out << "# Stop counting for tid " << tid << endl;
            PIN_ReleaseLock(&locks.lock);
            deactivate_counting(tid);
            if (KnobEarlyOut)
            {
                *out << "Exiting due to -early-out" << endl;
                Fini(0, 0);
                exit(0);
            }

            break;
        case EVENT_STATS_EMIT:
            PIN_GetLock(&locks.lock, tid + 2); // for output
            *out << "# Emit stats for tid " << static_cast< int >(tid) << endl;
            PIN_ReleaseLock(&locks.lock);
            emit_stats(tid);
            break;
        case EVENT_STATS_RESET:
            PIN_GetLock(&locks.lock, tid + 2); // for output
            *out << "# Reset stats for tid " << static_cast< int >(tid) << endl;
            PIN_ReleaseLock(&locks.lock);
            zero_stats(tid);
            break;

        default:
            ASSERTX(false);
    }
}

/* ===================================================================== */
VOID validate_bbl_count(THREADID tid, ADDRINT block_count_for_trace)
{
    thread_data_t* tdata = get_tls(tid);
    tdata->resize(block_count_for_trace + 1);
}

VOID PIN_FAST_ANALYSIS_CALL docount_bbl(ADDRINT block_id, THREADID tid)
{
    thread_data_t* tdata = get_tls(tid);
    //ASSERTX(tdata->size() > block_id);
    tdata->block_counts[block_id] += tdata->enabled;
}

VOID docount_predicated_true(UINT32 index, THREADID tid)
{
    thread_data_t* tdata = get_tls(tid);
    if (tdata->enabled)
    {
        stat_map_t::iterator i = tdata->cstats.predicated_true.find(index);
        if (i == tdata->cstats.predicated_true.end())
            tdata->cstats.predicated_true[index] = 1;
        else
            i->second += 1;
    }
}

/* ===================================================================== */

VOID zero_stats(THREADID tid)
{
    thread_data_t* tdata = get_tls(tid);
    tdata->cstats.clear();
    UINT32 limit = tdata->size();
    for (UINT32 i = 0; i < limit; i++)
        tdata->block_counts[i] = 0;
    tdata->stats_per_function.erase(tdata->stats_per_function.begin(), tdata->stats_per_function.end());
}
/* ===================================================================== */

VOID CheckForSpecialMarkers(INS ins, ADDRINT pc, unsigned int instruction_size)
{
    // This checks for single instances of special 3B NOPs.
    // 0F1FF3 - start
    // 0F1FF4 - stop
    // 0F1FF5 - emit stats
    // 0F1FF6 - zero stats

    // FIXME: if there are collisions with existing instructions, we can
    // change them here.

    //FIXME: Ideally this would be integrated in to the control.H so file
    //so that anything can use it.
    if (instruction_size != 3) return;

    UINT8* pc_ptr = reinterpret_cast< UINT8* >(pc);
    if (pc_ptr[0] == 0x0F && pc_ptr[1] == 0x1F)
    {
        switch (pc_ptr[2])
        {
            case 0xF3: // start
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)activate_counting, IARG_THREAD_ID, IARG_END);
                break;
            case 0xF4: // stop
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)deactivate_counting, IARG_THREAD_ID, IARG_END);
                break;
            case 0xF5: // emit
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)emit_stats, IARG_THREAD_ID, IARG_END);
                break;
            case 0xF6: // zero
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)zero_stats, IARG_THREAD_ID, IARG_END);
                break;
            default:
                break;
        }
    }
}

/* ===================================================================== */
class RTN_TABLE_ENTRY
{
  public:
    ADDRINT _address;
    ADDRINT _end_address;
    const char* _name;
    UINT32 _rtn_num; /* facilitates sorting later */
    UINT64 _total;

    RTN_TABLE_ENTRY(ADDRINT address = 0, ADDRINT end_address = 0, const char* name = 0, UINT32 rtn_num = 0)
        : _address(address), _end_address(end_address), _name(name), _rtn_num(rtn_num), _total(0)
    {}
};

static vector< RTN_TABLE_ENTRY > rtn_table;

static int qsort_rtn_compare_fn(const void* a, const void* b)
{
    /*descending sort*/
    const RTN_TABLE_ENTRY* ba = static_cast< const RTN_TABLE_ENTRY* >(a);
    const RTN_TABLE_ENTRY* bb = static_cast< const RTN_TABLE_ENTRY* >(b);

    if (ba->_total < bb->_total) return 1;
    if (ba->_total > bb->_total) return -1;
    return 0;
}

UINT32 routine_identifier(RTN rtn, THREADID tid)
{
    if (!RTN_Valid(rtn))
    {
        return 0; /* we never return nonzero global routine numbers */
    }
    ADDRINT rtn_address = RTN_Address(rtn);
    ADDRINT rtn_end_address;
    const char* rtn_name = RTN_Name(rtn).c_str();

    /* RTN_Id returns a unique global identifier. However, the numeration is not necessarily consecutive
       so we keep our own numeration for convenience (and legacy) - see comment below. */
    UINT32 rtn_num = RTN_Id(rtn);
    UINT32 img_num = IMG_Id(SEC_Img(RTN_Sec(rtn)));

    /* The pair <img_id, rtn_id> is used to index a map of global routine
       numbers.  The function is stored at the global routine number in the
       rtn_table.  The basic blocks are tagged with the same number so they
       can be reassociated later for printing.  */
    static UINT32 next_rtn_num = 1;
    UINT32 global_rtn_num;
    typedef pair< UINT32, UINT32 > rtn_identifier_t;
    static map< rtn_identifier_t, UINT32 > rtn_mapper;
    rtn_identifier_t ri(img_num, rtn_num);
    PIN_GetLock(&locks.rtn_table_lock, tid + 2);
    map< rtn_identifier_t, UINT32 >::iterator it = rtn_mapper.find(ri);
    if (it == rtn_mapper.end())
    {
        rtn_mapper[ri] = next_rtn_num;
        global_rtn_num = next_rtn_num;
        next_rtn_num++;

        /* store the RTN info using the global routine number */

        if (global_rtn_num >= rtn_table.size()) rtn_table.resize(global_rtn_num * 2 + 1);
        rtn_end_address           = RTN_Address(rtn) + RTN_Size(rtn);
        rtn_table[global_rtn_num] = RTN_TABLE_ENTRY(rtn_address, rtn_end_address, strdup(rtn_name), global_rtn_num);
    }
    else
    {
        global_rtn_num = it->second;
    }
    PIN_ReleaseLock(&locks.rtn_table_lock);

    return global_rtn_num;
}

VOID Trace(TRACE trace, VOID* v)
{
    static UINT32 basic_blocks = 0;

    if (KnobNoSharedLibs.Value() && IMG_Type(SEC_Img(RTN_Sec(TRACE_Rtn(trace)))) == IMG_TYPE_SHAREDLIB) return;

    const BOOL accurate_handling_of_predicates = KnobProfilePredicated.Value();
    ADDRINT pc                                 = TRACE_Address(trace);

    RTN rtn       = TRACE_Rtn(trace);
    UINT32 rtn_id = routine_identifier(rtn, 1); /* fake tid for the instrumenting code */

    UINT32 new_blocks = 0;
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        const INS head = BBL_InsHead(bbl);
        if (!INS_Valid(head)) continue;
        new_blocks++;
    }

    TRACE_InsertCall(trace, IPOINT_BEFORE, AFUNPTR(validate_bbl_count), IARG_THREAD_ID, IARG_UINT32, basic_blocks + new_blocks,
                     IARG_END);

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        const INS head = BBL_InsHead(bbl);
        if (!INS_Valid(head)) continue;

        // Summarize the stats for the bbl in a 0 terminated list
        // This is done at instrumentation time
        const UINT32 n = IndexStringLength(bbl, 1);

        ADDRINT block_start_pc = pc;

        // stats is an array of index types. We later multiply it by the
        // dynamic count for a block.
        stat_index_t* const stats     = new stat_index_t[n + 1];
        stat_index_t* const stats_end = stats + (n + 1);
        stat_index_t* curr            = stats;
        UINT32 ninsts                 = 0;
        for (INS ins = head; INS_Valid(ins); ins = INS_Next(ins))
        {
            unsigned int instruction_size = INS_Size(ins);
            // This checks for x86-specific opcodes
            CheckForSpecialMarkers(ins, pc, instruction_size);

            // Count the number of times a predicated instruction is actually executed
            // this is expensive and hence disabled by default
            if (INS_IsPredicated(ins) && accurate_handling_of_predicates)
            {
                INS_InsertPredicatedCall(ins, IPOINT_BEFORE, AFUNPTR(docount_predicated_true), IARG_UINT32, INS_GetIndex(ins),
                                         IARG_THREAD_ID, IARG_END);
            }

            if (KnobMapToFile)
            {
                INT32 line;
                string filename;
                PIN_GetSourceLocation(pc, NULL, &line, &filename);
                if (!filename.empty()) *out << "MAPADDR 0x" << hex << pc << " " << dec << line << " " << filename << endl;
            }
            curr = INS_GenerateIndexString(ins, curr, 1);
            pc   = pc + instruction_size;
            ninsts++;
        }

        // stats terminator
        *curr++ = 0;
        ASSERTX(curr == stats_end);

        // Insert instrumentation to count the number of times the bbl is executed
        BBLSTATS* bblstats = new BBLSTATS(stats, block_start_pc, ninsts, pc - block_start_pc, rtn_id);
        INS_InsertCall(head, IPOINT_BEFORE, AFUNPTR(docount_bbl), IARG_FAST_ANALYSIS_CALL, IARG_UINT32, basic_blocks,
                       IARG_THREAD_ID, IARG_END);

        // Remember the counter and stats so we can compute a summary at the end
        basic_blocks++;
        PIN_GetLock(&locks.bbl_list_lock, 1);
        statsList.push_back(bblstats);
        PIN_ReleaseLock(&locks.bbl_list_lock);
    }
}

/* ===================================================================== */
VOID DumpStats(ofstream& out, CSTATS& stats, BOOL predicated_true, const string& title, THREADID tid)
{
    out << "#\n# " << title << "\n#\n";
    if (tid != INVALID_THREADID) out << "# TID " << tid << "\n";
    out << "# ";
    const char* label = 0;
    if (measurement == measure_opcode)
        label = "opcode";
    else if (measurement == measure_ilen)
        label = "inslen";
    else if (measurement == measure_category)
        label = "category";
    else if (measurement == measure_iform)
        label = "iform";

    if (label) out << ljstr(label, 24);

    out << setw(16) << "count";
    if (predicated_true) out << "    count-predicated-true";
    out << "\n#\n";

    // Compute the "total" bin. Stop at the INDEX_SPECIAL for all histograms
    // except the iform. Iforms do not use the special rows, so we count everything.

    // build a map of the valid stats index values for all 3 tables.
    map< UINT32, bool > m;

    COUNTER tu = 0;
    for (stat_map_t::iterator it = stats.unpredicated.begin(); it != stats.unpredicated.end(); it++)
    {
        if (measurement == measure_iform || it->first < INDEX_SPECIAL) tu += it->second;
        m[it->first] = true;
    }

    COUNTER tpt = 0;
    for (stat_map_t::iterator it = stats.predicated_true.begin(); it != stats.predicated_true.end(); it++)
    {
        if (measurement == measure_iform || it->first < INDEX_SPECIAL) tpt += it->second;
        m[it->first] = true;
    }

    for (map< UINT32, bool >::iterator it = m.begin(); it != m.end(); it++)
    {
        stat_map_t::iterator s;
        COUNTER up  = 0;
        UINT32 indx = it->first;

        s = stats.unpredicated.find(indx);
        if (s != stats.unpredicated.end()) up = s->second;

        if (up == 0) continue;

        out << ljstr(IndexToString(indx), 25) << " " << setw(16) << up;
        if (predicated_true)
        {
            COUNTER prt = 0;
            s           = stats.predicated_true.find(indx);
            if (s != stats.predicated_true.end())
            {
                prt = s->second;
                out << " " << setw(16) << prt;
            }
        }
        out << endl;
    }

    // print the totals
    out << ljstr("*total", 25) << " " << setw(16) << tu;
    if (predicated_true) out << " " << setw(16) << tpt;
    out << endl;
}

/* ===================================================================== */
static UINT32 stat_dump_count = 0;

VOID emit_bbl_stats_by_function(THREADID tid)
{
    // dynamic counts for one thread and for each function of that thread.

    thread_data_t* tdata = get_tls(tid);

    // grab a copy of the routines list that we can sort and where we can
    // compute function totals. Need to lock because instrumentation might
    // reallocate the vector.
    PIN_GetLock(&locks.rtn_table_lock, tid + 2);
    UINT32 functions                  = rtn_table.size();
    RTN_TABLE_ENTRY* rtn_table_sorted = new RTN_TABLE_ENTRY[functions];
    for (UINT32 i = 0; i < functions; i++)
    {
        rtn_table_sorted[i] = rtn_table[i];
    }
    PIN_ReleaseLock(&locks.rtn_table_lock);

    // Need to lock here because we might be resize (and thus reallocing)
    // the statsList when we do a push_back in the instrumentation.
    PIN_GetLock(&locks.bbl_list_lock, tid + 2);

    if (tdata->stats_per_function.size() < functions) tdata->stats_per_function.resize(functions + 1);

    UINT32 limit = tdata->size();
    if (limit > statsList.size()) limit = statsList.size();
    for (UINT32 i = 0; i < limit; i++)
    {
        COUNTER bcount = tdata->block_counts[i];
        BBLSTATS* b    = statsList[i];
        /* the last test below is for when new bbl's get jitted while we
         * are emitting stats */
        if (b && b->_stats && b->_rtn_num < functions)
            for (const stat_index_t* stats = b->_stats; *stats; stats++)
            {
                tdata->cstats.unpredicated[*stats] += bcount;
                //*out << "# stat for block " << b->_rtn_num << endl;
                tdata->stats_per_function[b->_rtn_num].unpredicated[*stats] += bcount;
                if (*stats < INDEX_SPECIAL) rtn_table_sorted[b->_rtn_num]._total += bcount;
            }
    }

    PIN_ReleaseLock(&locks.bbl_list_lock);

    // emit the "normal" dynamic stats

    *out << "# EMIT_DYNAMIC_STATS FOR TID " << tid << " EMIT #" << stat_dump_count << endl;
    DumpStats(*out, tdata->cstats, KnobProfilePredicated, "$dynamic-counts", tid);
    *out << "# END_DYNAMIC_STATS" << endl;

    // sort the routines by the total instr count
    qsort(rtn_table_sorted, functions, sizeof(RTN_TABLE_ENTRY), qsort_rtn_compare_fn);

    // total every thing up for all functions in this thread
    UINT64 total = 0;
    for (UINT32 i = 0; i < functions; i++)
        if (rtn_table_sorted[i]._address && rtn_table_sorted[i]._total) total += rtn_table_sorted[i]._total;

    // print the functions out per thread, sorted order
    double cumulative = 0;
    *out << "# FUNCTION TOTALS FOR TID " << tid << endl;
    *out << "#rank        total     %   cumulative%      address  function name" << endl;
    for (UINT32 i = 0; i < functions; i++)
    {
        if (rtn_table_sorted[i]._address && rtn_table_sorted[i]._total)
        {
            cumulative += rtn_table_sorted[i]._total; // FP GUARD
            *out << setw(4) << i << ": " << setw(12) << rtn_table_sorted[i]._total << " "
                 << fltstr(100.0 * rtn_table_sorted[i]._total / total, 3, 7) //FP GUARD
                 << " " << fltstr(100.0 * cumulative / total, 3, 7)          //FP GUARD
                 << "  " << hex << setw(16) << rtn_table_sorted[i]._address << dec << " ";
            if (rtn_table_sorted[i]._name)
                *out << rtn_table_sorted[i]._name;
            else
                *out << "UNKNOWN";
            *out << endl;
        }
    }
    *out << setw(4) << " "
         << "  " << setw(12) << total << " "
         << "TOTAL" << endl;
    *out << "# END FUNCTION TOTALS" << endl;

    // print the functions histos for the nonempty functions
    *out << "# EMIT_PER_FUNCTION_STATS FOR TID " << tid << " EMIT# " << stat_dump_count << endl;
    for (UINT32 i = 0; i < functions; i++)
    {
        if (rtn_table_sorted[i]._address && rtn_table_sorted[i]._total)
        {
            string title = "$dynamic-counts-for-function: ";
            if (rtn_table_sorted[i]._name)
                title += rtn_table_sorted[i]._name;
            else
                title += "UNKNOWN";
            /* {x|a <= x < b}, or [a,b) */
            title += " at [" + hexstr(rtn_table_sorted[i]._address) + ", ";
            title += hexstr(rtn_table_sorted[i]._end_address) + ")"; /*  right endpoint is open interval */
            double pct = 100.0 * rtn_table_sorted[i]._total / total; // FP GUARD
            title += " " + fltstr(pct, 3, 7) + "%";
            // we sorted, so get the original routine number
            UINT32 rtn_num = rtn_table_sorted[i]._rtn_num;
            DumpStats(*out, tdata->stats_per_function[rtn_num], KnobProfilePredicated, title, tid);
        }
    }
    *out << "# END_PER_FUNCTION_STATS " << endl;
    delete[] rtn_table_sorted;
}

int qsort_compare_fn(const void* a, const void* b)
{
    const BBL_SORT_STATS* ba = static_cast< const BBL_SORT_STATS* >(a);
    const BBL_SORT_STATS* bb = static_cast< const BBL_SORT_STATS* >(b);
    if (bb->_icount > ba->_icount) return 1;
    if (bb->_icount < ba->_icount) return -1;
    return 0;
}

VOID emit_bbl_stats_sorted(THREADID tid)
{
    /* emit the top blocks for this tid */

    thread_data_t* tdata = get_tls(tid);
    // dynamic Counts

    // Need to lock here because we might be resize (and thus reallocing)
    // the statsList when we do a push_back in the instrumentation.
    PIN_GetLock(&locks.bbl_list_lock, tid + 2);
    UINT32 limit = tdata->size();
    if (limit > statsList.size()) limit = statsList.size();
    BBL_SORT_STATS* icounts = new BBL_SORT_STATS[limit];
    COUNTER thread_total    = 0;
    for (UINT32 i = 0; i < limit; i++)
    {
        BBLSTATS* b = statsList[i];
        if (b)
        {
            COUNTER bcount = tdata->block_counts[i];
            COUNTER x      = b->_ninst;
            x              = x * bcount;
            //*out << hex << "ALL PC: " << b->_pc << " COUNT: " << x << endl;
            icounts[i]._icount     = x;
            icounts[i]._pc         = b->_pc;
            icounts[i]._rtn_num    = b->_rtn_num;
            icounts[i]._executions = bcount;
            icounts[i]._nbytes     = b->_nbytes;
            thread_total += icounts[i]._icount;
        }
    }
    PIN_ReleaseLock(&locks.bbl_list_lock);

    qsort(icounts, limit, sizeof(BBL_SORT_STATS), qsort_compare_fn);

    PIN_GetLock(&locks.rtn_table_lock, tid + 2);

    *out << "# EMIT_TOP_BLOCK_STATS FOR TID " << tid << " EMIT # " << stat_dump_count << endl;
    if (limit > KnobTopBlocks.Value()) limit = KnobTopBlocks.Value();
    COUNTER t = 0;
    for (UINT32 i = 0; i < limit; i++)
    {
        t += icounts[i]._icount;
        *out << "BLOCK: " << setw(5) << i << "   PC: " << hex << setfill('0') << setw(sizeof(ADDRINT) * 2) << icounts[i]._pc
             << setfill(' ') << dec << "   ICOUNT: " << setw(9) << icounts[i]._icount << "   EXECUTIONS: " << setw(9)
             << icounts[i]._executions << "   #BYTES: " << setw(2) << icounts[i]._nbytes << "   %: " << setw(5) << setprecision(3)
             << 100.0 * icounts[i]._icount / thread_total                                 // FP GUARD
             << "   cumltv%: " << setw(5) << setprecision(3) << 100.0 * t / thread_total; // FP GUARD
        UINT32 idx = icounts[i]._rtn_num;

        if (rtn_table.size() > idx)
            if (rtn_table[idx]._name) *out << "  FN: " << rtn_table[idx]._name;
        *out << endl;
        if (KnobShowDisassembly)
        {
            string s = disassemble(icounts[i]._pc, icounts[i]._pc + icounts[i]._nbytes);
            *out << s << endl;
        }
    }

    *out << "# END_TOP_BLOCK_STATS" << endl;
    PIN_ReleaseLock(&locks.rtn_table_lock);
    delete[] icounts;
}

VOID emit_static_stats()
{
    *out << "# EMIT_STATIC_STATS " << endl;
    DumpStats(*out, GlobalStatsStatic, false, "$static-counts", INVALID_THREADID);
    *out << endl << "# END_STATIC_STATS" << endl;
}

VOID emit_pc_stats(THREADID tid)
{
    thread_data_t* tdata = get_tls(tid);
    // dynamic Counts

    // Need to lock here because we might be resize (and thus reallocing)
    // the statsList when we do a push_back in the instrumentation.

    *out << "# EMIT_PC_STATS FOR TID " << tid << endl;
    PIN_GetLock(&locks.bbl_list_lock, tid + 2);
    UINT32 limit = tdata->size();
    if (limit > statsList.size()) limit = statsList.size();
    for (UINT32 i = 0; i < limit; i++)
    {
        COUNTER bcount = tdata->block_counts[i];
        BBLSTATS* b    = statsList[i];
        if (bcount && b && b->_stats) *out << "BLOCKCOUNT 0x" << hex << b->_pc << " " << dec << (bcount * b->_ninst) << endl;
    }
    PIN_ReleaseLock(&locks.bbl_list_lock);
    *out << "# END_EMIT_PC_STATS FOR TID " << tid << endl;
}

VOID emit_stats(THREADID tid)
{
    /* we must save the X87 state because we do floating point computation
     * when doing output and on 32b platforms, that can use x87. Pin does
     * not save the X87 state, so analysis routines must do it
     * themselves. */
    mix_fp_save_buffer_t save_buf;
    unsigned char* p = MIX_FP_ALIGN(&save_buf); /* make sure it is 16B aligned */
    MIX_FP_SAVE(p);

    PIN_GetLock(&locks.lock, tid + 2); // for output
    stat_dump_count++;
    *out << "# ==============================================" << endl;
    *out << "# STATS FOR TID " << tid << " EMIT# " << stat_dump_count << endl;
    *out << "# ==============================================" << endl;
    emit_bbl_stats_sorted(tid);      // top blocks
    emit_bbl_stats_by_function(tid); // function level stats
    if (KnobMapToFile) emit_pc_stats(tid);
    PIN_ReleaseLock(&locks.lock);
    MIX_FP_RELOAD(p);
}

/* ===================================================================== */

void combine_dynamic_stats(unsigned int numThreads)
{
    // combine all the rows from each thread in to the total variable.
    CSTATS total;
    for (THREADID i = 0; i < numThreads; i++)
    {
        thread_data_t* tdata = get_tls(i);

        for (stat_map_t::iterator it = tdata->cstats.unpredicated.begin(); it != tdata->cstats.unpredicated.end(); it++)
        {
            stat_map_t::iterator x = total.unpredicated.find(it->first);
            if (x == total.unpredicated.end())
                total.unpredicated[it->first] = it->second;
            else
                x->second += it->second;
        }

        for (stat_map_t::iterator it = tdata->cstats.predicated.begin(); it != tdata->cstats.predicated.end(); it++)
        {
            stat_map_t::iterator x = total.predicated.find(it->first);
            if (x == total.predicated.end())
                total.predicated[it->first] = it->second;
            else
                x->second += it->second;
        }

        for (stat_map_t::iterator it = tdata->cstats.predicated_true.begin(); it != tdata->cstats.predicated_true.end(); it++)
        {
            stat_map_t::iterator x = total.predicated_true.find(it->first);
            if (x == total.predicated_true.end())
                total.predicated_true[it->first] = it->second;
            else
                x->second += it->second;
        }
    }

    *out << "# EMIT_GLOBAL_DYNAMIC_STATS   EMIT# " << stat_dump_count << endl;
    DumpStats(*out, total, false, "$global-dynamic-counts", INVALID_THREADID);
    *out << endl << "# END_GLOBAL_DYNAMIC_STATS" << endl;
}

VOID Fini(int, VOID* v) // only runs once for the application
{
    *out << "# FINI: end of program" << endl;
    for (unsigned int i = 0; i < numThreads; i++)
        emit_stats(i);
    emit_static_stats();
    combine_dynamic_stats(numThreads);

    out->close();
}

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
                stat_index_t array[256];
                stat_index_t* mid = INS_GenerateIndexString(ins, array, 1);
                stat_index_t* end = mid;

                if (INS_IsPredicated(ins))
                {
                    for (stat_index_t* start = array; start < end; start++)
                    {
                        GlobalStatsStatic.predicated[*start]++;
                    }
                }
                else
                {
                    for (stat_index_t* start = array; start < end; start++)
                    {
                        GlobalStatsStatic.unpredicated[*start]++;
                    }
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

/////////////////////////////////////////////////////////////////////////
// Add a disassembler
/////////////////////////////////////////////////////////////////////////

static char nibble_to_ascii_hex(UINT8 i)
{
    if (i < 10) return i + '0';
    if (i < 16) return i - 10 + 'A';
    return '?';
}

static void print_hex_line(char* buf, const UINT8* array, const int length)
{
    int n = length;
    int i = 0;
    if (length == 0) n = XED_MAX_INSTRUCTION_BYTES;
    for (i = 0; i < n; i++)
    {
        buf[2 * i + 0] = nibble_to_ascii_hex(array[i] >> 4);
        buf[2 * i + 1] = nibble_to_ascii_hex(array[i] & 0xF);
    }
    buf[2 * i] = 0;
}

static string disassemble(UINT64 start, UINT64 stop)
{
    UINT64 pc = start;
    xed_state_t dstate;
    xed_syntax_enum_t syntax = XED_SYNTAX_INTEL;
    xed_error_enum_t xed_error;
    xed_decoded_inst_t xedd;
    ostringstream os;
    if (sizeof(ADDRINT) == 4)
        xed_state_init(&dstate, XED_MACHINE_MODE_LEGACY_32, XED_ADDRESS_WIDTH_32b, XED_ADDRESS_WIDTH_32b);
    else
        xed_state_init(&dstate, XED_MACHINE_MODE_LONG_64, XED_ADDRESS_WIDTH_64b, XED_ADDRESS_WIDTH_64b);

    while (pc < stop)
    {
        xed_decoded_inst_zero_set_mode(&xedd, &dstate);
        UINT32 len = 15;
        if (stop - pc < 15) len = stop - pc;

        xed_error              = xed_decode(&xedd, reinterpret_cast< const UINT8* >(pc), len);
        bool okay              = (xed_error == XED_ERROR_NONE);
        iostream::fmtflags fmt = os.flags();
        os << std::setfill('0') << "XDIS " << std::hex << std::setw(sizeof(ADDRINT) * 2) << pc << std::dec << ": "
           << std::setfill(' ') << std::setw(4);

        if (okay)
        {
            char buffer[200];
            unsigned int dec_len, sp;

            os << xed_extension_enum_t2str(xed_decoded_inst_get_extension(&xedd));
            dec_len = xed_decoded_inst_get_length(&xedd);
            print_hex_line(buffer, reinterpret_cast< UINT8* >(pc), dec_len);
            os << " " << buffer;
            for (sp = dec_len; sp < 12; sp++) // pad out the instruction bytes
                os << "  ";
            os << " ";
            memset(buffer, 0, 200);
            int dis_okay = xed_format_context(syntax, &xedd, buffer, 200, pc, 0, 0);
            if (dis_okay)
                os << buffer << endl;
            else
                os << "Error disasassembling pc 0x" << std::hex << pc << std::dec << endl;
            pc += dec_len;
        }
        else
        { // print the byte and keep going.
            UINT8 memval = *reinterpret_cast< UINT8* >(pc);
            os << "???? " // no extension
               << std::hex << std::setw(2) << std::setfill('0') << static_cast< UINT32 >(memval) << std::endl;
            pc += 1;
        }
        os.flags(fmt);
    }
    return os.str();
}

/* ===================================================================== */

int main(int argc, CHAR** argv)
{
    PIN_InitSymbols();
    if (PIN_Init(argc, argv)) return Usage();

    PIN_InitLock(&locks.lock);
    PIN_InitLock(&locks.bbl_list_lock);
    PIN_InitLock(&locks.rtn_table_lock);

    // obtain  a key for TLS storage
    tls_key = PIN_CreateThreadDataKey(0);

    string filename = KnobOutputFile.Value();
    if (KnobPid)
    {
        filename += "." + decstr(getpid());
    }
    out = new std::ofstream(filename.c_str());
    *out << "# Mix output version 2" << endl;
    control.RegisterHandler(Handler, 0, FALSE);
    control.Activate();

    // make sure that exactly one thing-to-count knob is specified.
    if (KnobInstructionLengthMix.Value() && KnobCategoryMix.Value())
    {
        cerr << "Must have at most  one of: -iform, -ilen or -category "
             << "as a pintool option" << endl;
        exit(1);
    }
    if (KnobInstructionLengthMix.Value()) measurement = measure_ilen;
    if (KnobCategoryMix.Value()) measurement = measure_category;
    if (KnobIformMix.Value())
    {
        measurement = measure_iform;
    }

    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddFiniFunction(Fini, 0);

    if (!KnobProfileDynamicOnly.Value()) IMG_AddInstrumentFunction(Image, 0);

    PIN_StartProgram(); // Never returns
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
