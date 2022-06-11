/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Rewrite all memory operands, though we don't actually change the addresses they access.
 */
#include <fstream>
#include <iostream>
#include <iomanip>

#include <string.h>
#include "pin.H"
using std::dec;
using std::endl;
using std::hex;
using std::ofstream;
using std::string;

KNOB< BOOL > KnobTrace(KNOB_MODE_WRITEONCE, "pintool", "t", "0", "trace rewrites");
KNOB< BOOL > KnobOne(KNOB_MODE_WRITEONCE, "pintool", "1", "0", "Don't rewrite the first memory operand");
KNOB< BOOL > KnobTwo(KNOB_MODE_WRITEONCE, "pintool", "2", "0", "Don't rewrite the second memory operand");
KNOB< string > KnobOutput(KNOB_MODE_WRITEONCE, "pintool", "o", "rewritememop1.out", "Name for log file");

KNOB< BOOL > KnobUseIargConstContext(KNOB_MODE_WRITEONCE, "pintool", "const_context", "0", "use IARG_CONST_CONTEXT");

static ofstream out;
static UINT64 rewritten   = 0;
static UINT64 afterBranch = 0;
static BOOL rewriteOp[32];

#define STRINGIZE(v) #v
#define REGENTRY(n)           \
    {                         \
        REG_##n, STRINGIZE(n) \
    }

/* ===================================================================== */
/* Tracing code. It prints each instruction, and, after it, the register state
 * changes it made.
 */

/* Information for each thread. */
class threadState
{
  public:
    UINT32 iCount;
    CONTEXT context;
};

static threadState threadStates[32];

/************************************************************************/
/* We simply allocate space for the dis-assembled instruction strings and
 * let them leak.
 */
static char const* formatInstruction(INS ins)
{
    ADDRINT ip       = INS_Address(ins);
    string formatted = hexstr(ip) + " " + INS_Disassemble(ins);
    char* res        = new char[formatted.length() + 1];

    strcpy(res, formatted.c_str());

    return res;
}

// This function is called before every instruction is executed
// and prints the pre-formatted dis-assembled instruction
static VOID printInstruction(THREADID thread, ADDRINT disas)
{
    threadState* s = &threadStates[thread];
    UINT32 seqNo   = ++s->iCount;

    out << dec << seqNo << ":" << ((const char*)Addrint2VoidStar(disas)) << endl;
}

// Table of registers to check and display
static const struct
{
    REG regnum;
    const char* name;
} checkedRegisters[] =
#if defined(TARGET_IA32E)
    {REGENTRY(RFLAGS), REGENTRY(RAX), REGENTRY(RBX), REGENTRY(RCX), REGENTRY(RDX), REGENTRY(RBP),
     REGENTRY(RSP),    REGENTRY(RDI), REGENTRY(RSI), REGENTRY(R8),  REGENTRY(R9),  REGENTRY(R10),
     REGENTRY(R11),    REGENTRY(R12), REGENTRY(R13), REGENTRY(R14), REGENTRY(R15)};
#elif defined(TARGET_IA32)
    {
        REGENTRY(EFLAGS), REGENTRY(EAX), REGENTRY(EBX), REGENTRY(ECX), REGENTRY(EDX),
        REGENTRY(EBP),    REGENTRY(ESP), REGENTRY(EDI), REGENTRY(ESI),
};
#else
#error Unknown processor
#endif

static VOID printRegisterDiffs(THREADID tid, CONTEXT* ctx)
{
    threadState* s    = &threadStates[tid];
    UINT32 seqNo      = s->iCount;
    CONTEXT* savedCtx = &s->context;

    // Save the context if this was the first instruction
    if (seqNo == 0)
        PIN_SaveContext(ctx, savedCtx);
    else
    {
        for (UINT32 i = 0; i < sizeof(checkedRegisters) / sizeof(checkedRegisters[0]); i++)
        {
            REG r            = checkedRegisters[i].regnum;
            ADDRINT newValue = PIN_GetContextReg(ctx, r);

            if (PIN_GetContextReg(savedCtx, r) != newValue)
            {
                out << dec << seqNo << ": " << checkedRegisters[i].name << " = " << hex << newValue << endl;
                PIN_SetContextReg(savedCtx, r, newValue);
            }
        }
    }
}

static UINT32 opcodesRewritten[XED_ICLASS_LAST];

static void printHex(ADDRINT p) { out << std::setw(2 * (sizeof(p) + 1)) << std::hex << p << std::dec; }

static ADDRINT ProcessAddress(UINT32 operand, ADDRINT val, ADDRINT ip)
{
    if (KnobTrace)
    {
        out << "  ";
        printHex(ip);
        out << ": [" << operand << "] ";
        printHex(val);
        out << endl;
    }
    rewritten++;
    return val;
}

static void BranchTaken(ADDRINT ip)
{
    if (KnobTrace)
    {
        out << "  ";
        printHex(ip);
        out << ": on branch taken" << endl;
    }
    afterBranch++;
}

// Choose the classes of instruction to rewrite addressing on.
// Useful to track down bugs to specific instructions.
static BOOL doTranslate(INS ins)
{
    if (!INS_IsStandardMemop(ins) && !INS_HasMemoryVector(ins))
    {
        // We don't know how to treat these instructions
        return FALSE;
    }
    switch (INS_Category(ins))
    {
        case XED_CATEGORY_CALL:
        case XED_CATEGORY_RET:
        case XED_CATEGORY_PUSH:
        case XED_CATEGORY_POP:
        case XED_CATEGORY_STRINGOP:
            return TRUE;

        case XED_CATEGORY_MISC:
            switch (INS_Opcode(ins))
            {
                case XED_ICLASS_ENTER:
                    return TRUE;
                case XED_ICLASS_LEAVE:
                    return TRUE;
                default:
                    break;
            }

        default:
            return TRUE;
    }
}

static VOID RewriteIns(INS ins)
{
    /* Rewrite all the memory operands */
    int memops = INS_MemoryOperandCount(ins);

    if (KnobTrace)
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(printRegisterDiffs), IARG_THREAD_ID,
                       (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(printInstruction), IARG_THREAD_ID, IARG_ADDRINT, formatInstruction(ins),
                       IARG_END);
    }

    if (memops && doTranslate(ins))
    {
        UINT32 op = INS_Opcode(ins);

        opcodesRewritten[op]++;

        for (int i = 0; i < memops; i++)
        {
            // Only rewrite the operands we were asked to.
            if (rewriteOp[i])
            {
                INS_RewriteMemoryOperand(ins, i, REG(REG_INST_G0 + i));

                INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(ProcessAddress), IARG_UINT32, i, IARG_MEMORYOP_EA, i, IARG_INST_PTR,
                               IARG_RETURN_REGS, REG_INST_G0 + i, IARG_END);
            }
        }

        if (INS_IsValidForIpointTakenBranch(ins))
        { // Check that IPOINT_TAKEN_BRANCH instrumentation is called.
            INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, AFUNPTR(BranchTaken), IARG_INST_PTR, IARG_END);
        }
    }
}

VOID Trace(TRACE trace, VOID* v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            RewriteIns(ins);
        }
    }
}

void AtEnd(INT32 code, VOID* arg)
{
    out << "Target exited with code : " << code << endl << endl;
    out << "Dynamically rewrote " << rewritten << " memory operands" << endl;
    out << "Branch taken " << afterBranch << endl;

    out << "Statically rewritten opcodes" << endl;
    for (UINT32 i = 0; i < XED_ICLASS_LAST; i++)
    {
        if (opcodesRewritten[i])
        {
            out << std::setw(12) << std::left << xed_iclass_enum_t2str(xed_iclass_enum_t(i)) << std::right << " " << std::setw(9)
                << opcodesRewritten[i] << endl;
        }
    }
    out.close();
}

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    out.open(KnobOutput.Value().c_str());

    rewriteOp[0] = !KnobOne;
    rewriteOp[1] = !KnobTwo;

    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddFiniFunction(AtEnd, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
