/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool instruments all memory accesses in order to verify that the IARG_MEMORY(READ/WRITE)_EA
 * is correct.
 */

#include <cstdio>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstdlib>

#ifdef TARGET_LINUX
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#if defined(TARGET_IA32E)
#include <asm/prctl.h>
#include <sys/prctl.h>
#endif // TARGET_IA32E
#endif // TARGET_LINUX

#include "pin.H"
#include "instlib.H"
using std::iostream;
using std::ostringstream;

#ifdef TARGET_WINDOWS
namespace WIND
{
#include <windows.h>
}
using namespace INSTLIB;
inline ADDRINT InitializeThreadData() { return reinterpret_cast< ADDRINT >(WIND::NtCurrentTeb()); }
#endif // TARGET_WINDOWS

#ifdef TARGET_LINUX

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

typedef struct
{
    unsigned int entry_number;
    unsigned int base_addr;
    unsigned int limit;
    unsigned int seg_32bit : 1;
    unsigned int contents : 2;
    unsigned int read_exec_only : 1;
    unsigned int limit_in_pages : 1;
    unsigned int seg_not_present : 1;
    unsigned int useable : 1;
} UserDesc;

#define TLS_GET_GS_REG()                           \
    (                                              \
        {                                          \
            int __seg;                             \
            __asm("movw %%gs, %w0" : "=q"(__seg)); \
            __seg & 0xffff;                        \
        })

#define TLS_SET_GS_REG(val) __asm("movw %w0, %%gs" ::"q"(val))

#define TLS_GET_FS_REG()                           \
    (                                              \
        {                                          \
            int __seg;                             \
            __asm("movw %%fs, %w0" : "=q"(__seg)); \
            __seg & 0xffff;                        \
        })

#define TLS_SET_FS_REG(val) __asm("movw %w0, %%fs" ::"q"(val))

ADDRINT GetTlsBaseAddress()
{
#ifdef TARGET_IA32
    unsigned int gsVal = TLS_GET_GS_REG();
    //printf("Current gs val is 0x%x\n", gsVal);
    UserDesc td;
    td.entry_number = gsVal >> 3;
    if (gsVal == 0) return 0;
    int res = syscall(SYS_get_thread_area, &td);
    if (res != 0)
    {
        printf("SYS_get_thread_area failed with error: %s\n", strerror(errno));
        return 0;
    }
    //printf("Current td.base_addr 0x%x\n", td.base_addr);
    return td.base_addr;
#else
    ADDRINT baseAddr;
    int res = syscall(SYS_arch_prctl, ARCH_GET_FS, &baseAddr);
    if (res != 0)
    {
        return 0;
    }
    return baseAddr;
#endif // TARGET_IA32
}

inline ADDRINT InitializeThreadData() { return GetTlsBaseAddress(); }

#endif // TARGET_LINUX

#define MAX_THREADS 1024
#define DISPLACEMENT_ONLY_ADDRESSING_READ_TYPE 0
#define DISPLACEMENT_ONLY_ADDRESSING_WRITE_TYPE 1
#define BASE_DISPLACEMENT_ADDRESSING_READ_TYPE 2
#define BASE_DISPLACEMENT_ADDRESSING_WRITE_TYPE 3
#define BASE_INDEX_DISPLACEMENT_ADDRESSING_READ_TYPE 4
#define BASE_INDEX_DISPLACEMENT_ADDRESSING_WRITE_TYPE 5
#define INDEX_DISPLACEMENT_ADDRESSING_READ_TYPE 6
#define INDEX_DISPLACEMENT_ADDRESSING_WRITE_TYPE 7

struct tdata
{
    ADDRINT threadTeb;

} THREAD_DATA;

//struct tdata
ADDRINT threadData[MAX_THREADS] = {0};

int numThreads = 0;
BOOL hadError  = FALSE;

const char* GetMemoryAccessTypeString(int j)
{
    switch (j)
    {
        case DISPLACEMENT_ONLY_ADDRESSING_READ_TYPE:
            return ("DISPLACEMENT_ONLY_ADDRESSING_READ_TYPE");
        case BASE_DISPLACEMENT_ADDRESSING_READ_TYPE:
            return ("BASE_DISPLACEMENT_ADDRESSING_READ_TYPE");
        case BASE_INDEX_DISPLACEMENT_ADDRESSING_READ_TYPE:
            return ("BASE_INDEX_DISPLACEMENT_ADDRESSING_READ_TYPE");
        case DISPLACEMENT_ONLY_ADDRESSING_WRITE_TYPE:
            return ("DISPLACEMENT_ONLY_ADDRESSING_WRITE_TYPE");
        case BASE_DISPLACEMENT_ADDRESSING_WRITE_TYPE:
            return ("BASE_DISPLACEMENT_ADDRESSING_WRITE_TYPE");
        case BASE_INDEX_DISPLACEMENT_ADDRESSING_WRITE_TYPE:
            return ("BASE_INDEX_DISPLACEMENT_ADDRESSING_WRITE_TYPE");
        case INDEX_DISPLACEMENT_ADDRESSING_WRITE_TYPE:
            return ("INDEX_DISPLACEMENT_ADDRESSING_WRITE_TYPE");
        case INDEX_DISPLACEMENT_ADDRESSING_READ_TYPE:
            return ("INDEX_DISPLACEMENT_ADDRESSING_READ_TYPE");
        default:
            return ("UNKNOWN_ADDRESSING_TYPE");
    }
}

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

    /*while( pc < stop )*/ {
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

VOID AnalyzeMemAddr(UINT32 effectiveAddressWidth, UINT32 hasSegmentedMemAccess, INT32 subFromAddrComputation, VOID* ip,
                    VOID* addr, UINT32 accessType, ADDRINT gaxRegVal, ADDRINT baseRegVal, ADDRINT indexRegVal, INT32 scale,
                    ADDRINT displacement, UINT32 insSizeToAdd, THREADID tid)
{
    if (tid < MAX_THREADS)
    {
        if (threadData[tid] == 0)
        {
            threadData[tid] = InitializeThreadData();
        }

        ADDRINT threadTeb                       = threadData[tid];
        ADDRINT memoryEA                        = reinterpret_cast< ADDRINT >(addr);
        ADDRINT baseRegValForAddressComputation = 0, indexRegValForAddressComputation = 0;

        switch (accessType)
        {
            case BASE_DISPLACEMENT_ADDRESSING_READ_TYPE:
            case BASE_DISPLACEMENT_ADDRESSING_WRITE_TYPE:
                baseRegValForAddressComputation  = baseRegVal;
                indexRegValForAddressComputation = 0;
                break;

            case DISPLACEMENT_ONLY_ADDRESSING_READ_TYPE:
            case DISPLACEMENT_ONLY_ADDRESSING_WRITE_TYPE:
                baseRegValForAddressComputation  = 0;
                indexRegValForAddressComputation = 0;
                break;

            case BASE_INDEX_DISPLACEMENT_ADDRESSING_READ_TYPE:
            case BASE_INDEX_DISPLACEMENT_ADDRESSING_WRITE_TYPE:
                baseRegValForAddressComputation  = baseRegVal;
                indexRegValForAddressComputation = indexRegVal;
                break;

            case INDEX_DISPLACEMENT_ADDRESSING_READ_TYPE:
            case INDEX_DISPLACEMENT_ADDRESSING_WRITE_TYPE:
                baseRegValForAddressComputation  = 0;
                indexRegValForAddressComputation = indexRegVal;
                break;

            default:
                ASSERTX(0);
                break;
        }
        ADDRINT computedAddr = static_cast< ADDRINT >(displacement) + static_cast< ADDRINT >(baseRegValForAddressComputation) +
                               (static_cast< ADDRINT >(indexRegValForAddressComputation) * scale) -
                               static_cast< ADDRINT >(subFromAddrComputation) + +static_cast< ADDRINT >(insSizeToAdd);
        if (hasSegmentedMemAccess)
        {
            computedAddr += static_cast< ADDRINT >(threadTeb);
        }
        if (computedAddr != memoryEA)
        {
            printf("**ERROR conflicting memoryEA   %s  ip: %p  (tid %x)\n  (computedAddr %p expectedEA %p teb %p isSegmented %x "
                   "displacement %p  baseRegVal %p indexRegVal %p scale %d\n   gaxRegVal %p)\n",
                   GetMemoryAccessTypeString(accessType), ip, tid, reinterpret_cast< VOID* >(computedAddr),
                   reinterpret_cast< VOID* >(memoryEA), reinterpret_cast< VOID* >(threadTeb), hasSegmentedMemAccess,
                   reinterpret_cast< VOID* >(displacement), reinterpret_cast< VOID* >(baseRegValForAddressComputation),
                   reinterpret_cast< VOID* >(indexRegValForAddressComputation), scale, reinterpret_cast< VOID* >(gaxRegVal));
            string s = disassemble(reinterpret_cast< ADDRINT >(ip), reinterpret_cast< ADDRINT >(ip) + 15);
            printf("    %s\n", s.c_str());
            hadError = TRUE;
            exit(-1);
        }
    }
}

// Handle BTx instructions, where some of the bit index is added to the EA
VOID AnalyzeBTMemAddr(UINT32 effectiveAddressWidth, UINT32 hasSegmentedMemAccess, ADDRINT bitIndex, VOID* ip, VOID* addr,
                      UINT32 accessType, ADDRINT gaxRegVal, ADDRINT baseRegVal, ADDRINT indexRegVal, INT32 scale,
                      ADDRINT displacement, UINT32 readSize, THREADID tid)
{
    // Compute the additional offset from the bitIndex and pass it down to AnalyzeMemAddr
    UINT32 shift = (readSize == 2) ? 4 : (readSize == 4) ? 5 : 6;

    // fprintf (stderr, "0x%08x: BTx bitIndex 0x%08x, addr 0x%08x, readsize %d\n", ip, bitIndex, addr, readSize);

    AnalyzeMemAddr(effectiveAddressWidth, hasSegmentedMemAccess,
                   0, // subFromAddrComputation
                   ip, addr, accessType, gaxRegVal, baseRegVal, indexRegVal, scale, displacement,
                   (bitIndex >> shift) * readSize, // insSizeToAdd (cheating, really, but it gets added in which is what we need).
                   tid);
}

#ifndef TARGET_LINUX
#ifdef TARGET_IA32
#define TESTED_SEG_REG REG_SEG_FS
#else
#define TESTED_SEG_REG REG_SEG_GS
#endif
#else
#ifdef TARGET_IA32
#define TESTED_SEG_REG REG_SEG_GS
#else
#define TESTED_SEG_REG REG_SEG_FS
#endif
#endif

BOOL doNotInstrumentSegmentedAccess = FALSE;

BOOL InstrumentMemAccess(INS ins, BOOL isRead)
{
    UINT32 i, scale = 0;
    ADDRINT displacement = 0;
    REG baseReg = REG_INVALID(), indexReg = REG_INVALID();
    UINT32 hasSegmentedMemAccess = 0;
    INT32 subFromAddrComputation = 0;
    OPCODE opcode                = INS_Opcode(ins);

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

    if (opcode == XED_ICLASS_PUSH || opcode == XED_ICLASS_PUSHA || opcode == XED_ICLASS_PUSHAD || opcode == XED_ICLASS_PUSHF ||
        opcode == XED_ICLASS_PUSHFD || opcode == XED_ICLASS_PUSHFQ || opcode == XED_ICLASS_CALL_NEAR ||
        opcode == XED_ICLASS_CALL_FAR || opcode == XED_ICLASS_ENTER)
    { // These all decrement the stack pointer before the write
        subFromAddrComputation = writeSize;
    }

    BOOL foundMemOp     = FALSE;
    UINT32 operandCount = INS_OperandCount(ins);
    for (i = 0; i < operandCount; i++)
    {
        if (INS_OperandIsMemory(ins, i) && ((isRead && INS_OperandRead(ins, i)) || (!isRead && INS_OperandWritten(ins, i))))
        {
            displacement          = INS_OperandMemoryDisplacement(ins, i);
            baseReg               = INS_OperandMemoryBaseReg(ins, i);
            indexReg              = INS_OperandMemoryIndexReg(ins, i);
            scale                 = INS_OperandMemoryScale(ins, i);
            hasSegmentedMemAccess = (INS_OperandMemorySegmentReg(ins, i) == TESTED_SEG_REG);
            foundMemOp            = TRUE;
            // printf ("op %d memop isRead %d baseReg %s indexReg %s scale %x\n",
            //        i, isRead, REG_StringShort(baseReg).c_str(), REG_StringShort(indexReg).c_str(),
            //        scale);
            break;
        }
    }
    ASSERTX(foundMemOp);
#ifdef TARGET_LINUX
    /* do not support segmented addresses on linux - due to virtual segments
    */
    if (hasSegmentedMemAccess)
    {
        return (FALSE);
    }
#endif
    UINT32 addressingType;
    IARG_TYPE iargMemoryEffectiveAddrType;
    UINT32 foundAddressingType = 0;
    if (!(baseReg != REG_INVALID() && indexReg == REG_INVALID()))
    {
        ASSERTX(!INS_HasMemoryRead2(ins));
    }
    if (isRead)
    {
        iargMemoryEffectiveAddrType = IARG_MEMORYREAD_EA;
        subFromAddrComputation      = 0;
    }
    else
    {
        iargMemoryEffectiveAddrType = IARG_MEMORYWRITE_EA;
    }
    if (baseReg != REG_INVALID())
    {
        if (indexReg != REG_INVALID())
        {
            if (isRead)
            {
                addressingType = BASE_INDEX_DISPLACEMENT_ADDRESSING_READ_TYPE;
            }
            else
            {
                addressingType = BASE_INDEX_DISPLACEMENT_ADDRESSING_WRITE_TYPE;
            }
            foundAddressingType++;
        }
        else // indexReg == REG_INVALID()
        {
            indexReg = REG_GDX;
            if (isRead)
            {
                addressingType = BASE_DISPLACEMENT_ADDRESSING_READ_TYPE;
            }
            else
            {
                addressingType = BASE_DISPLACEMENT_ADDRESSING_WRITE_TYPE;
            }
            foundAddressingType++;
        }
    }
    else // baseReg == REG_INVALID()
    {
        baseReg = REG_GDX;
        if (indexReg != REG_INVALID())
        {
            if (isRead)
            {
                addressingType = INDEX_DISPLACEMENT_ADDRESSING_READ_TYPE;
            }
            else
            {
                addressingType = INDEX_DISPLACEMENT_ADDRESSING_WRITE_TYPE;
            }

            foundAddressingType++;
        }
        else // indexReg == REG_INVALID()
        {
            indexReg = REG_GDX;
            if (isRead)
            {
                addressingType = DISPLACEMENT_ONLY_ADDRESSING_READ_TYPE;
            }
            else
            {
                addressingType = DISPLACEMENT_ONLY_ADDRESSING_WRITE_TYPE;
            }

            foundAddressingType++;
        }
    }
    ASSERTX(foundAddressingType == 1);
    UINT32 insSizeToAdd = 0;
    if (baseReg == REG_INST_PTR)
    {
        insSizeToAdd = INS_Size(ins);
    }

    // Need also to worry about instructions where some of the operand spills over into the
    // EA calculation. This applies to BTx with a register operand to hold the bit index.
    if ((opcode == XED_ICLASS_BT || opcode == XED_ICLASS_BTC || opcode == XED_ICLASS_BTR || opcode == XED_ICLASS_BTS) &&
        !INS_OperandIsImmediate(ins, 1))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)AnalyzeBTMemAddr, IARG_UINT32, INS_EffectiveAddressWidth(ins), IARG_UINT32,
                       hasSegmentedMemAccess, IARG_REG_VALUE, INS_OperandReg(ins, 1), // The bit index
                       IARG_INST_PTR, iargMemoryEffectiveAddrType, IARG_UINT32, addressingType, IARG_REG_VALUE, REG_GAX,
                       IARG_REG_VALUE, baseReg, IARG_REG_VALUE, indexReg, IARG_UINT32, scale, IARG_ADDRINT, displacement,
                       IARG_UINT32, readSize, IARG_THREAD_ID, IARG_END);
    }
    else
    {
        INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)AnalyzeMemAddr, IARG_UINT32, INS_EffectiveAddressWidth(ins),
                                 IARG_UINT32, hasSegmentedMemAccess, IARG_UINT32, subFromAddrComputation, IARG_INST_PTR,
                                 iargMemoryEffectiveAddrType, IARG_UINT32, addressingType, IARG_REG_VALUE, REG_GAX,
                                 IARG_REG_VALUE, baseReg, IARG_REG_VALUE, indexReg, IARG_UINT32, scale, IARG_ADDRINT,
                                 displacement, IARG_UINT32, insSizeToAdd, IARG_THREAD_ID, IARG_END);
    }
    if (isRead && INS_HasMemoryRead2(ins))
    {
        addressingType              = BASE_DISPLACEMENT_ADDRESSING_READ_TYPE;
        iargMemoryEffectiveAddrType = IARG_MEMORYREAD2_EA;
        subFromAddrComputation      = 0;

        INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)AnalyzeMemAddr, IARG_UINT32, INS_EffectiveAddressWidth(ins),
                                 IARG_UINT32, hasSegmentedMemAccess, IARG_UINT32, subFromAddrComputation, IARG_INST_PTR,
                                 iargMemoryEffectiveAddrType, IARG_UINT32, addressingType, IARG_REG_VALUE, REG_GAX,
                                 IARG_REG_VALUE, REG_GDI, /* Read2 is always GDI */
                                 IARG_REG_VALUE, indexReg, IARG_UINT32, scale, IARG_ADDRINT, displacement, IARG_UINT32,
                                 insSizeToAdd, IARG_THREAD_ID, IARG_END);
    }
    return (TRUE);
}

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID* v) {
#ifdef TARGET_LINUX
    {UINT32 operandCount = INS_OperandCount(ins);
UINT32 i;

for (i = 0; i < operandCount; i++)
{
    if (INS_OperandIsReg(ins, i) && REG_is_seg(INS_OperandReg(ins, i)) && INS_OperandWritten(ins, i))
    {
        printf("**WARNING SegOperand-WRITE not supported, no longer instrumenting segmented mem-accesses\n  %p: %s\n",
               reinterpret_cast< VOID* >(INS_Address(ins)), INS_Disassemble(ins).c_str());
        doNotInstrumentSegmentedAccess = TRUE;
    }
}
}
#endif
/*
    BOOL instrumentedRead = FALSE;
    BOOL instrumentedWrite = FALSE;
    if (INS_IsMemoryRead(ins))
    {
        instrumentedRead = InstrumentMemAccess (ins, TRUE);
    }
    if (INS_IsMemoryWrite(ins))
    {
        instrumentedWrite = InstrumentMemAccess (ins, FALSE);
    }

    if (instrumentedRead || instrumentedWrite)
    {
        fprintf (stdout, "Instrumented ins forRead %d forWrite %d: %x   %s\n",
                 instrumentedRead, instrumentedWrite, INS_Address(ins), INS_Disassemble(ins).c_str());
    }
    */

/*fprintf(trace, "%p %s\n", INS_Address(ins), INS_Disassemble(ins).c_str());
    */
}

VOID Fini(INT32 code, VOID* v)
{
    /*if (!hadError)
    {
        printf ("SUCCESS\n");
    }
    else
    {
        printf ("Had ERRORS - search above for string ERROR\n");
    }*/
}

PIN_LOCK pinLock;
VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    PIN_GetLock(&pinLock, threadid + 1);
    //fprintf(trace, "thread begin %x %x\n",threadid, numThreads);
    numThreads++;
    if (threadid < MAX_THREADS)
    {
#ifndef TARGET_LINUX
        threadData[threadid] = InitializeThreadData();
#endif
    }
    else
    {
        printf("ERROR - maximum #threads exceeded\n");
    }
    fflush(stdout);
    PIN_ReleaseLock(&pinLock);
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    PIN_InitLock(&pinLock);
    PIN_AddThreadStartFunction(ThreadStart, 0);
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
