/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef ICOUNT_H
#define ICOUNT_H

namespace INSTLIB
{
/*! @defgroup ICOUNT
  Instrumentation for counting instruction execution
*/

/*! @ingroup ICOUNT
  The example below can be found in InstLibExamples/icount.cpp

  \include icount.cpp
*/
class ICOUNT
{
  public:
    ICOUNT()
    {
        _mode = ModeInactive;

        /* Allocate 64 byte aligned data for the statistics. */
        _space = new char[(ISIMPOINT_MAX_THREADS + 1) * sizeof(threadStats) - 1];

        ADDRINT space   = VoidStar2Addrint(_space);
        ADDRINT align_1 = static_cast< ADDRINT >(cacheLineSize - 1);
        _stats          = reinterpret_cast< threadStats* >((space + align_1) & ~align_1);
        memset(_stats, 0, ISIMPOINT_MAX_THREADS * sizeof(threadStats));
    };

    ~ICOUNT() { delete[] _space; }
    /*! @ingroup ICOUNT
      @return Total number of instructions executed. (But see @ref mode for what this means).
    */
    UINT64 Count(THREADID tid = 0) const
    {
        ASSERTX(tid < ISIMPOINT_MAX_THREADS);
        return _stats[tid].count;
    }

    UINT64 CountWithoutRep(THREADID tid = 0) const
    {
        ASSERTX(tid < ISIMPOINT_MAX_THREADS);
        ASSERTX(Mode() == ModeBoth);
        threadStats* s = &_stats[tid];

        return s->count - s->repDuplicateCount;
    }

    /*! @ingroup ICOUNT
      Set the current count
    */
    VOID SetCount(UINT64 count, THREADID tid = 0)
    {
        ASSERTX(_mode != ModeInactive);
        ASSERTX(tid < ISIMPOINT_MAX_THREADS);
        _stats[tid].count             = count;
        _stats[tid].repDuplicateCount = 0;
    }

    /*! @ingroup ICOUNT
     * The mode used for counting REP prefixed instructions.
     */
    enum mode
    {
        ModeInactive = -1,
        ModeNormal   = 0, /**< Count all instructions, each REP "iteration" adds 1 */
        ModeBoth          /**< Provide both the normal count and a count in which REP prefixed
                                                   instructions are only counted once. */
    };

    /*! @ingroup ICOUNT
     * @return the mode of the ICOUNT object.
     */
    mode Mode() const { return _mode; }

    /*! @ingroup ICOUNT
      Activate the counter, must be called before PIN_StartProgram.
      @param [in] mode Determine the way in which REP prefixed operations are counted. By default (ICOUNT::ModeNormal),
                       REP prefixed instructions are counted as if REP is an implicit loop. By passing 
                       ICOUNT::ModeRepsCountedOnlyOnce you can have the counter treat each REP as only one dynamic instruction.
    */
    VOID Activate(mode m = ModeNormal)
    {
        ASSERTX(_mode == ModeInactive);
        _mode = m;
        TRACE_AddInstrumentFunction(Trace, this);
    }

  private:
    enum
    {
        cacheLineSize = 64
    };

    static VOID Trace(TRACE trace, VOID* icount)
    {
#if (defined(TARGET_IA32) || defined(TARGET_IA32E))
        ICOUNT const* ic = reinterpret_cast< ICOUNT const* >(icount);
        mode m           = ic->Mode();
#endif
        for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
        {
            BBL_InsertCall(bbl, IPOINT_ANYWHERE, AFUNPTR(Advance), IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT, icount, IARG_ADDRINT,
                           ADDRINT(BBL_NumIns(bbl)), IARG_THREAD_ID, IARG_END);

            // REP prefixed instructions are an IA-32 and Intel(R) 64 feature
#if (defined(TARGET_IA32) || defined(TARGET_IA32E))
            if (m == ModeBoth)
            { // Check whether there are any REP prefixed instructions in the BBL
                // and, if so, subtract out their execution unless it is the first
                // iteration.
                for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
                {
                    if (INS_HasRealRep(ins))
                    {
                        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(CountDuplicates), IARG_FAST_ANALYSIS_CALL, IARG_ADDRINT,
                                       icount, IARG_FIRST_REP_ITERATION, IARG_THREAD_ID, IARG_END);
                    }
                }
            }
#endif
        }
    }

    static VOID PIN_FAST_ANALYSIS_CALL Advance(ICOUNT* ic, ADDRINT c, THREADID tid)
    {
        // ASSERTX(tid < ISIMPOINT_MAX_THREADS);
        ic->_stats[tid].count += c;
    }

    // Accumulate the count of REP prefixed executions which aren't the first iteration.
    //
    // We are assuming that this will be inlined, and is small, so there is no point
    // in guarding it with an InsertIf call testing IARG_FIRST_REP_ITERATION.
    static VOID PIN_FAST_ANALYSIS_CALL CountDuplicates(ICOUNT* ic, BOOL first, THREADID tid)
    {
        // ASSERTX(tid < ISIMPOINT_MAX_THREADS);
        ic->_stats[tid].repDuplicateCount += !first;
    }

    struct threadStats
    {
        UINT64 count;
        UINT64 repDuplicateCount;                         /* Number of REP iterations after the first */
        char padding[cacheLineSize - 2 * sizeof(UINT64)]; /* Expand so we can cache align this.
                                                            * We want to avoid false sharing of the stats between threads.
                                                            */
    };

    threadStats* _stats;
    char* _space;
    mode _mode;
};
} // namespace INSTLIB
#endif
