/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include "pin.H"
#ifdef TARGET_WINDOWS
namespace WIND
{
#include <windows.h>
}
#define EXPORT_CSYM extern "C" __declspec(dllexport)
#else
#define EXPORT_CSYM extern "C"
#endif

EXPORT_CSYM void TestIargPreserveInReplacement() {}

EXPORT_CSYM void TestIargPreserveInReplacement1() {}

EXPORT_CSYM void TestIargPreserveInReplacement2() {}

EXPORT_CSYM void TestIargPreserveInProbed() {}

EXPORT_CSYM void TestIargPreserveInProbed1() {}

EXPORT_CSYM void TestIargPreserveInProbed2() {}
/*
This tool is used in conjunction with  set_fp_context_xmm_regs_app.cpp applictaion
- the test verifies that the xmm registers can be set for the application by
  PIN_CallApplicationFunction,  PIN_ExecuteAt and the function registered by PIN_AddContextChangeFunction
  that is invoked when an exception or non-fatal (i.e. handled) signal oocurs.
  Note that the callback function registered by the PIN_AddThreadStartFunction, can also change the xmm
  registers, however since there is much code executed before the actual application Thread main function is
  invoked, they will likely change before reaching there
*/

extern "C" UINT64 vals[];
UINT64 vals[] = {1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 15, 0, 16, 0};

extern "C" UINT64* val1;
extern "C" UINT64* val2;
extern "C" UINT64* val3;
extern "C" UINT64* val4;
extern "C" UINT64* val5;
extern "C" UINT64* val6;
extern "C" UINT64* val7;
extern "C" UINT64* val8;
extern "C" UINT64* val9;
extern "C" UINT64* val10;
extern "C" UINT64* val11;
extern "C" UINT64* val12;
extern "C" UINT64* val13;
extern "C" UINT64* val14;
extern "C" UINT64* val15;
extern "C" UINT64* val16;
extern "C" ADDRINT setFlagsX;
UINT64* val1      = &vals[0];
UINT64* val2      = &vals[2];
UINT64* val3      = &vals[4];
UINT64* val4      = &vals[6];
UINT64* val5      = &vals[8];
UINT64* val6      = &vals[10];
UINT64* val7      = &vals[12];
UINT64* val8      = &vals[14];
UINT64* val9      = &vals[16];
UINT64* val10     = &vals[18];
UINT64* val11     = &vals[20];
UINT64* val12     = &vals[22];
UINT64* val13     = &vals[24];
UINT64* val14     = &vals[26];
UINT64* val15     = &vals[28];
UINT64* val16     = &vals[30];
ADDRINT setFlagsX = 0;

#ifdef TARGET_IA32E
#define NUM_XMM_REGS 16
#else
#define NUM_XMM_REGS 8
#endif

KNOB< BOOL > KnobUseIargConstContext(KNOB_MODE_WRITEONCE, "pintool", "const_context", "0", "use IARG_CONST_CONTEXT");

ADDRINT executeAtAddr              = 0;
ADDRINT dumpX87RegsAtExceptionAddr = 0;

extern "C" double var1;
extern "C" double var2;
extern "C" double var3;
double var1 = 2.0;
double var2 = 2.0;
double var3 = 2.0;

typedef struct
{
    UINT32 _lo1; ///< Least significant part of value.
    UINT32 _lo2; ///< Least significant part of value.
    UINT32 _hi1; ///< Most significant part of value.
    UINT32 _hi2; ///< Most significant part of value.
} RAW32;

VOID REPLACE_ReplacedX87Regs(CONTEXT* context, THREADID tid, AFUNPTR originalFunction)
{
    printf("TOOL in REPLACE_ReplacedX87Regs x87 regs are:\n");
    fflush(stdout);
    CHAR fpContextSpaceForFpConextFromPin[FPSTATE_SIZE + FPSTATE_ALIGNMENT];
    FPSTATE* fpContextFromPin = reinterpret_cast< FPSTATE* >(
        (reinterpret_cast< ADDRINT >(fpContextSpaceForFpConextFromPin) + FPSTATE_ALIGNMENT - 1) & (-1 * FPSTATE_ALIGNMENT));

    PIN_GetContextFPState(context, fpContextFromPin);

    // verfiy that x87 registers are as they were set by the app just before the call to
    // ReplacedX87Regs, which is replaced by this function
    /*
    app set the x87 fp regs just before the call to ReplacedX87Regs as follows
    _mxcsr 1f80
    _st[0] 0 3fff 80000000 0
    _st[1] 0 3fff 80000000 0
    _st[2] 0 3fff 80000000 0
    _st[3] 0 5a5a 5a5a5a5a 5a5a5a5a
    _st[4] 0 5a5a 5a5a5a5a 5a5a5a5a
    _st[5] 0 5a5a 5a5a5a5a 5a5a5a5a
    _st[6] 0 5a5a 5a5a5a5a 5a5a5a5a
    _st[7] 0 5a5a 5a5a5a5a 5a5a5a5a
    */
    printf("_mxcsr %x\n", fpContextFromPin->fxsave_legacy._mxcsr);
    if (fpContextFromPin->fxsave_legacy._mxcsr & 0x200)
    {
        printf("***Error divide by zero should be masked\n");
        exit(-1);
    }
    int i;

    for (i = 0; i < 3; i++)
    {
        RAW32* ptr = reinterpret_cast< RAW32* >(&fpContextFromPin->fxsave_legacy._sts[i]._raw);
        printf("_st[%d] %x %x %x %x\n", i, ptr->_hi2, ptr->_hi1, ptr->_lo2, ptr->_lo1);
        if (ptr->_hi2 != 0 && ptr->_hi1 != 0x3fff && ptr->_lo2 != 0x80000000 && ptr->_lo1 != 0)
        {
            printf("***Error in this _st\n");
            exit(-1);
        }
    }

    for (; i < 8; i++)
    {
        RAW32* ptr = reinterpret_cast< RAW32* >(&fpContextFromPin->fxsave_legacy._sts[i]._raw);
        printf("_st[%d] %x %x %x %x\n", i, ptr->_hi2, ptr->_hi1, ptr->_lo2, ptr->_lo1);
        if (ptr->_hi2 != 0 && ptr->_hi1 != 0x5a5a && ptr->_lo2 != 0x5a5a5a5a && ptr->_lo1 != 0x5a5a5a5a)
        {
            printf("***Error in this _st\n");
            exit(-1);
        }
    }

    CONTEXT writableContext, *ctxt;
    if (KnobUseIargConstContext)
    { // need to copy the ctxt into a writable context
        PIN_SaveContext(context, &writableContext);
        ctxt = &writableContext;
    }
    else
    {
        ctxt = context;
    }

    /* set the x87 regs in the ctxt which is used to execute the 
       originalFunction (via PIN_CallApplicationFunction) */

    PIN_GetContextFPState(ctxt, fpContextFromPin);
    for (i = 0; i < 8; i++)
    {
        RAW32* ptr = reinterpret_cast< RAW32* >(&fpContextFromPin->fxsave_legacy._sts[i]._raw);
        ptr->_hi2  = 0xacdcacdc;
        ptr->_hi1  = 0xacdcacdc;
        ptr->_lo2  = 0xacdcacdc;
        ptr->_lo1  = 0xacdcacdc;
    }
    fpContextFromPin->fxsave_legacy._mxcsr |= (0x200); // mask divide by zero
    PIN_SetContextFPState(ctxt, fpContextFromPin);
    // verify the setting worked
    for (i = 0; i < 8; i++)
    {
        RAW32* ptr = reinterpret_cast< RAW32* >(&fpContextFromPin->fxsave_legacy._sts[i]._raw);
        ptr->_hi2  = 0x0;
        ptr->_hi1  = 0x0;
        ptr->_lo2  = 0x0;
        ptr->_lo1  = 0x0;
    }
    PIN_GetContextFPState(ctxt, fpContextFromPin);
    for (i = 0; i < 8; i++)
    {
        RAW32* ptr = reinterpret_cast< RAW32* >(&fpContextFromPin->fxsave_legacy._sts[i]._raw);
        if (ptr->_hi2 != 0xacdcacdc || ptr->_hi2 != 0xacdcacdc || ptr->_lo2 != 0xacdcacdc || ptr->_lo1 != 0xacdcacdc)
        {
            printf("TOOL error1 in setting fp context in REPLACE_ReplacedX87Regs\n");
            exit(-1);
        }
    }

    // call the originalFunction function with the xmm regs set from above
    printf("TOOL Calling replaced ReplacedX87Regs()\n");
    fflush(stdout);
    PIN_CallApplicationFunction(ctxt, tid, CALLINGSTD_DEFAULT, originalFunction, NULL, PIN_PARG_END());
    printf("TOOL Returned from replaced ReplacedX87Regs()\n");
    fflush(stdout);

    if (executeAtAddr != 0)
    {
        for (i = 0; i < 8; i++)
        {
            RAW32* ptr = reinterpret_cast< RAW32* >(&fpContextFromPin->fxsave_legacy._sts[i]._raw);
            ptr->_hi2  = 0xcacdcacd;
            ptr->_hi1  = 0xcacdcacd;
            ptr->_lo2  = 0xcacdcacd;
            ptr->_lo1  = 0xcacdcacd;
        }

        PIN_SetContextFPState(ctxt, fpContextFromPin);
        // verify the setting worked
        for (i = 0; i < 8; i++)
        {
            RAW32* ptr = reinterpret_cast< RAW32* >(&fpContextFromPin->fxsave_legacy._sts[i]._raw);
            ptr->_hi2  = 0x0;
            ptr->_hi1  = 0x0;
            ptr->_lo2  = 0x0;
            ptr->_lo1  = 0x0;
        }
        PIN_GetContextFPState(ctxt, fpContextFromPin);
        for (i = 0; i < 8; i++)
        {
            RAW32* ptr = reinterpret_cast< RAW32* >(&fpContextFromPin->fxsave_legacy._sts[i]._raw);
            if (ptr->_hi2 != 0xcacdcacd || ptr->_hi2 != 0xcacdcacd || ptr->_lo2 != 0xcacdcacd || ptr->_lo1 != 0xcacdcacd)
            {
                printf("TOOL error2 in setting fp context in REPLACE_ReplacedX87Regs\n");
                exit(-1);
            }
        }
        // execute the application function ExecuteAtFunc with the xmm regs set
        PIN_SetContextReg(ctxt, REG_INST_PTR, executeAtAddr);
        printf("TOOL Calling ExecutedAtFunc\n");
        fflush(stdout);
        PIN_ExecuteAt(ctxt);
        printf("TOOL returned from ExecutedAtFunc\n");
        fflush(stdout);
    }
}

void CheckAndSetFpContextX87RegsAtException(const CONTEXT* ctxtFrom, CONTEXT* ctxtTo)
{
    fprintf(stdout, "TOOL CheckAndSetFpContextX87Regs\n");
    fflush(stdout);
    CHAR fpContextSpaceForFpConextFromPin[FPSTATE_SIZE + FPSTATE_ALIGNMENT];
    FPSTATE* fpContextFromPin = reinterpret_cast< FPSTATE* >(
        (reinterpret_cast< ADDRINT >(fpContextSpaceForFpConextFromPin) + FPSTATE_ALIGNMENT - 1) & (-1 * FPSTATE_ALIGNMENT));

    PIN_GetContextFPState(ctxtFrom, fpContextFromPin);

    // verfiy that x87 registers are as they were set by the app just before the exception
    /*
    app set the x87 fp regs just before the exepction as follows
    _mxcsr 1f80
    _st[0] 0 3fff 80000000 0
    _st[1] 0 3fff 80000000 0
    _st[2] 0 3fff 80000000 0
    _st[3] 0 5a5a 5a5a5a5a 5a5a5a5a
    _st[4] 0 5a5a 5a5a5a5a 5a5a5a5a
    _st[5] 0 5a5a 5a5a5a5a 5a5a5a5a
    _st[6] 0 5a5a 5a5a5a5a 5a5a5a5a
    _st[7] 0 5a5a 5a5a5a5a 5a5a5a5a
    */
    printf("_mxcsr %x\n", fpContextFromPin->fxsave_legacy._mxcsr);
    if (fpContextFromPin->fxsave_legacy._mxcsr & 0x200)
    {
        printf("***Error divide by zero should be masked\n");
        exit(-1);
    }

    int i;
    for (i = 0; i < 3; i++)
    {
        RAW32* ptr = reinterpret_cast< RAW32* >(&fpContextFromPin->fxsave_legacy._sts[i]._raw);
        printf("_st[%d] %x %x %x %x\n", i, ptr->_hi2, ptr->_hi1, ptr->_lo2, ptr->_lo1);
        if (ptr->_hi2 != 0 && ptr->_hi1 != 0x3fff && ptr->_lo2 != 0x80000000 && ptr->_lo1 != 0)
        {
            printf("***Error in this _st\n");
            exit(-1);
        }
    }

    for (; i < 8; i++)
    {
        RAW32* ptr = reinterpret_cast< RAW32* >(&fpContextFromPin->fxsave_legacy._sts[i]._raw);
        printf("_st[%d] %x %x %x %x\n", i, ptr->_hi2, ptr->_hi1, ptr->_lo2, ptr->_lo1);
        if (ptr->_hi2 != 0 && ptr->_hi1 != 0x5a5a && ptr->_lo2 != 0x5a5a5a5a && ptr->_lo1 != 0x5a5a5a5a)
        {
            printf("***Error in this _st\n");
            exit(-1);
        }
    }

    fprintf(stdout, "TOOL Checked ctxtFrom OK\n");
    fflush(stdout);

    for (i = 0; i < 8; i++)
    {
        RAW32* ptr = reinterpret_cast< RAW32* >(&fpContextFromPin->fxsave_legacy._sts[i]._raw);
        ptr->_hi2  = 0xbaadf00d;
        ptr->_hi1  = 0xbaadf00d;
        ptr->_lo2  = 0xbaadf00d;
        ptr->_lo1  = 0xbaadf00d;
    }
    fpContextFromPin->fxsave_legacy._mxcsr |= (0x200); // mask divide by zero
    PIN_SetContextFPState(ctxtTo, fpContextFromPin);

    // verify the setting worked
    for (i = 0; i < 8; i++)
    {
        RAW32* ptr = reinterpret_cast< RAW32* >(&fpContextFromPin->fxsave_legacy._sts[i]._raw);
        ptr->_hi2  = 0x0;
        ptr->_hi1  = 0x0;
        ptr->_lo2  = 0x0;
        ptr->_lo1  = 0x0;
    }
    PIN_GetContextFPState(ctxtTo, fpContextFromPin);
    for (i = 0; i < 8; i++)
    {
        RAW32* ptr = reinterpret_cast< RAW32* >(&fpContextFromPin->fxsave_legacy._sts[i]._raw);
        if (ptr->_hi2 != 0xbaadf00d || ptr->_hi2 != 0xbaadf00d || ptr->_lo2 != 0xbaadf00d || ptr->_lo1 != 0xbaadf00d)
        {
            printf("TOOL error in setting fp context in CheckAndSetFpContextX87Regs\n");
            exit(-1);
        }
    }
    printf("TOOL Checked ctxtTo OK\n");
    fflush(stdout);
}

// Special stack alignment ( n mod 16 ) at callee entry point, after return address has been pushed on the stack.
// n == 0 means no special alignment, e.g. regular void* alignment
// n == 16 means alignment on 16
// (reference document http://www.agner.org/optimize/calling_conventions.pdf)
#if defined(TARGET_IA32E)
static const INT32 StackEntryAlignment = 8;
#elif defined(TARGET_LINUX) || defined(TARGET_MAC) || defined(TARGET_BSD)
static const INT32 StackEntryAlignment = 12;
#else
static const INT32 StackEntryAlignment = 0;
#endif

INT32 GetStackAdjustmentForRedirectionToFunction(INT32 currentAlignment)
{
    INT32 adjustment = (currentAlignment - StackEntryAlignment) % 16;

    if (adjustment < 0)
    {
        // adjustment > -16
        adjustment = 16 + adjustment;
    }
    return adjustment;
}

// this function verifies that the x87 regs in the ctxtFrom are as they were set in the app just before the
// exception occurs. Then it sets the x87 regs in the ctxtTo to a different value, finally it causes the
// execution to continue in the application function DumpX87RegsAtException
static void OnException(THREADID threadIndex, CONTEXT_CHANGE_REASON reason, const CONTEXT* ctxtFrom, CONTEXT* ctxtTo, INT32 info,
                        VOID* v)
{
    if ((ctxtTo == NULL) || ((CONTEXT_CHANGE_REASON_SIGNAL != reason) && (CONTEXT_CHANGE_REASON_EXCEPTION != reason)))
    { // Only exceptions and signals are handled
        return;
    }
    ADDRINT curIp = PIN_GetContextReg(ctxtFrom, REG_INST_PTR);
    IMG img       = IMG_FindByAddress(curIp);
    if (!IMG_Valid(img) || !IMG_IsMainExecutable(img))
    { // Events of interest should occur in main executable
        return;
    }

    fprintf(stdout, "TOOL OnException callback\n");
    fflush(stdout);

    CheckAndSetFpContextX87RegsAtException(ctxtFrom, ctxtTo);

    // call the application function with the ctxtTo context
    PIN_SetContextReg(ctxtTo, REG_INST_PTR, dumpX87RegsAtExceptionAddr);
#ifdef TARGET_IA32E
    // take care of stack alignment
    ADDRINT curSp          = PIN_GetContextReg(ctxtTo, REG_RSP);
    INT32 currentAlignment = curSp % 16;
    PIN_SetContextReg(ctxtTo, REG_RSP, curSp - GetStackAdjustmentForRedirectionToFunction(currentAlignment));
#endif
}

extern "C" VOID FldzToTop3_a();

VOID CallToFldzToTop3() { FldzToTop3_a(); }

VOID Trace(TRACE trace, VOID* v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            xed_iclass_enum_t iclass1 = static_cast< xed_iclass_enum_t >(INS_Opcode(ins));
            if (iclass1 == XED_ICLASS_FLD1 && INS_Valid(INS_Next(ins)))
            {
                xed_iclass_enum_t iclass2 = static_cast< xed_iclass_enum_t >(INS_Opcode(INS_Next(ins)));
                if (iclass2 == XED_ICLASS_FLD1 && INS_Valid(INS_Next(INS_Next(ins))))
                {
                    xed_iclass_enum_t iclass3 = static_cast< xed_iclass_enum_t >(INS_Opcode(INS_Next(INS_Next(ins))));
                    if (iclass3 == XED_ICLASS_FLD1)
                    {
                        printf("found fld1 sequence at %lx\n", (unsigned long)INS_Address(INS_Next(INS_Next(ins))));

                        {
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToFldzToTop3), IARG_END);
                            printf("Inserted call1 to FldzToTop3 after instruction at %lx\n",
                                   (unsigned long)INS_Address(INS_Next(INS_Next(ins))));
                        }
                    }
                }
            }
        }
    }
}

VOID Image(IMG img, void* v)
{
    RTN rtn = RTN_FindByName(img, "ReplacedX87Regs");
    if (RTN_Valid(rtn))
    {
        PROTO proto = PROTO_Allocate(PIN_PARG(int), CALLINGSTD_DEFAULT, "ReplacedX87Regs", PIN_PARG_END());
        RTN_ReplaceSignature(rtn, AFUNPTR(REPLACE_ReplacedX87Regs), IARG_PROTOTYPE, proto,
                             (KnobUseIargConstContext) ? IARG_CONST_CONTEXT : IARG_CONTEXT, IARG_THREAD_ID, IARG_ORIG_FUNCPTR,
                             IARG_END);
        PROTO_Free(proto);
        printf("TOOL found and replaced ReplacedX87Regs\n");
        fflush(stdout);

        RTN rtn = RTN_FindByName(img, "ExecutedAtFunc");
        if (RTN_Valid(rtn))
        {
            executeAtAddr = RTN_Address(rtn);
            printf("TOOL found ExecutedAtFunc for later PIN_ExecuteAt\n");
            fflush(stdout);
        }

        rtn = RTN_FindByName(img, "DumpX87RegsAtException");
        if (RTN_Valid(rtn))
        {
            dumpX87RegsAtExceptionAddr = RTN_Address(rtn);
            printf("TOOL found DumpX87RegsAtException for later Exception\n");
            fflush(stdout);
        }
    }
}

int main(int argc, char** argv)
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    IMG_AddInstrumentFunction(Image, 0);
    PIN_AddContextChangeFunction(OnException, 0);
    TRACE_AddInstrumentFunction(Trace, 0);

    PIN_StartProgram();
    return 0;
}
