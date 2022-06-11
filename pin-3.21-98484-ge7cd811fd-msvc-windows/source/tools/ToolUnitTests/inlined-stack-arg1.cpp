/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
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
#ifdef TARGET_IA32E
#include <asm/prctl.h>
#include <sys/prctl.h>
#endif // TARGET_IA32E
#endif // TARGET_LINUX

#include "pin.H"
#include "instlib.H"
using std::iostream;
using std::ostringstream;

// windows.h must be included after pin.H
#ifdef TARGET_WINDOWS
namespace WIND
{
#include <windows.h>
}
#endif // TARGET_WINDOWS

FILE* log_inl;
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "inlined-stack-arg1.out", "output file");

typedef struct
{
    ADDRINT tid;
    ADDRINT pc;
    ADDRINT flagsVal;
    ADDRINT nextAddress;
    UINT32 readRegCount;
    UINT32 writeRegCount;
    UINT32 instrSize;
    UINT8 readByte;
    UINT16 readWord;
    UINT32 readDWord;
    ADDRINT readAddrInt;
    UINT8 readValue1;
    UINT16 readValue2;
    UINT32 readValue3;
    INT32 readValue4;
    INT32 readValue5;
    INT32 readValue6;
} AnalysisInfo;

AnalysisInfo accessInfo;
AnalysisInfo accessInfoNonFastCall;
AnalysisInfo accessInfoFastCall;

//main app thread utils

THREADID myThread = INVALID_THREADID;

VOID ThreadStart(THREADID threadid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    if (myThread == INVALID_THREADID)
    {
        myThread = threadid;
    }
}

ADDRINT IfMyThread(THREADID threadId) { return threadId == myThread; }

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

VOID RecordInstructionInfo(UINT32 dummy1, UINT32 dummy2, UINT32 dummy3, UINT32 dummy4, ADDRINT tid, ADDRINT pcval,
                           ADDRINT flagsval, ADDRINT nxtaddr, UINT32 rregcnt, UINT32 wregcnt, ADDRINT inssz, ADDRINT readValue,
                           UINT8 readValue1, UINT16 readValue2, UINT32 readValue3, INT8 readValue4, INT16 readValue5,
                           INT32 readValue6)
{
    accessInfoNonFastCall.tid           = tid;
    accessInfoNonFastCall.pc            = pcval;
    accessInfoNonFastCall.flagsVal      = flagsval;
    accessInfoNonFastCall.nextAddress   = nxtaddr;
    accessInfoNonFastCall.readRegCount  = rregcnt;
    accessInfoNonFastCall.writeRegCount = wregcnt;
    accessInfoNonFastCall.instrSize     = inssz;
    accessInfoNonFastCall.readByte      = readValue;
    accessInfoNonFastCall.readWord      = readValue;
    accessInfoNonFastCall.readDWord     = readValue;
    accessInfoNonFastCall.readAddrInt   = readValue;
    accessInfoNonFastCall.readValue1    = readValue1;
    accessInfoNonFastCall.readValue2    = readValue2;
    accessInfoNonFastCall.readValue3    = readValue3;
    accessInfoNonFastCall.readValue4    = readValue4;
    accessInfoNonFastCall.readValue5    = readValue5;
    accessInfoNonFastCall.readValue6    = readValue6;
}

VOID PIN_FAST_ANALYSIS_CALL RecordInstructionInfoFastCall(ADDRINT tid, ADDRINT pcval, ADDRINT flagsval, ADDRINT nxtaddr,
                                                          UINT32 rregcnt, UINT32 wregcnt, ADDRINT inssz, ADDRINT readValue,
                                                          UINT8 readValue1, UINT16 readValue2, UINT32 readValue3, INT8 readValue4,
                                                          INT16 readValue5, INT32 readValue6)
{
    accessInfoFastCall.tid           = tid;
    accessInfoFastCall.pc            = pcval;
    accessInfoFastCall.flagsVal      = flagsval;
    accessInfoFastCall.nextAddress   = nxtaddr;
    accessInfoFastCall.readRegCount  = rregcnt;
    accessInfoFastCall.writeRegCount = wregcnt;
    accessInfoFastCall.instrSize     = inssz;
    accessInfoFastCall.readByte      = readValue;
    accessInfoFastCall.readWord      = readValue;
    accessInfoFastCall.readDWord     = readValue;
    accessInfoFastCall.readAddrInt   = readValue;
    accessInfoFastCall.readValue1    = readValue1;
    accessInfoFastCall.readValue2    = readValue2;
    accessInfoFastCall.readValue3    = readValue3;
    accessInfoFastCall.readValue4    = readValue4;
    accessInfoFastCall.readValue5    = readValue5;
    accessInfoFastCall.readValue6    = readValue6;
}

VOID VerifyInstructionInfo(ADDRINT tid, ADDRINT pcval, ADDRINT flagsval, ADDRINT nxtaddr, UINT32 rregcnt, UINT32 wregcnt,
                           ADDRINT inssz, ADDRINT readValue, UINT8 readValue1, UINT16 readValue2, UINT32 readValue3,
                           INT8 readValue4, INT16 readValue5, INT32 readValue6)
{
    BOOL hadError = FALSE;

    accessInfo.tid           = tid;
    accessInfo.pc            = pcval;
    accessInfo.flagsVal      = flagsval;
    accessInfo.nextAddress   = nxtaddr;
    accessInfo.readRegCount  = rregcnt;
    accessInfo.writeRegCount = wregcnt;
    accessInfo.instrSize     = inssz;
    accessInfo.readByte      = readValue;
    accessInfo.readWord      = readValue;
    accessInfo.readDWord     = readValue;
    accessInfo.readAddrInt   = readValue;
    accessInfo.readValue1    = readValue1;
    accessInfo.readValue2    = readValue2;
    accessInfo.readValue3    = readValue3;
    accessInfo.readValue4    = readValue4;
    accessInfo.readValue5    = readValue5;
    accessInfo.readValue6    = readValue6;
    if (accessInfoNonFastCall.tid != accessInfo.tid)
    {
        fprintf(log_inl, "***tid in Error\n");
        hadError = TRUE;
    }
    if (accessInfoNonFastCall.pc != accessInfo.pc)
    {
        fprintf(log_inl, "***pc in Error\n");
        hadError = TRUE;
    }
    if (accessInfoNonFastCall.flagsVal != accessInfo.flagsVal)
    {
        fprintf(log_inl, "***flagsVal in Error\n");
        hadError = TRUE;
    }
    if (accessInfoNonFastCall.nextAddress != accessInfo.nextAddress)
    {
        fprintf(log_inl, "***nextAddress in Error\n");
        hadError = TRUE;
    }
    if (accessInfoNonFastCall.readRegCount != accessInfo.readRegCount)
    {
        fprintf(log_inl, "***readRegCount in Error\n");
        hadError = TRUE;
    }
    if (accessInfoNonFastCall.writeRegCount != accessInfo.writeRegCount)
    {
        fprintf(log_inl, "***writeRegCount in Error\n");
        hadError = TRUE;
    }
    if (accessInfoNonFastCall.instrSize != accessInfo.instrSize)
    {
        fprintf(log_inl, "***instrSize in Error\n");
        hadError = TRUE;
    }
    if (accessInfoNonFastCall.readByte != accessInfo.readByte)
    {
        fprintf(log_inl, "***readByte in Error\n");
        hadError = TRUE;
    }
    if (accessInfoNonFastCall.readWord != accessInfo.readWord)
    {
        fprintf(log_inl, "***readWord in Error\n");
        hadError = TRUE;
    }
    if (accessInfoNonFastCall.readDWord != accessInfo.readDWord)
    {
        fprintf(log_inl, "***readDWord in Error\n");
        hadError = TRUE;
    }
    if (accessInfoNonFastCall.readAddrInt != accessInfo.readAddrInt)
    {
        fprintf(log_inl, "***readAddrInt in Error\n");
        hadError = TRUE;
    }
    if (accessInfoNonFastCall.readValue1 != accessInfo.readValue1)
    {
        fprintf(log_inl, "***readValue1 in Error\n");
        hadError = TRUE;
    }
    if (accessInfoNonFastCall.readValue2 != accessInfo.readValue2)
    {
        fprintf(log_inl, "***readValue2 in Error\n");
        hadError = TRUE;
    }
    if (accessInfoNonFastCall.readValue3 != accessInfo.readValue3)
    {
        fprintf(log_inl, "***readValue3 in Error\n");
        hadError = TRUE;
    }
    if (accessInfoNonFastCall.readValue4 != accessInfo.readValue4)
    {
        fprintf(log_inl, "***readValue4 in Error\n");
        hadError = TRUE;
    }
    if (accessInfoNonFastCall.readValue5 != accessInfo.readValue5)
    {
        fprintf(log_inl, "***readValue5 in Error\n");
        hadError = TRUE;
    }
    if (accessInfoNonFastCall.readValue6 != accessInfo.readValue6)
    {
        fprintf(log_inl, "***readValue6 in Error\n");
        hadError = TRUE;
    }
    if (hadError)
    {
        fprintf(log_inl, "In accessInfoNonFastCall\n");
        string s = disassemble((pcval), (pcval) + 15);
        fprintf(log_inl, "    %s\n", s.c_str());
        exit(-1);
    }
    hadError = FALSE;
    if (accessInfoFastCall.tid != accessInfo.tid)
    {
        fprintf(log_inl, "***tid in Error\n");
        hadError = TRUE;
    }
    if (accessInfoFastCall.pc != accessInfo.pc)
    {
        fprintf(log_inl, "***pc in Error\n");
        hadError = TRUE;
    }
    if (accessInfoFastCall.flagsVal != accessInfo.flagsVal)
    {
        fprintf(log_inl, "***flagsVal in Error\n");
        hadError = TRUE;
    }
    if (accessInfoFastCall.nextAddress != accessInfo.nextAddress)
    {
        fprintf(log_inl, "***nextAddress in Error\n");
        hadError = TRUE;
    }
    if (accessInfoFastCall.readRegCount != accessInfo.readRegCount)
    {
        fprintf(log_inl, "***readRegCount in Error\n");
        hadError = TRUE;
    }
    if (accessInfoFastCall.writeRegCount != accessInfo.writeRegCount)
    {
        fprintf(log_inl, "***writeRegCount in Error\n");
        hadError = TRUE;
    }
    if (accessInfoFastCall.instrSize != accessInfo.instrSize)
    {
        fprintf(log_inl, "***instrSize in Error\n");
        hadError = TRUE;
    }
    if (accessInfoFastCall.readByte != accessInfo.readByte)
    {
        fprintf(log_inl, "***readByte in Error\n");
        hadError = TRUE;
    }
    if (accessInfoFastCall.readWord != accessInfo.readWord)
    {
        fprintf(log_inl, "***readWord in Error\n");
        hadError = TRUE;
    }
    if (accessInfoFastCall.readDWord != accessInfo.readDWord)
    {
        fprintf(log_inl, "***readDWord in Error\n");
        hadError = TRUE;
    }
    if (accessInfoFastCall.readAddrInt != accessInfo.readAddrInt)
    {
        fprintf(log_inl, "***readAddrInt in Error\n");
        hadError = TRUE;
    }
    if (accessInfoFastCall.readValue1 != accessInfo.readValue1)
    {
        fprintf(log_inl, "***readValue1 in Error\n");
        hadError = TRUE;
    }
    if (accessInfoFastCall.readValue2 != accessInfo.readValue2)
    {
        fprintf(log_inl, "***readValue2 in Error\n");
        hadError = TRUE;
    }
    if (accessInfoFastCall.readValue3 != accessInfo.readValue3)
    {
        fprintf(log_inl, "***readValue3 in Error\n");
        hadError = TRUE;
    }
    if (hadError)
    {
        fprintf(log_inl, "In accessInfoFastCall\n");
        string s = disassemble((pcval), (pcval) + 15);
        fprintf(log_inl, "    %s\n", s.c_str());
        exit(-1);
    }
}
CONTEXT contextAtVerify;
CONTEXT contextAtRecordFastCall;
CONTEXT contextAtRecordNonFastCall;

VOID RecordContext(UINT32 dummy1, UINT32 dummy2, UINT32 dummy3, UINT32 dummy4, CONTEXT* context, THREADID tid)
{
    if (tid == myThread)
    {
        PIN_SaveContext(context, &contextAtRecordNonFastCall);
    }
}

VOID PIN_FAST_ANALYSIS_CALL RecordContextFastCall(CONTEXT* context, THREADID tid)
{
    if (tid == myThread)
    {
        PIN_SaveContext(context, &contextAtRecordFastCall);
    }
}

BOOL CompareIntContext(CONTEXT* context1, CONTEXT* context2)
{
    BOOL compareOk = TRUE;
    if (PIN_GetContextReg(context1, REG_INST_PTR) != PIN_GetContextReg(context2, REG_INST_PTR))
    {
        fprintf(log_inl, "REG_INST_PTR ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_GAX) != PIN_GetContextReg(context2, REG_GAX))
    {
        fprintf(log_inl, "REG_GAX ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_GBX) != PIN_GetContextReg(context2, REG_GBX))
    {
        fprintf(log_inl, "REG_GBX ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_GCX) != PIN_GetContextReg(context2, REG_GCX))
    {
        fprintf(log_inl, "REG_GCX ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_GDX) != PIN_GetContextReg(context2, REG_GDX))
    {
        fprintf(log_inl, "REG_GDX ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_GSI) != PIN_GetContextReg(context2, REG_GSI))
    {
        fprintf(log_inl, "REG_GSI ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_GDI) != PIN_GetContextReg(context2, REG_GDI))
    {
        fprintf(log_inl, "REG_GDI ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_GBP) != PIN_GetContextReg(context2, REG_GBP))
    {
        fprintf(log_inl, "REG_GBP ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_STACK_PTR) != PIN_GetContextReg(context2, REG_STACK_PTR))
    {
        fprintf(log_inl, "REG_STACK_PTR ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_SEG_SS) != PIN_GetContextReg(context2, REG_SEG_SS))
    {
        fprintf(log_inl, "REG_SEG_SS ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_SEG_CS) != PIN_GetContextReg(context2, REG_SEG_CS))
    {
        fprintf(log_inl, "REG_SEG_CS ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_SEG_DS) != PIN_GetContextReg(context2, REG_SEG_DS))
    {
        fprintf(log_inl, "REG_SEG_DS ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_SEG_ES) != PIN_GetContextReg(context2, REG_SEG_ES))
    {
        fprintf(log_inl, "REG_SEG_ES ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_SEG_FS) != PIN_GetContextReg(context2, REG_SEG_FS))
    {
        fprintf(log_inl, "REG_SEG_FS ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_SEG_GS) != PIN_GetContextReg(context2, REG_SEG_GS))
    {
        fprintf(log_inl, "REG_SEG_GS ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_GFLAGS) != PIN_GetContextReg(context2, REG_GFLAGS))
    {
        fprintf(log_inl, "REG_GFLAGS ERROR\n");
        compareOk = FALSE;
    }
#ifdef TARGET_IA32E
    if (PIN_GetContextReg(context1, REG_R8) != PIN_GetContextReg(context2, REG_R8))
    {
        fprintf(log_inl, "REG_R8 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_R9) != PIN_GetContextReg(context2, REG_R9))
    {
        fprintf(log_inl, "REG_R9 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_R10) != PIN_GetContextReg(context2, REG_R10))
    {
        fprintf(log_inl, "REG_R10 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_R11) != PIN_GetContextReg(context2, REG_R11))
    {
        fprintf(log_inl, "REG_R11 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_R12) != PIN_GetContextReg(context2, REG_R12))
    {
        fprintf(log_inl, "REG_R12 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_R13) != PIN_GetContextReg(context2, REG_R13))
    {
        fprintf(log_inl, "REG_R13 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_R14) != PIN_GetContextReg(context2, REG_R14))
    {
        fprintf(log_inl, "REG_R14 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_R15) != PIN_GetContextReg(context2, REG_R15))
    {
        fprintf(log_inl, "REG_R15 ERROR\n");
        compareOk = FALSE;
    }
#endif
    return (compareOk);
}

BOOL CompareFpContext(CONTEXT* context1, CONTEXT* context2)
{
    BOOL compareOk = TRUE;
    FPSTATE fpContext1, fpContext2;
    FPSTATE* fpContextPtr1 = &fpContext1;
    FPSTATE* fpContextPtr2 = &fpContext2;
    PIN_GetContextFPState(context1, fpContextPtr1);
    PIN_GetContextFPState(context2, fpContextPtr2);

    if (fpContextPtr1->fxsave_legacy._fcw != fpContextPtr2->fxsave_legacy._fcw)
    {
        fprintf(log_inl, "fcw ERROR\n");
        compareOk = FALSE;
    }
    if (fpContextPtr1->fxsave_legacy._fsw != fpContextPtr2->fxsave_legacy._fsw)
    {
        fprintf(log_inl, "_fsw ERROR\n");
        compareOk = FALSE;
    }
    if (fpContextPtr1->fxsave_legacy._ftw != fpContextPtr2->fxsave_legacy._ftw)
    {
        fprintf(log_inl, "_ftw ERROR\n");
        compareOk = FALSE;
    }
    if (fpContextPtr1->fxsave_legacy._fop != fpContextPtr2->fxsave_legacy._fop)
    {
        fprintf(log_inl, "_fop ERROR\n");
        compareOk = FALSE;
    }
    if (fpContextPtr1->fxsave_legacy._fpuip != fpContextPtr2->fxsave_legacy._fpuip)
    {
        fprintf(log_inl, "_fpuip ERROR\n");
        compareOk = FALSE;
    }
    /* the _cs field seems to be changing randomly when running 32on64 linux
       needs further investigation to prove it is not a Pin bug */
    if (fpContextPtr1->fxsave_legacy._cs != fpContextPtr2->fxsave_legacy._cs)
    {
        fprintf(log_inl, "_cs ERROR\n");
        compareOk = FALSE;
    }
    if (fpContextPtr1->fxsave_legacy._fpudp != fpContextPtr2->fxsave_legacy._fpudp)
    {
        fprintf(log_inl, "_fpudp ERROR\n");
        compareOk = FALSE;
    }
    /* the _ds field seems to be changing randomly when running 32on64 linux
       needs further investigation to prove it is not a Pin bug */
    if (fpContextPtr1->fxsave_legacy._ds != fpContextPtr2->fxsave_legacy._ds)
    {
        fprintf(log_inl, "_ds ERROR\n");
        compareOk = FALSE;
    }
    if (fpContextPtr1->fxsave_legacy._mxcsr != fpContextPtr2->fxsave_legacy._mxcsr)
    {
        fprintf(log_inl, "_mxcsr ERROR\n");
        compareOk = FALSE;
    }
    if (fpContextPtr1->fxsave_legacy._mxcsrmask != fpContextPtr2->fxsave_legacy._mxcsrmask)
    {
        fprintf(log_inl, "_mxcsrmask ERROR\n");
        compareOk = FALSE;
    }
    int i;
    for (i = 0; i < 8; i++)
    {
        if ((fpContextPtr1->fxsave_legacy._sts[i]._raw._lo != fpContextPtr2->fxsave_legacy._sts[i]._raw._lo) ||
            (fpContextPtr1->fxsave_legacy._sts[i]._raw._hi != fpContextPtr2->fxsave_legacy._sts[i]._raw._hi))
        {
            fprintf(log_inl, "_st[%d] ERROR\n", i);
            compareOk = FALSE;
        }
    }
    for (i = 0;
#ifdef TARGET_IA32E
         i < 16;
#else
         i < 8;
#endif
         i++)
    {
        if ((fpContextPtr1->fxsave_legacy._xmms[i]._vec64[0] != fpContextPtr2->fxsave_legacy._xmms[i]._vec64[0]) ||
            (fpContextPtr1->fxsave_legacy._xmms[i]._vec64[1] != fpContextPtr2->fxsave_legacy._xmms[i]._vec64[1]))
        {
            fprintf(log_inl, "_xmm[%d] ERROR\n", i);
            compareOk = FALSE;
        }
    }

    return (compareOk);
}

BOOL CompareContext(CONTEXT* context1, CONTEXT* context2)
{
    BOOL compareIntOk = CompareIntContext(context1, context2);
    BOOL compareFpOk  = CompareFpContext(context1, context2);
    return (compareIntOk && compareFpOk);
}

VOID VerifyContext(ADDRINT pcval, CONTEXT* context, THREADID tid)
{
    if (tid == myThread)
    {
        PIN_SaveContext(context, &contextAtVerify);

        BOOL hadError = FALSE;
        if (!CompareContext(&contextAtVerify, &contextAtRecordFastCall))
        {
            fprintf(log_inl, "contextAtRecordFastCall ERROR\n");
            hadError = TRUE;
        }

        if (!CompareContext(&contextAtVerify, &contextAtRecordNonFastCall))
        {
            fprintf(log_inl, "contextAtRecordNonFastCall ERROR\n");
            hadError = TRUE;
        }
        if (hadError)
        {
            string s = disassemble((pcval), (pcval) + 15);
            fprintf(log_inl, "    %s\n", s.c_str());
            exit(-1);
        }
    }
}

INT32 Usage()
{
    cerr << "This tests if the stack arguments are passed correctly by an inlined analysis function"
            "\n";

    cerr << endl;

    return -1;
}

int numContextsInstrumented = 0;
int numRegularInstrumented  = 0;
ADDRINT readValue1;

VOID Instruction(INS ins, VOID* v)
{
    ADDRINT nextAddr = INS_NextAddress(ins);
    UINT32 maxRRegs  = INS_MaxNumRRegs(ins);
    UINT32 maxWRegs  = INS_MaxNumWRegs(ins);
    ADDRINT sz       = INS_Size(ins);

    if (numContextsInstrumented < 100)
    {
        numContextsInstrumented++;
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordContext,
                       // 4 dummy params to get the real params to be pushed on the stack in Intel64
                       IARG_UINT32, 1, IARG_UINT32, 2, IARG_UINT32, 3, IARG_UINT32, 4, IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordContextFastCall, IARG_FAST_ANALYSIS_CALL, IARG_CONTEXT, IARG_THREAD_ID,
                       IARG_END);
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)VerifyContext, IARG_INST_PTR, IARG_CONTEXT, IARG_THREAD_ID, IARG_END);
    }
    else if (numRegularInstrumented < 100)
    {
        numRegularInstrumented++;
        INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordInstructionInfoFastCall, IARG_FAST_ANALYSIS_CALL, IARG_THREAD_ID,
                           IARG_INST_PTR, IARG_REG_VALUE, REG_GFLAGS, IARG_ADDRINT, nextAddr, IARG_UINT32, maxRRegs, IARG_UINT32,
                           maxWRegs, IARG_ADDRINT, sz,
#ifdef TARGET_IA32E
                           IARG_ADDRINT, 0xdeadbeefdeadbeefLL,
#else
                           IARG_ADDRINT, 0xdeadbeef,
#endif
                           IARG_REG_VALUE, REG_GDX, IARG_REG_VALUE, REG_GDX, IARG_REG_VALUE, REG_GDX, IARG_REG_VALUE, REG_GDX,
                           IARG_REG_VALUE, REG_GDX, IARG_REG_VALUE, REG_GDX, IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordInstructionInfo,
                           // 4 dummy params to get the real params to be pushed on the stack in Intel64
                           IARG_UINT32, 1, IARG_UINT32, 2, IARG_UINT32, 3, IARG_UINT32, 4, IARG_THREAD_ID, IARG_INST_PTR,
                           IARG_REG_VALUE, REG_GFLAGS, IARG_ADDRINT, nextAddr, IARG_UINT32, maxRRegs, IARG_UINT32, maxWRegs,
                           IARG_ADDRINT, sz,
#ifdef TARGET_IA32E
                           IARG_ADDRINT, 0xdeadbeefdeadbeefLL,
#else
                           IARG_ADDRINT, 0xdeadbeef,
#endif
                           IARG_REG_VALUE, REG_GDX, IARG_REG_VALUE, REG_GDX, IARG_REG_VALUE, REG_GDX, IARG_REG_VALUE, REG_GDX,
                           IARG_REG_VALUE, REG_GDX, IARG_REG_VALUE, REG_GDX, IARG_END);

        INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)VerifyInstructionInfo, IARG_THREAD_ID, IARG_INST_PTR, IARG_REG_VALUE,
                           REG_GFLAGS, IARG_ADDRINT, nextAddr, IARG_UINT32, maxRRegs, IARG_UINT32, maxWRegs, IARG_ADDRINT, sz,
#ifdef TARGET_IA32E
                           IARG_ADDRINT, 0xdeadbeefdeadbeefLL,
#else
                           IARG_ADDRINT, 0xdeadbeef,
#endif
                           IARG_REG_VALUE, REG_GDX, IARG_REG_VALUE, REG_GDX, IARG_REG_VALUE, REG_GDX, IARG_REG_VALUE, REG_GDX,
                           IARG_REG_VALUE, REG_GDX, IARG_REG_VALUE, REG_GDX, IARG_END);
    }
}

VOID Fini(INT32 code, VOID* v)
{
    fprintf(log_inl, "SUCCESS\n");
    fclose(log_inl);
}

int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    string logfile = KnobOutputFile.Value();

    log_inl = fopen(logfile.c_str(), "w");

    INS_AddInstrumentFunction(Instruction, NULL);
    PIN_AddFiniFunction(Fini, NULL);

    PIN_AddThreadStartFunction(ThreadStart, NULL);

    // Never returns
    PIN_StartProgram();

    return 1;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
