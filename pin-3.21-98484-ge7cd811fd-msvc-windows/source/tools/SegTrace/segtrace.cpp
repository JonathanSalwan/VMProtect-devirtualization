/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *
 * This file contains a tool that traces all uses and changes to the FS and GS
 * segments, which are typically used to implement thread-local storage on x86
 * platforms.  Currently, this tool only knows about the segment-related system
 * calls on Linux.
 *
 * This tool also check:
 * - IARG_REG_REFERENCE/IARG_REG_CONST_REFERENCE to a segment register works (segment selector)
 *   and that it is equal to the same segment register when being passed by value.
 * - IARG_REG_VALUE, IARG_REG_CONST_REFERENCE and IARG_CONST_CONTEXT for segment base address
 *   (virtual register REG_SEG_GS_BASE/REG_SEG_FS_BASE) works and that their values are equal.
 *
 * Possible to add these checks:
 * - IARG_REG_REFERENCE/IARG_REG_CONST_REFERENCE and IARG_REG_VALUE for segment for segment register return the correct value where applicable (32 Linux).
 * - IARG_REG_VALUE and IARG_REG_CONST_REFERENCE for segment base address (virtual register REG_SEG_GS_BASE/REG_SEG_FS_BASE) return the correct value
 *   which the thread area (TLS) (Currently TLS address is not tracked in this tool, if we want enable this check we need
 *   to add code to this tool that will track the TLS address)
 * Currently verified these by looking at the logs.
 */

#include "pin.H"
#include "pending_syscalls.H"
#include "disasm_container.H"

#include <iostream>
#include <ostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <map>
using std::string;

#if defined(TARGET_LINUX)
#include <sys/syscall.h>
#include <asm/ldt.h>
#endif

#if defined(TARGET_LINUX) && defined(TARGET_IA32E)
#include <asm/prctl.h>
#endif

// These constants are not defined on old kernels.
//
#ifndef __NR_set_thread_area
#define __NR_set_thread_area 243
#endif
#ifndef __NR_get_thread_area
#define __NR_get_thread_area 244
#endif
#ifndef SYS_set_thread_area
#define SYS_set_thread_area __NR_set_thread_area
#endif
#ifndef SYS_get_thread_area
#define SYS_get_thread_area __NR_get_thread_area
#endif
#ifndef CLONE_SETTLS
#define CLONE_SETTLS 0x00080000
#endif

// Specifies a segment descriptor entry.  Used with modify_ldt(), etc.
//
struct USER_DESC
{
    UINT32 entry_number;
    ADDRINT base_addr;
    UINT32 limit;
    UINT32 seg_32bit : 1;
    UINT32 contents : 2;
    UINT32 read_exec_only : 1;
    UINT32 limit_in_pages : 1;
    UINT32 seg_not_present : 1;
    UINT32 useable : 1;
};

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "segtrace.out", "trace file");

std::ofstream Out;
PENDING_SYSCALLS* PendingSyscalls; // Holds syscall information between "before" and "after" instrumentation
DISASM_CONTAINER* Disassemblies;   // Holds disassemblies for "interesting" instructions

static VOID Instruction(INS, VOID*);
static BOOL WritesSegment(INS, REG*);
static VOID OnSegReference(UINT32, ADDRINT, ADDRINT*, ADDRINT, THREADID);
static VOID OnSegGsOrFsReference(UINT32, ADDRINT, ADDRINT*, ADDRINT, ADDRINT*, ADDRINT, THREADID);
static VOID OnSegWriteBefore(UINT32, UINT16, ADDRINT*, ADDRINT, THREADID);
static VOID OnSegWriteAfter(UINT32, UINT16, ADDRINT*, ADDRINT*, ADDRINT, THREADID);
static VOID OnSegWriteAfterContext(CONTEXT*);
static VOID OnSyscallBefore(ADDRINT, ADDRINT, ADDRINT, ADDRINT, ADDRINT, ADDRINT, ADDRINT, ADDRINT, ADDRINT, THREADID);
static VOID OnSyscallAfter(ADDRINT, ADDRINT, ADDRINT, ADDRINT, THREADID);
static std::string Header(THREADID, ADDRINT);
static std::string PasteDisassembly(const std::string&, const std::string&);
static std::string SegName(REG);
static std::string SegSelector(ADDRINT);
static VOID SyscallEntry(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v);
static VOID SyscallExit(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v);

#if defined(TARGET_LINUX)
static std::string UserDesc(USER_DESC*);
#endif

#if defined(TARGET_LINUX) && defined(TARGET_IA32E)
static std::string PrctlFunc(ADDRINT);
#endif

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    Out.open(KnobOutputFile.Value().c_str());
    PendingSyscalls = new PENDING_SYSCALLS();
    Disassemblies   = new DISASM_CONTAINER();

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddSyscallEntryFunction(SyscallEntry, 0);
    PIN_AddSyscallExitFunction(SyscallExit, 0);

    PIN_StartProgram();
    return 0;
}

static VOID Instruction(INS ins, VOID* v)
{
    REG seg;
    if (INS_SegmentPrefix(ins))
    {
        seg = INS_SegmentRegPrefix(ins);
        if ((seg == REG_SEG_GS) || (seg == REG_SEG_FS))
        {
            REG segBaseReg = (seg == REG_SEG_GS) ? REG_SEG_GS_BASE : REG_SEG_FS_BASE;
            // Using IARG_REG_CONST_REFERENCE and not IARG_REG_CONST_REFERENCE for segment here since in 64 bits
            // you cannot write back to a segment register in user level
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(OnSegGsOrFsReference), IARG_UINT32, seg, IARG_REG_VALUE, seg,
                           IARG_REG_CONST_REFERENCE, seg, IARG_REG_VALUE, segBaseReg, IARG_REG_CONST_REFERENCE, segBaseReg,
                           IARG_INST_PTR, IARG_THREAD_ID, IARG_END);
        }
        else
        {
            // See above comment about IARG_REG_CONST_REFERENCE
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(OnSegReference), IARG_UINT32, seg, IARG_REG_VALUE, seg,
                           IARG_REG_CONST_REFERENCE, seg, IARG_INST_PTR, IARG_THREAD_ID, IARG_END);
        }
        Disassemblies->Add(INS_Address(ins), INS_Disassemble(ins));
    }

    if (WritesSegment(ins, &seg))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(OnSegWriteBefore), IARG_UINT32, seg, IARG_REG_VALUE, seg,
                       IARG_REG_CONST_REFERENCE, seg, IARG_INST_PTR, IARG_THREAD_ID, IARG_END);

        INS_InsertCall(ins, IPOINT_AFTER, AFUNPTR(OnSegWriteAfterContext), IARG_CONST_CONTEXT, IARG_END);

        // This is only possible in Linux 32 bits so using IARG_REG_REFERENCE for segment is allowed (although not changing value)
        INS_InsertCall(ins, IPOINT_AFTER, AFUNPTR(OnSegWriteAfter), IARG_UINT32, seg, IARG_REG_VALUE, seg, IARG_REG_REFERENCE,
                       seg, IARG_REG_CONST_REFERENCE, REG_SEG_GS_BASE, IARG_INST_PTR, IARG_THREAD_ID, IARG_END);

        Disassemblies->Add(INS_Address(ins), INS_Disassemble(ins));
    }

    // For O/S's (macOS*) that don't support PIN_AddSyscallEntryFunction(),
    // instrument the system call instruction.
    //
    if (INS_IsSyscall(ins) && INS_IsValidForIpointAfter(ins))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(OnSyscallBefore), IARG_SYSCALL_NUMBER, IARG_SYSARG_VALUE, 0, IARG_SYSARG_VALUE,
                       1, IARG_SYSARG_VALUE, 2, IARG_SYSARG_VALUE, 3, IARG_SYSARG_VALUE, 4, IARG_REG_VALUE, REG_SEG_FS,
                       IARG_REG_VALUE, REG_SEG_GS, IARG_INST_PTR, IARG_THREAD_ID, IARG_END);

        INS_InsertCall(ins, IPOINT_AFTER, AFUNPTR(OnSyscallAfter), IARG_SYSRET_VALUE, IARG_REG_VALUE, REG_SEG_FS, IARG_REG_VALUE,
                       REG_SEG_GS, IARG_INST_PTR, IARG_THREAD_ID, IARG_END);
    }
}

static BOOL WritesSegment(INS ins, REG* seg)
{
    if (INS_RegWContain(ins, REG_SEG_FS))
    {
        *seg = REG_SEG_FS;
        return TRUE;
    }
    if (INS_RegWContain(ins, REG_SEG_GS))
    {
        *seg = REG_SEG_GS;
        return TRUE;
    }
    if (INS_RegWContain(ins, REG_SEG_ES))
    {
        *seg = REG_SEG_ES;
        return TRUE;
    }
    if (INS_RegWContain(ins, REG_SEG_CS))
    {
        *seg = REG_SEG_CS;
        return TRUE;
    }
    if (INS_RegWContain(ins, REG_SEG_DS))
    {
        *seg = REG_SEG_DS;
        return TRUE;
    }
    if (INS_RegWContain(ins, REG_SEG_SS))
    {
        *seg = REG_SEG_SS;
        return TRUE;
    }
    return FALSE;
}

static VOID OnSegReference(UINT32 ireg, ADDRINT val, ADDRINT* seg_ref, ADDRINT pc, THREADID tid)
{
    REG reg = static_cast< REG >(ireg);
    std::ostringstream s;
    s << Header(tid, pc) << "reference via " << SegName(reg) << " " << SegSelector((UINT16)val);
    Out << PasteDisassembly(s.str(), Disassemblies->Get(pc)) << std::endl;

    // Verifying segment selector returned by value and by reference match
    assert((UINT16)val == (UINT16)*seg_ref);
}

static VOID OnSegGsOrFsReference(UINT32 ireg, ADDRINT val, ADDRINT* seg_ref, ADDRINT segBaseAddr, ADDRINT* segBaseAddrRef,
                                 ADDRINT pc, THREADID tid)
{
    REG reg = static_cast< REG >(ireg);
    std::ostringstream s;
    s << Header(tid, pc) << "reference via " << SegName(reg) << " " << SegSelector((UINT16)val) << " Segment base address (TLS) "
      << std::hex << "0x" << segBaseAddr;
    Out << PasteDisassembly(s.str(), Disassemblies->Get(pc)) << std::endl;

    // Verifying segment selector returned by value and by reference match
    assert((UINT16)val == (UINT16)*seg_ref);

    // Verifying thread area (TLS) is not 0.
    assert(segBaseAddr != 0);

    // Verifying GS/FS segment base address selector returned by value and by reference match
    assert(segBaseAddr == *segBaseAddrRef);
}

static VOID OnSegWriteBefore(UINT32 ireg, UINT16 val, ADDRINT* seg_ref, ADDRINT pc, THREADID tid)
{
    REG reg = static_cast< REG >(ireg);
    std::ostringstream s;
    s << Header(tid, pc) << "modify " << SegName(reg) << "=" << SegSelector(val);
    Out << PasteDisassembly(s.str(), Disassemblies->Get(pc)) << std::endl;

    // Verifying segment selector returned by value and by reference match
    assert((UINT16)val == (UINT16)*seg_ref);
}

static VOID OnSegWriteAfterContext(CONTEXT* ctxt)
{
    ADDRINT gsbase = PIN_GetContextReg(ctxt, REG_SEG_GS_BASE);

    // Verifying thread area (TLS) is not 0.
    assert(gsbase != 0);
}

static VOID OnSegWriteAfter(UINT32 ireg, UINT16 val, ADDRINT* seg_ref, ADDRINT* segBaseAddrRef, ADDRINT pc, THREADID tid)
{
    REG reg = static_cast< REG >(ireg);
    std::ostringstream s;
    s << Header(tid, pc) << "modify " << SegName(reg) << "=" << SegSelector(val) << " Segment base address (TLS) " << std::hex
      << "0x" << *segBaseAddrRef;
    Out << PasteDisassembly(s.str(), Disassemblies->Get(pc)) << std::endl;

    // Verifying segment selector returned by value and by reference match
    assert((UINT16)val == (UINT16)*seg_ref);

    // Verifying thread area (TLS) is not 0.
    assert(*segBaseAddrRef != 0);
}

static VOID OnSyscallBefore(ADDRINT num, ADDRINT arg1, ADDRINT arg2, ADDRINT arg3, ADDRINT arg4, ADDRINT arg5, ADDRINT fs,
                            ADDRINT gs, ADDRINT pc, THREADID tid)
{
    PendingSyscalls->Add(tid, PENDING_SYSCALL(fs, gs, num, arg1, arg2, arg3, arg4, arg5));

    switch (num)
    {
#if defined(TARGET_LINUX)
        case SYS_modify_ldt:
        {
            int func = static_cast< int >(arg1);
            if (func == 1)
            {
                USER_DESC* tls = reinterpret_cast< USER_DESC* >(arg2);
                Out << Header(tid, pc) << "modify_ldt(WRITE, " << UserDesc(tls) << ", 0x" << std::hex << arg3 << ")" << std::endl;
            }
            else
            {
                Out << Header(tid, pc) << "modify_ldt(READ, 0x" << std::hex << arg2 << ", 0x" << arg3 << ")" << std::endl;
            }
            break;
        }
#endif

#if defined(TARGET_LINUX) && defined(TARGET_IA32E)
        case SYS_arch_prctl:
            Out << Header(tid, pc) << "arch_prctl(" << PrctlFunc(arg1) << ", 0x" << std::hex << arg2 << ")" << std::endl;
            break;

        case SYS_clone:
        {
            int flags = static_cast< int >(arg1);
            if (flags & CLONE_SETTLS) Out << Header(tid, pc) << "clone(CLONE_SETTLS, 0x" << std::hex << arg5 << ")" << std::endl;
            break;
        }

        case SYS_set_thread_area:
            Out << Header(tid, pc) << "UNEXPECTED: set_thread_area" << std::endl;
            break;

        case SYS_get_thread_area:
            Out << Header(tid, pc) << "UNEXPECTED: get_thread_area" << std::endl;
            break;
#endif

#if defined(TARGET_LINUX) && defined(TARGET_IA32)
        case SYS_set_thread_area:
        {
            USER_DESC* tls = reinterpret_cast< USER_DESC* >(arg1);
            Out << Header(tid, pc) << "set_thread_area(" << UserDesc(tls) << ")" << std::endl;
            break;
        }

        case SYS_get_thread_area:
        {
            USER_DESC* tls = reinterpret_cast< USER_DESC* >(arg1);
            Out << Header(tid, pc) << "get_thread_area([entry_number=0x" << std::hex << tls->entry_number << "])" << std::endl;
            break;
        }

        case SYS_clone:
        {
            int flags = static_cast< int >(arg1);
            if (flags & CLONE_SETTLS)
            {
                USER_DESC* tls = reinterpret_cast< USER_DESC* >(arg4);
                Out << Header(tid, pc) << "clone(CLONE_SETTLS, " << UserDesc(tls) << ")" << std::endl;
            }
            break;
        }
#endif
    }
}

static VOID OnSyscallAfter(ADDRINT ret, ADDRINT fs, ADDRINT gs, ADDRINT pc, THREADID tid)
{
    PENDING_SYSCALL pend;
    if (!PendingSyscalls->Remove(tid, &pend)) return;

    switch (pend._number)
    {
#if defined(TARGET_LINUX)
        case SYS_modify_ldt:
            if (ret == ADDRINT(-1)) Out << Header(tid, pc) << "=>modify_ldt(FAILED)" << std::endl;
            break;
#endif

#if defined(TARGET_LINUX) && defined(TARGET_IA32E)
        case SYS_arch_prctl:
            if (ret == ADDRINT(-1)) Out << Header(tid, pc) << "=>arch_prctl(FAILED)" << std::endl;
            break;
#endif

#if defined(TARGET_LINUX) && defined(TARGET_IA32)
        case SYS_set_thread_area:
            if (ret == ADDRINT(-1))
            {
                Out << Header(tid, pc) << "=>set_thread_area(FAILED)" << std::endl;
            }
            else
            {
                USER_DESC* tls = reinterpret_cast< USER_DESC* >(pend._arg1);
                Out << Header(tid, pc) << "=>set_thread_area(" << UserDesc(tls) << ")" << std::endl;
            }
            break;

        case SYS_get_thread_area:
            if (ret == ADDRINT(-1))
            {
                Out << Header(tid, pc) << "=>get_thread_area(FAILED)" << std::endl;
            }
            else
            {
                USER_DESC* tls = reinterpret_cast< USER_DESC* >(pend._arg1);
                Out << Header(tid, pc) << "=>get_thread_area(" << UserDesc(tls) << ")" << std::endl;
            }
            break;
#endif
    }

    if (fs != pend._fs) Out << Header(tid, pc) << "syscall modified FS=" << SegSelector(fs) << std::endl;
    if (gs != pend._gs) Out << Header(tid, pc) << "syscall modified GS=" << SegSelector(gs) << std::endl;
}

static VOID SyscallEntry(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    OnSyscallBefore(PIN_GetSyscallNumber(ctxt, std), PIN_GetSyscallArgument(ctxt, std, 0), PIN_GetSyscallArgument(ctxt, std, 1),
                    PIN_GetSyscallArgument(ctxt, std, 2), PIN_GetSyscallArgument(ctxt, std, 3),
                    PIN_GetSyscallArgument(ctxt, std, 4), PIN_GetContextReg(ctxt, REG_SEG_FS),
                    PIN_GetContextReg(ctxt, REG_SEG_GS), PIN_GetContextReg(ctxt, REG_INST_PTR), threadIndex);
}

static VOID SyscallExit(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    OnSyscallAfter(PIN_GetSyscallReturn(ctxt, std), PIN_GetContextReg(ctxt, REG_SEG_FS), PIN_GetContextReg(ctxt, REG_SEG_GS),
                   PIN_GetContextReg(ctxt, REG_INST_PTR), threadIndex);
}

static std::string Header(THREADID tid, ADDRINT pc)
{
    std::ostringstream s;
    s << "tid " << std::dec << tid << ", pc 0x" << std::hex << pc << ": ";
    return s.str();
}

static std::string PasteDisassembly(const std::string& body, const std::string& dis)
{
    std::ostringstream s;
    s << std::left << std::setw(110) << body << dis;
    return s.str();
}

static std::string SegName(REG reg)
{
    switch (reg)
    {
        case REG_SEG_FS:
            return "FS";
        case REG_SEG_GS:
            return "GS";
        case REG_SEG_ES:
            return "ES";
        case REG_SEG_CS:
            return "CS";
        case REG_SEG_DS:
            return "DS";
        case REG_SEG_SS:
            return "SS";
        default:
            return "OTHER";
    }
}

static std::string SegSelector(ADDRINT val)
{
    std::ostringstream s;
    s << "[";
    if (val & 4)
        s << "LDT";
    else
        s << "GDT";
    s << " index=" << std::dec << (val >> 3);
    s << " priv=" << std::dec << (val & 3);
    s << "]";
    return s.str();
}

#if defined(TARGET_LINUX)
static std::string UserDesc(USER_DESC* tls)
{
    std::ostringstream s;
    s << "[";
    s << "index=" << std::dec << static_cast< INT32 >(tls->entry_number);
    s << " base=0x" << std::hex << tls->base_addr;
    s << "]";
    return s.str();
}
#endif

#if defined(TARGET_LINUX) && defined(TARGET_IA32E)
static std::string PrctlFunc(ADDRINT fun)
{
    switch (fun)
    {
        case ARCH_SET_GS:
            return "ARCH_SET_GS";
        case ARCH_SET_FS:
            return "ARCH_SET_FS";
        case ARCH_GET_FS:
            return "ARCH_GET_FS";
        case ARCH_GET_GS:
            return "ARCH_GET_GS";
        default:
        {
            std::ostringstream s;
            s << "0x" << std::hex << fun;
            return s.str();
        }
    }
}
#endif
