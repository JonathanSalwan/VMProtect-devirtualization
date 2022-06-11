/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <iomanip>

#include "pin.H"
#include "CallStack.H"
#include "syscall_names.H"
#include "argv_readparam.h"

///////////////////////// Prototypes //////////////////////////////////////////

const string& Target2RtnName(ADDRINT target);
const string& Target2LibName(ADDRINT target);

///////////////////////// Global Variables ////////////////////////////////////

ostream* Output;

CallStack callStack(Target2RtnName, Target2LibName);

bool main_entry_seen = false;
bool prevIpDoesPush  = FALSE;

set< void* > addrsToDump;
set< ADDRINT > pushIps;

///////////////////////// Utility functions ///////////////////////////////////
void RecordPush(INS ins) { pushIps.insert(INS_Address(ins)); }

bool IpDoesPush(ADDRINT ip) { return (pushIps.find(ip) != pushIps.end()); }

const string& Target2RtnName(ADDRINT target)
{
    const string& name = RTN_FindNameByAddress(target);

    if (name == "")
        return *new string("[Unknown routine]");
    else
        return *new string(name);
}

const string& Target2LibName(ADDRINT target)
{
    PIN_LockClient();

    const RTN rtn = RTN_FindByAddress(target);
    static const string _invalid_rtn("[Unknown image]");

    string name;

    if (RTN_Valid(rtn))
    {
        name = IMG_Name(SEC_Img(RTN_Sec(rtn)));
    }
    else
    {
        name = _invalid_rtn;
    }

    PIN_UnlockClient();

    return *new string(name);
}

///////////////////////// Analysis Functions //////////////////////////////////

void A_RegisterAddr(void* addr) { addrsToDump.insert((void*)addr); }

void A_UnregisterAddr(void* addr)
{
    if (addrsToDump.find(addr) == addrsToDump.end())
    {
        cerr << "MAID ERROR: unregistered address " << hex << addr << dec << endl;
    }
    else
    {
        addrsToDump.erase(addr);
    }
}

void ProcessInst(ADDRINT ip) { prevIpDoesPush = IpDoesPush(ip); }

void A_ProcessSyscall(ADDRINT ip, UINT32 num, ADDRINT sp, ADDRINT arg0)
{
    if (main_entry_seen)
    {
        //cout << callStack.Depth() << ":" << SYS_SyscallName(num) << endl;
    }
    //callStack.ProcessSysCall(sp, target);
}

void A_ProcessDirectCall(ADDRINT ip, ADDRINT target, ADDRINT sp)
{
    //cout << "Direct call: " << Target2String(target) << endl;
    callStack.ProcessCall(sp, target);
}

void A_ProcessIndirectCall(ADDRINT ip, ADDRINT target, ADDRINT sp)
{
    //cout << "Indirect call: " << Target2String(target) << endl;
    callStack.ProcessCall(sp, target);
}

static void A_ProcessStub(ADDRINT ip, ADDRINT target, ADDRINT sp)
{
    //cout << "Instrumenting stub: " << Target2String(target) << endl;
    //cout << "STUB: ";
    //cout << Target2RtnName(target) << endl;
    callStack.ProcessCall(sp, target);
}

static void A_ProcessReturn(ADDRINT ip, ADDRINT sp) { callStack.ProcessReturn(sp, prevIpDoesPush); }

static void A_EnterMainImage(ADDRINT ip, ADDRINT target, ADDRINT sp)
{
    //assert(current_pc == main_entry_addr);
    //cout << "main" << endl;
    main_entry_seen = true;
    callStack.ProcessMainEntry(sp, target);
}

static void A_DoMem(bool isStore, void* ea, ADDRINT pc)
{
    string filename;
    int lineno;
    if (addrsToDump.find(ea) != addrsToDump.end())
    {
        PIN_LockClient();

        PIN_GetSourceLocation(pc, NULL, &lineno, &filename);

        PIN_UnlockClient();

        *Output << (isStore ? "store" : "load") << " pc=" << (void*)pc << " ea=" << ea << endl;
        if (filename != "")
        {
            *Output << filename << ":" << lineno;
        }
        else
        {
            *Output << "UNKNOWN:0";
        }
        *Output << endl;
        callStack.DumpStack(Output);
        *Output << endl;
    }
}

///////////////////////// Instrumentation functions ///////////////////////////

static BOOL IsPLT(TRACE trace)
{
    RTN rtn = TRACE_Rtn(trace);

    // All .plt thunks have a valid RTN
    if (!RTN_Valid(rtn)) return FALSE;

    if (".plt" == SEC_Name(RTN_Sec(rtn))) return TRUE;
    return FALSE;
}

static void I_Trace(TRACE trace, void* v)
{
    //FIXME if (PIN_IsSignalHandler()) {Sequence_ProcessSignalHandler(head)};

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        INS tail = BBL_InsTail(bbl);

        // All memory reads/writes
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            if (INS_IsMemoryRead(ins) || INS_HasMemoryRead2(ins) || INS_IsMemoryWrite(ins))
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)A_DoMem, IARG_BOOL, INS_IsMemoryWrite(ins),
                               (INS_IsMemoryWrite(ins) ? IARG_MEMORYWRITE_EA
                                                       : (INS_IsMemoryRead(ins) ? IARG_MEMORYREAD_EA : IARG_MEMORYREAD2_EA)),
                               IARG_INST_PTR, IARG_END);
            }
#if defined(TARGET_IA32) && defined(TARGET_WINDOWS)
            // on ia-32 windows need to identify
            // push
            // ret
            // in order to process callstack correctly
            if (ins != tail)
            {
                INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)ProcessInst, IARG_INST_PTR, IARG_END);
                if (INS_Opcode(ins) == XED_ICLASS_PUSH)
                {
                    RecordPush(ins);
                }
            }
#endif
        }

        // All calls and returns
        if (INS_IsSyscall(tail))
        {
            INS_InsertPredicatedCall(tail, IPOINT_BEFORE, (AFUNPTR)A_ProcessSyscall, IARG_INST_PTR, IARG_SYSCALL_NUMBER,
                                     IARG_REG_VALUE, REG_STACK_PTR, IARG_SYSARG_VALUE, 0, IARG_END);
        }
        else
        {
            if (INS_IsCall(tail))
            {
                if (INS_IsDirectControlFlow(tail))
                {
                    ADDRINT target = INS_DirectControlFlowTargetAddress(tail);
                    INS_InsertPredicatedCall(tail, IPOINT_BEFORE, (AFUNPTR)A_ProcessDirectCall, IARG_INST_PTR, IARG_ADDRINT,
                                             target, IARG_REG_VALUE, REG_STACK_PTR, IARG_END);
                }
                else if (!IsPLT(trace))
                {
                    INS_InsertPredicatedCall(tail, IPOINT_BEFORE, (AFUNPTR)A_ProcessIndirectCall, IARG_INST_PTR,
                                             IARG_BRANCH_TARGET_ADDR, IARG_REG_VALUE, REG_STACK_PTR, IARG_END);
                }
            }
            if (IsPLT(trace))
            {
                INS_InsertCall(tail, IPOINT_BEFORE, (AFUNPTR)A_ProcessStub, IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR,
                               IARG_REG_VALUE, REG_STACK_PTR, IARG_END);
            }
            if (INS_IsRet(tail))
            {
                INS_InsertPredicatedCall(tail, IPOINT_BEFORE, (AFUNPTR)A_ProcessReturn, IARG_INST_PTR, IARG_REG_VALUE,
                                         REG_STACK_PTR, IARG_END);
            }
        }
    }
}

static void I_ImageLoad(IMG img, void* v)
{
    static bool main_rtn_instrumented = false;

    if (!main_rtn_instrumented)
    {
        RTN rtn = RTN_FindByName(img, "main");
        if (rtn == RTN_Invalid())
        {
            rtn = RTN_FindByName(img, "__libc_start_main");
        }
        // Instrument main
        if (rtn != RTN_Invalid())
        {
            main_rtn_instrumented = true;
            RTN_Open(rtn);
            RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)A_EnterMainImage, IARG_INST_PTR, IARG_ADDRINT, RTN_Address(rtn),
                           IARG_REG_VALUE, REG_STACK_PTR, IARG_END);
            RTN_Close(rtn);
        }
    }

    for (SYM sym = IMG_RegsymHead(img); SYM_Valid(sym); sym = SYM_Next(sym))
    {
        if (strstr(SYM_Name(sym).c_str(), "MAID_register_address"))
        {
            RTN rtn;
            rtn = RTN_FindByName(img, SYM_Name(sym).c_str());
            ASSERTX(RTN_Valid(rtn));

            RTN_Open(rtn);
            RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)A_RegisterAddr, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
            RTN_Close(rtn);
        }
        else if (strstr(SYM_Name(sym).c_str(), "MAID_unregister_address"))
        {
            RTN rtn;
            rtn = RTN_FindByName(img, SYM_Name(sym).c_str());
            ASSERTX(RTN_Valid(rtn));
            RTN_Open(rtn);
            RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)A_UnregisterAddr, IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
            RTN_Close(rtn);
        }
    }
}

///////////////////////// main ////////////////////////////////////////////////

int main(int argc, char** argv)
{
    char* strTmp;

    PIN_Init(argc, argv);
    PIN_InitSymbols();

    if ((strTmp = argv_getString(argc, argv, "--outfile=", NULL)) != NULL)
    {
        if (!(Output = new ofstream(strTmp)))
        {
            perror(strTmp);
            exit(1);
        }
    }
    else
    {
        Output = &cout;
    }

    if ((strTmp = argv_getString(argc, argv, "--addrfile=", NULL)) != NULL)
    {
        string s;
        ifstream infile(strTmp);
        if (!infile)
        {
            perror(s.c_str());
            exit(1);
        }
        infile >> hex;
        while (!infile.eof())
        {
            static void* addr;
            infile >> addr;
            addrsToDump.insert((void*)addr);
        }
    }

    IMG_AddInstrumentFunction(I_ImageLoad, 0);
    TRACE_AddInstrumentFunction(I_Trace, 0);
    PIN_StartProgram();
}
