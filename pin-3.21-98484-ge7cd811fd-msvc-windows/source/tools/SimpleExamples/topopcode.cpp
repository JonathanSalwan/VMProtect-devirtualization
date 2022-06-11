/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This file contains a real time  opcode  mix profiler
 */

#include <map>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm> // for sort
#include <vector>
#include "pin.H"
using std::cerr;
using std::endl;
using std::flush;
using std::pair;
using std::setw;
using std::string;
using std::vector;

#if defined(TARGET_IA32E)
extern "C" void fxsaveWrap(char*);
extern "C" void fxrstorWrap(char*);
#endif

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "opcodetop.out", "specify output file name");

KNOB< UINT64 > KnobThreshold(KNOB_MODE_WRITEONCE, "pintool", "t", "10000", "print histogram after every n BBLs");

KNOB< UINT64 > KnobCutoff(KNOB_MODE_WRITEONCE, "pintool", "c", "100", "minimum count for opcode to show");

KNOB< UINT64 > KnobMaxLines(KNOB_MODE_WRITEONCE, "pintool", "l", "24", "max number of output lines");

KNOB< UINT64 > KnobDetachUpdates(KNOB_MODE_WRITEONCE, "pintool", "d", "0", "detach after n screen updates");

KNOB< FLT64 > KnobDecayFactor(KNOB_MODE_WRITEONCE, "pintool", "f", "0.0", "x");

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool provides a real time  opcode mix profile\n"
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
const UINT32 MAX_MEM_SIZE  = 512;

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

/* ===================================================================== */

static UINT32 INS_GetIndex(INS ins) { return INS_Opcode(ins); }

/* ===================================================================== */

static UINT32 IndexStringLength(BBL bbl, BOOL memory_acess_profile)
{
    UINT32 count = 0;

    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
    {
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
static UINT32 MemsizeToIndex(UINT32 size, BOOL write) { return (write ? INDEX_MEM_WRITE_SIZE : INDEX_MEM_READ_SIZE) + size; }

/* ===================================================================== */
static UINT16* INS_GenerateIndexString(INS ins, UINT16* stats, BOOL memory_acess_profile)
{
    *stats++ = INS_GetIndex(ins);

    if (memory_acess_profile)
    {
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

        if (readOperandCount > 0) *stats++ = MemsizeToIndex(readSize, 0);
        if (writeOperandCount > 0) *stats++ = MemsizeToIndex(writeSize, 1);

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

    VOID Clear(FLT64 factor)
    {
        for (UINT32 i = 0; i < MAX_INDEX; i++)
        {
            unpredicated[i] = COUNTER(unpredicated[i] * factor);
        }
    }
};

STATS GlobalStats;

class BBLSTATS
{
  public:
    COUNTER _counter;
    const UINT16* const _stats;

  public:
    BBLSTATS(UINT16* stats) : _counter(0), _stats(stats) {};
};

static vector< BBLSTATS* > statsList;

/* ===================================================================== */

static UINT64 bbl_counter = 0;
static UINT64 updates     = 0;

/* ===================================================================== */

typedef pair< UINT32, UINT64 > PAIR;
typedef vector< PAIR > VEC;

/* ===================================================================== */

static BOOL CompareLess(PAIR s1, PAIR s2) { return s1.second > s2.second; }

/* ===================================================================== */
VOID DumpHistogram(std::ostream& out)
{
    const UINT64 cutoff   = KnobCutoff.Value();
    const UINT64 maxlines = KnobMaxLines.Value();

    out << "\033[0;0H";
    out << "\033[2J";
    out << "\033[44m";
    out << ljstr("OPCODE", 15) << " " << setw(16) << "COUNT";
    out << "\033[0m";
    out << endl;

    for (vector< BBLSTATS* >::iterator bi = statsList.begin(); bi != statsList.end(); bi++)
    {
        BBLSTATS* b = (*bi);

        for (const UINT16* stats = b->_stats; *stats; stats++)
        {
            GlobalStats.unpredicated[*stats] += b->_counter;
        }
        b->_counter = 0;
    }

    COUNTER total = 0;
    VEC CountMap;

    for (UINT32 index = 0; index <= INDEX_SPECIAL_END; index++)
    {
        total += GlobalStats.unpredicated[index];
        if (GlobalStats.unpredicated[index] < cutoff) continue;

        CountMap.push_back(PAIR(index, GlobalStats.unpredicated[index]));
    }
    CountMap.push_back(PAIR(INDEX_TOTAL, total));

    sort(CountMap.begin(), CountMap.end(), CompareLess);
    UINT32 lines = 0;
    for (VEC::iterator bi = CountMap.begin(); bi != CountMap.end(); bi++)
    {
        UINT32 i = bi->first;
        //        out << setw(4) << i << " " <<  ljstr(IndexToOpcodeString(i),15) << " " <<
        out << ljstr(IndexToOpcodeString(i), 15) << " " << setw(16) << bi->second << endl;
        lines++;
        if (lines >= maxlines) break;
    }
}
/* ===================================================================== */

std::ofstream Out;

/* ===================================================================== */

VOID docount(COUNTER* counter)
{
    (*counter)++;

    if (bbl_counter == 0)
    {
#if defined(TARGET_IA32)
        static char buffer[512 + 16];
        static char* aligned_bufp = reinterpret_cast< char* >(((reinterpret_cast< ADDRINT >(buffer) + 16) >> 4) << 4);
#if defined(PIN_GNU_COMPATIBLE)
        asm("fxsave %0" : "=m"(*aligned_bufp));
#else
        __asm {
            push eax

            mov  eax, aligned_bufp
            fxsave [eax]

            pop eax
        }
#endif
#endif

#if defined(TARGET_IA32E)
        static char buffer[512 + 16];
        static char* aligned_bufp = reinterpret_cast< char* >(((reinterpret_cast< ADDRINT >(buffer) + 16) >> 4) << 4);
#if defined(PIN_GNU_COMPATIBLE)
        asm("fxsave %0" : "=m"(*aligned_bufp));
#else
        fxsaveWrap(aligned_bufp);
#endif
#endif

        DumpHistogram(Out);
        Out << flush;

        FLT64 factor = KnobDecayFactor.Value();
        GlobalStats.Clear(factor);

        bbl_counter = KnobThreshold.Value();
        updates++;
        if (updates == KnobDetachUpdates.Value())
        {
            PIN_Detach();
        }

#if defined(TARGET_IA32)
#if defined(PIN_GNU_COMPATIBLE)
        asm("fxrstor %0" ::"m"(*aligned_bufp));
#else
        __asm {
                    push eax

                    mov  eax, aligned_bufp
                    fxrstor [eax]

                    pop eax
                }
#endif
#endif

#if defined(TARGET_IA32E)
#if defined(PIN_GNU_COMPATIBLE)
        asm("fxrstor %0" ::"m"(*aligned_bufp));
#else
        fxrstorWrap(aligned_bufp);
#endif
#endif
    }

    bbl_counter--;
}

/* ===================================================================== */

VOID Trace(TRACE trace, VOID* v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        const INS head = BBL_InsHead(bbl);
        if (!INS_Valid(head)) continue;

        // Summarize the stats for the bbl in a 0 terminated list
        // This is done at instrumentation time
        const UINT32 n = IndexStringLength(bbl, 1);

        UINT16* const stats     = new UINT16[n + 1];
        UINT16* const stats_end = stats + (n + 1);
        UINT16* curr            = stats;

        for (INS ins = head; INS_Valid(ins); ins = INS_Next(ins))
        {
            curr = INS_GenerateIndexString(ins, curr, 1);
        }

        // string terminator
        *curr++ = 0;

        ASSERTX(curr == stats_end);

        // Insert instrumentation to count the number of times the bbl is executed
        BBLSTATS* bblstats = new BBLSTATS(stats);
        INS_InsertCall(head, IPOINT_BEFORE, AFUNPTR(docount), IARG_PTR, &(bblstats->_counter), IARG_END);

        // Remember the counter and stats so we can compute a summary at the end
        statsList.push_back(bblstats);
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

    Out.open(KnobOutputFile.Value().c_str());
    TRACE_AddInstrumentFunction(Trace, 0);

    // Never returns

    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
