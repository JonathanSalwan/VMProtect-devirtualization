/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>
#include "pin.H"
#include "instlib.H"
using std::hex;
using std::ios;
using std::ofstream;

ofstream OutFile;

// Counters
static UINT64 icountMemRead  = 0;
static UINT64 icountMemRead2 = 0;
static UINT64 icountMemWrite = 0;
static UINT64 icountMemOp    = 0;
static UINT64 icountMemCall  = 0;
static ADDRINT lastIp        = 0;
static ADDRINT lastReadAddr  = 0;
static ADDRINT lastWriteAddr = 0;
static const ADDRINT mask(~(16 - 1));
static UINT64 errors              = 0;
static size_t lastBytes           = 0;
static BOOL lastIsPrefetch        = 0;
static BOOL lastIsRmw             = 0;
static BOOL lastIsAtomic          = 0;
static volatile THREADID myThread = INVALID_THREADID;

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    if (INVALID_THREADID == myThread) myThread = threadid;
}

VOID PIN_FAST_ANALYSIS_CALL readXmmMemoryFunc(ADDRINT memea_callback, UINT32 bytes, string* dis, ADDRINT ip)
{
    // case of multithreading - we care just about the main thread
    if (PIN_ThreadId() != myThread) return;

    // Check xmm size
    if (16 != lastBytes || bytes != lastBytes)
    {
        OutFile << "XMM bytes error found: " << lastBytes << " and not 16 for " << *dis << endl;
        errors++;
    }
}

VOID PIN_FAST_ANALYSIS_CALL verifyPrefetchFunc(ADDRINT memea_callback, UINT32 bytes, string* dis, ADDRINT ip)
{
    // case of multithreading - we care just about the main thread
    if (PIN_ThreadId() != myThread) return;

    if (!lastIsPrefetch)
    {
        OutFile << "Prefetch flag not set: " << *dis << endl;
        errors++;
    }
}

VOID PIN_FAST_ANALYSIS_CALL verifyRmwFunc(ADDRINT memea_callback, UINT32 bytes, string* dis, ADDRINT ip)
{
    // case of multithreading - we care just about the main thread
    if (PIN_ThreadId() != myThread) return;

    if (!lastIsRmw)
    {
        OutFile << "RMW flag not set: " << *dis << endl;
        errors++;
    }
}

VOID PIN_FAST_ANALYSIS_CALL verifyAtomicFunc(ADDRINT memea_callback, UINT32 bytes, string* dis, ADDRINT ip)
{
    // case of multithreading - we care just about the main thread
    if (PIN_ThreadId() != myThread) return;

    if (!lastIsAtomic)
    {
        OutFile << "Atomic flag not set: " << *dis << endl;
        errors++;
    }
}

VOID PIN_FAST_ANALYSIS_CALL readMemoryFunc(ADDRINT memea_orig, ADDRINT memea_callback, THREADID threadIndex, string* dis,
                                           ADDRINT ip)
{
    // case of multithreading - we care just about the main thread
    if (PIN_ThreadId() != myThread) return;

    if (ip != lastIp)
    {
        OutFile << "read analysis missing: " << hex << ip << " " << *dis << endl;
        errors++;
    }

    if (memea_orig != lastReadAddr)
    {
        OutFile << "read orig address incorrect: " << hex << memea_orig << " " << *dis << endl;
        errors++;
    }

    if ((memea_callback & 0xf) != 0)
    {
        OutFile << "read address not 16 aligned: " << hex << memea_callback << " " << *dis << endl;
        errors++;
    }

    if ((memea_orig & mask) != memea_callback)
    {
        OutFile << "read addresses incorrect: " << hex << memea_orig << " " << memea_callback << " " << *dis << endl;
        errors++;
    }

    icountMemRead++;
}

VOID PIN_FAST_ANALYSIS_CALL read2MemoryFunc(ADDRINT memea_orig, ADDRINT memea_callback, ADDRINT memea2_orig,
                                            ADDRINT memea2_callback, THREADID threadIndex, string* dis, CONTEXT* ctxt, ADDRINT ip)
{
    // case of multithreading - we care just about the main thread
    if (PIN_ThreadId() != myThread) return;

    if (ip != lastIp)
    {
        OutFile << "read2 analysis missing: "
                << " " << *dis << endl;
        errors++;
    }

    if ((memea_callback & 0xf) != 0)
    {
        OutFile << "read2 first address not 16 aligned: " << hex << memea_callback << " " << *dis << endl;
        errors++;
    }

    if ((memea_orig & mask) != memea_callback)
    {
        OutFile << "read2 first addresses incorrect: " << hex << memea_orig << " " << memea_callback << " " << *dis << endl;
        errors++;
    }

    if ((memea2_callback & 0xf) != 0)
    {
        OutFile << "read2 second address not 16 aligned: " << hex << memea2_callback << " " << *dis << endl;
        errors++;
    }

    if ((memea_orig & mask) != memea_callback)
    {
        OutFile << "read2 second addresses incorrect: " << hex << memea2_orig << " " << memea2_callback << " " << *dis << endl;
        errors++;
    }

    icountMemRead2++;
}

VOID PIN_FAST_ANALYSIS_CALL writeMemoryFunc(THREADID threadIndex, ADDRINT memea_orig, ADDRINT memea_callback, ADDRINT ip,
                                            string* dis)
{
    // case of multithreading - we care just about the main thread
    if (PIN_ThreadId() != myThread) return;

    if (ip != lastIp)
    {
        OutFile << "write analysis missing: "
                << " " << *dis << endl;
        errors++;
    }

    if (memea_orig != lastWriteAddr)
    {
        OutFile << "write orig address incorrect: " << hex << memea_orig << " " << ip << " " << *dis << endl;
        errors++;
    }

    if ((memea_callback & 0xf) != 0)
    {
        OutFile << "write address not 16 aligned: " << hex << memea_callback << " " << *dis << endl;
        errors++;
    }

    if ((memea_orig & mask) != memea_callback)
    {
        OutFile << "write addresses incorrect: " << hex << memea_orig << " " << memea_callback << " " << *dis << endl;
        errors++;
    }

    icountMemWrite++;
}

VOID PIN_FAST_ANALYSIS_CALL opMemoryFunc(ADDRINT memea_orig, ADDRINT memea_callback, UINT32 bytes, ADDRINT ip, string* dis)
{
    // case of multithreading - we care just about the main thread
    if (PIN_ThreadId() != myThread) return;

    if (ip != lastIp)
    {
        OutFile << "op analysis missing: "
                << " " << *dis << endl;
        errors++;
    }

    if (bytes != lastBytes)
    {
        OutFile << "op bytes error found: " << bytes << " and not " << lastBytes << " for " << *dis << endl;
        errors++;
    }

    if ((memea_callback & 0xf) != 0)
    {
        OutFile << "op address not 16 aligned: " << hex << memea_callback << " " << *dis << endl;
        errors++;
    }

    if ((memea_orig & mask) != memea_callback)
    {
        OutFile << "op addresses incorrect: " << hex << memea_orig << " " << memea_callback << " " << ip << " " << *dis << endl;
        errors++;
    }

    icountMemOp++;
}

ADDRINT PIN_FAST_ANALYSIS_CALL memoryCallback(PIN_MEM_TRANS_INFO* memTransInfo, VOID* v)
{
    // Test the threadIndex field (Mantis 0003429)
    if (memTransInfo->threadIndex != PIN_ThreadId())
    {
        cout << "PIN_MEM_TRANS_INFO.threadIndex bad value " << memTransInfo->threadIndex << " (should be " << PIN_ThreadId()
             << ")" << endl;
        errors++;
    }

    if (memTransInfo->flags.bits.isFromPin)
    {
        // PIN Internal memory dereference
        return memTransInfo->addr;
    }
    if (PIN_ThreadId() == myThread)
    {
        icountMemCall++;
        lastIp         = memTransInfo->ip;
        lastBytes      = memTransInfo->bytes;
        lastIsAtomic   = memTransInfo->flags.bits.isAtomic;
        lastIsPrefetch = memTransInfo->flags.bits.isPrefetch;
        lastIsRmw      = memTransInfo->flags.bits.isRmw;
    }

    if (memTransInfo->memOpType == PIN_MEMOP_STORE)
    {
        if (PIN_ThreadId() == myThread) lastWriteAddr = memTransInfo->addr;

        // Verify that we can call PIN API functions inside PIN
        PIN_SafeCopy((void*)memTransInfo->addr, (void*)memTransInfo->addr, memTransInfo->bytes);
    }
    else if (PIN_ThreadId() == myThread)
        lastReadAddr = memTransInfo->addr;

    // Check void parameter
    if ((ADDRINT)v != 0xa5a5a5a5)
    {
        OutFile << "v incorrect inside callback: " << hex << v << endl;
        errors++;
    }

    //OutFile << "callback addresses: " << hex << (memTransInfo->addr)  << " " << (memTransInfo->addr & mask) << " " <<memTransInfo->ip  << endl;

    return (memTransInfo->addr & mask);
}

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID* v)
{
    string* disptr = new string(INS_Disassemble(ins));

    // reads
    if (INS_IsMemoryRead(ins))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)readMemoryFunc, IARG_FAST_ANALYSIS_CALL, IARG_MEMORYREAD_EA,
                       IARG_MEMORYREAD_PTR, IARG_THREAD_ID, IARG_PTR, disptr, IARG_INST_PTR, IARG_END);
    }

    // Handle read from memory to XMM
    if (INS_Opcode(ins) == XED_ICLASS_MOVDQU && INS_IsMemoryRead(ins) && INS_OperandIsReg(ins, 0) && INS_OperandIsMemory(ins, 1))
    {
        if (REG_is_xmm(INS_OperandReg(ins, 0)))
        {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)readXmmMemoryFunc, IARG_FAST_ANALYSIS_CALL, IARG_MEMORYREAD_PTR,
                           IARG_MEMORYREAD_SIZE, IARG_PTR, disptr, IARG_INST_PTR, IARG_END);
        }
    }

    // Handle Prefetch
    if (INS_IsMemoryRead(ins) && xed_decoded_inst_is_prefetch(INS_XedDec(ins)))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)verifyPrefetchFunc, IARG_FAST_ANALYSIS_CALL, IARG_MEMORYREAD_PTR,
                       IARG_MEMORYREAD_SIZE, IARG_PTR, disptr, IARG_INST_PTR, IARG_END);
    }

    // Handle Atomic
    if (INS_IsAtomicUpdate(ins))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)verifyAtomicFunc, IARG_FAST_ANALYSIS_CALL, IARG_MEMORYWRITE_PTR,
                       IARG_MEMORYWRITE_SIZE, IARG_PTR, disptr, IARG_INST_PTR, IARG_END);
    }

    // writes
    if (INS_IsMemoryWrite(ins))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)writeMemoryFunc, IARG_FAST_ANALYSIS_CALL, IARG_THREAD_ID, IARG_MEMORYWRITE_EA,
                       IARG_MEMORYWRITE_PTR, IARG_INST_PTR, IARG_PTR, disptr, IARG_END);
    }

    UINT32 memOperands = INS_MemoryOperandCount(ins);
    if (!INS_IsVgather(ins) && memOperands)
    {
        // OPs
        for (UINT32 memOp = 0; memOp < memOperands; memOp++)
        {
            if (INS_MemoryOperandIsRead(ins, memOp) || INS_MemoryOperandIsWritten(ins, memOp))
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)opMemoryFunc, IARG_FAST_ANALYSIS_CALL, IARG_MEMORYOP_EA, memOp,
                               IARG_MEMORYOP_PTR, memOp, IARG_UINT32, INS_MemoryOperandSize(ins, memOp), IARG_INST_PTR, IARG_PTR,
                               disptr, IARG_END);
            }

            // Handle RMW
            if (INS_MemoryOperandIsRead(ins, memOp) && INS_MemoryOperandIsWritten(ins, memOp))
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)verifyRmwFunc, IARG_FAST_ANALYSIS_CALL, IARG_MEMORYOP_PTR, memOp,
                               IARG_UINT32, INS_MemoryOperandSize(ins, memOp), IARG_PTR, disptr, IARG_INST_PTR, IARG_END);
            }
        }
    }

    // READ2
    if (INS_HasMemoryRead2(ins))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)read2MemoryFunc, IARG_FAST_ANALYSIS_CALL, IARG_MEMORYREAD_EA,
                       IARG_MEMORYREAD_PTR, IARG_MEMORYREAD2_EA, IARG_MEMORYREAD2_PTR, IARG_THREAD_ID, IARG_PTR, disptr,
                       IARG_CONTEXT, IARG_INST_PTR, IARG_END);
    }
}

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "memaddrcall.out", "specify output file name");

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v)
{
    // Write to a file since cout and cerr maybe closed by the application
    OutFile.setf(ios::showbase);
    OutFile << "Count Mem Reads " << icountMemRead << endl;
    OutFile << "Count Mem Read2s " << icountMemRead2 << endl;
    OutFile << "Count Mem Writes " << icountMemWrite << endl;
    OutFile << "Count Mem Ops " << icountMemOp << endl;
    OutFile << "Count Mem callbacks " << icountMemCall << endl;
    OutFile << "Errors " << errors << endl;
    OutFile.close();

    // If we have errors then terminate abnormally
    if (errors)
    {
        cout << "Test memory_addr_callback is terminated cause found " << errors << " errors " << endl;
        PIN_ExitProcess(errors);
    }
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool tests memory address translation callback" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    ADDRINT dummy = 0xa5a5a5a5;

    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    PIN_AddThreadStartFunction(ThreadStart, NULL);

    OutFile.open(KnobOutputFile.Value().c_str());

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Verify that the PIN API is null before registration
    if (PIN_GetMemoryAddressTransFunction())
    {
        cout << "Test memory_addr_callback found PIN API callback not null before registration " << endl;
        PIN_ExitProcess(-1);
    }

    // Register memory callback
    PIN_AddMemoryAddressTransFunction(memoryCallback, (VOID*)dummy);

    // Verify that the PIN API is not null after registration
    if (!PIN_GetMemoryAddressTransFunction())
    {
        cout << "Test memory_addr_callback found PIN API callback null after registration " << endl;
        PIN_ExitProcess(-1);
    }

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return errors;
}
