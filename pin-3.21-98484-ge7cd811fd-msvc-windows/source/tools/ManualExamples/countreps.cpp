/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Count REP prefixed instructions and their repeat counts.
 *
 * This tool demonstrates how to optimize common REP cases, where the
 * repeat count is known at the start of the REP instruction execution
 * (e.g. STOS and MOVS). Often these are the most commonly executed
 * REP operations (you can use this tool to investigate!), so
 * optimizing them and using he simpler instrumentation for the
 * conditional REPs (SCAS, CMPS) is sufficient.
 *
 * Using the -slow flag you can see how  much slower the unoptimized
 * instrumentation is.
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <iomanip>
using std::cerr;
using std::endl;
using std::ofstream;
using std::string;

static KNOB< string > KnobOutput(KNOB_MODE_WRITEONCE, "pintool", "o", "countreps.out", "output file");
static KNOB< BOOL > KnobCountMemory(KNOB_MODE_WRITEONCE, "pintool", "memory", "0", "count memory operations");
static KNOB< BOOL > KnobSlow(KNOB_MODE_WRITEONCE, "pintool", "slow", "0", "use simple (but slow) instrumentation");
static KNOB< BOOL > KnobAddresses(KNOB_MODE_WRITEONCE, "pintool", "address", "0", "log addresses accessed in first 1000 REP ops");
static ofstream out;

THREADID myThread = INVALID_THREADID;

#define STRINGIZE(a) #a

struct opInfo
{
    const char* name; /* Instruction name (we could get it from XED, but we nearly have it anyway) */
    UINT32 opcode;    /* Opcode enumeration from XED */
    UINT32 reads;     /* Number of reads per iteration */
    UINT32 writes;    /* Number of writes per iteration */
    UINT32 size;      /* Size of the memory access(es) at each iteration */
};

// Expand the names and properties of an instruction for all possible widths.
#define EXPAND_OPCODE(op, r, w)                                                                        \
    {STRINGIZE(op##B), XED_ICLASS_##op##B, r, w, 1}, {STRINGIZE(op##W), XED_ICLASS_##op##W, r, w, 2},  \
                                                      {STRINGIZE(op##D), XED_ICLASS_##op##D, r, w, 4}, \
    {                                                                                                  \
        STRINGIZE(op##Q), XED_ICLASS_##op##Q, r, w, 8                                                  \
    }

// Instructions which can be REP prefixed (we ignore I/O operations!)  We
// encode knowledge of the number of reads and writes each op performs
// here. We could determine this dynamically from INS_IsMemoryRead,
// INS_HasMemoryRead2, INS_IsMemoryWrite, but since we're special
// casing these instructions anyway, we may as well just use our
// knowledge. (Code for doing it the general way is in instrumentTrace,
// where we don't know which instructions we're dealing with).
//
// Order here matters, we test specifically for CMPS and SCAS based
// on their position in this table...
static const opInfo opcodes[] = {
    EXPAND_OPCODE(CMPS, 2, 0), /* two reads, no writes */
    EXPAND_OPCODE(SCAS, 1, 0), /* one read,  no writes */
    EXPAND_OPCODE(MOVS, 1, 1), /* one read,  one write */
    EXPAND_OPCODE(STOS, 0, 1), /* no reads,  one write */
    EXPAND_OPCODE(LODS, 1, 0), /* one read,  no writes */
};

#define NumOps (5 * 4) /* Five instructions times four lengths */

// Does the instrution have a REPZ/REPNZ prefix, or is the length solely determined by
// the value of the count register?
//
// If KnobSlow has been asserted we pretend that we have to work the slow way with
// all instructions so that we can measure the benefit of being smarter...
static BOOL takesConditionalRep(UINT32 opIdx)
{
    if (KnobSlow) return TRUE; /* Do everything the simple, slow, way */

    return opIdx < (2 * 4); /* CMPS and SCAS are the first two sets of instructions */
}

// Convert an opcode into an index in our tables.
static UINT32 opcodeIndex(UINT32 opcode)
{
    for (UINT32 i = 0; i < NumOps; i++)
        if (opcodes[i].opcode == opcode) return i;

    ASSERT(FALSE, "Missing instruction " + decstr(opcode) + "\n");
    return 0;
}

// Formatting
enum
{
    fieldWidth = 16
};

// Dynamic count of all instructions.
static UINT64 totalCount = 0;

// Counts of memory operations.
class memoryStats
{
    UINT64 reads;
    UINT64 writes;

  public:
    memoryStats() : reads(0), writes(0) {}
    VOID add(UINT32 r, UINT32 w)
    {
        reads += r;
        writes += w;
    }
    VOID output() const;
    BOOL empty() const { return reads == 0 && writes == 0; }
    memoryStats& operator+=(const memoryStats& other)
    {
        reads += other.reads;
        writes += other.writes;
        return *this;
    }
    UINT64 allOps() const { return reads + writes; }
};

static memoryStats totalMemoryOps;
static memoryStats memOps[NumOps];

VOID memoryStats::output() const
{
    out << std::setw(fieldWidth) << reads;
    out << std::setw(8) << std::fixed << std::setprecision(1) << (100.0 * reads) / totalMemoryOps.reads;
    out << std::setw(fieldWidth) << writes;
    out << std::setw(8) << std::fixed << std::setprecision(1) << (100.0 * writes) / totalMemoryOps.writes;
    out << endl;
}

class stats
{
    UINT64 count;         /* Times we start the REP prefixed op */
    UINT64 repeatedCount; /* Times we execute the inner instruction */
    UINT64 zeroLength;    /* Times we start but don't execute the inner instruction because count is zero */
  public:
    stats() : count(0), repeatedCount(0), zeroLength(0) {}
    VOID output() const;
    VOID add(UINT32 firstRep, UINT32 repCount)
    {
        count += firstRep;
        repeatedCount += repCount;
        if (repCount == 0) zeroLength += 1;
    }
    BOOL empty() const { return count == 0; }
    stats& operator+=(const stats& other)
    {
        count += other.count;
        repeatedCount += other.repeatedCount;
        zeroLength += other.zeroLength;
        return *this;
    }
};

// Memory statistics
static stats statistics[NumOps];

VOID stats::output() const
{
    out << std::setw(fieldWidth) << count;
    out << std::setw(fieldWidth) << repeatedCount;
    out << std::setw(8) << std::fixed << std::setprecision(1) << (100.0 * repeatedCount) / totalCount;
    out << std::setw(fieldWidth) << zeroLength;
    if (zeroLength != count)
        out << std::setw(fieldWidth) << (repeatedCount / (count - zeroLength));
    else
        out << std::setw(fieldWidth) << 0; /* Avoid 0/0 calculation! */
    out << endl;
}

static VOID printDynamicIntructionCounts()
{
    out << endl << "Dynamic Instruction Count" << endl;
    out << "Total dynamic instructions: " << totalCount << endl << endl;

    out << "Op              Count  Repeated Count       %      Zero Count   Mean reps(NZ)" << endl;

    stats repTotal;

    for (UINT32 i = 0; i < NumOps; i++)
    {
        stats* s = &statistics[i];
        if (s->empty()) continue;
        out << opcodes[i].name;
        s->output();
        repTotal += *s;
    }
    out << endl;
    out << "REPS ";
    repTotal.output();
}

static VOID printMemoryAccessStats()
{
    memoryStats allReps;

    out << endl << endl;
    out << "Dynamic Memory Operation Count" << endl;
    out << "Memory Ops:     Reads       %          Writes       %" << endl;

    for (UINT32 i = 0; i < NumOps; i++)
    {
        memoryStats* s = &memOps[i];
        if (s->empty()) continue;
        out << opcodes[i].name;
        s->output();
        allReps += *s;
    }

    out << endl;
    out << "Prog ";
    totalMemoryOps.output();
    out << "REPS ";
    allReps.output();
    out << endl;
    out << "Reads+Writes" << endl;
    out << " Whole Program ";
    out << std::setw(fieldWidth) << totalMemoryOps.allOps() << endl;
    out << " REP prefixed  " << std::setw(fieldWidth) << allReps.allOps();
    out << " (" << std::fixed << std::setprecision(1) << (100.0 * allReps.allOps()) / totalMemoryOps.allOps() << "%)" << endl;
}

// Generic instrumentation to count all instructions.
static VOID addTotal(UINT32 delta) { totalCount += delta; }

// Analysis functions for counting memory operations.
static VOID addTotalMemops(UINT32 reads, UINT32 writes) { totalMemoryOps.add(reads, writes); }

static VOID InstrumentTrace(TRACE trace, VOID* v)
{
    // Visit every basic block in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        // Insert a call to addTotal somewhere in each bbl, passing the number of instructions
        // in the BBL.
        BBL_InsertCall(bbl, IPOINT_ANYWHERE, (AFUNPTR)addTotal, IARG_UINT32, BBL_NumIns(bbl), IARG_END);

        if (KnobCountMemory)
        {
            // Compute the number of memory accesses generated by the BBL
            UINT32 reads  = 0;
            UINT32 writes = 0;

            for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
            {
                if (INS_IsMemoryRead(ins)) reads++;
                if (INS_HasMemoryRead2(ins)) reads++;
                if (INS_IsMemoryWrite(ins)) writes++;
            }

            // If we have memory accesses, then add instrumentation to count them.
            if (reads != 0 || writes != 0)
            {
                BBL_InsertCall(bbl, IPOINT_ANYWHERE, (AFUNPTR)addTotalMemops, IARG_UINT32, reads, IARG_UINT32, writes, IARG_END);
            }
        }
    }
}

// Sanity check that the tool isn't being run in threaded code.
// (It would be relatively simple to make it work there, but it's all extra
// code which just confuses the educational points being made).
static VOID CheckThreadCount(THREADID threadIndex, CONTEXT*, INT32, VOID*)
{
    if (myThread == INVALID_THREADID)
    {
        myThread = threadIndex;
    }

#ifndef _WIN32
    ASSERT(threadIndex == myThread, "This tool does not handle multiple threads\n");
#endif
}

static VOID Fini(INT32 code, VOID* v)
{
    printDynamicIntructionCounts();
    if (KnobCountMemory)
    {
        printMemoryAccessStats();
    }
}

// Trivial analysis routine to pass its argument back in an IfCall so that we can use it
// to control the next piece of instrumentation.
static ADDRINT returnArg(BOOL arg) { return arg; }

// Analysis functions for execution counts.
// Analysis routine, FirstRep and Executing tell us the properties of the execution.
static VOID addCount(UINT32 opIdx, UINT32 firstRep, UINT32 repCount)
{
    stats* s = &statistics[opIdx];

    s->add(firstRep, repCount);
}

static VOID addMemops(UINT32 opcodeIdx, UINT32 repeats, UINT32 readsPerRep, UINT32 writesPerRep)
{
    memOps[opcodeIdx].add(repeats * readsPerRep, repeats * writesPerRep);
}

// Code for logging memory addresses accessed by REP prefixed instructions.
const UINT32 memoryOpsToLog   = 1000;
static UINT32 memoryOpsLogged = 0;

// Compute the base address of the whole access given the initial address,
// repeat count and element size. It has to adjust for DF if it is asserted.
static ADDRINT computeEA(ADDRINT firstEA, UINT32 eflags, UINT32 count, UINT32 elementSize)
{
    enum
    {
        DF_MASK = 0x0400
    };

    if (eflags & DF_MASK)
    {
        ADDRINT size = elementSize * count;

        return firstEA - size + elementSize; /* ops use post-decrement, so the lowest address is one elementSize above
                                                 * where you might think it is...
                                                 */
    }
    else
        return firstEA;
}

static VOID logMemoryAddress(UINT32 op, BOOL first, ADDRINT baseEA, ADDRINT count, UINT32 size, UINT32 eflags, ADDRINT tag)
{
    const char* tagString = reinterpret_cast< const char* >(tag);

    if ((memoryOpsLogged < memoryOpsToLog) && count != 0)
    {
        UINT32 width = 20;
        if (!first)
        {
            out << "  ";
            width -= 2;
        }
        out << opcodes[op].name << ' ' << tagString << ' ';
        out << std::hex << std::setw(width) << computeEA(baseEA, eflags, count, size) << ':';
        out << std::dec << std::setw(20) << size * count << endl;
        memoryOpsLogged += first;
    }
}

// Instrumentation routines.
// Insert code for counting how many times the instruction is executed
static VOID insertRepExecutionCountInstrumentation(INS ins, UINT32 opIdx)
{
    if (takesConditionalRep(opIdx))
    {
        // We have no smart way to lessen the number of
        // instrumentation calls because we can't determine when
        // the conditional instruction will finish.  So we just
        // let the instruction execute and have our
        // instrumentation be called on each iteration.  This is
        // the simplest way of handling REP prefixed instructions, where
        // each iteration appears as a separate instruction, and
        // is independently instrumented.
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)addCount, IARG_UINT32, opIdx, IARG_FIRST_REP_ITERATION, IARG_EXECUTING,
                       IARG_END);
    }
    else
    {
        // The number of iterations is determined solely by the count register value,
        // therefore we can log all we need at the start of each REP "loop", and skip the
        // instrumentation on all the other iterations of the REP prefixed operation. Simply use
        // IF/THEN instrumentation which tests IARG_FIRST_REP_ITERATION.
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)returnArg, IARG_FIRST_REP_ITERATION, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)addCount, IARG_UINT32, opIdx, IARG_UINT32, 1, IARG_REG_VALUE,
                           INS_RepCountRegister(ins), IARG_END);
    }
}

// Insert instrumentation to count memory operations
// The optimisations here are similar to those above.
static VOID insertRepMemoryCountInstrumentation(INS ins, UINT32 opIdx)
{
    const opInfo* op = &opcodes[opIdx];

    if (takesConditionalRep(opIdx))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)addMemops, IARG_UINT32, opIdx, IARG_EXECUTING, IARG_UINT32, op->reads,
                       IARG_UINT32, op->writes, IARG_END);
    }
    else
    {
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)returnArg, IARG_FIRST_REP_ITERATION, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)addMemops, IARG_UINT32, opIdx, IARG_REG_VALUE, INS_RepCountRegister(ins),
                           IARG_UINT32, op->reads, IARG_UINT32, op->writes, IARG_END);
    }
}

// Insert instrumentation to log memory addresses accessed.
static VOID insertRepMemoryTraceInstrumentation(INS ins, UINT32 opIdx)
{
    const opInfo* op = &opcodes[opIdx];

    if (takesConditionalRep(opIdx))
    {
        if (INS_IsMemoryRead(ins))
        {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)logMemoryAddress, IARG_UINT32, opIdx, IARG_FIRST_REP_ITERATION,
                           IARG_MEMORYREAD_EA, IARG_EXECUTING, IARG_UINT32, op->size, IARG_UINT32,
                           0, /* Fake Eflags, since we're called at each iteration it doesn't matter */
                           IARG_ADDRINT, (ADDRINT) "Read ", IARG_END);
        }
        if (INS_HasMemoryRead2(ins))
        {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)logMemoryAddress, IARG_UINT32, opIdx, IARG_FIRST_REP_ITERATION,
                           IARG_MEMORYREAD2_EA, IARG_EXECUTING, IARG_UINT32, op->size, IARG_UINT32,
                           0, /* Fake Eflags, since we're called at each iteration it doesn't matter */
                           IARG_ADDRINT, (ADDRINT) "Read2", IARG_END);
        }
        if (INS_IsMemoryWrite(ins))
        {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)logMemoryAddress, IARG_UINT32, opIdx, IARG_FIRST_REP_ITERATION,
                           IARG_MEMORYWRITE_EA, IARG_EXECUTING, IARG_UINT32, op->size, IARG_UINT32,
                           0, /* Fake Eflags, since we're called at each iteration it doesn't matter */
                           IARG_ADDRINT, (ADDRINT) "Write", IARG_END);
        }
    }
    else
    {
        if (INS_IsMemoryRead(ins))
        {
            INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)returnArg, IARG_FIRST_REP_ITERATION, IARG_END);
            INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)logMemoryAddress, IARG_UINT32, opIdx, IARG_BOOL,
                               TRUE, /* First must be one else we wouldn't be called */
                               IARG_MEMORYREAD_EA, IARG_REG_VALUE, INS_RepCountRegister(ins), IARG_UINT32, op->size,
                               IARG_REG_VALUE, REG_EFLAGS, IARG_ADDRINT, (ADDRINT) "Read ", IARG_END);
        }
        if (INS_HasMemoryRead2(ins))
        {
            INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)returnArg, IARG_FIRST_REP_ITERATION, IARG_END);
            INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)logMemoryAddress, IARG_UINT32, opIdx, IARG_BOOL,
                               TRUE, /* First must be one else we wouldn't be called */
                               IARG_MEMORYREAD2_EA, IARG_REG_VALUE, INS_RepCountRegister(ins), IARG_UINT32, op->size,
                               IARG_REG_VALUE, REG_EFLAGS, IARG_ADDRINT, (ADDRINT) "Read2", IARG_END);
        }
        if (INS_IsMemoryWrite(ins))
        {
            INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)returnArg, IARG_FIRST_REP_ITERATION, IARG_END);
            INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)logMemoryAddress, IARG_UINT32, opIdx, IARG_BOOL,
                               TRUE, /* First must be one else we wouldn't be called */
                               IARG_MEMORYWRITE_EA, IARG_REG_VALUE, INS_RepCountRegister(ins), IARG_UINT32, op->size,
                               IARG_REG_VALUE, REG_EFLAGS, IARG_ADDRINT, (ADDRINT) "Write", IARG_END);
        }
    }
}

// Instrument individual instructions.
// Specific instrumentation for REP prefixed instructions.
static VOID InstrumentInstruction(INS ins, VOID*)
{
    // We're only interested in REP prefixed instructions.
    if (!INS_HasRealRep(ins)) return;

    UINT32 opIdx = opcodeIndex(INS_Opcode(ins));

    insertRepExecutionCountInstrumentation(ins, opIdx);

    // If requested also add the instrumentation to count memory references.
    if (KnobCountMemory) insertRepMemoryCountInstrumentation(ins, opIdx);

    if (KnobAddresses) insertRepMemoryTraceInstrumentation(ins, opIdx);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool demonstrates how to optimize common REP cases, " << endl;
    cerr << "where the repeat count is known at the start of the REP " << endl;
    cerr << "instruction execution." << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv)) return Usage();

    out.open(KnobOutput.Value().c_str());

    // Our instruction instrumentation collectsinformation for REP prefixed instructions.
    INS_AddInstrumentFunction(InstrumentInstruction, 0);
    // Our trace instrumentation collects the information for all instructions.
    // It is similar to inscount2.
    TRACE_AddInstrumentFunction(InstrumentTrace, 0);

    // Fini prints the results.
    PIN_AddFiniFunction(Fini, 0);
    PIN_AddThreadStartFunction(CheckThreadCount, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
