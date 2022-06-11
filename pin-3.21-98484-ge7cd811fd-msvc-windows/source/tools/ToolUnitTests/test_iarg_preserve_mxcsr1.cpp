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

#if defined(__cplusplus)
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

extern "C" BOOL ProcessorSupportsAvx();
EXTERN_C VOID FldzToTop3();
EXTERN_C VOID SetXmmRegsToZero();
EXTERN_C VOID SetIntRegsToZero();
EXTERN_C void UnMaskZeroDivideInMxcsr();
EXTERN_C void MaskZeroDivideInMxcsr();

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

VOID CallToFldzToTop3() { FldzToTop3(); }

VOID CallToSetXmmRegsToZero() { SetXmmRegsToZero(); }

VOID CallToSetIntRegsToZero() { SetIntRegsToZero(); }

VOID CallToUnMaskZeroDivideInMxcsr() { UnMaskZeroDivideInMxcsr(); }

VOID CallToMaskZeroDivideInMxcsr() { MaskZeroDivideInMxcsr(); }

#define NUM_TESTS 1
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
                    INS_InsertCall(INS_Next(ins), IPOINT_AFTER, AFUNPTR(CallToMaskZeroDivideInMxcsr), IARG_END);
                    xed_iclass_enum_t iclass3 = static_cast< xed_iclass_enum_t >(INS_Opcode(INS_Next(INS_Next(ins))));
                    if (iclass3 == XED_ICLASS_FLD1)
                    {
                        printf("found fld1 sequence at %lx\n", (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                        if (testNum == 0)
                        {
                            INS_InsertCall(INS_Next(INS_Next(ins)), IPOINT_AFTER, AFUNPTR(CallToUnMaskZeroDivideInMxcsr),
                                           IARG_END);
                            printf("Inserted call1 to UnMaskZeroDivideInMxcsr after instruction at %lx\n",
                                   (unsigned long)(INS_Address(INS_Next(INS_Next(ins)))));
                            printf("Inserted call1 to MaskZeroDivideInMxcsr after instruction at %lx\n",
                                   (unsigned long)(INS_Address((INS_Next(ins)))));
                            testNum++;
                        }
                        return;
                    }
                }
            }
        }
    }
}

EXTERN_C VOID TestIargPreserveReplacement()
{
    testNum++;
    FldzToTop3();
}

EXTERN_C VOID TestIargPreserveReplacement1()
{
    testNum++;
    FldzToTop3();
}

EXTERN_C VOID TestIargPreserveProbedBefore()
{
    testNum++;
    FldzToTop3();
}

EXTERN_C VOID TestIargPreserveProbedBefore1()
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
