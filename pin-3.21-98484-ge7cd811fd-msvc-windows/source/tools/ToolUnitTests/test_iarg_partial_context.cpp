/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pin.H>
#ifdef TARGET_WINDOWS
namespace WIND
{
#include <windows.h>
}
#define EXPORT_CSYM extern "C" __declspec(dllexport)
#else
#define EXPORT_CSYM extern "C"
#endif
#if defined(__GNUC__)

#include <stdint.h>

#if defined(__cplusplus)
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

#define ALIGN16 __attribute__((aligned(16)))
#define ALIGN8 __attribute__((aligned(8)))

#elif defined(_MSC_VER)

#define ALIGN16 __declspec(align(16))
#define ALIGN8 __declspec(align(8))

#else
#error Expect usage of either GNU or MS compiler.
#endif

ADDRINT executeAtAddr = 0;

#ifdef TARGET_IA32E
#define NUM_XMM_REGS 16
#if defined(__GNUC__)
#define NUM_XMM_SCRATCH_REGS 16
#else
#define NUM_XMM_SCRATCH_REGS 5
#endif
typedef UINT64 reg_t;
#else
#define NUM_XMM_REGS 8
#define NUM_XMM_SCRATCH_REGS 8
typedef UINT32 reg_t;
#endif

extern "C" BOOL ProcessorSupportsAvx();
EXTERN_C VOID FldzToTop3();
EXTERN_C VOID Fld2tToTop3();
EXTERN_C VOID SetXmmRegsToZero();
EXTERN_C VOID SetIntRegsToZero();
EXTERN_C VOID AddToXmmRegs();
EXTERN_C void UnMaskZeroDivideInMxcsr();

EXTERN_C double var1;
EXTERN_C double var2;
EXTERN_C double var3;
double var1 = 2.0;
double var2 = 2.0;
double var3 = 2.0;

EXTERN_C VOID DummyProc() {}

EXPORT_CSYM void TestIargPreserveInReplacement() {}

EXPORT_CSYM void TestIargPreserveInReplacement1() {}

EXPORT_CSYM void TestIargPreserveInReplacement2() {}

EXPORT_CSYM void TestIargPreserveInProbed() {}

EXPORT_CSYM void TestIargPreserveInProbed1() {}

EXPORT_CSYM void TestIargPreserveInProbed2() {}

VOID SetFpStackRegsForPartial(CONTEXT* context)
{
    FPSTATE fpContext;
    FPSTATE* fpContextPtr = &fpContext;
    PIN_GetContextFPState(context, fpContextPtr);

    int i;
    for (i = 0; i < 8; i++)
    {
        fpContextPtr->fxsave_legacy._sts[i]._raw._lo = 0;
        fpContextPtr->fxsave_legacy._sts[i]._raw._hi = 0;
    }

    Fld2tToTop3();

    PIN_SetContextFPState(context, fpContextPtr);
}

VOID SetFpFcwForPartial_1(CONTEXT* context)
{
    FPSTATE fpContext;
    FPSTATE* fpContextPtr = &fpContext;
    PIN_GetContextFPState(context, fpContextPtr);

    fpContextPtr->fxsave_legacy._fcw &= ~0x300; // set rounding mode to 0 (single precision)

    PIN_SetContextFPState(context, fpContextPtr);
}

VOID SetFpFcwForPartial_2(CONTEXT* context)
{
    FPSTATE fpContext;
    FPSTATE* fpContextPtr = &fpContext;
    PIN_GetContextFPState(context, fpContextPtr);

    fpContextPtr->fxsave_legacy._fcw &= ~0x300; // set rounding mode to 0 (single precision)
    fpContextPtr->fxsave_legacy._fcw |= 0x200;  // set rounding mode to 10 binary (double precision)

    PIN_SetContextFPState(context, fpContextPtr);
}

VOID CallToFldzToTop3() { FldzToTop3(); }

VOID CallToSetXmmRegsToZero() { SetXmmRegsToZero(); }

VOID CallToAddToXmmRegs(CONTEXT* context)
{
    FPSTATE fpContext;
    FPSTATE* fpContextPtr = &fpContext;
    PIN_GetContextFPState(context, fpContextPtr);

    for (int i = 0; i < NUM_XMM_REGS; i++)
    {
        fpContextPtr->fxsave_legacy._xmms[i]._vec32[0] += i + 1;
    }
    PIN_SetContextFPState(context, fpContextPtr);
    SetXmmRegsToZero();
}

VOID CallToSetXmmRegs(CONTEXT* context)
{
    FPSTATE fpContext;
    FPSTATE* fpContextPtr = &fpContext;
    PIN_GetContextFPState(context, fpContextPtr);

    for (int i = 0; i < NUM_XMM_REGS; i++)
    {
        fpContextPtr->fxsave_legacy._xmms[i]._vec32[0] = 0xacdcacdc;
        fpContextPtr->fxsave_legacy._xmms[i]._vec32[1] = 0xacdcacdc;
        fpContextPtr->fxsave_legacy._xmms[i]._vec32[2] = 0xacdcacdc;
        fpContextPtr->fxsave_legacy._xmms[i]._vec32[3] = 0xacdcacdc;
    }
    PIN_SetContextFPState(context, fpContextPtr);
    SetXmmRegsToZero();
}

VOID CallToTestFlags(CONTEXT* context)
{
    ADDRINT flagsVal = PIN_GetContextReg(context, REG_GFLAGS);
    if (0xed7 != (flagsVal & 0xffff))
    {
        printf("***Error tool got wrong value in flags %lx\n", (unsigned long)flagsVal);
    }
    // turn off df flag and cf flag
    flagsVal &= 0xfffffbfe;
    PIN_SetContextReg(context, REG_GFLAGS, flagsVal);
}

VOID CallToSetIntRegsToZero() { SetIntRegsToZero(); }

VOID CallToSetIntRegs(CONTEXT* context)
{
    ADDRINT regVal;

    regVal = 3;
    PIN_SetContextReg(context, REG_GAX, regVal);

    regVal += 1;
    PIN_SetContextReg(context, REG_GBX, regVal);

    regVal += 1;
    PIN_SetContextReg(context, REG_GCX, regVal);

    regVal += 1;
    PIN_SetContextReg(context, REG_GDX, regVal);

    regVal += 1;
    PIN_SetContextReg(context, REG_GDI, regVal);

    regVal += 1;
    PIN_SetContextReg(context, REG_GSI, regVal);

    regVal += 1;
    PIN_SetContextReg(context, REG_GBP, regVal);
}

VOID CallToAdd1ToIntRegs(CONTEXT* context)
{
    ADDRINT regVal;

    regVal = PIN_GetContextReg(context, REG_GAX);
    regVal += 1;
    PIN_SetContextReg(context, REG_GAX, regVal);

    regVal = PIN_GetContextReg(context, REG_GBX);

    regVal += 1;
    PIN_SetContextReg(context, REG_GBX, regVal);

    regVal = PIN_GetContextReg(context, REG_GCX);
    regVal += 1;
    PIN_SetContextReg(context, REG_GCX, regVal);

    regVal = PIN_GetContextReg(context, REG_GDX);
    regVal += 1;
    PIN_SetContextReg(context, REG_GDX, regVal);

    regVal = PIN_GetContextReg(context, REG_GDI);
    regVal += 1;
    PIN_SetContextReg(context, REG_GDI, regVal);

    regVal = PIN_GetContextReg(context, REG_GSI);
    regVal += 1;
    PIN_SetContextReg(context, REG_GSI, regVal);

    regVal = PIN_GetContextReg(context, REG_GBP);
    regVal += 1;
    PIN_SetContextReg(context, REG_GBP, regVal);
}

VOID CallToUnMaskZeroDivideInMxcsr(CONTEXT* context)
{
    ADDRINT curMxcsr = PIN_GetContextReg(context, REG_MXCSR);
    if (!(curMxcsr & 0x200))
    {
        printf("***Error1 tool got unexpected value in mxcsr %lx\n", (unsigned long)curMxcsr);
        exit(-1);
    }
    curMxcsr &= ~0x200;
    PIN_SetContextReg(context, REG_MXCSR, curMxcsr);

    ADDRINT newMxcsr;
    newMxcsr = 0x200;
    newMxcsr = PIN_GetContextReg(context, REG_MXCSR);
    if ((newMxcsr & 0x200))
    {
        printf("***Error2 tool got unexpected value in mxcsr %lx\n", (unsigned long)newMxcsr);
        exit(-1);
    }

    FPSTATE fpContext;
    FPSTATE* fpContextPtr = &fpContext;
    PIN_GetContextFPState(context, fpContextPtr);

    if (fpContextPtr->fxsave_legacy._mxcsr != newMxcsr)
    {
        printf("***Error3 tool got unexpected value in mxcsr %lx   %lx\n", (unsigned long)newMxcsr,
               (unsigned long)(fpContextPtr->fxsave_legacy._mxcsr));
        exit(-1);
    }
}

VOID CallToUnMaskZeroDivideInMxcsr2(CONTEXT* context)
{
    FPSTATE fpContext;
    FPSTATE* fpContextPtr = &fpContext;
    PIN_GetContextFPState(context, fpContextPtr);

    if (!(fpContextPtr->fxsave_legacy._mxcsr & 0x200))
    {
        printf("***Error4 tool got unexpected value in mxcsr %lx\n", (unsigned long)(fpContextPtr->fxsave_legacy._mxcsr));
        exit(-1);
    }

    fpContextPtr->fxsave_legacy._mxcsr &= ~0x200;

    PIN_SetContextFPState(context, fpContextPtr);

    ADDRINT newMxcsr;
    newMxcsr = 0x200;
    newMxcsr = PIN_GetContextReg(context, REG_MXCSR);
    if ((newMxcsr & 0x200))
    {
        printf("***Error5 tool got unexpected value in mxcsr %lx\n", (unsigned long)newMxcsr);
        exit(-1);
    }

    memset(reinterpret_cast< VOID* >(fpContextPtr), 0, FPSTATE_SIZE);
    PIN_GetContextFPState(context, fpContextPtr);
    if (fpContextPtr->fxsave_legacy._mxcsr != newMxcsr)
    {
        printf("***Error6 tool got unexpected value in mxcsr %lx   %lx\n", (unsigned long)newMxcsr,
               (unsigned long)(fpContextPtr->fxsave_legacy._mxcsr));
        exit(-1);
    }
}

#define NUM_TESTS 21
static int testNum = 0;
//ALIGN16
static UINT64 vals[] = {1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 15, 0, 16, 0};

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
UINT64* val1  = &vals[0];
UINT64* val2  = &vals[2];
UINT64* val3  = &vals[4];
UINT64* val4  = &vals[6];
UINT64* val5  = &vals[8];
UINT64* val6  = &vals[10];
UINT64* val7  = &vals[12];
UINT64* val8  = &vals[14];
UINT64* val9  = &vals[16];
UINT64* val10 = &vals[18];
UINT64* val11 = &vals[20];
UINT64* val12 = &vals[22];
UINT64* val13 = &vals[24];
UINT64* val14 = &vals[26];
UINT64* val15 = &vals[28];
UINT64* val16 = &vals[30];

#ifdef TARGET_IA32E
extern "C" UINT64 setFlagsX;
UINT64 setFlagsX = 0;
#else
extern "C" UINT32 setFlagsX;
UINT32 setFlagsX = 0;
#endif

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
                        printf("found fld1 sequence at %lx\n", (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                        fflush(stdout);

                        if (testNum == 0)
                        {
                            REGSET regsIn;
                            REGSET_AddAll(regsIn);
                            REGSET regsOut;
                            REGSET_Clear(regsOut);
                            REGSET_Insert(regsOut, REG_X87);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(SetFpStackRegsForPartial),
                                           IARG_PARTIAL_CONTEXT, &regsIn, &regsOut, IARG_END);
                            printf("Inserted call1 to SetFpStackRegsForPartial after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
                        else if (testNum == 1)
                        {
                            REGSET regsIn;
                            REGSET_AddAll(regsIn);
                            REGSET_Remove(regsIn, REG_X87);
                            REGSET regsOut;
                            REGSET_Clear(regsOut);
                            REGSET_Insert(regsOut, REG_X87);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(SetFpStackRegsForPartial),
                                           IARG_PARTIAL_CONTEXT, &regsIn, &regsOut, IARG_END);
                            printf("Inserted call2 to SetFpStackRegsForPartial after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            fflush(stdout);
                            testNum++;
                        }
                        else if (testNum == 2)
                        {
                            REGSET regsIn;
                            REGSET_Clear(regsIn);
                            REGSET regsOut;
                            REGSET_Clear(regsOut);
                            REGSET_Insert(regsOut, REG_X87);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(SetFpStackRegsForPartial),
                                           IARG_PARTIAL_CONTEXT, &regsIn, &regsOut, IARG_END);
                            printf("Inserted call3 to SetFpStackRegsForPartial after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            fflush(stdout);
                            testNum++;
                        }
                        else if (testNum == 3)
                        {
                            REGSET regsIn;
                            REGSET_AddAll(regsIn);
                            REGSET regsOut;
                            REGSET_Clear(regsOut);
                            REGSET_Insert(regsOut, REG_XMM0);
                            REGSET_Insert(regsOut, REG_XMM1);
                            REGSET_Insert(regsOut, REG_XMM2);
                            REGSET_Insert(regsOut, REG_XMM3);
                            REGSET_Insert(regsOut, REG_XMM4);
                            REGSET_Insert(regsOut, REG_XMM5);
                            REGSET_Insert(regsOut, REG_XMM6);
                            REGSET_Insert(regsOut, REG_XMM7);
                            if (ProcessorSupportsAvx())
                            {
                                REGSET_Insert(regsOut, REG_YMM0);
                                REGSET_Insert(regsOut, REG_YMM1);
                                REGSET_Insert(regsOut, REG_YMM2);
                                REGSET_Insert(regsOut, REG_YMM3);
                                REGSET_Insert(regsOut, REG_YMM4);
                                REGSET_Insert(regsOut, REG_YMM5);
                                REGSET_Insert(regsOut, REG_YMM6);
                                REGSET_Insert(regsOut, REG_YMM7);
                            }
#ifdef TARGET_IA32E
                            REGSET_Insert(regsOut, REG_XMM8);
                            REGSET_Insert(regsOut, REG_XMM9);
                            REGSET_Insert(regsOut, REG_XMM10);
                            REGSET_Insert(regsOut, REG_XMM11);
                            REGSET_Insert(regsOut, REG_XMM12);
                            REGSET_Insert(regsOut, REG_XMM13);
                            REGSET_Insert(regsOut, REG_XMM14);
                            REGSET_Insert(regsOut, REG_XMM15);
                            if (ProcessorSupportsAvx())
                            {
                                REGSET_Insert(regsOut, REG_YMM8);
                                REGSET_Insert(regsOut, REG_YMM9);
                                REGSET_Insert(regsOut, REG_YMM10);
                                REGSET_Insert(regsOut, REG_YMM11);
                                REGSET_Insert(regsOut, REG_YMM12);
                                REGSET_Insert(regsOut, REG_YMM13);
                                REGSET_Insert(regsOut, REG_YMM14);
                                REGSET_Insert(regsOut, REG_YMM15);
                            }
#endif
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToAddToXmmRegs),
                                           IARG_PARTIAL_CONTEXT, &regsIn, &regsOut, IARG_END);
                            printf("Inserted call1 to CallToAddToXmmRegs after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            fflush(stdout);
                            testNum++;
                        }
                        else if (testNum == 4)
                        {
                            REGSET regsIn;
                            REGSET_Clear(regsIn);
                            REGSET_Insert(regsIn, REG_XMM0);
                            REGSET_Insert(regsIn, REG_XMM1);
                            REGSET_Insert(regsIn, REG_XMM2);
                            REGSET_Insert(regsIn, REG_XMM3);
                            REGSET_Insert(regsIn, REG_XMM4);
                            REGSET_Insert(regsIn, REG_XMM5);
                            REGSET_Insert(regsIn, REG_XMM6);
                            REGSET_Insert(regsIn, REG_XMM7);
                            if (ProcessorSupportsAvx())
                            {
                                REGSET_Insert(regsIn, REG_YMM0);
                                REGSET_Insert(regsIn, REG_YMM1);
                                REGSET_Insert(regsIn, REG_YMM2);
                                REGSET_Insert(regsIn, REG_YMM3);
                                REGSET_Insert(regsIn, REG_YMM4);
                                REGSET_Insert(regsIn, REG_YMM5);
                                REGSET_Insert(regsIn, REG_YMM6);
                                REGSET_Insert(regsIn, REG_YMM7);
                            }
#ifdef TARGET_IA32E
                            REGSET_Insert(regsIn, REG_XMM8);
                            REGSET_Insert(regsIn, REG_XMM9);
                            REGSET_Insert(regsIn, REG_XMM10);
                            REGSET_Insert(regsIn, REG_XMM11);
                            REGSET_Insert(regsIn, REG_XMM12);
                            REGSET_Insert(regsIn, REG_XMM13);
                            REGSET_Insert(regsIn, REG_XMM14);
                            REGSET_Insert(regsIn, REG_XMM15);
                            if (ProcessorSupportsAvx())
                            {
                                REGSET_Insert(regsIn, REG_YMM8);
                                REGSET_Insert(regsIn, REG_YMM9);
                                REGSET_Insert(regsIn, REG_YMM10);
                                REGSET_Insert(regsIn, REG_YMM11);
                                REGSET_Insert(regsIn, REG_YMM12);
                                REGSET_Insert(regsIn, REG_YMM13);
                                REGSET_Insert(regsIn, REG_YMM14);
                                REGSET_Insert(regsIn, REG_YMM15);
                            }
#endif
                            REGSET regsOut;
                            REGSET_Clear(regsOut);
                            REGSET_Insert(regsOut, REG_X87);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToAddToXmmRegs),
                                           IARG_PARTIAL_CONTEXT, &regsIn, &regsOut, IARG_END);
                            printf("Inserted call2 to CallToAddToXmmRegs after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            fflush(stdout);
                            testNum++;
                        }
                        else if (testNum == 5)
                        {
                            REGSET regsIn;
                            REGSET_Clear(regsIn);
                            REGSET regsOut;
                            REGSET_Clear(regsOut);
                            REGSET_Insert(regsOut, REG_FPCW);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(SetFpFcwForPartial_1),
                                           IARG_PARTIAL_CONTEXT, &regsIn, &regsOut, IARG_END);
                            printf("Inserted call1 to SetFpFcwForPartial_1 after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            fflush(stdout);
                            testNum++;
                        }
                        else if (testNum == 6)
                        {
                            REGSET regsIn;
                            REGSET_Clear(regsIn);
                            REGSET regsOut;
                            REGSET_Clear(regsOut);
                            REGSET_Insert(regsOut, REG_FPCW);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(SetFpFcwForPartial_2),
                                           IARG_PARTIAL_CONTEXT, &regsIn, &regsOut, IARG_END);
                            printf("Inserted call1 to SetFpFcwForPartial_2 after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            fflush(stdout);
                            testNum++;
                        }
                        else if (testNum == 7)
                        {
                            REGSET regsIn;
                            REGSET_Clear(regsIn);
                            REGSET regsOut;
                            REGSET_Clear(regsOut);
                            REGSET_Insert(regsOut, REG_X87);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(SetFpFcwForPartial_1),
                                           IARG_PARTIAL_CONTEXT, &regsIn, &regsOut, IARG_END);
                            printf("Inserted call2 to SetFpFcwForPartial_1 after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            fflush(stdout);
                            testNum++;
                        }
                        else if (testNum == 8)
                        {
                            REGSET regsIn;
                            REGSET_Clear(regsIn);
                            REGSET regsOut;
                            REGSET_Clear(regsOut);
                            REGSET_Insert(regsOut, REG_X87);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(SetFpFcwForPartial_2),
                                           IARG_PARTIAL_CONTEXT, &regsIn, &regsOut, IARG_END);
                            printf("Inserted call2 to SetFpFcwForPartial_2 after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            fflush(stdout);
                            testNum++;
                        }
                        else if (testNum == 9)
                        {
                            REGSET regsIn;
                            REGSET_Clear(regsIn);
                            REGSET_Insert(regsIn, REG_GAX);
                            REGSET_Insert(regsIn, REG_GBX);
                            REGSET_Insert(regsIn, REG_GCX);
                            REGSET_Insert(regsIn, REG_GDX);
                            REGSET_Insert(regsIn, REG_GSI);
                            REGSET_Insert(regsIn, REG_GDI);
                            REGSET_Insert(regsIn, REG_GBP);
#ifdef TARGET_IA32E
                            REGSET_Insert(regsIn, REG_R8);
                            REGSET_Insert(regsIn, REG_R9);
                            REGSET_Insert(regsIn, REG_R10);
                            REGSET_Insert(regsIn, REG_R11);
                            REGSET_Insert(regsIn, REG_R12);
                            REGSET_Insert(regsIn, REG_R13);
                            REGSET_Insert(regsIn, REG_R14);
                            REGSET_Insert(regsIn, REG_R15);
#endif
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToAdd1ToIntRegs),
                                           IARG_PARTIAL_CONTEXT, &regsIn, &regsIn, IARG_END);
                            printf("Inserted call1 to CallToAdd1ToIntRegs after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
                        else if (testNum == 10)
                        {
                            REGSET regsIn;
                            REGSET_Clear(regsIn);
                            REGSET regsOut;
                            REGSET_Clear(regsOut);
                            REGSET_Insert(regsOut, REG_GAX);
                            REGSET_Insert(regsOut, REG_GBX);
                            REGSET_Insert(regsOut, REG_GCX);
                            REGSET_Insert(regsOut, REG_GDX);
                            REGSET_Insert(regsOut, REG_GSI);
                            REGSET_Insert(regsOut, REG_GDI);
                            REGSET_Insert(regsOut, REG_GBP);
#ifdef TARGET_IA32E
                            REGSET_Insert(regsOut, REG_R8);
                            REGSET_Insert(regsOut, REG_R9);
                            REGSET_Insert(regsOut, REG_R10);
                            REGSET_Insert(regsOut, REG_R11);
                            REGSET_Insert(regsOut, REG_R12);
                            REGSET_Insert(regsOut, REG_R13);
                            REGSET_Insert(regsOut, REG_R14);
                            REGSET_Insert(regsOut, REG_R15);
#endif
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToSetIntRegs), IARG_PARTIAL_CONTEXT,
                                           &regsIn, &regsOut, IARG_END);
                            printf("Inserted call1 to CallToSetIntRegs after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
                        else if (testNum == 11)
                        {
                            REGSET regsIn;
                            REGSET_Clear(regsIn);
                            REGSET_Insert(regsIn, REG_X87);
                            REGSET regsOut;
                            REGSET_Clear(regsOut);
                            REGSET_Insert(regsOut, REG_MXCSR);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToUnMaskZeroDivideInMxcsr),
                                           IARG_PARTIAL_CONTEXT, &regsIn, &regsOut, IARG_END);
                            printf("Inserted call1 to UnMaskZeroDivideInMxcsr after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
                        else if (testNum == 12)
                        {
                            REGSET regsIn;
                            REGSET_Clear(regsIn);
                            REGSET_Insert(regsIn, REG_X87);
                            REGSET regsOut;
                            REGSET_Clear(regsOut);
                            REGSET_Insert(regsOut, REG_MXCSR);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToUnMaskZeroDivideInMxcsr2),
                                           IARG_PARTIAL_CONTEXT, &regsIn, &regsOut, IARG_END);
                            printf("Inserted call1 to UnMaskZeroDivideInMxcsr2 after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
                        else if (testNum == 13)
                        {
                            REGSET regsIn;
                            REGSET_Clear(regsIn);
                            REGSET_Insert(regsIn, REG_MXCSR);
                            REGSET regsOut;
                            REGSET_Clear(regsOut);
                            REGSET_Insert(regsOut, REG_MXCSR);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToUnMaskZeroDivideInMxcsr),
                                           IARG_PARTIAL_CONTEXT, &regsIn, &regsOut, IARG_END);
                            printf("Inserted call2 to UnMaskZeroDivideInMxcsr after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
                        else if (testNum == 14)
                        {
                            REGSET regsIn;
                            REGSET_Clear(regsIn);
                            REGSET_Insert(regsIn, REG_MXCSR);
                            REGSET regsOut;
                            REGSET_Clear(regsOut);
                            REGSET_Insert(regsOut, REG_MXCSR);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToUnMaskZeroDivideInMxcsr2),
                                           IARG_PARTIAL_CONTEXT, &regsIn, &regsOut, IARG_END);
                            printf("Inserted call2 to UnMaskZeroDivideInMxcsr2 after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
                        else if (testNum == 15)
                        {
                            REGSET regsIn;
                            REGSET_Clear(regsIn);
                            REGSET_Insert(regsIn, REG_X87);
                            REGSET regsOut;
                            REGSET_Clear(regsOut);
                            REGSET_Insert(regsOut, REG_X87);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToUnMaskZeroDivideInMxcsr2),
                                           IARG_PARTIAL_CONTEXT, &regsIn, &regsOut, IARG_END);
                            printf("Inserted call3 to UnMaskZeroDivideInMxcsr2 after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
                        else if (testNum == 16)
                        {
                            REGSET regsIn;
                            REGSET_Clear(regsIn);
                            REGSET_Insert(regsIn, REG_X87);
                            REGSET regsOut;
                            REGSET_Clear(regsOut);
                            REGSET_Insert(regsOut, REG_X87);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToAddToXmmRegs),
                                           IARG_PARTIAL_CONTEXT, &regsIn, &regsOut, IARG_END);
                            printf("Inserted call3 to CallToAddToXmmRegs after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            fflush(stdout);
                            testNum++;
                        }
                        else if (testNum == 17)
                        {
                            REGSET regsOut;
                            REGSET_Clear(regsOut);
                            REGSET_Insert(regsOut, REG_XMM0);
                            REGSET_Insert(regsOut, REG_XMM1);
                            REGSET_Insert(regsOut, REG_XMM2);
                            REGSET_Insert(regsOut, REG_XMM3);
                            REGSET_Insert(regsOut, REG_XMM4);
                            REGSET_Insert(regsOut, REG_XMM5);
                            REGSET_Insert(regsOut, REG_XMM6);
                            REGSET_Insert(regsOut, REG_XMM7);
                            if (ProcessorSupportsAvx())
                            {
                                REGSET_Insert(regsOut, REG_YMM0);
                                REGSET_Insert(regsOut, REG_YMM1);
                                REGSET_Insert(regsOut, REG_YMM2);
                                REGSET_Insert(regsOut, REG_YMM3);
                                REGSET_Insert(regsOut, REG_YMM4);
                                REGSET_Insert(regsOut, REG_YMM5);
                                REGSET_Insert(regsOut, REG_YMM6);
                                REGSET_Insert(regsOut, REG_YMM7);
                            }
#ifdef TARGET_IA32E
                            REGSET_Insert(regsOut, REG_XMM8);
                            REGSET_Insert(regsOut, REG_XMM9);
                            REGSET_Insert(regsOut, REG_XMM10);
                            REGSET_Insert(regsOut, REG_XMM11);
                            REGSET_Insert(regsOut, REG_XMM12);
                            REGSET_Insert(regsOut, REG_XMM13);
                            REGSET_Insert(regsOut, REG_XMM14);
                            REGSET_Insert(regsOut, REG_XMM15);
                            if (ProcessorSupportsAvx())
                            {
                                REGSET_Insert(regsOut, REG_YMM8);
                                REGSET_Insert(regsOut, REG_YMM9);
                                REGSET_Insert(regsOut, REG_YMM10);
                                REGSET_Insert(regsOut, REG_YMM11);
                                REGSET_Insert(regsOut, REG_YMM12);
                                REGSET_Insert(regsOut, REG_YMM13);
                                REGSET_Insert(regsOut, REG_YMM14);
                                REGSET_Insert(regsOut, REG_YMM15);
                            }
#endif
                            REGSET regsIn;
                            REGSET_Clear(regsIn);

                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToSetXmmRegs), IARG_PARTIAL_CONTEXT,
                                           &regsIn, &regsOut, IARG_END);
                            printf("Inserted call1 to CallToSetXmmRegs after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            fflush(stdout);
                            testNum++;
                        }
                        else if (testNum == 18)
                        {
                            REGSET regsIn;
                            REGSET_Clear(regsIn);
                            REGSET_Insert(regsIn, REG_GFLAGS);
                            REGSET regsOut;
                            REGSET_Clear(regsOut);
                            REGSET_Insert(regsOut, REG_GFLAGS);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToTestFlags), IARG_PARTIAL_CONTEXT,
                                           &regsIn, &regsOut, IARG_END);
                            printf("Inserted call to CallToTestFlags after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            fflush(stdout);
                            testNum++;
                        }
                        return;
                    }
                }
            }
        }
    }
}

typedef struct
{
    UINT32 _lo1; ///< Least significant part of value.
    UINT32 _lo2; ///< Least significant part of value.
    UINT32 _hi1; ///< Most significant part of value.
    UINT32 _hi2; ///< Most significant part of value.
} RAW32;

VOID REPLACE_ReplacedX87Regs(CONTEXT* context, THREADID tid, AFUNPTR originalFunction)
{
    testNum++;
    printf("TOOL in REPLACE_ReplacedX87Regs x87 regs are:\n");
    fflush(stdout);
    CHAR fpContextSpaceForFpConextFromPin[FPSTATE_SIZE];
    FPSTATE* fpContextFromPin = reinterpret_cast< FPSTATE* >(fpContextSpaceForFpConextFromPin);

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
    printf("_mxcsr %lx\n", (unsigned long)(fpContextFromPin->fxsave_legacy._mxcsr));
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
    ctxt = context;

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

EXTERN_C VOID TestIargPreserveReplacement(CONTEXT* context)
{
    testNum++;
    printf("TestIargPreserveReplacement called\n");
    fflush(stdout);
    FPSTATE fpContext;
    FPSTATE* fpContextPtr = &fpContext;
    PIN_GetContextFPState(context, fpContextPtr);

    int i;
    for (i = 0; i < 8; i++)
    {
        fpContextPtr->fxsave_legacy._sts[i]._raw._lo = 0;
        fpContextPtr->fxsave_legacy._sts[i]._raw._hi = 0;
    }

    Fld2tToTop3();

    PIN_SetContextFPState(context, fpContextPtr);
}

EXTERN_C VOID TestIargPreserveReplacement1() { testNum++; }

EXTERN_C VOID TestIargPreserveProbedBefore()
{
    testNum++;
    FldzToTop3();
}

EXTERN_C VOID TestIargPreserveProbedBefore1() { testNum++; }

VOID ImageLoad(IMG img, VOID* v)
{
    RTN rtn = RTN_FindByName(img, "TestIargPreserveInReplacement");

    if (RTN_Valid(rtn))
    {
        printf("found TestIargPreserveInReplacement\n");
        fflush(stdout);
        PROTO proto = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "TestIargPreserveInReplacementProto", PIN_PARG_END());
        REGSET regsIn;
        REGSET_Clear(regsIn);
        REGSET_Insert(regsIn, REG_X87);
        REGSET regsOut;
        REGSET_Clear(regsOut);
        REGSET_Insert(regsOut, REG_X87);
        RTN_ReplaceSignature(rtn, AFUNPTR(TestIargPreserveReplacement), IARG_PROTOTYPE, proto, IARG_PARTIAL_CONTEXT, &regsIn,
                             &regsOut, IARG_ORIG_FUNCPTR, IARG_END);
        PROTO_Free(proto);
    }

    rtn = RTN_FindByName(img, "TestIargPreserveInReplacement1");

    if (RTN_Valid(rtn))
    {
        printf("found TestIargPreserveInReplacement1\n");
        fflush(stdout);
        PROTO proto = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "TestIargPreserveInReplacementProto", PIN_PARG_END());

        RTN_ReplaceSignature(rtn, AFUNPTR(TestIargPreserveReplacement1), IARG_PROTOTYPE, proto, IARG_CONTEXT, IARG_ORIG_FUNCPTR,
                             IARG_END);
        PROTO_Free(proto);
    }

    rtn = RTN_FindByName(img, "ReplacedX87Regs");
    if (RTN_Valid(rtn))
    {
        printf("found ReplacedX87Regs\n");
        fflush(stdout);
        REGSET regsIn;
        REGSET_AddAll(regsIn);
        REGSET regsOut;
        REGSET_Clear(regsOut);
        REGSET_Insert(regsOut, REG_X87);
        REGSET_Insert(regsOut, REG_INST_PTR);
        PROTO proto = PROTO_Allocate(PIN_PARG(int), CALLINGSTD_DEFAULT, "ReplacedX87Regs", PIN_PARG_END());
        RTN_ReplaceSignature(rtn, AFUNPTR(REPLACE_ReplacedX87Regs), IARG_PROTOTYPE, proto, IARG_PARTIAL_CONTEXT, &regsIn,
                             &regsOut, IARG_THREAD_ID, IARG_ORIG_FUNCPTR, IARG_END);
        PROTO_Free(proto);
        printf("TOOL found and replaced ReplacedX87Regs\n");
        fflush(stdout);
    }

    rtn = RTN_FindByName(img, "ExecutedAtFunc");
    if (RTN_Valid(rtn))
    {
        executeAtAddr = RTN_Address(rtn);
        printf("TOOL found ExecutedAtFunc for later PIN_ExecuteAt\n");
        fflush(stdout);
    }
}

VOID Fini(INT32 code, VOID* v)
{
    if (testNum != NUM_TESTS)
    {
        printf("***Error not all expected tests ran testNum %d\n", testNum);
        exit(-1);
    }
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    PIN_InitSymbols();

    IMG_AddInstrumentFunction(ImageLoad, 0);
    TRACE_AddInstrumentFunction(Trace, 0);

    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
