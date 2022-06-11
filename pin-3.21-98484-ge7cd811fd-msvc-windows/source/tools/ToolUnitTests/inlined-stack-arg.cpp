/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>
#include <fstream>
using std::cerr;
using std::endl;
using std::string;

typedef struct
{
    THREADID tid;
    ADDRINT pc;
    ADDRINT nextAddress;
    UINT32 readRegCount;
    UINT32 writeRegCount;
    ADDRINT instrSize;
    CHAR type;
} FooBar;

FooBar accessInfo;

static std::ofstream log_inl;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "inlined-stack-arg.out", "output file");

#if defined(TARGET_LINUX) && defined(TARGET_IA32E) && !defined(__INTEL_COMPILER)
// New GCC compilers (on 64 bits) do optimizations to RecordFirstInstructionInfo() (below) which today deny us from doing
// "stack value" optimization (See knob opt_stack_param_failure_assert, _disableStackOptimization and
// _canDeleteArgPushsAndArgStackUpdates for more details about the optimization).
// Therefore we use a pragma equivalent for -mno-sse2 compiler directive to turn off these optimizations.
// See comments below inside RecordFirstInstructionInfo() for more information)
#pragma GCC push_options
#pragma GCC target("no-sse2")
#endif // #if defined(TARGET_LINUX) && defined(TARGET_IA32E) && !defined(__INTEL_COMPILER)

VOID RecordFirstInstructionInfo(UINT32 tid, ADDRINT pcval, ADDRINT nxtaddr, UINT32 rregcnt, UINT32 wregcnt, ADDRINT inssz,
                                CHAR type)
{
    accessInfo.tid           = tid;
    accessInfo.pc            = pcval;
    accessInfo.nextAddress   = nxtaddr;
    accessInfo.readRegCount  = rregcnt;
    accessInfo.writeRegCount = wregcnt;
    accessInfo.instrSize     = inssz;
    accessInfo.type          = type;
    /*
     * Note that this function is considered simple and is expected to be inlineable and in addition "stack value" optimization
     * (KnobStackValueOpt) is also expected to work for this function on some tests (:inlined-stack-arg.test).
     * However new compilers may do all kind of optimizations which will deny the "stack value" optimization. Currently added
     * compiler directive flags (where needed) to deny these compiler optimizations which kill our optimization (at least until
     * will be able to perform our optimization).
     * Another option will be to write this function ourself in assembly. On one hand it is very logical since we want to test our
     * current optimization and it is hard to do that if the compiler keeps doing new things which interfere our compiler
     * optimizer.
     * On the other hand letting the compiler do optimizations introduces new compiler optimizations which we need to be aware
     * about in order to make updates to our optimizer (for example instructions which uses XMM registers which we didn't
     * handle before)
     */
}

#if defined(TARGET_LINUX) && defined(TARGET_IA32E) && !defined(__INTEL_COMPILER)
#pragma GCC pop_options
#endif // #if defined(TARGET_LINUX) && defined(TARGET_IA32E) && !defined(__INTEL_COMPILER)

INT32 Usage()
{
    cerr << "This tests if the stack arguments are passed correctly by an inlined analysis function"
            "\n";

    cerr << endl;

    return -1;
}

VOID Instruction(INS ins, VOID* v)
{
    ADDRINT nextAddr = INS_NextAddress(ins);
    UINT32 maxRRegs  = INS_MaxNumRRegs(ins);
    UINT32 maxWRegs  = INS_MaxNumWRegs(ins);
    USIZE sz         = INS_Size(ins);

    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordFirstInstructionInfo, IARG_THREAD_ID, IARG_INST_PTR, IARG_ADDRINT, nextAddr,
                   IARG_UINT32, maxRRegs, IARG_UINT32, maxWRegs, IARG_ADDRINT, sz, IARG_UINT32, 'r', IARG_END);
}

VOID Fini(INT32 code, VOID* v)
{
    log_inl << "Type " << accessInfo.type << "\n";
    log_inl.close();
}

int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    string logfile = KnobOutputFile.Value();

    log_inl.open(logfile.c_str());

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
