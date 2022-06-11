/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This file contains a tool that generates instructions traces with values.
 *  It is designed to help debugging.
 */

#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <unistd.h>
#include "pin.H"
#include "instlib.H"
#include "control_manager.H"

using namespace CONTROLLER;
using namespace INSTLIB;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "debugtrace.out", "trace file");
KNOB< BOOL > KnobPid(KNOB_MODE_WRITEONCE, "pintool", "i", "0", "append pid to output");
KNOB< THREADID > KnobWatchThread(KNOB_MODE_WRITEONCE, "pintool", "watch_thread", "-1", "thread to watch, -1 for all");
KNOB< BOOL > KnobFlush(KNOB_MODE_WRITEONCE, "pintool", "flush", "0", "Flush output after every instruction");
KNOB< BOOL > KnobSymbols(KNOB_MODE_WRITEONCE, "pintool", "symbols", "1", "Include symbol information");
KNOB< BOOL > KnobLines(KNOB_MODE_WRITEONCE, "pintool", "lines", "0", "Include line number information");
KNOB< BOOL > KnobTraceInstructions(KNOB_MODE_WRITEONCE, "pintool", "instruction", "0", "Trace instructions");
KNOB< BOOL > KnobTraceCalls(KNOB_MODE_WRITEONCE, "pintool", "call", "1", "Trace calls");
KNOB< BOOL > KnobTraceMemory(KNOB_MODE_WRITEONCE, "pintool", "memory", "0", "Trace memory");
KNOB< BOOL > KnobSilent(KNOB_MODE_WRITEONCE, "pintool", "silent", "0", "Do everything but write file (for debugging).");
KNOB< BOOL > KnobEarlyOut(KNOB_MODE_WRITEONCE, "pintool", "early_out", "0", "Exit after tracing the first region.");

/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool collects an instruction trace for debugging\n"
            "\n";

    cerr << KNOB_BASE::StringKnobSummary();

    cerr << endl;

    return -1;
}

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

static std::ofstream out;

static INT32 enabled = 0;

static FILTER filter;

static ICOUNT icount;

static BOOL Emit(THREADID threadid)
{
    if (!enabled || KnobSilent || (KnobWatchThread != static_cast< THREADID >(-1) && KnobWatchThread != threadid)) return false;
    return true;
}

static VOID Flush()
{
    if (KnobFlush) out << flush;
}

/* ===================================================================== */

static VOID Fini(int, VOID* v);

static VOID Handler(EVENT_TYPE ev, VOID*, CONTEXT* ctxt, VOID*, THREADID, bool bcast)
{
    switch (ev)
    {
        case EVENT_START:
            enabled = 1;
            PIN_RemoveInstrumentation();
#if defined(TARGET_IA32) || defined(TARGET_IA32E)
            // So that the rest of the current trace is re-instrumented.
            if (ctxt) PIN_ExecuteAt(ctxt);
#endif
            break;

        case EVENT_STOP:
            enabled = 0;
            PIN_RemoveInstrumentation();
            if (KnobEarlyOut)
            {
                cerr << "Exiting due to -early_out" << endl;
                Fini(0, NULL);
                exit(0);
            }
#if defined(TARGET_IA32) || defined(TARGET_IA32E)
            // So that the rest of the current trace is re-instrumented.
            if (ctxt) PIN_ExecuteAt(ctxt);
#endif
            break;

        default:
            ASSERTX(false);
    }
}

/* ===================================================================== */

VOID EmitNoValues(THREADID threadid, string* str)
{
    if (!Emit(threadid)) return;

    out << *str << endl;

    Flush();
}

VOID Emit1Values(THREADID threadid, string* str, string* reg1str, ADDRINT reg1val)
{
    if (!Emit(threadid)) return;

    out << *str << " | " << *reg1str << " = " << reg1val << endl;

    Flush();
}

VOID Emit2Values(THREADID threadid, string* str, string* reg1str, ADDRINT reg1val, string* reg2str, ADDRINT reg2val)
{
    if (!Emit(threadid)) return;

    out << *str << " | " << *reg1str << " = " << reg1val << ", " << *reg2str << " = " << reg2val << endl;

    Flush();
}

VOID Emit3Values(THREADID threadid, string* str, string* reg1str, ADDRINT reg1val, string* reg2str, ADDRINT reg2val,
                 string* reg3str, ADDRINT reg3val)
{
    if (!Emit(threadid)) return;

    out << *str << " | " << *reg1str << " = " << reg1val << ", " << *reg2str << " = " << reg2val << ", " << *reg3str << " = "
        << reg3val << endl;

    Flush();
}

VOID Emit4Values(THREADID threadid, string* str, string* reg1str, ADDRINT reg1val, string* reg2str, ADDRINT reg2val,
                 string* reg3str, ADDRINT reg3val, string* reg4str, ADDRINT reg4val)
{
    if (!Emit(threadid)) return;

    out << *str << " | " << *reg1str << " = " << reg1val << ", " << *reg2str << " = " << reg2val << ", " << *reg3str << " = "
        << reg3val << ", " << *reg4str << " = " << reg4val << endl;

    Flush();
}

const UINT32 MaxEmitArgs = 4;

AFUNPTR emitFuns[] = {AFUNPTR(EmitNoValues), AFUNPTR(Emit1Values), AFUNPTR(Emit2Values), AFUNPTR(Emit3Values),
                      AFUNPTR(Emit4Values)};

/* ===================================================================== */

VOID EmitXMM(THREADID threadid, UINT32 regno, PIN_REGISTER* xmm)
{
    if (!Emit(threadid)) return;
    out << "\t\t\tXMM" << dec << regno << " := " << setfill('0') << hex;
    out.unsetf(ios::showbase);
    for (int i = 0; i < 16; i++)
    {
        if (i == 4 || i == 8 || i == 12) out << "_";
        out << setw(2) << (int)xmm->byte[15 - i]; // msb on the left as in registers
    }
    out << setfill(' ') << endl;
    out.setf(ios::showbase);
    Flush();
}

VOID AddXMMEmit(INS ins, IPOINT point, REG xmm_dst)
{
    INS_InsertCall(ins, point, AFUNPTR(EmitXMM), IARG_THREAD_ID, IARG_UINT32, xmm_dst - REG_XMM0, IARG_REG_CONST_REFERENCE,
                   xmm_dst, IARG_END);
}

VOID AddEmit(INS ins, IPOINT point, string& traceString, UINT32 regCount, REG regs[])
{
    if (regCount > MaxEmitArgs) regCount = MaxEmitArgs;

    IARGLIST args = IARGLIST_Alloc();
    for (UINT32 i = 0; i < regCount; i++)
    {
        IARGLIST_AddArguments(args, IARG_PTR, new string(REG_StringShort(regs[i])), IARG_REG_VALUE, regs[i], IARG_END);
    }

    INS_InsertCall(ins, point, emitFuns[regCount], IARG_THREAD_ID, IARG_PTR, new string(traceString), IARG_IARGLIST, args,
                   IARG_END);
    IARGLIST_Free(args);
}

static VOID* WriteEa[PIN_MAX_THREADS];

VOID CaptureWriteEa(THREADID threadid, VOID* addr) { WriteEa[threadid] = addr; }

VOID ShowN(UINT32 n, VOID* ea)
{
    out.unsetf(ios::showbase);
    // Print out the bytes in "big endian even though they are in memory little endian.
    // This is most natural for 8B and 16B quantities that show up most frequently.
    // The address pointed to
    out << std::setfill('0');
    UINT8 b[512];
    UINT8* x;
    if (n > 512)
        x = new UINT8[n];
    else
        x = b;
    PIN_SafeCopy(x, static_cast< UINT8* >(ea), n);
    for (UINT32 i = 0; i < n; i++)
    {
        out << std::setw(2) << static_cast< UINT32 >(x[n - i - 1]);
        if (((reinterpret_cast< ADDRINT >(ea) + n - i - 1) & 0x3) == 0 && i < n - 1) out << "_";
    }
    out << std::setfill(' ');
    out.setf(ios::showbase);
    if (n > 512) delete[] x;
}

VOID EmitWrite(THREADID threadid, UINT32 size)
{
    if (!Emit(threadid)) return;

    out << "                                 Write ";

    VOID* ea = WriteEa[threadid];

    switch (size)
    {
        case 0:
            out << "0 repeat count" << endl;
            break;

        case 1:
        {
            UINT8 x;
            PIN_SafeCopy(&x, static_cast< UINT8* >(ea), 1);
            out << "*(UINT8*)" << ea << " = " << static_cast< UINT32 >(x) << endl;
        }
        break;

        case 2:
        {
            UINT16 x;
            PIN_SafeCopy(&x, static_cast< UINT16* >(ea), 2);
            out << "*(UINT16*)" << ea << " = " << x << endl;
        }
        break;

        case 4:
        {
            UINT32 x;
            PIN_SafeCopy(&x, static_cast< UINT32* >(ea), 4);
            out << "*(UINT32*)" << ea << " = " << x << endl;
        }
        break;

        case 8:
        {
            UINT64 x;
            PIN_SafeCopy(&x, static_cast< UINT64* >(ea), 8);
            out << "*(UINT64*)" << ea << " = " << x << endl;
        }
        break;

        default:
            out << "*(UINT" << dec << size * 8 << hex << ")" << ea << " = ";
            ShowN(size, ea);
            out << endl;
            break;
    }

    Flush();
}

VOID EmitRead(THREADID threadid, VOID* ea, UINT32 size)
{
    if (!Emit(threadid)) return;

    out << "                                 Read ";

    switch (size)
    {
        case 0:
            out << "0 repeat count" << endl;
            break;

        case 1:
        {
            UINT8 x;
            PIN_SafeCopy(&x, static_cast< UINT8* >(ea), 1);
            out << static_cast< UINT32 >(x) << " = *(UINT8*)" << ea << endl;
        }
        break;

        case 2:
        {
            UINT16 x;
            PIN_SafeCopy(&x, static_cast< UINT16* >(ea), 2);
            out << x << " = *(UINT16*)" << ea << endl;
        }
        break;

        case 4:
        {
            UINT32 x;
            PIN_SafeCopy(&x, static_cast< UINT32* >(ea), 4);
            out << x << " = *(UINT32*)" << ea << endl;
        }
        break;

        case 8:
        {
            UINT64 x;
            PIN_SafeCopy(&x, static_cast< UINT64* >(ea), 8);
            out << x << " = *(UINT64*)" << ea << endl;
        }
        break;

        default:
            ShowN(size, ea);
            out << " = *(UINT" << dec << size * 8 << hex << ")" << ea << endl;
            break;
    }

    Flush();
}

static INT32 indent = 0;

VOID Indent()
{
    for (INT32 i = 0; i < indent; i++)
    {
        out << "| ";
    }
}

VOID EmitICount() { out << setw(10) << dec << icount.Count() << hex << " "; }

VOID EmitDirectCall(THREADID threadid, string* str, INT32 tailCall, ADDRINT arg0, ADDRINT arg1)
{
    if (!Emit(threadid)) return;

    EmitICount();

    if (tailCall)
    {
        // A tail call is like an implicit return followed by an immediate call
        indent--;
    }

    Indent();
    out << *str << "(" << arg0 << ", " << arg1 << ", ...)" << endl;

    indent++;

    Flush();
}

string FormatAddress(ADDRINT address, RTN rtn)
{
    string s = StringFromAddrint(address);

    if (KnobSymbols && RTN_Valid(rtn))
    {
        IMG img = SEC_Img(RTN_Sec(rtn));
        s += " ";
        if (IMG_Valid(img))
        {
            s += IMG_Name(img) + ":";
        }

        s += RTN_Name(rtn);

        ADDRINT delta = address - RTN_Address(rtn);
        if (delta != 0)
        {
            s += "+" + hexstr(delta, 4);
        }
    }

    if (KnobLines)
    {
        INT32 line;
        string file;

        PIN_GetSourceLocation(address, NULL, &line, &file);

        if (file != "")
        {
            s += " (" + file + ":" + decstr(line) + ")";
        }
    }
    return s;
}

VOID EmitIndirectCall(THREADID threadid, string* str, ADDRINT target, ADDRINT arg0, ADDRINT arg1)
{
    if (!Emit(threadid)) return;

    EmitICount();
    Indent();
    out << *str;

    PIN_LockClient();

    string s = FormatAddress(target, RTN_FindByAddress(target));

    PIN_UnlockClient();

    out << s << "(" << arg0 << ", " << arg1 << ", ...)" << endl;
    indent++;

    Flush();
}

VOID EmitReturn(THREADID threadid, string* str, ADDRINT ret0)
{
    if (!Emit(threadid)) return;

    EmitICount();
    indent--;
    if (indent < 0)
    {
        out << "@@@ return underflow\n";
        indent = 0;
    }

    Indent();
    out << *str << " returns: " << ret0 << endl;

    Flush();
}

VOID CallTrace(TRACE trace, INS ins)
{
    if (!KnobTraceCalls) return;

    if (INS_IsCall(ins) && !INS_IsDirectControlFlow(ins))
    {
        // Indirect call
        string s = "Call " + FormatAddress(INS_Address(ins), TRACE_Rtn(trace));
        s += " -> ";

        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(EmitIndirectCall), IARG_THREAD_ID, IARG_PTR, new string(s),
                       IARG_BRANCH_TARGET_ADDR, IARG_FUNCARG_CALLSITE_VALUE, 0, IARG_FUNCARG_CALLSITE_VALUE, 1, IARG_END);
    }
    else if (INS_IsDirectControlFlow(ins))
    {
        // Is this a tail call?
        RTN sourceRtn = TRACE_Rtn(trace);
        RTN destRtn   = RTN_FindByAddress(INS_DirectControlFlowTargetAddress(ins));

        if (INS_IsCall(ins)         // conventional call
            || sourceRtn != destRtn // tail call
        )
        {
            BOOL tailcall = !INS_IsCall(ins);

            string s = "";
            if (tailcall)
            {
                s += "Tailcall ";
            }
            else
            {
                if (INS_IsProcedureCall(ins))
                    s += "Call ";
                else
                {
                    s += "PcMaterialization ";
                    tailcall = 1;
                }
            }

            //s += INS_Mnemonic(ins) + " ";

            s += FormatAddress(INS_Address(ins), TRACE_Rtn(trace));
            s += " -> ";

            ADDRINT target = INS_DirectControlFlowTargetAddress(ins);

            s += FormatAddress(target, RTN_FindByAddress(target));

            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(EmitDirectCall), IARG_THREAD_ID, IARG_PTR, new string(s), IARG_BOOL,
                           tailcall, IARG_FUNCARG_CALLSITE_VALUE, 0, IARG_FUNCARG_CALLSITE_VALUE, 1, IARG_END);
        }
    }
    else if (INS_IsRet(ins))
    {
        RTN rtn = TRACE_Rtn(trace);

#if defined(TARGET_LINUX) && defined(TARGET_IA32)
        //        if( RTN_Name(rtn) ==  "_dl_debug_state") return;
        if (RTN_Valid(rtn) && RTN_Name(rtn) == "_dl_runtime_resolve") return;
#endif
        string tracestring = "Return " + FormatAddress(INS_Address(ins), rtn);
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(EmitReturn), IARG_THREAD_ID, IARG_PTR, new string(tracestring),
                       IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);
    }
}

VOID InstructionTrace(TRACE trace, INS ins)
{
    if (!KnobTraceInstructions) return;

    ADDRINT addr = INS_Address(ins);
    ASSERTX(addr);

    // Format the string at instrumentation time
    string traceString = "";
    string astring     = FormatAddress(INS_Address(ins), TRACE_Rtn(trace));
    for (INT32 length = astring.length(); length < 30; length++)
    {
        traceString += " ";
    }
    traceString = astring + traceString;

    traceString += " " + INS_Disassemble(ins);

    for (INT32 length = traceString.length(); length < 80; length++)
    {
        traceString += " ";
    }

    INT32 regCount = 0;
    REG regs[20];
    REG xmm_dst = REG_INVALID();

    for (UINT32 i = 0; i < INS_MaxNumWRegs(ins); i++)
    {
        REG x = REG_FullRegName(INS_RegW(ins, i));

        if (REG_is_gr(x)
#if defined(TARGET_IA32)
            || x == REG_EFLAGS
#elif defined(TARGET_IA32E)
            || x == REG_RFLAGS
#endif
        )
        {
            regs[regCount] = x;
            regCount++;
        }

        if (REG_is_xmm(x)) xmm_dst = x;
    }

    if (INS_IsValidForIpointAfter(ins))
    {
        AddEmit(ins, IPOINT_AFTER, traceString, regCount, regs);
    }
    if (INS_IsValidForIpointTakenBranch(ins))
    {
        AddEmit(ins, IPOINT_TAKEN_BRANCH, traceString, regCount, regs);
    }

    if (xmm_dst != REG_INVALID())
    {
        if (INS_IsValidForIpointAfter(ins)) AddXMMEmit(ins, IPOINT_AFTER, xmm_dst);
        if (INS_IsValidForIpointTakenBranch(ins)) AddXMMEmit(ins, IPOINT_TAKEN_BRANCH, xmm_dst);
    }
}

VOID MemoryTrace(INS ins)
{
    if (!KnobTraceMemory) return;

    if (INS_IsMemoryWrite(ins) && INS_IsStandardMemop(ins))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(CaptureWriteEa), IARG_THREAD_ID, IARG_MEMORYWRITE_EA, IARG_END);

        if (INS_IsValidForIpointAfter(ins))
        {
            INS_InsertPredicatedCall(ins, IPOINT_AFTER, AFUNPTR(EmitWrite), IARG_THREAD_ID, IARG_MEMORYWRITE_SIZE, IARG_END);
        }
        if (INS_IsValidForIpointTakenBranch(ins))
        {
            INS_InsertPredicatedCall(ins, IPOINT_TAKEN_BRANCH, AFUNPTR(EmitWrite), IARG_THREAD_ID, IARG_MEMORYWRITE_SIZE,
                                     IARG_END);
        }
    }

    if (INS_HasMemoryRead2(ins) && INS_IsStandardMemop(ins))
    {
        INS_InsertPredicatedCall(ins, IPOINT_BEFORE, AFUNPTR(EmitRead), IARG_THREAD_ID, IARG_MEMORYREAD2_EA, IARG_MEMORYREAD_SIZE,
                                 IARG_END);
    }

    if (INS_IsMemoryRead(ins) && !INS_IsPrefetch(ins) && INS_IsStandardMemop(ins))
    {
        INS_InsertPredicatedCall(ins, IPOINT_BEFORE, AFUNPTR(EmitRead), IARG_THREAD_ID, IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE,
                                 IARG_END);
    }
}

/* ===================================================================== */

VOID Trace(TRACE trace, VOID* v)
{
    if (!filter.SelectTrace(trace)) return;

    if (enabled)
    {
        for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
        {
            for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
            {
                InstructionTrace(trace, ins);

                CallTrace(trace, ins);

                MemoryTrace(ins);
            }
        }
    }
}

/* ===================================================================== */

VOID Fini(int, VOID* v)
{
    out << "# $eof" << endl;

    out.close();
}

/* ===================================================================== */

static void OnSig(THREADID threadIndex, CONTEXT_CHANGE_REASON reason, const CONTEXT* ctxtFrom, CONTEXT* ctxtTo, INT32 sig,
                  VOID* v)
{
    if (ctxtFrom != 0)
    {
        ADDRINT address = PIN_GetContextReg(ctxtFrom, REG_INST_PTR);
        out << "SIG signal=" << sig << " on thread " << threadIndex << " at address " << hex << address << dec << " ";
    }

    switch (reason)
    {
        case CONTEXT_CHANGE_REASON_FATALSIGNAL:
            out << "FATALSIG" << sig;
            break;
        case CONTEXT_CHANGE_REASON_SIGNAL:
            out << "SIGNAL " << sig;
            break;
        case CONTEXT_CHANGE_REASON_SIGRETURN:
            out << "SIGRET";
            break;

        case CONTEXT_CHANGE_REASON_APC:
            out << "APC";
            break;

        case CONTEXT_CHANGE_REASON_EXCEPTION:
            out << "EXCEPTION";
            break;

        case CONTEXT_CHANGE_REASON_CALLBACK:
            out << "CALLBACK";
            break;

        default:
            break;
    }
    out << std::endl;
}

/* ===================================================================== */

static CONTROL_MANAGER control;
static SKIPPER skipper;

/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    string filename = KnobOutputFile.Value();

    if (KnobPid)
    {
        filename += "." + decstr(getpid());
    }

    // Do this before we activate controllers
    out.open(filename.c_str());
    out << hex << right;
    out.setf(ios::showbase);

    control.RegisterHandler(Handler, 0, FALSE);
    control.Activate();
    skipper.CheckKnobs(0);

    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddContextChangeFunction(OnSig, 0);

    PIN_AddFiniFunction(Fini, 0);

    filter.Activate();
    icount.Activate();

    // Never returns

    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
