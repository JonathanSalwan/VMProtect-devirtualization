/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <stdlib.h>
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
#ifdef TARGET_WINDOWS
#define ASMNAME(name)
#else
#define ASMNAME(name) asm(name)
#endif

#if defined(__cplusplus)
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

EXTERN_C BOOL ProcessorSupportsAvx() ASMNAME("ProcessorSupportsAvx");
EXTERN_C BOOL SupportsAvx512f() ASMNAME("SupportsAvx512f");
EXTERN_C VOID FldzToTop3() ASMNAME("FldzToTop3");
EXTERN_C VOID SetXmmRegsToZero() ASMNAME("SetXmmRegsToZero");
EXTERN_C VOID SetIntRegsToZero() ASMNAME("SetIntRegsToZero");
EXTERN_C void UnMaskZeroDivideInMxcsr() ASMNAME("UnMaskZeroDivideInMxcsr");

EXTERN_C double var1;
EXTERN_C double var2;
EXTERN_C double var3;
double var1 = 2.0;
double var2 = 2.0;
double var3 = 2.0;

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

EXTERN_C VOID DummyProc() {}

EXPORT_CSYM void TestIargPreserveInReplacement() {}

EXPORT_CSYM void TestIargPreserveInReplacement1() {}

EXPORT_CSYM void TestIargPreserveInReplacement2() {}

EXPORT_CSYM void TestIargPreserveInProbed() {}

EXPORT_CSYM void TestIargPreserveInProbed1() {}

EXPORT_CSYM void TestIargPreserveInProbed2() {}

VOID CallToFldzToTop3() { FldzToTop3(); }

VOID CallToSetXmmRegsToZero() { SetXmmRegsToZero(); }

VOID CallToSetIntRegsToZero() { SetIntRegsToZero(); }

VOID CallToUnMaskZeroDivideInMxcsr() { UnMaskZeroDivideInMxcsr(); }

#define NUM_TESTS 19
static int testNum = 0;

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
                        if (testNum == 0)
                        {
                            REGSET regsFP;
                            REGSET_Clear(regsFP);
                            REGSET_Insert(regsFP, REG_X87);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToFldzToTop3), IARG_PRESERVE,
                                           &regsFP, IARG_END);
                            printf("Inserted call1 to FldzToTop3 after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
                        else if (testNum == 1)
                        {
                            REGSET regsNone;
                            REGSET_Clear(regsNone);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToFldzToTop3), IARG_PRESERVE,
                                           &regsNone, IARG_END);
                            printf("Inserted call2 to FldzToTop3 after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
                        else if (testNum == 2)
                        {
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToFldzToTop3), IARG_END);
                            printf("Inserted call3 to FldzToTop3 after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
                        else if (testNum == 3)
                        {
                            REGSET regsNone;
                            REGSET_Clear(regsNone);
                            REGSET_Insert(regsNone, REG_X87);
                            REGSET_Insert(regsNone, REG_MXCSR);
                            REGSET_Remove(regsNone, REG_X87);
                            REGSET_Remove(regsNone, REG_MXCSR);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToFldzToTop3), IARG_PRESERVE,
                                           &regsNone, IARG_END);
                            printf("Inserted call4 to FldzToTop3 after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
                        else if (testNum == 4)
                        {
                            REGSET regsXMM;
                            REGSET_Clear(regsXMM);

                            // need to declare X87 preserved also - because otherwise the fxsave/fxrstor will preserve the
                            // xmm regs
                            REGSET_Insert(regsXMM, REG_X87);

                            REGSET_Insert(regsXMM, REG_XMM0);
                            REGSET_Insert(regsXMM, REG_XMM1);
                            REGSET_Insert(regsXMM, REG_XMM2);
                            REGSET_Insert(regsXMM, REG_XMM3);
                            REGSET_Insert(regsXMM, REG_XMM4);
                            REGSET_Insert(regsXMM, REG_XMM5);
                            REGSET_Insert(regsXMM, REG_XMM6);
                            REGSET_Insert(regsXMM, REG_XMM7);
                            if (ProcessorSupportsAvx())
                            {
                                REGSET_Insert(regsXMM, REG_YMM0);
                                REGSET_Insert(regsXMM, REG_YMM1);
                                REGSET_Insert(regsXMM, REG_YMM2);
                                REGSET_Insert(regsXMM, REG_YMM3);
                                REGSET_Insert(regsXMM, REG_YMM4);
                                REGSET_Insert(regsXMM, REG_YMM5);
                                REGSET_Insert(regsXMM, REG_YMM6);
                                REGSET_Insert(regsXMM, REG_YMM7);
                            }
                            if (SupportsAvx512f())
                            {
                                REGSET_Insert(regsXMM, REG_ZMM0);
                                REGSET_Insert(regsXMM, REG_ZMM1);
                                REGSET_Insert(regsXMM, REG_ZMM2);
                                REGSET_Insert(regsXMM, REG_ZMM3);
                                REGSET_Insert(regsXMM, REG_ZMM4);
                                REGSET_Insert(regsXMM, REG_ZMM5);
                                REGSET_Insert(regsXMM, REG_ZMM6);
                                REGSET_Insert(regsXMM, REG_ZMM7);
                            }
#ifdef TARGET_IA32E
                            REGSET_Insert(regsXMM, REG_XMM8);
                            REGSET_Insert(regsXMM, REG_XMM9);
                            REGSET_Insert(regsXMM, REG_XMM10);
                            REGSET_Insert(regsXMM, REG_XMM11);
                            REGSET_Insert(regsXMM, REG_XMM12);
                            REGSET_Insert(regsXMM, REG_XMM13);
                            REGSET_Insert(regsXMM, REG_XMM14);
                            REGSET_Insert(regsXMM, REG_XMM15);
                            if (ProcessorSupportsAvx())
                            {
                                REGSET_Insert(regsXMM, REG_YMM8);
                                REGSET_Insert(regsXMM, REG_YMM9);
                                REGSET_Insert(regsXMM, REG_YMM10);
                                REGSET_Insert(regsXMM, REG_YMM11);
                                REGSET_Insert(regsXMM, REG_YMM12);
                                REGSET_Insert(regsXMM, REG_YMM13);
                                REGSET_Insert(regsXMM, REG_YMM14);
                                REGSET_Insert(regsXMM, REG_YMM15);
                            }
                            if (SupportsAvx512f())
                            {
                                REGSET_Insert(regsXMM, REG_ZMM8);
                                REGSET_Insert(regsXMM, REG_ZMM9);
                                REGSET_Insert(regsXMM, REG_ZMM10);
                                REGSET_Insert(regsXMM, REG_ZMM11);
                                REGSET_Insert(regsXMM, REG_ZMM12);
                                REGSET_Insert(regsXMM, REG_ZMM13);
                                REGSET_Insert(regsXMM, REG_ZMM14);
                                REGSET_Insert(regsXMM, REG_ZMM15);
                                REGSET_Insert(regsXMM, REG_ZMM16);
                                REGSET_Insert(regsXMM, REG_ZMM17);
                                REGSET_Insert(regsXMM, REG_ZMM18);
                                REGSET_Insert(regsXMM, REG_ZMM19);
                                REGSET_Insert(regsXMM, REG_ZMM20);
                                REGSET_Insert(regsXMM, REG_ZMM21);
                                REGSET_Insert(regsXMM, REG_ZMM22);
                                REGSET_Insert(regsXMM, REG_ZMM23);
                                REGSET_Insert(regsXMM, REG_ZMM24);
                                REGSET_Insert(regsXMM, REG_ZMM25);
                                REGSET_Insert(regsXMM, REG_ZMM26);
                                REGSET_Insert(regsXMM, REG_ZMM27);
                                REGSET_Insert(regsXMM, REG_ZMM28);
                                REGSET_Insert(regsXMM, REG_ZMM29);
                                REGSET_Insert(regsXMM, REG_ZMM30);
                                REGSET_Insert(regsXMM, REG_ZMM31);
                            }
#endif
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToSetXmmRegsToZero), IARG_PRESERVE,
                                           &regsXMM, IARG_END);
                            printf("Inserted call1 to CallToSetXmmRegsToZero after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
                        else if (testNum == 5)
                        {
                            REGSET regsNone;
                            REGSET_Clear(regsNone);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToSetXmmRegsToZero), IARG_PRESERVE,
                                           &regsNone, IARG_END);
                            printf("Inserted call2 to CallToSetXmmRegsToZero after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
                        else if (testNum == 6)
                        {
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToSetXmmRegsToZero), IARG_END);
                            printf("Inserted call3 to CallToSetXmmRegsToZero after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
                        else if (testNum == 7)
                        {
                            REGSET regsXMM;
                            REGSET_Clear(regsXMM);
                            REGSET_Insert(regsXMM, REG_X87);
                            REGSET_Insert(regsXMM, REG_XMM0);
                            REGSET_Insert(regsXMM, REG_XMM1);
                            REGSET_Insert(regsXMM, REG_XMM2);
                            REGSET_Insert(regsXMM, REG_XMM3);
                            REGSET_Insert(regsXMM, REG_XMM4);
                            REGSET_Insert(regsXMM, REG_XMM5);
                            REGSET_Insert(regsXMM, REG_XMM6);
                            REGSET_Insert(regsXMM, REG_XMM7);
#ifdef TARGET_IA32E
                            REGSET_Insert(regsXMM, REG_XMM8);
                            REGSET_Insert(regsXMM, REG_XMM9);
                            REGSET_Insert(regsXMM, REG_XMM10);
                            REGSET_Insert(regsXMM, REG_XMM11);
                            REGSET_Insert(regsXMM, REG_XMM12);
                            REGSET_Insert(regsXMM, REG_XMM13);
                            REGSET_Insert(regsXMM, REG_XMM14);
                            REGSET_Insert(regsXMM, REG_XMM15);
#endif
                            REGSET_Remove(regsXMM, REG_X87);
                            REGSET_Remove(regsXMM, REG_XMM0);
                            REGSET_Remove(regsXMM, REG_XMM1);
                            REGSET_Remove(regsXMM, REG_XMM2);
                            REGSET_Remove(regsXMM, REG_XMM3);
                            REGSET_Remove(regsXMM, REG_XMM4);
                            REGSET_Remove(regsXMM, REG_XMM5);
                            REGSET_Remove(regsXMM, REG_XMM6);
                            REGSET_Remove(regsXMM, REG_XMM7);
#ifdef TARGET_IA32E
                            REGSET_Remove(regsXMM, REG_XMM8);
                            REGSET_Remove(regsXMM, REG_XMM9);
                            REGSET_Remove(regsXMM, REG_XMM10);
                            REGSET_Remove(regsXMM, REG_XMM11);
                            REGSET_Remove(regsXMM, REG_XMM12);
                            REGSET_Remove(regsXMM, REG_XMM13);
                            REGSET_Remove(regsXMM, REG_XMM14);
                            REGSET_Remove(regsXMM, REG_XMM15);
#endif
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToSetXmmRegsToZero), IARG_PRESERVE,
                                           &regsXMM, IARG_END);
                            printf("Inserted call4 to CallToSetXmmRegsToZero after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
                        else if (testNum == 8)
                        {
                            REGSET regsX87;
                            REGSET_Clear(regsX87);
                            REGSET_Insert(regsX87, REG_X87);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToSetXmmRegsToZero), IARG_PRESERVE,
                                           &regsX87, IARG_END);
                            printf("Inserted call5 to CallToSetXmmRegsToZero after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
#if 0
                        // this test is not run because overwriting the integer registers can cause Pin to fail
                        else if(testNum == 9)
                        {
                            REGSET regsIntScratches;
                            REGSET_Clear(regsIntScratches);
                            REGSET_Insert (regsIntScratches, REG_GAX);
                            REGSET_Insert (regsIntScratches, REG_GCX);
                            REGSET_Insert (regsIntScratches, REG_GDX);
#ifdef TARGET_IA32E
                            REGSET_Insert (regsIntScratches, REG_R8);
                            REGSET_Insert (regsIntScratches, REG_R9);
                            REGSET_Insert (regsIntScratches, REG_R10);
                            REGSET_Insert (regsIntScratches, REG_R11);
#if defined(__GNUC__)
                            REGSET_Insert (regsIntScratches, REG_GSI);
                            REGSET_Insert (regsIntScratches, REG_GDI);
#endif
#endif
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToSetIntRegsToZero), IARG_PRESERVE, &regsIntScratches, IARG_END);
                            printf ("Inserted call1 to CallToSetIntRegsToZero after instruction at %lx\n",
                            		(unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
#endif
                        else if (testNum == 9)
                        {
                            REGSET regsNone;
                            REGSET_Clear(regsNone);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToSetIntRegsToZero), IARG_PRESERVE,
                                           &regsNone, IARG_END);
                            printf("Inserted call2 to CallToSetIntRegsToZero after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
                        else if (testNum == 10)
                        {
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToSetIntRegsToZero), IARG_END);
                            printf("Inserted call3 to CallToSetIntRegsToZero after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
                        else if (testNum == 11)
                        {
                            REGSET regsNone;
                            REGSET_Clear(regsNone);
                            REGSET_Insert(regsNone, REG_GAX);
                            REGSET_Insert(regsNone, REG_GCX);
                            REGSET_Insert(regsNone, REG_GDX);
#ifdef TARGET_IA32E
                            REGSET_Insert(regsNone, REG_R8);
                            REGSET_Insert(regsNone, REG_R9);
                            REGSET_Insert(regsNone, REG_R10);
                            REGSET_Insert(regsNone, REG_R11);
#if defined(__GNUC__)
                            REGSET_Insert(regsNone, REG_GSI);
                            REGSET_Insert(regsNone, REG_GDI);
#endif
#endif
                            REGSET_Remove(regsNone, REG_GAX);
                            REGSET_Remove(regsNone, REG_GCX);
                            REGSET_Remove(regsNone, REG_GDX);
#ifdef TARGET_IA32E
                            REGSET_Remove(regsNone, REG_R8);
                            REGSET_Remove(regsNone, REG_R9);
                            REGSET_Remove(regsNone, REG_R10);
                            REGSET_Remove(regsNone, REG_R11);
#if defined(__GNUC__)
                            REGSET_Remove(regsNone, REG_GSI);
                            REGSET_Remove(regsNone, REG_GDI);
#endif
#endif
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToSetIntRegsToZero), IARG_PRESERVE,
                                           &regsNone, IARG_END);
                            printf("Inserted call4 to CallToSetIntRegsToZero after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
                        else if (testNum == 12)
                        {
                            REGSET regsFP;
                            REGSET_Clear(regsFP);
                            REGSET_Insert(regsFP, REG_X87);
                            REGSET_Insert(regsFP, REG_MXCSR);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToUnMaskZeroDivideInMxcsr),
                                           IARG_PRESERVE, &regsFP, IARG_END);
                            printf("Inserted call1 to UnMaskZeroDivideInMxcsr after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
                        else if (testNum == 13)
                        {
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToUnMaskZeroDivideInMxcsr),
                                           IARG_END);
                            printf("Inserted call2 to UnMaskZeroDivideInMxcsr after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
                        else if (testNum == 14)
                        {
                            REGSET regsFP;
                            REGSET_Clear(regsFP);
                            REGSET_Insert(regsFP, REG_X87);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToUnMaskZeroDivideInMxcsr),
                                           IARG_END);
                            printf("Inserted call3 to UnMaskZeroDivideInMxcsr after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                        }
                        else if (testNum == 15)
                        {
                            REGSET regsNone;
                            REGSET_Clear(regsNone);
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToFldzToTop3), IARG_PRESERVE,
                                           &regsNone, IARG_END);
                            printf("Inserted call5 to FldzToTop3 after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            testNum++;
                            printf("testNum %d\n", testNum);
                        }
                        return;
                    }
                }
            }
        }
    }
}

#if defined(__cplusplus)
extern "C"
#endif
    VOID
    TestIargPreserveReplacement()
{
    testNum++;
    printf("TestIargPreserveReplacement testNum %d\n", testNum);
    FldzToTop3();
}

#if defined(__cplusplus)
extern "C"
#endif
    VOID
    TestIargPreserveReplacement1()
{
    testNum++;
    printf("TestIargPreserveReplacement1 testNum %d\n", testNum);
    FldzToTop3();
}

#if defined(__cplusplus)
extern "C"
#endif
    VOID
    TestIargPreserveReplacement2()
{
    testNum++;
    printf("TestIargPreserveReplacement2 testNum %d\n", testNum);
    FldzToTop3();
    SetXmmRegsToZero();
}

#if defined(__cplusplus)
extern "C"
#endif
    VOID
    TestIargPreserveProbedBefore()
{
    testNum++;
    FldzToTop3();
}

#if defined(__cplusplus)
extern "C"
#endif
    VOID
    TestIargPreserveProbedBefore1()
{
    testNum++;
    FldzToTop3();
}

VOID ImageLoad(IMG img, VOID* v)
{
    RTN rtn = RTN_FindByName(img, "TestIargPreserveInReplacement");

    if (RTN_Valid(rtn))
    {
        printf("found TestIargPreserveInReplacement\n");
        fflush(stdout);
        PROTO proto = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "TestIargPreserveInReplacementProto", PIN_PARG_END());
        REGSET regsFP;
        REGSET_Clear(regsFP);
        REGSET_Insert(regsFP, REG_X87);
        RTN_ReplaceSignature(rtn, AFUNPTR(TestIargPreserveReplacement), IARG_PROTOTYPE, proto, IARG_CONTEXT, IARG_ORIG_FUNCPTR,
                             IARG_PRESERVE, &regsFP, IARG_END);
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

    rtn = RTN_FindByName(img, "TestIargPreserveInReplacement2");

    if (RTN_Valid(rtn))
    {
        printf("found TestIargPreserveInReplacement2\n");
        fflush(stdout);
        PROTO proto = PROTO_Allocate(PIN_PARG(void), CALLINGSTD_DEFAULT, "TestIargPreserveInReplacementProto", PIN_PARG_END());

        RTN_ReplaceSignature(rtn, AFUNPTR(TestIargPreserveReplacement2), IARG_PROTOTYPE, proto, IARG_CONTEXT, IARG_ORIG_FUNCPTR,
                             IARG_END);
        PROTO_Free(proto);
    }
}

VOID Fini(INT32 code, VOID* v)
{
    if (testNum != NUM_TESTS)
    {
        printf("***Error not all expected tests ran testNum %d\n", testNum);
        exit(-1);
    }
    printf("all expected tests ran testNum %d\n", testNum);
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
