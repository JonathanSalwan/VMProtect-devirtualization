/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <cstdio>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include "pin.H"
#include "instlib.H"
using std::iostream;
using std::ostringstream;

KNOB< BOOL > KnobGetIntContext(KNOB_MODE_WRITEONCE, "pintool", "get_int_context", "0", "get int context");
KNOB< BOOL > KnobGetPartOfIntContext(KNOB_MODE_WRITEONCE, "pintool", "get_part_of_int_context", "0", "get part of int context");
KNOB< BOOL > KnobGetFpContext(KNOB_MODE_WRITEONCE, "pintool", "get_fp_context", "0", "get fp context");
KNOB< BOOL > KnobCompareContexts(KNOB_MODE_WRITEONCE, "pintool", "compare_contexts", "1", "compare contexts");
KNOB< BOOL > KnobCompareReverseContexts(KNOB_MODE_WRITEONCE, "pintool", "reverse_compare_contexts", "0",
                                        "reverse compare contexts");
KNOB< BOOL > KnobOnStackContextOnly(KNOB_MODE_WRITEONCE, "pintool", "on_stack_context_only", "0", "on stack context only");
KNOB< BOOL > KnobGetSpillAreaContextOnly(KNOB_MODE_WRITEONCE, "pintool", "spill_area_context_only", "0",
                                         "spillArea context only");
KNOB< BOOL > KnobContextAtTrace(KNOB_MODE_WRITEONCE, "pintool", "context_at_trace", "1", "context at trace");
KNOB< BOOL > KnobContextAtIns(KNOB_MODE_WRITEONCE, "pintool", "context_at_ins", "0", "context at ins");

BOOL supportsAvx;

extern "C" BOOL ProcessorSupportsAvx();

// The tool assumes single-threaded application.
// This may not be the case on Windows 10.
// We arbitrary choose single thread to profile.
THREADID myThread = INVALID_THREADID;

ADDRINT IfMyThread(THREADID threadId)
{
    // Profile only single thread at any time
    return threadId == myThread;
}

VOID ThreadStart(THREADID tid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
    // Determine single thread to profile.
    if (myThread == INVALID_THREADID) myThread = tid;
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
ADDRINT eaxReg;

VOID GetIntRegsFromContext(CONTEXT* ctxt)
{
    PIN_GetContextReg(ctxt, REG_INST_PTR);

    PIN_GetContextReg(ctxt, REG_GAX);

    PIN_GetContextReg(ctxt, REG_GBX);

    if (KnobGetPartOfIntContext)
    {
        return;
    }

    PIN_GetContextReg(ctxt, REG_GCX);

    PIN_GetContextReg(ctxt, REG_GDX);

    PIN_GetContextReg(ctxt, REG_GSI);

    PIN_GetContextReg(ctxt, REG_GDI);

    PIN_GetContextReg(ctxt, REG_GBP);

    PIN_GetContextReg(ctxt, REG_STACK_PTR);

    PIN_GetContextReg(ctxt, REG_SEG_SS);

    PIN_GetContextReg(ctxt, REG_SEG_CS);

    PIN_GetContextReg(ctxt, REG_SEG_DS);

    PIN_GetContextReg(ctxt, REG_SEG_ES);

    PIN_GetContextReg(ctxt, REG_SEG_FS);

    PIN_GetContextReg(ctxt, REG_SEG_GS);

    PIN_GetContextReg(ctxt, REG_GFLAGS);

#ifdef TARGET_IA32E
    PIN_GetContextReg(ctxt, REG_R8);

    PIN_GetContextReg(ctxt, REG_R9);

    PIN_GetContextReg(ctxt, REG_R10);

    PIN_GetContextReg(ctxt, REG_R11);

    PIN_GetContextReg(ctxt, REG_R12);

    PIN_GetContextReg(ctxt, REG_R13);

    PIN_GetContextReg(ctxt, REG_R14);

    PIN_GetContextReg(ctxt, REG_R15);

#endif
}

CHAR fpContextSpaceForFpConextFromPin[FPSTATE_SIZE + FPSTATE_ALIGNMENT];

VOID GetFpContextFromContext(CONTEXT* ctxt)
{
    FPSTATE* fpContextFromPin = reinterpret_cast< FPSTATE* >(
        (reinterpret_cast< ADDRINT >(fpContextSpaceForFpConextFromPin) + (FPSTATE_ALIGNMENT - 1)) & (-FPSTATE_ALIGNMENT));
    PIN_GetContextFPState(ctxt, fpContextFromPin);
}

BOOL CompareIntContext(CONTEXT* context1, CONTEXT* context2)
{
    BOOL compareOk      = TRUE;
    ADDRINT regInstPtr1 = PIN_GetContextReg(context1, REG_INST_PTR);
    ADDRINT regInstPtr2 = PIN_GetContextReg(context2, REG_INST_PTR);
    if (regInstPtr1 != regInstPtr2)
    {
        printf("REG_INST_PTR %p   REG_INST_PTR %p\n", reinterpret_cast< VOID* >(regInstPtr1),
               reinterpret_cast< VOID* >(regInstPtr2));
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_GAX) != PIN_GetContextReg(context2, REG_GAX))
    {
        printf("REG_GAX ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_GBX) != PIN_GetContextReg(context2, REG_GBX))
    {
        printf("REG_GBX ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_GCX) != PIN_GetContextReg(context2, REG_GCX))
    {
        printf("REG_GCX ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_GDX) != PIN_GetContextReg(context2, REG_GDX))
    {
        printf("REG_GDX ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_GSI) != PIN_GetContextReg(context2, REG_GSI))
    {
        printf("REG_GSI ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_GDI) != PIN_GetContextReg(context2, REG_GDI))
    {
        printf("REG_GDI ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_GBP) != PIN_GetContextReg(context2, REG_GBP))
    {
        printf("REG_GBP ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_STACK_PTR) != PIN_GetContextReg(context2, REG_STACK_PTR))
    {
        printf("REG_STACK_PTR ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_SEG_SS) != PIN_GetContextReg(context2, REG_SEG_SS))
    {
        printf("REG_SEG_SS ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_SEG_CS) != PIN_GetContextReg(context2, REG_SEG_CS))
    {
        printf("REG_SEG_CS ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_SEG_DS) != PIN_GetContextReg(context2, REG_SEG_DS))
    {
        printf("REG_SEG_DS ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_SEG_ES) != PIN_GetContextReg(context2, REG_SEG_ES))
    {
        printf("REG_SEG_ES ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_SEG_FS) != PIN_GetContextReg(context2, REG_SEG_FS))
    {
        ADDRINT fs1 = PIN_GetContextReg(context1, REG_SEG_FS);
        ADDRINT fs2 = PIN_GetContextReg(context2, REG_SEG_FS);
        if ((fs1 & (~0x3)) != (fs2 & (~0x3)))
        { // rpl of fs may be changed without Pin noticing - e.g. printf changes rpl
            printf("REG_SEG_FS ERROR\n");
            compareOk = FALSE;
        }
    }
    if (PIN_GetContextReg(context1, REG_SEG_GS) != PIN_GetContextReg(context2, REG_SEG_GS))
    {
        ADDRINT gs1 = PIN_GetContextReg(context1, REG_SEG_GS);
        ADDRINT gs2 = PIN_GetContextReg(context2, REG_SEG_GS);
        if ((gs1 & (~0x3)) != (gs2 & (~0x3)))
        { // rpl of gs may be changed without Pin noticing - e.g. printf changes rpl
            printf("REG_SEG_GS ERROR\n");
            compareOk = FALSE;
        }
    }
    if (PIN_GetContextReg(context1, REG_SEG_GS_BASE) != PIN_GetContextReg(context2, REG_SEG_GS_BASE))
    {
        printf("REG_SEG_GS_BASE ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_SEG_FS_BASE) != PIN_GetContextReg(context2, REG_SEG_FS_BASE))
    {
        printf("REG_SEG_FS_BASE ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_GFLAGS) != PIN_GetContextReg(context2, REG_GFLAGS))
    {
        printf("REG_GFLAGS ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_INST_G0) != PIN_GetContextReg(context2, REG_INST_G0))
    {
        printf("REG_INST_G0 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_INST_G1) != PIN_GetContextReg(context2, REG_INST_G1))
    {
        printf("REG_INST_G1 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_INST_G2) != PIN_GetContextReg(context2, REG_INST_G2))
    {
        printf("REG_INST_G2 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_INST_G3) != PIN_GetContextReg(context2, REG_INST_G3))
    {
        printf("REG_INST_G3 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_INST_G4) != PIN_GetContextReg(context2, REG_INST_G4))
    {
        printf("REG_INST_G4 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_INST_G5) != PIN_GetContextReg(context2, REG_INST_G5))
    {
        printf("REG_INST_G5 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_INST_G6) != PIN_GetContextReg(context2, REG_INST_G6))
    {
        printf("REG_INST_G6 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_INST_G7) != PIN_GetContextReg(context2, REG_INST_G7))
    {
        printf("REG_INST_G7 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_INST_G8) != PIN_GetContextReg(context2, REG_INST_G8))
    {
        printf("REG_INST_G8 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_INST_G9) != PIN_GetContextReg(context2, REG_INST_G9))
    {
        printf("REG_INST_G9 ERROR\n");
        compareOk = FALSE;
    }

    if (PIN_GetContextReg(context1, REG_BUF_BASE0) != PIN_GetContextReg(context2, REG_BUF_BASE0))
    {
        printf("REG_BUF_BASE0 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_BUF_BASE1) != PIN_GetContextReg(context2, REG_BUF_BASE1))
    {
        printf("REG_BUF_BASE1 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_BUF_BASE2) != PIN_GetContextReg(context2, REG_BUF_BASE2))
    {
        printf("REG_BUF_BASE2 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_BUF_BASE3) != PIN_GetContextReg(context2, REG_BUF_BASE3))
    {
        printf("REG_BUF_BASE3 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_BUF_BASE4) != PIN_GetContextReg(context2, REG_BUF_BASE4))
    {
        printf("REG_BUF_BASE4 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_BUF_BASE5) != PIN_GetContextReg(context2, REG_BUF_BASE5))
    {
        printf("REG_BUF_BASE5 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_BUF_BASE6) != PIN_GetContextReg(context2, REG_BUF_BASE6))
    {
        printf("REG_BUF_BASE6 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_BUF_BASE7) != PIN_GetContextReg(context2, REG_BUF_BASE7))
    {
        printf("REG_BUF_BASE7 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_BUF_BASE8) != PIN_GetContextReg(context2, REG_BUF_BASE8))
    {
        printf("REG_BUF_BASE8 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_BUF_BASE9) != PIN_GetContextReg(context2, REG_BUF_BASE9))
    {
        printf("REG_BUF_BASE9 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_BUF_END0) != PIN_GetContextReg(context2, REG_BUF_END0))
    {
        printf("REG_BUF_END0 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_BUF_END1) != PIN_GetContextReg(context2, REG_BUF_END1))
    {
        printf("REG_BUF_END1 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_BUF_END2) != PIN_GetContextReg(context2, REG_BUF_END2))
    {
        printf("REG_BUF_END2 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_BUF_END3) != PIN_GetContextReg(context2, REG_BUF_END3))
    {
        printf("REG_BUF_END3 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_BUF_END4) != PIN_GetContextReg(context2, REG_BUF_END4))
    {
        printf("REG_BUF_END4 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_BUF_END5) != PIN_GetContextReg(context2, REG_BUF_END5))
    {
        printf("REG_BUF_END5 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_BUF_END6) != PIN_GetContextReg(context2, REG_BUF_END6))
    {
        printf("REG_BUF_END6 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_BUF_END7) != PIN_GetContextReg(context2, REG_BUF_END7))
    {
        printf("REG_BUF_END7 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_BUF_END8) != PIN_GetContextReg(context2, REG_BUF_END8))
    {
        printf("REG_BUF_END8 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_BUF_END9) != PIN_GetContextReg(context2, REG_BUF_END9))
    {
        printf("REG_BUF_END9 ERROR\n");
        compareOk = FALSE;
    }

#ifdef TARGET_IA32E
    if (PIN_GetContextReg(context1, REG_R8) != PIN_GetContextReg(context2, REG_R8))
    {
        printf("REG_R8 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_R9) != PIN_GetContextReg(context2, REG_R9))
    {
        printf("REG_R9 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_R10) != PIN_GetContextReg(context2, REG_R10))
    {
        printf("REG_R10 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_R11) != PIN_GetContextReg(context2, REG_R11))
    {
        printf("REG_R11 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_R12) != PIN_GetContextReg(context2, REG_R12))
    {
        printf("REG_R12 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_R13) != PIN_GetContextReg(context2, REG_R13))
    {
        printf("REG_R13 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_R14) != PIN_GetContextReg(context2, REG_R14))
    {
        printf("REG_R14 ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_R15) != PIN_GetContextReg(context2, REG_R15))
    {
        printf("REG_R15 ERROR\n");
        compareOk = FALSE;
    }
#endif
    // short fp regs
    if (PIN_GetContextReg(context1, REG_FPCW) != PIN_GetContextReg(context2, REG_FPCW))
    {
        printf("REG_FPCW ERRO\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_FPSW) != PIN_GetContextReg(context2, REG_FPSW))
    {
        printf("REG_FPSW ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_FPTAG) != PIN_GetContextReg(context2, REG_FPTAG))
    {
        printf("REG_FPTAG ERROR\n");
        compareOk = FALSE;
    }
#if 0
    // these appear to change unexpectedly
    if (PIN_GetContextReg( context1, REG_FPIP_SEL ) != PIN_GetContextReg( context2, REG_FPIP_SEL ))
    {
        printf ("REG_FPIP_SEL ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg( context1, REG_FPDP_SEL ) != PIN_GetContextReg( context2, REG_FPDP_SEL ))
    {
        printf ("REG_FPDP_SEL ERROR\n");
        compareOk = FALSE;
    }
#endif
    if (PIN_GetContextReg(context1, REG_FPIP_OFF) != PIN_GetContextReg(context2, REG_FPIP_OFF))
    {
        printf("REG_FPIP_OFF ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_FPDP_OFF) != PIN_GetContextReg(context2, REG_FPDP_OFF))
    {
        printf("REG_FPDP_OFF ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_FPOPCODE) != PIN_GetContextReg(context2, REG_FPOPCODE))
    {
        printf("REG_FPOPCODE ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_MXCSR) != PIN_GetContextReg(context2, REG_MXCSR))
    {
        printf("REG_MXCSR ERROR\n");
        compareOk = FALSE;
    }
    if (PIN_GetContextReg(context1, REG_MXCSRMASK) != PIN_GetContextReg(context2, REG_MXCSRMASK))
    {
        printf("REG_MXCSRMASK ERROR\n");
        compareOk = FALSE;
    }

    if (!compareOk)
    {
        string s = disassemble((regInstPtr1), (regInstPtr1) + 15);
        printf("Failure at %p: %s\n", reinterpret_cast< VOID* >(regInstPtr1), s.c_str());
    }

    return (compareOk);
}

BOOL CompareFpContext(CONTEXT* context1, CONTEXT* context2)
{
    ADDRINT regInstPtr1 = PIN_GetContextReg(context1, REG_INST_PTR);

    BOOL compareOk = TRUE;
    CHAR fpContext1[FPSTATE_SIZE];
    CHAR fpContext2[FPSTATE_SIZE];
    FPSTATE *fpVerboseContext1, *fpVerboseContext2;
    fpVerboseContext1 = reinterpret_cast< FPSTATE* >(fpContext1);
    fpVerboseContext2 = reinterpret_cast< FPSTATE* >(fpContext2);
    PIN_GetContextFPState(context1, fpVerboseContext1);
    PIN_GetContextFPState(context2, fpVerboseContext2);

    if (fpVerboseContext1->fxsave_legacy._fcw != fpVerboseContext2->fxsave_legacy._fcw)
    {
        printf("fcw ERROR\n");
        compareOk = FALSE;
    }
    if (fpVerboseContext1->fxsave_legacy._fsw != fpVerboseContext2->fxsave_legacy._fsw)
    {
        printf("_fsw ERROR\n");
        compareOk = FALSE;
    }
    if (fpVerboseContext1->fxsave_legacy._ftw != fpVerboseContext2->fxsave_legacy._ftw)
    {
        printf("_ftw ERROR\n");
        compareOk = FALSE;
    }
    if (fpVerboseContext1->fxsave_legacy._fop != fpVerboseContext2->fxsave_legacy._fop)
    {
        printf("_fop ERROR\n");
        compareOk = FALSE;
    }
    if (fpVerboseContext1->fxsave_legacy._fpuip != fpVerboseContext2->fxsave_legacy._fpuip)
    {
        printf("_fpuip ERROR\n");
        compareOk = FALSE;
    }
#if 0
    /* the _cs field seems to be changing randomly when running 32on64 linux
       needs further investigation to prove it is not a Pin bug */
    if ( fpVerboseContext1->fxsave_legacy._cs != fpVerboseContext2->fxsave_legacy._cs)
    {
        printf ("_cs ERROR\n");
        compareOk = FALSE;
    }
    /* the _ds field seems to be changing randomly when running 32on64 linux
       needs further investigation to prove it is not a Pin bug */
    if ( fpVerboseContext1->fxsave_legacy._ds != fpVerboseContext2->fxsave_legacy._ds)
    {
        printf ("_ds ERROR\n");
        compareOk = FALSE;
    }
#endif
    if (fpVerboseContext1->fxsave_legacy._fpudp != fpVerboseContext2->fxsave_legacy._fpudp)
    {
        printf("_fpudp ERROR\n");
        compareOk = FALSE;
    }
    if (fpVerboseContext1->fxsave_legacy._mxcsr != fpVerboseContext2->fxsave_legacy._mxcsr)
    {
        printf("_mxcsr ERROR\n");
        compareOk = FALSE;
    }
    if (fpVerboseContext1->fxsave_legacy._mxcsrmask != fpVerboseContext2->fxsave_legacy._mxcsrmask)
    {
        printf("_mxcsrmask ERROR\n");
        compareOk = FALSE;
    }
    int i;
    for (i = 0; i < 8; i++)
    {
        if ((fpVerboseContext1->fxsave_legacy._sts[i]._raw._lo != fpVerboseContext2->fxsave_legacy._sts[i]._raw._lo) ||
            (fpVerboseContext1->fxsave_legacy._sts[i]._raw._hi != fpVerboseContext2->fxsave_legacy._sts[i]._raw._hi))
        {
            printf("_st[%d] ERROR\n", i);
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
        if ((fpVerboseContext1->fxsave_legacy._xmms[i]._vec64[0] != fpVerboseContext2->fxsave_legacy._xmms[i]._vec64[0]) ||
            (fpVerboseContext1->fxsave_legacy._xmms[i]._vec64[1] != fpVerboseContext2->fxsave_legacy._xmms[i]._vec64[1]))
        {
            printf("_xmm[%d] ERROR\n", i);
            fflush(stdout);
            compareOk = FALSE;
        }
    }

    if (supportsAvx)
    {
        int k = 0;
        for (int i = 0;
#ifdef TARGET_IA32E
             i < 16;
#else
             i < 8;
#endif
             ++i)
        {
            for (int j = 0; j < 16; j++)
            {
                if (fpVerboseContext1->_xstate._ymmUpper[k] != fpVerboseContext2->_xstate._ymmUpper[k])
                {
                    printf("ymm[%d] ERROR\n", i);
                    fflush(stdout);
                    compareOk = FALSE;
                }
                k++;
            }
        }
    }

    if (!compareOk)
    {
        string s = disassemble((regInstPtr1), (regInstPtr1) + 15);
        printf("Failure at %p: %s\n", reinterpret_cast< VOID* >(regInstPtr1), s.c_str());
    }
    return (compareOk);
}

CONTEXT contextAtReceive;
ADDRINT mxcsr;
VOID ReceiveContext(CONTEXT* ctxt)
{
    if (KnobGetIntContext || KnobGetPartOfIntContext)
    {
        GetIntRegsFromContext(ctxt);
    }
    if (KnobGetFpContext)
    {
        GetFpContextFromContext(ctxt);
    }
    if (!(KnobOnStackContextOnly || KnobGetSpillAreaContextOnly || !KnobCompareContexts))
    {
        PIN_SaveContext(ctxt, &contextAtReceive);
    }
}

VOID VerifyContext(CONTEXT* contextAtVerify)
{
    BOOL successInt = CompareIntContext(contextAtVerify, &contextAtReceive);
    BOOL successFp  = CompareFpContext(contextAtVerify, &contextAtReceive);
    if (!successInt || !successFp)
    {
        exit(-1);
    }
}

VOID Trace(TRACE trace, VOID* v)
{
    if (KnobCompareContexts)
    {
        TRACE_InsertIfCall(trace, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
        TRACE_InsertThenCall(trace, IPOINT_BEFORE, (AFUNPTR)ReceiveContext, IARG_CONST_CONTEXT, IARG_END);
        TRACE_InsertIfCall(trace, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
        TRACE_InsertThenCall(trace, IPOINT_BEFORE, (AFUNPTR)VerifyContext, IARG_CONTEXT, IARG_END);
        if (KnobCompareReverseContexts)
        {
            TRACE_InsertIfCall(trace, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
            TRACE_InsertThenCall(trace, IPOINT_BEFORE, (AFUNPTR)ReceiveContext, IARG_CONTEXT, IARG_END);
            TRACE_InsertIfCall(trace, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
            TRACE_InsertThenCall(trace, IPOINT_BEFORE, (AFUNPTR)VerifyContext, IARG_CONST_CONTEXT, IARG_END);
        }
    }
    else if (KnobOnStackContextOnly)
    {
        TRACE_InsertIfCall(trace, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
        TRACE_InsertThenCall(trace, IPOINT_BEFORE, (AFUNPTR)ReceiveContext, IARG_CONTEXT, IARG_END);
    }
    else if (KnobGetSpillAreaContextOnly)
    {
        TRACE_InsertIfCall(trace, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
        TRACE_InsertThenCall(trace, IPOINT_BEFORE, (AFUNPTR)ReceiveContext, IARG_CONST_CONTEXT, IARG_END);
    }
}

VOID Instruction(INS ins, VOID* v)
{
    if (KnobCompareContexts)
    {
        INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)ReceiveContext, IARG_CONTEXT, IARG_END);
        INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)VerifyContext, IARG_CONST_CONTEXT, IARG_END);
        if (KnobCompareReverseContexts)
        {
            INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
            INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)ReceiveContext, IARG_CONST_CONTEXT, IARG_END);
            INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
            INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)VerifyContext, IARG_CONTEXT, IARG_END);
        }
    }
    else if (KnobOnStackContextOnly)
    {
        INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)ReceiveContext, IARG_CONTEXT, IARG_END);
    }
    else if (KnobGetSpillAreaContextOnly)
    {
        INS_InsertIfCall(ins, IPOINT_BEFORE, AFUNPTR(IfMyThread), IARG_THREAD_ID, IARG_END);
        INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)ReceiveContext, IARG_CONST_CONTEXT, IARG_END);
    }
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    if (KnobContextAtTrace)
    {
        TRACE_AddInstrumentFunction(Trace, 0);
    }
    if (KnobContextAtIns)
    {
        INS_AddInstrumentFunction(Instruction, 0);
    }

    // Add callbacks
    PIN_AddThreadStartFunction(ThreadStart, 0);

    supportsAvx = ProcessorSupportsAvx();

    // Never returns
    PIN_StartProgram();

    return 0;
}
