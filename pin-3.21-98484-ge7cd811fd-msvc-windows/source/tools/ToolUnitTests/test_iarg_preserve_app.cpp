/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#if defined(TARGET_WINDOWS)
#include "windows.h"
#define EXPORT_CSYM extern "C" __declspec(dllexport)
#else
#define EXPORT_CSYM extern "C"
#endif

#if defined(__cplusplus)
extern "C"
#endif
    double var1 = 2.0;
#if defined(__cplusplus)
extern "C"
#endif
    double var2 = 2.0;
#if defined(__cplusplus)
extern "C"
#endif
    double var3 = 2.0;

#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1_x();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fstp_x();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1Fstp_q(void (*pt2Function)());
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1Fstp_n1(void (*pt2Function)());
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1Fstp_a();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1Fstp_b();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1Fstp_c();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1Fstp_d();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1Fstp_e();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1Fstp_f();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1Fstp_g();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1Fstp_h();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1Fstp_i();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1Fstp_j();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1Fstp_k();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1Fstp_l();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1Fstp_m(void (*pt2Function)());
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1Fstp_n(void (*pt2Function)());
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1FstpSetXmmsMaskMxcsr();

#if defined(__cplusplus)
extern "C"
#endif
    void
    IntScratchFld1Fstp_a();

#if defined(__cplusplus)
extern "C"
#endif
    void
    IntScratchFld1Fstp_b();

#if defined(__cplusplus)
extern "C"
#endif
    void
    IntScratchFld1Fstp_c();

#if defined(__cplusplus)
extern "C"
#endif
    void
    IntScratchFld1Fstp_d();

#if defined(__cplusplus)
extern "C"
#endif
    void
    MaskZeroDivideInMxcsr();

#if defined(__cplusplus)
extern "C"
#endif
    void
    UnMaskZeroDivideInMxcsr();

#if defined(__GNUC__)

#include <stdint.h>
typedef uint8_t UINT8; //LINUX HOSTS
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef int8_t INT8;
typedef int16_t INT16;
typedef int32_t INT32;
typedef int64_t INT64;

#define ALIGN16 __attribute__((aligned(16)))
#define ALIGN8 __attribute__((aligned(8)))

#elif defined(_MSC_VER)

typedef unsigned __int8 UINT8;
typedef unsigned __int16 UINT16;
typedef unsigned __int32 UINT32;
typedef unsigned __int64 UINT64;

#define ALIGN16 __declspec(align(16))
#define ALIGN8 __declspec(align(8))

#else
#error Expect usage of either GNU or MS compiler.
#endif

#if defined(__cplusplus)
extern "C"
#endif
    UINT64
    GetMxcsr();

#define MAX_BYTES_PER_XMM_REG 16
#define MAX_WORDS_PER_XMM_REG (MAX_BYTES_PER_XMM_REG / 2)
#define MAX_DWORDS_PER_XMM_REG (MAX_WORDS_PER_XMM_REG / 2)
#define MAX_QWORDS_PER_XMM_REG (MAX_DWORDS_PER_XMM_REG / 2)
#define MAX_FLOATS_PER_XMM_REG (MAX_BYTES_PER_XMM_REG / sizeof(float))
#define MAX_DOUBLES_PER_XMM_REG (MAX_BYTES_PER_XMM_REG / sizeof(double))

extern "C" ALIGN16 UINT64 vals[] = {1, 0, 2,  0, 3,  0, 4,  0, 5,  0, 6,  0, 7,  0, 8,  0,
                                    9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 15, 0, 16, 0};

extern "C" UINT64* val1 = &vals[0];

extern "C" UINT64* val2 = &vals[2];

extern "C" UINT64* val3 = &vals[4];

extern "C" UINT64* val4 = &vals[6];

extern "C" UINT64* val5 = &vals[8];

extern "C" UINT64* val6 = &vals[10];

extern "C" UINT64* val7 = &vals[12];

extern "C" UINT64* val8 = &vals[14];

extern "C" UINT64* val9 = &vals[16];

extern "C" UINT64* val10 = &vals[18];

extern "C" UINT64* val11 = &vals[20];

extern "C" UINT64* val12 = &vals[22];

extern "C" UINT64* val13 = &vals[24];

extern "C" UINT64* val14 = &vals[26];

extern "C" UINT64* val15 = &vals[28];

extern "C" UINT64* val16 = &vals[30];

#if defined(TARGET_IA32)
typedef UINT32 ADDRINT;
#else
typedef UINT64 ADDRINT;
#endif

extern "C" ADDRINT setFlagsX = 0;

union ALIGN16 xmm_reg_t
{
    UINT8 byte[MAX_BYTES_PER_XMM_REG];
    UINT16 word[MAX_WORDS_PER_XMM_REG];
    UINT32 dword[MAX_DWORDS_PER_XMM_REG];
    UINT64 qword[MAX_QWORDS_PER_XMM_REG];

    INT8 s_byte[MAX_BYTES_PER_XMM_REG];
    INT16 s_word[MAX_WORDS_PER_XMM_REG];
    INT32 s_dword[MAX_DWORDS_PER_XMM_REG];
    INT64 s_qword[MAX_QWORDS_PER_XMM_REG];

    float flt[MAX_FLOATS_PER_XMM_REG];
    double dbl[MAX_DOUBLES_PER_XMM_REG];
};

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

extern "C" reg_t get_gax();
extern "C" reg_t get_gcx();
extern "C" reg_t get_gdx();
extern "C" void set_gax(reg_t regVal);
extern "C" void set_gcx(reg_t regVal);
extern "C" void set_gdx(reg_t regVal);
#if defined(__GNUC__)
#include <stdint.h>
static void get_xmm_regA(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm0,%0" : "=m"(xmm_reg)); }

static void set_xmm_reg0(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm0" : : "m"(xmm_reg) : "%xmm0"); }

static void get_xmm_reg0(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm0,%0" : "=m"(xmm_reg)); }

static void set_xmm_reg1(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm1" : : "m"(xmm_reg) : "%xmm1"); }

static void get_xmm_reg1(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm1, %0" : "=m"(xmm_reg)); }

static void set_xmm_reg2(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm2" : : "m"(xmm_reg) : "%xmm2"); }

static void get_xmm_reg2(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm2,%0" : "=m"(xmm_reg)); }

static void set_xmm_reg3(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm3" : : "m"(xmm_reg) : "%xmm3"); }

static void get_xmm_reg3(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm3,%0" : "=m"(xmm_reg)); }

static void set_xmm_reg4(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm4" : : "m"(xmm_reg) : "%xmm4"); }

static void get_xmm_reg4(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm4,%0" : "=m"(xmm_reg)); }

static void set_xmm_reg5(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm5" : : "m"(xmm_reg) : "%xmm5"); }

static void get_xmm_reg5(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm5,%0" : "=m"(xmm_reg)); }

static void set_xmm_reg6(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm6" : : "m"(xmm_reg) : "%xmm6"); }

static void get_xmm_reg6(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm6,%0" : "=m"(xmm_reg)); }

static void set_xmm_reg7(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm7" : : "m"(xmm_reg) : "%xmm7"); }

static void get_xmm_reg7(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm7,%0" : "=m"(xmm_reg)); }
#ifdef TARGET_IA32E
static void get_xmm_reg8(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm8,%0" : "=m"(xmm_reg)); }

static void get_xmm_reg9(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm9,%0" : "=m"(xmm_reg)); }

static void get_xmm_reg10(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm10,%0" : "=m"(xmm_reg)); }

static void get_xmm_reg11(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm11,%0" : "=m"(xmm_reg)); }

static void get_xmm_reg12(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm12,%0" : "=m"(xmm_reg)); }

static void get_xmm_reg13(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm13,%0" : "=m"(xmm_reg)); }

static void get_xmm_reg14(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm14,%0" : "=m"(xmm_reg)); }

static void get_xmm_reg15(xmm_reg_t& xmm_reg) { asm volatile("movdqu %%xmm15,%0" : "=m"(xmm_reg)); }

static void set_xmm_reg8(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm8" : : "m"(xmm_reg) : "%xmm8"); }

static void set_xmm_reg9(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm9" : : "m"(xmm_reg) : "%xmm9"); }

static void set_xmm_reg10(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm10" : : "m"(xmm_reg) : "%xmm10"); }

static void set_xmm_reg11(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm11" : : "m"(xmm_reg) : "%xmm11"); }

static void set_xmm_reg12(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm12" : : "m"(xmm_reg) : "%xmm12"); }

static void set_xmm_reg13(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm13" : : "m"(xmm_reg) : "%xmm13"); }

static void set_xmm_reg14(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm14" : : "m"(xmm_reg) : "%xmm14"); }

static void set_xmm_reg15(xmm_reg_t& xmm_reg) { asm volatile("movdqu %0, %%xmm15" : : "m"(xmm_reg) : "%xmm15"); }

extern "C" reg_t get_r12();
extern "C" reg_t get_r13();
extern "C" reg_t get_r14();
extern "C" reg_t get_r15();
#endif

#else
extern "C" void set_xmm_reg0(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg0(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg1(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg1(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg2(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg2(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg3(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg3(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg4(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg4(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg5(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg5(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg6(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg6(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg7(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg7(xmm_reg_t& xmm_reg);
#ifdef TARGET_IA32E
extern "C" void set_xmm_reg8(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg8(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg9(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg9(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg10(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg10(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg11(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg11(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg12(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg12(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg13(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg13(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg14(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg14(xmm_reg_t& xmm_reg);
extern "C" void set_xmm_reg15(xmm_reg_t& xmm_reg);
extern "C" void get_xmm_reg15(xmm_reg_t& xmm_reg);

#endif
#endif
#ifdef TARGET_IA32E
extern "C" reg_t get_gdi();
extern "C" reg_t get_gsi();
extern "C" reg_t get_r8();
extern "C" reg_t get_r9();
extern "C" reg_t get_r10();
extern "C" reg_t get_r11();
extern "C" void set_gdi(reg_t regVal);
extern "C" void set_gsi(reg_t regVal);
extern "C" void set_r8(reg_t regVal);
extern "C" void set_r9(reg_t regVal);
extern "C" void set_r10(reg_t regVal);
extern "C" void set_r11(reg_t regVal);
#endif

static void set_xmm_reg(xmm_reg_t& xmm_reg, UINT32 reg_no)
{
    switch (reg_no)
    {
        case 0:
            set_xmm_reg0(xmm_reg);
            break;
        case 1:
            set_xmm_reg1(xmm_reg);
            break;
        case 2:
            set_xmm_reg2(xmm_reg);
            break;
        case 3:
            set_xmm_reg3(xmm_reg);
            break;
        case 4:
            set_xmm_reg4(xmm_reg);
            break;
        case 5:
            set_xmm_reg5(xmm_reg);
            break;
        case 6:
            set_xmm_reg6(xmm_reg);
            break;
        case 7:
            set_xmm_reg7(xmm_reg);
            break;
#ifdef TARGET_IA32E
        case 8:
            set_xmm_reg8(xmm_reg);
            break;
        case 9:
            set_xmm_reg9(xmm_reg);
            break;
        case 10:
            set_xmm_reg10(xmm_reg);
            break;
        case 11:
            set_xmm_reg11(xmm_reg);
            break;
        case 12:
            set_xmm_reg12(xmm_reg);
            break;
        case 13:
            set_xmm_reg13(xmm_reg);
            break;
        case 14:
            set_xmm_reg14(xmm_reg);
            break;
        case 15:
            set_xmm_reg15(xmm_reg);
            break;
#endif
    }
}
static void get_xmm_reg(xmm_reg_t& xmm_reg, UINT32 reg_no)
{
    switch (reg_no)
    {
        case 0:
            get_xmm_reg0(xmm_reg);
            break;
        case 1:
            get_xmm_reg1(xmm_reg);
            break;
        case 2:
            get_xmm_reg2(xmm_reg);
            break;
        case 3:
            get_xmm_reg3(xmm_reg);
            break;
        case 4:
            get_xmm_reg4(xmm_reg);
            break;
        case 5:
            get_xmm_reg5(xmm_reg);
            break;
        case 6:
            get_xmm_reg6(xmm_reg);
            break;
        case 7:
            get_xmm_reg7(xmm_reg);
            break;
#ifdef TARGET_IA32E
        case 8:
            get_xmm_reg8(xmm_reg);
            break;
        case 9:
            get_xmm_reg9(xmm_reg);
            break;
        case 10:
            get_xmm_reg10(xmm_reg);
            break;
        case 11:
            get_xmm_reg11(xmm_reg);
            break;
        case 12:
            get_xmm_reg12(xmm_reg);
            break;
        case 13:
            get_xmm_reg13(xmm_reg);
            break;
        case 14:
            get_xmm_reg14(xmm_reg);
            break;
        case 15:
            get_xmm_reg15(xmm_reg);
            break;
#endif
    }
}
void write_xmm_reg(UINT32 reg_no, UINT32 val)
{
    xmm_reg_t xmm;
    xmm.dword[0] = val;
    xmm.dword[1] = val;
    xmm.dword[2] = val;
    xmm.dword[3] = val;

    set_xmm_reg(xmm, reg_no);
}

void read_xmm_reg(UINT32 reg_no, xmm_reg_t& xmm)
{
    xmm.dword[0] = 0;
    xmm.dword[1] = 0;
    xmm.dword[2] = 0;
    xmm.dword[3] = 0;

    get_xmm_reg(xmm, reg_no);
}

EXPORT_CSYM void TestIargPreserveInReplacement() {}

EXPORT_CSYM void TestIargPreserveInReplacement1() {}

EXPORT_CSYM void TestIargPreserveInReplacement2() {}

EXPORT_CSYM void TestIargPreserveInProbed() {}

EXPORT_CSYM void TestIargPreserveInProbed1() {}

EXPORT_CSYM void TestIargPreserveInProbed2() {}

int main()
{
    /*
	 * IARG_PRESERVE is Used to specify registers whose values will be the same upon return from the analysis routine, and should be
	 * used to specify caller-saved registers which are preserved by the analysis routine.
	 * When a non-inlinable analysis routine preserves caller-saved registers, Pin can avoid generating code to preserve these registers.
	 *
	 * The fact that the analysis routine changed registers which were stated not to be changed doesn't effect the register
	 * mapping at that point.Meaning the analysis routine can change XMM3 register value although it stated it wouldn't do that,
	 * but the mapping at that point (before and after the analysis) can be that virtual xmm3 is mapped to the spill area.
	 * So when the application will use xmm3 it will use the the value taken from the spill area and not from the xmm3 register
	 * that was modified by the tool. See JIRA PINT-5011.
	 */

    var1 = 2.0;
    var2 = 2.0;
    var3 = 2.0;

    // the test_iarg_preserve tool will insert a call to an analysis routine
    // after the third fld1 in Fld1Fstp that sets the first 3 x87 regs to 0
    Fld1Fstp_a();

    // make sure the values were changed by the tool test_iarg_preserve - since that tool
    // specified that it's analysis routine preserved the x87 regs when it doesn't. Note - some registers
    // may hold the previous value, see IARG_PRESERVE note at the beginning of this function.
    if ((var1 != 0.0 && var1 != 2.0) || (var2 != 0.0 && var2 != 2.0) || (var3 != 0.0 && var3 != 2.0))
    {
        printf("***Error in x87 values_1 : var1 = %f, var2 = %f, var3 = %f\n", var1, var2, var3);
        return (-1);
    }

    var1 = 2.0;
    var2 = 2.0;
    var3 = 2.0;

    // the test_iarg_preserve tool will insert a call to an analysis routine
    // after the third fld1 in Fld1Fstp that sets the first 3 x87 regs to 0
    Fld1Fstp_b();

    // make sure the values were NOT changed by the tool test_iarg_preserve - since that tool
    // specified that it's analysis routine does not preserve the x87 regs. Note - some registers
    // may hold the previous value, see IARG_PRESERVE note at the beginning of this function.
    if ((var1 != 1.0 && var1 != 2.0) || (var2 != 1.0 && var2 != 2.0) || (var3 != 1.0 && var3 != 2.0))
    {
        printf("***Error in x87 values_2 : var1 = %f, var2 = %f, var3 = %f\n", var1, var2, var3);
        return (-1);
    }

    var1 = 2.0;
    var2 = 2.0;
    var3 = 2.0;

    // the test_iarg_preserve tool will insert a call to an analysis routine
    // after the third fld1 in Fld1Fstp that sets the first 3 x87 regs to 0
    Fld1Fstp_c();

    // make sure the values were NOT changed by the tool test_iarg_preserve - since that tool
    // did not specify any IARG_PRESERVE and Pin saves the x87 state by default
    if (var1 != 1.0 || var2 != 1.0 || var3 != 1.0)
    {
        printf("***Error in x87 values_3 : var1 = %f, var2 = %f, var3 = %f\n", var1, var2, var3);
        return (-1);
    }

    var1 = 2.0;
    var2 = 2.0;
    var3 = 2.0;

    // the test_iarg_preserve tool will insert a call to an analysis routine
    // after the third fld1 in Fld1Fstp that sets the first 3 x87 regs to 0
    Fld1Fstp_d();

    // make sure the values were NOT changed by the tool test_iarg_preserve - since that tool
    // used IARG_PRESERVE to specify that no regs are preserved at Pin saves the x87 state by default.
    // Note - some registers may hold the previous value, see IARG_PRESERVE note at the beginning of this function.
    if ((var1 != 1.0 && var1 != 2.0) || (var2 != 1.0 && var2 != 2.0) || (var3 != 1.0 && var3 != 2.0))
    {
        printf("***Error in x87 values_4 : var1 = %f, var2 = %f, var3 = %f\n", var1, var2, var3);
        return (-1);
    }

    xmm_reg_t xmm_regs[NUM_XMM_SCRATCH_REGS];
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        xmm_regs[i].dword[0] = xmm_regs[i].dword[1] = xmm_regs[i].dword[2] = xmm_regs[i].dword[3] = 0;
    }
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        write_xmm_reg(i, 0xdeadbeef);
    }
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        read_xmm_reg(i, xmm_regs[i]);
    }
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        if (!(xmm_regs[i].dword[0] == 0xdeadbeef && xmm_regs[i].dword[1] == 0xdeadbeef && xmm_regs[i].dword[2] == 0xdeadbeef &&
              xmm_regs[i].dword[3] == 0xdeadbeef))
        {
            printf("***Error in initializing xmm regs\n");
            return (-1);
        }
    }
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        write_xmm_reg(i, 0xdeadbeef);
    }

    // the test_iarg_preserve tool will insert a call to an analysis routine
    // after the third fld1 in Fld1Fstp that sets the xmmregs to 0
    Fld1Fstp_e();

    // make sure the values were changed by the tool test_iarg_preserve - since that tool
    // specified that it's analysis routine preserved the xmm regs when it doesn't. Note - some registers
    // may hold the previous value, see IARG_PRESERVE note at the beginning of this function.
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        read_xmm_reg(i, xmm_regs[i]);
    }
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        if (!(xmm_regs[i].dword[0] == 0 || xmm_regs[i].dword[0] == 0xdeadbeef) &&
            !(xmm_regs[i].dword[1] == 0 || xmm_regs[i].dword[1] == 0xdeadbeef) &&
            !(xmm_regs[i].dword[2] == 0 || xmm_regs[i].dword[2] == 0xdeadbeef) &&
            !(xmm_regs[i].dword[3] == 0 || xmm_regs[i].dword[3] == 0xdeadbeef))
        {
            printf("***Error in xmm regs values_1 xmm%d\n", i);
            return (-1);
        }
    }

    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        write_xmm_reg(i, 0xdeadbeef);
    }

    // the test_iarg_preserve tool will insert a call to an analysis routine
    // after the third fld1 in Fld1Fstp that sets the xmmregs to 0
    Fld1Fstp_f();

    // make sure the values were NOT changed by the tool test_iarg_preserve - since that tool
    // specified that it's analysis routine does not preserve the x87 regs. Note - some registers
    // may hold the new value, see IARG_PRESERVE note at the beginning of this function.
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        read_xmm_reg(i, xmm_regs[i]);
    }
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        if (!(xmm_regs[i].dword[0] == 0 || xmm_regs[i].dword[0] == 0xdeadbeef) &&
            !(xmm_regs[i].dword[1] == 0 || xmm_regs[i].dword[1] == 0xdeadbeef) &&
            !(xmm_regs[i].dword[2] == 0 || xmm_regs[i].dword[2] == 0xdeadbeef) &&
            !(xmm_regs[i].dword[3] == 0 || xmm_regs[i].dword[3] == 0xdeadbeef))
        {
            printf("***Error in xmm regs values_2 xmm%d\n", i);
            return (-1);
        }
    }

    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        write_xmm_reg(i, 0xdeadbeef);
    }

    // the test_iarg_preserve tool will insert a call to an analysis routine
    // after the third fld1 in Fld1Fstp that sets the xmmregs to 0
    Fld1Fstp_g();

    // make sure the values were NOT changed by the tool test_iarg_preserve - since that tool
    // did not specify any IARG_PRESERVE at Pin saves the xmm regs
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        read_xmm_reg(i, xmm_regs[i]);
    }
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        if (!(xmm_regs[i].dword[0] == 0xdeadbeef && xmm_regs[i].dword[1] == 0xdeadbeef && xmm_regs[i].dword[2] == 0xdeadbeef &&
              xmm_regs[i].dword[3] == 0xdeadbeef))
        {
            printf("***Error in xmm regs values_3 xmm%d\n", i);
            return (-1);
        }
    }

    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        write_xmm_reg(i, 0xdeadbeef);
    }

    // the test_iarg_preserve tool will insert a call to an analysis routine
    // after the third fld1 in Fld1Fstp that sets the xmmregs to 0
    Fld1Fstp_h();

    // make sure the values were NOT changed by the tool test_iarg_preserve - since that tool
    // did not specify any IARG_PRESERVE at Pin saves the xmm regs. Note - some registers
    // may hold the new value, see IARG_PRESERVE note at the beginning of this function.
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        read_xmm_reg(i, xmm_regs[i]);
    }
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        if (!(xmm_regs[i].dword[0] == 0 || xmm_regs[i].dword[0] == 0xdeadbeef) &&
            !(xmm_regs[i].dword[1] == 0 || xmm_regs[i].dword[1] == 0xdeadbeef) &&
            !(xmm_regs[i].dword[2] == 0 || xmm_regs[i].dword[2] == 0xdeadbeef) &&
            !(xmm_regs[i].dword[3] == 0 || xmm_regs[i].dword[3] == 0xdeadbeef))
        {
            printf("***Error in xmm regs values_4 xmm%d\n", i);
            return (-1);
        }
    }

    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        write_xmm_reg(i, 0xdeadbeef);
    }

    // the test_iarg_preserve tool will insert a call to an analysis routine
    // after the third fld1 in Fld1Fstp that sets the xmmregs to 0
    Fld1Fstp_i();

    // make sure the values were NOT changed by the tool test_iarg_preserve - since that tool
    // did not specify any IARG_PRESERVE. Note - some registers may hold the new value,
    // see IARG_PRESERVE note at the beginning of this function.
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        read_xmm_reg(i, xmm_regs[i]);
    }
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        if (!(xmm_regs[i].dword[0] == 0 || xmm_regs[i].dword[0] == 0xdeadbeef) &&
            !(xmm_regs[i].dword[1] == 0 || xmm_regs[i].dword[1] == 0xdeadbeef) &&
            !(xmm_regs[i].dword[2] == 0 || xmm_regs[i].dword[2] == 0xdeadbeef) &&
            !(xmm_regs[i].dword[3] == 0 || xmm_regs[i].dword[3] == 0xdeadbeef))
        {
            printf("***Error in xmm regs values_5 xmm%d\n", i);
            return (-1);
        }
    }

    reg_t regGax, regGcx, regGdx, regGsi, regGdi, regR8, regR9, regR10, regR11;
    reg_t savedGax, savedGcx, savedGdx, savedGsi, savedGdi, savedR8, savedR9, savedR10, savedR11;

#if 0
    // this test not run - because overwriting the integer registers can cause Pin to fail
    savedGax = get_gax(); // must be first
    savedGcx = get_gcx();
    savedGdx = get_gdx();
#ifdef TARGET_IA32E
    savedR8 = get_r8();
    savedR9 = get_r9();
    savedR10 = get_r10();
    savedR11 = get_r11();
#if defined(__GNUC__)
    savedGdi = get_gdi();
    savedGsi = get_gsi();
#endif
#endif

    // the test_iarg_preserve tool will insert a call to an analysis routine
    // after the third fld1 in IntScratchFld1Fstp_a that sets the scratch registers to 0
    IntScratchFld1Fstp_a();


    // make sure values were changed by the tool test_iarg_preserve - since that tool
    // specified that it's analysis routine preserved the integer scratch regs when it doesn't
    // NOTE - due to Pin's register allocation, it may be that some of the scratch registers
    // are NOT changed - because they were in the spill area, this is OK. We assume that at
    // least one of the scratch registers will not be in the spill area
    regGax = get_gax(); // must be first
    regGcx = get_gcx();
    regGdx = get_gdx();

#ifdef TARGET_IA32E
    regR8 = get_r8();
    regR9 = get_r9();
    regR10 = get_r10();
    regR11 = get_r11();
#if defined(__GNUC__)
    regGdi = get_gdi();
    regGsi = get_gsi();
#endif
#endif

    // reset the values to what they were
    set_gax(savedGax);
#if defined(__GNUC__)
    set_gcx(savedGcx);
#endif
    set_gdx(savedGdx);
#ifdef TARGET_IA32E
    set_r8(savedR8);
    set_r9(savedR9);
    set_r10(savedR10);
    set_r11(savedR11);
#if defined(__GNUC__)
    set_gdi(savedGdi);
    set_gsi(savedGsi);
#endif
#endif
#if !defined(__GNUC__)
    set_gcx(savedGcx);
#endif

    if (!(regGax==0 || regGcx==0 || regGdx==0
#ifdef TARGET_IA32E
           || regR8==0 || regR9==0 || regR10==0 || regR11==0
#if defined(__GNUC__)
          || regGdi==0 || regGsi==0
#endif
#endif
       ))
    {
        printf ("***Error in int scratch regs values\n");
        printf ("***Even though test will fail on this, it may not be a true failure - see the above comment in the test\n");
        return (-1);
    }

#endif

    // the test_iarg_preserve tool will insert a call to an analysis routine
    // after the third fld1 in IntScratchFld1Fstp_a that sets the scratch registers to 0
    IntScratchFld1Fstp_b();

    // make sure the values were NOT changed by the tool test_iarg_preserve - since that tool
    // specified that it's analysis routine does not preserve the integer scratch regs
    regGax = get_gax(); // must be first
    regGcx = get_gcx();
    regGdx = get_gdx();
    if (!(regGax == 0xdeadbeef && regGcx == 0xdeadbeef && regGdx == 0xdeadbeef))
    {
        printf("***Error in int scratch regs values_2\n");
        return (-1);
    }
#ifdef TARGET_IA32E
    regR8  = get_r8();
    regR9  = get_r9();
    regR10 = get_r10();
    regR11 = get_r11();
    if (!(regR8 == 0xdeadbeef && regR9 == 0xdeadbeef && regR10 == 0xdeadbeef && regR11 == 0xdeadbeef))
    {
        printf("***Error in int scratch regs values_2_ia32e regR8 %p regR9 %p regR10 %p regR11 %p\n",
               reinterpret_cast< void* >(regR8), reinterpret_cast< void* >(regR9), reinterpret_cast< void* >(regR10),
               reinterpret_cast< void* >(regR11));
        return (-1);
    }
#if defined(__GNUC__)
    regGdi = get_gdi();
    regGsi = get_gsi();
    if (!(regGdi == 0xdeadbeef && regGsi == 0xdeadbeef))
    {
        printf("***Error in int scratch regs values_2_ia32e linux\n");
        return (-1);
    }
#endif
#endif

    // the test_iarg_preserve tool will insert a call to an analysis routine
    // after the third fld1 in IntScratchFld1Fstp_a that sets the scratch registers to 0
    IntScratchFld1Fstp_c();

    // make sure the values were NOT changed by the tool test_iarg_preserve - since that tool
    // did not specify any IARG_PRESERVE
    regGax = get_gax(); // must be first
    regGcx = get_gcx();
    regGdx = get_gdx();
    if (!(regGax == 0xdeadbeef && regGcx == 0xdeadbeef && regGdx == 0xdeadbeef))
    {
        printf("***Error in int scratch regs values_3\n");
        return (-1);
    }
#ifdef TARGET_IA32E
    regR8  = get_r8();
    regR9  = get_r9();
    regR10 = get_r10();
    regR11 = get_r11();
    if (!(regR8 == 0xdeadbeef && regR9 == 0xdeadbeef && regR10 == 0xdeadbeef && regR11 == 0xdeadbeef))
    {
        printf("***Error in int scratch regs values_3_ia32e\n");
        return (-1);
    }
#if defined(__GNUC__)
    regGdi = get_gdi();
    regGsi = get_gsi();
    if (!(regGdi == 0xdeadbeef && regGsi == 0xdeadbeef))
    {
        printf("***Error in int scratch regs values_3_ia32e linux\n");
        return (-1);
    }
#endif
#endif

    // the test_iarg_preserve tool will insert a call to an analysis routine
    // after the third fld1 in IntScratchFld1Fstp_a that sets the scratch registers to 0
    IntScratchFld1Fstp_d();

    // make sure the values were NOT changed by the tool test_iarg_preserve - since that tool
    // did not specify any IARG_PRESERVE
    regGax = get_gax(); // must be first
    regGcx = get_gcx();
    regGdx = get_gdx();

    if (!(regGax == 0xdeadbeef && regGcx == 0xdeadbeef && regGdx == 0xdeadbeef))
    {
        printf("***Error in int scratch regs values_4\n");
        return (-1);
    }

#ifdef TARGET_IA32E
    regR8  = get_r8();
    regR9  = get_r9();
    regR10 = get_r10();
    regR11 = get_r11();
    if (!(regR8 == 0xdeadbeef && regR9 == 0xdeadbeef && regR10 == 0xdeadbeef && regR11 == 0xdeadbeef))
    {
        printf("***Error in int scratch regs values_4_ia32e\n");
        return (-1);
    }
#if defined(__GNUC__)
    regGdi = get_gdi();
    regGsi = get_gsi();
    if (!(regGdi == 0xdeadbeef && regGsi == 0xdeadbeef))
    {
        printf("***Error in int scratch regs values_4_ia32e linux\n");
        return (-1);
    }
#endif
#endif
    reg_t curMxcsr;
    UnMaskZeroDivideInMxcsr();
    curMxcsr = GetMxcsr();
    if ((curMxcsr & 0x200))
    {
        printf("***Error0 in UNmasking the divide by zero exception mask in mxcsr\n");
        return (-1);
    }
    MaskZeroDivideInMxcsr();
    curMxcsr = GetMxcsr();
    if (!(curMxcsr & 0x200))
    {
        printf("***Error1 in masking the divide by zero exception mask in mxcsr\n");
        return (-1);
    }
    // the test_iarg_preserve tool will insert a call to an analysis routine
    // after the third fld1 in Fld1Fstp that unmasks the divide by zero exception mask in mxcsr
    Fld1Fstp_j();

    // make sure the  divide by zero exception mask in mxcsr is not masked - since that tool
    // specified IARG_PRESERVE of the mxcsr mask when it really unmasked the the divide by zero exception mask in mxcsr
    curMxcsr = GetMxcsr();
    if (curMxcsr & 0x200)
    {
        printf("***Error expected the divide by zero exception mask in mxcsr to have been unmasked by the tool\n");
        return (-1);
    }

    MaskZeroDivideInMxcsr();
    curMxcsr = GetMxcsr();
    if (!(curMxcsr & 0x200))
    {
        printf("***Error2 in masking the divide by zero exception mask in mxcsr\n");
        return (-1);
    }

    // the test_iarg_preserve tool will insert a call to an analysis routine
    // after the third fld1 in Fld1Fstp that unmasks the divide by zero exception mask in mxcsr
    Fld1Fstp_k();

    // make sure the  divide by zero exception mask in mxcsr is not unmasked - since the tool
    // did not specify any IARG_PRESERVE and Pin preserves the mxcsr by default
    curMxcsr = GetMxcsr();
    if (!(curMxcsr & 0x200))
    {
        printf("***Error1 expected the divide by zero exception mask in mxcsr to remain unmasked around the analysis call in the "
               "tool\n");
        return (-1);
    }

    MaskZeroDivideInMxcsr();
    curMxcsr = GetMxcsr();
    if (!(curMxcsr & 0x200))
    {
        printf("***Error3 in masking the divide by zero exception mask in mxcsr\n");
        return (-1);
    }

    // the test_iarg_preserve tool will insert a call to an analysis routine
    // after the third fld1 in Fld1Fstp that unmasks the divide by zero exception mask in mxcsr
    Fld1Fstp_l();

    // make sure the  divide by zero exception mask in mxcsr is not unmasked - since the tool
    // did not specify any IARG_PRESERVE on REG_MXCSR and Pin preserves the mxcsr by default
    curMxcsr = GetMxcsr();
    if (!(curMxcsr & 0x200))
    {
        printf("***Error2 expected the divide by zero exception mask in mxcsr to remain unmasked around the analysis call in the "
               "tool\n");
        return (-1);
    }

    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        write_xmm_reg(i, 0xbaadf00d);
    }
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        read_xmm_reg(i, xmm_regs[i]);
    }
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        if (!(xmm_regs[i].dword[0] == 0xbaadf00d && xmm_regs[i].dword[1] == 0xbaadf00d && xmm_regs[i].dword[2] == 0xbaadf00d &&
              xmm_regs[i].dword[3] == 0xbaadf00d))
        {
            printf("***Error2 in initializing xmm regs\n");
            return (-1);
        }
    }
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        write_xmm_reg(i, 0xbaadf00d);
    }

    UnMaskZeroDivideInMxcsr();
    curMxcsr = GetMxcsr();
    if ((curMxcsr & 0x200))
    {
        printf("***Error4 in masking the divide by zero exception mask in mxcsr\n");
        return (-1);
    }
    var1 = 2.0;
    var2 = 2.0;
    var3 = 2.0;
    // the test_iarg_preserve tool will insert a call to an analysis routine
    // after the third fld1 that sets the first 3 x87 regs to 0

    Fld1FstpSetXmmsMaskMxcsr();
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        read_xmm_reg(i, xmm_regs[i]);
    }
    // make sure the values were NOT changed by the tool test_iarg_preserve - since that tool
    // did not specify any IARG_PRESERVE and Pin saves the x87 state by default. Note - some registers
    // may hold the new value, see IARG_PRESERVE note at the beginning of this function.
    if (var1 != 1.0 || var2 != 1.0 || var3 != 1.0)
    {
        printf("***Error in x87 values_3\n");
        return (-1);
    }
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        if (!(xmm_regs[i].dword[0] == 0 && xmm_regs[i].dword[1] == 0 && xmm_regs[i].dword[2] == 0 && xmm_regs[i].dword[3] == 0))
        {
            printf("***Error xmm values after fxrstor i %d  %x %x %x %x\n", i, xmm_regs[i].dword[0], xmm_regs[i].dword[1],
                   xmm_regs[i].dword[2], xmm_regs[i].dword[3]);
            return (-1);
        }
    }
    // make sure the  divide by zero exception mask in mxcsr is not unmasked
    curMxcsr = GetMxcsr();
    if (!(curMxcsr & 0x200))
    {
        printf("***Error3 expected the divide by zero exception mask in mxcsr to remain unmasked around the analysis call in the "
               "tool\n");
        return (-1);
    }

    var1 = 2.0;
    var2 = 2.0;
    var3 = 2.0;

    Fld1Fstp_m(
        (TestIargPreserveInReplacement)); // this function calls TestIargPreserveInReplacement, which is replaced by the tool

    // make sure the values were changed by the tool test_iarg_preserve - since that tool
    // specified that it's replacement routine preserved the x87 regs when it doesn't. Note - some registers
    // may hold the previous value, see IARG_PRESERVE note at the beginning of this function.
    if ((var1 != 0.0 && var1 != 2.0) || (var2 != 0.0 && var2 != 2.0) || (var3 != 0.0 && var3 != 2.0))
    {
        printf("***Error in x87 values_1a : var1 = %f, var2 = %f, var3 = %f\n", var1, var2, var3);
        return (-1);
    }

    var1 = 2.0;
    var2 = 2.0;
    var3 = 2.0;

    Fld1Fstp_n(
        (TestIargPreserveInReplacement1)); // this function calls TestIargPreserveInReplacement, which is replaced by the tool

    // make sure the values were NOT changed by the tool test_iarg_preserve - since that tool
    // specified that it's replacement routine does not preserve the x87 regs
    if (var1 != 1.0 || var2 != 1.0 || var3 != 1.0)
    {
        printf("***Error in x87 values_1a : var1 = %f, var2 = %f, var3 = %f\n", var1, var2, var3);
        return (-1);
    }

    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        write_xmm_reg(i, 0xbaadf00d);
    }
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        read_xmm_reg(i, xmm_regs[i]);
    }
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        if (!(xmm_regs[i].dword[0] == 0xbaadf00d && xmm_regs[i].dword[1] == 0xbaadf00d && xmm_regs[i].dword[2] == 0xbaadf00d &&
              xmm_regs[i].dword[3] == 0xbaadf00d))
        {
            printf("***Error2 in initializing xmm regs\n");
            return (-1);
        }
    }
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        write_xmm_reg(i, 0xbaadf00d);
    }

    Fld1_x();
    Fld1Fstp_n1(
        (TestIargPreserveInReplacement2)); // this function calls TestIargPreserveInReplacement2, which is replaced by the tool
    Fstp_x();

    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        read_xmm_reg(i, xmm_regs[i]);
    }

    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        if (!(xmm_regs[i].dword[0] == 0xbaadf00d && xmm_regs[i].dword[1] == 0xbaadf00d && xmm_regs[i].dword[2] == 0xbaadf00d &&
              xmm_regs[i].dword[3] == 0xbaadf00d))
        {
            printf("***Error xmm values_1 after fxrstor i %d  %x %x %x %x\n", i, xmm_regs[i].dword[0], xmm_regs[i].dword[1],
                   xmm_regs[i].dword[2], xmm_regs[i].dword[3]);
            return (-1);
        }
    }

    if (var1 != 1.0 || var2 != 1.0 || var3 != 1.0)
    {
        printf("***Error in x87 values_1b var1 %f var2 %f var3 %f\n", var1, var2, var3);
        return (-1);
    }

    printf("app finished\n");
    fflush(stdout);
    return (0);
}
