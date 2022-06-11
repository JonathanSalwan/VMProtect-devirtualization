/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#if defined(TARGET_WINDOWS)
#include "windows.h"
#define EXPORT_CSYM extern "C" __declspec(dllexport)
#else
#include <stdlib.h>
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
    AddToXmmRegs();

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
extern "C" void Fld1Fstp_f1();
#endif
#if defined(__cplusplus)
extern "C" void Fld1Fstp_f2();
#endif
#if defined(__cplusplus)
extern "C" void Fld1Fstp_f();
#endif
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
    Fld1Fstp_l1();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1Fstp_l2();
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
    Fld1_aa();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1_ab();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1_ac();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1_ad();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1_ae();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1_af();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fld1_ag();
#if defined(__cplusplus)
extern "C"
#endif
    unsigned int
    GetFlags();
extern "C" void SetFlags();
#if defined(__cplusplus)
extern "C"
#endif
    void
    Fstp3();
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

extern "C" void mmx_save(char* ptr);
extern "C" void mmx_restore(char* ptr);

typedef struct
{
    UINT32 _lo1; ///< Least significant part of value.
    UINT32 _lo2; ///< Least significant part of value.
    UINT32 _hi1; ///< Most significant part of value.
    UINT32 _hi2; ///< Most significant part of value.
} RAW32;

/*!
 * An 80-bit X87 data register padded out to 128-bits.
 */
union /*<POD>*/ X87REG_PADDED
{
    struct
    {
        UINT64 _significand; ///< The floating-point significand.
        UINT16 _exponent;    ///< The floating-point exponent, top bit is the sign bit.
        UINT16 _pad[3];
    } _fp;
    struct
    {
        UINT64 _lo; ///< Least significant part of value.
        UINT64 _hi; ///< Most significant part of value.
    } _raw;
    struct
    {
        UINT32 _lo1; ///< Least significant part of value.
        UINT32 _lo2; ///< Least significant part of value.
        UINT32 _hi1; ///< Most significant part of value.
        UINT32 _hi2; ///< Most significant part of value.
    } _raw32;
};

/*!
 * A 128-bit XMM register value.
 */
union /*<POD>*/ XMMREG
{
    UINT8 _vec8[16];  ///< Vector of 16 8-bit elements.
    UINT16 _vec16[8]; ///< Vector of 8 16-bit elements.
    UINT32 _vec32[4]; ///< Vector of 4 32-bit elements.
    UINT64 _vec64[2]; ///< Vector of 2 64-bit elements.
};

#if defined(TARGET_IA32)
typedef UINT32 ADDRINT;

struct /*<POD>*/ FXSAVE_STRUCT
{
    UINT16 _fcw; ///< X87 control word.
    UINT16 _fsw; ///< X87 status word.
    UINT8 _ftw;  ///< Abridged X87 tag value.
    UINT8 _pad1;
    UINT16 _fop;   ///< Last X87 non-control instruction opcode.
    UINT32 _fpuip; ///< Last X87 non-control instruction address.
    UINT16 _cs;    ///< Last X87 non-control instruction CS selector.
    UINT16 _pad2;
    UINT32 _fpudp; ///< Last X87 non-control instruction operand address.
    UINT16 _ds;    ///< Last X87 non-control instruction operand DS selector.
    UINT16 _pad3;
    UINT32 _mxcsr;         ///< MXCSR control and status register.
    UINT32 _mxcsrmask;     ///< Mask of valid MXCSR bits.
    X87REG_PADDED _sts[8]; ///< X87 data registers in top-of-stack order.
    XMMREG _xmms[8];       ///< XMM registers.
    UINT8 _pad4[224];
};

struct FPSTATE_STRUCT
{
    // fxsave_legacy is applicable on all IA-32 and Intel(R) 64
    // processors
    struct FXSAVE_STRUCT fxsave_legacy;
    // the following are only applicable on processors with AVX
    UINT8 _header[64];
    UINT8 _ymmUpper[8 * 16];
    UINT8 _pad5[8 * 16];
};

typedef FPSTATE_STRUCT FPSTATE;

#elif defined(TARGET_IA32E)
typedef UINT64 ADDRINT;

struct /*<POD>*/ FXSAVE_STRUCT
{
    UINT16 _fcw; ///< X87 control word.
    UINT16 _fsw; ///< X87 status word.
    UINT8 _ftw;  ///< Abridged X87 tag value.
    UINT8 _pad1;
    UINT16 _fop;   ///< Last X87 non-control instruction opcode.
    UINT32 _fpuip; ///< Last X87 non-control instruction segment offset.
    UINT16 _cs;    ///< Last X87 non-control instruction CS selector.
    UINT16 _pad2;
    UINT32 _fpudp; ///< Last X87 non-control instruction operand segment offset.
    UINT16 _ds;    ///< Last X87 non-control instruction operand DS selector.
    UINT16 _pad3;
    UINT32 _mxcsr;         ///< MXCSR control and status register.
    UINT32 _mxcsrmask;     ///< Mask of valid MXCSR bits.
    X87REG_PADDED _sts[8]; ///< X87 data registers in top-of-stack order.
    XMMREG _xmms[16];      ///< XMM registers.
    UINT8 _pad4[96];
};

struct FPSTATE_STRUCT
{
    // fxsave_legacy is applicable on all IA-32 and Intel(R) 64
    // processors
    struct FXSAVE_STRUCT fxsave_legacy;
    // the following are only applicable on processors with AVX
    UINT8 _header[64];
    UINT8 _ymmUpper[16 * 16];
};

typedef FPSTATE_STRUCT FPSTATE;

#endif

extern "C" reg_t get_gax();
extern "C" reg_t get_gbx();
extern "C" reg_t get_gcx();
extern "C" reg_t get_gdx();
extern "C" reg_t get_gdi();
extern "C" reg_t get_gsi();
extern "C" reg_t get_gbp();
extern "C" void set_gax(reg_t regVal);
extern "C" void set_gbx(reg_t regVal);
extern "C" void set_gcx(reg_t regVal);
extern "C" void set_gdx(reg_t regVal);
extern "C" void set_gdi(reg_t regVal);
extern "C" void set_gsi(reg_t regVal);
extern "C" void set_gbp(reg_t regVal);
#if defined(TARGET_IA32E)
extern "C" reg_t get_r8();
extern "C" reg_t get_r9();
extern "C" reg_t get_r10();
extern "C" reg_t get_r11();
extern "C" reg_t get_r12();
extern "C" reg_t get_r13();
extern "C" reg_t get_r14();
extern "C" reg_t get_r15();
extern "C" void set_r8(reg_t regVal);
extern "C" void set_r9(reg_t regVal);
extern "C" void set_r10(reg_t regVal);
extern "C" void set_r11(reg_t regVal);
extern "C" void set_r12(reg_t regVal);
extern "C" void set_r13(reg_t regVal);
extern "C" void set_r14(reg_t regVal);
extern "C" void set_r15(reg_t regVal);
#endif

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

reg_t savedGax, savedGbx, savedGcx, savedGdx, savedGdi, savedGsi, savedGbp;
reg_t savedR8, savedR9, savedR10, savedR11, savedR12, savedR13, savedR14, savedR15;
reg_t regGax, regGbx, regGcx, regGdx, regGdi, regGsi, regGbp;
reg_t regR8, regR9, regR10, regR11, regR12, regR13, regR14, regR15;

extern "C" ADDRINT setFlagsX = 0;

void VerifyAndDumpX87RegsAtReplacedFunction()
{
    static char buffer[2048 + 16];
    static char* aligned_bufp  = reinterpret_cast< char* >(((reinterpret_cast< ADDRINT >(buffer) + 16) >> 4) << 4);
    FPSTATE* fpVerboseContext1 = reinterpret_cast< FPSTATE* >(aligned_bufp);
    mmx_save(aligned_bufp);
    printf("VerifyAndDumpX87RegsAtReplacedFunction\n");
    // tool set the fp regs, just before calling the replaced function to:
    /*
    _mxcsr 1d80
    _sts[0] 0 acdc acdcacdc acdcacdc
    _sts[1] 0 acdc acdcacdc acdcacdc
    _sts[2] 0 acdc acdcacdc acdcacdc
    _sts[3] 0 acdc acdcacdc acdcacdc
    _sts[4] 0 acdc acdcacdc acdcacdc
    _sts[5] 0 acdc acdcacdc acdcacdc
    _sts[6] 0 acdc acdcacdc acdcacdc
    _sts[7] 0 acdc acdcacdc acdcacdc
    */
    printf("_mxcsr %x\n", fpVerboseContext1->fxsave_legacy._mxcsr);
    if (!(fpVerboseContext1->fxsave_legacy._mxcsr &= 0x200))
    {
        printf("***Error divide by zero should be masked\n");
        exit(-1);
    }
    int i;

    for (i = 0; i < 8; i++)
    {
        RAW32* ptr = reinterpret_cast< RAW32* >(&fpVerboseContext1->fxsave_legacy._sts[i]._raw);
        printf("_st[%d] %x %x %x %x\n", i, ptr->_hi2, ptr->_hi1, ptr->_lo2, ptr->_lo1);
        if (ptr->_hi2 != 0 && ptr->_hi1 != 0xacdc && ptr->_lo2 != 0xacdcacdc && ptr->_lo1 != 0xacdcacdc)
        {
            printf("***Error in this _st\n");
            exit(-1);
        }
    }
}

/* when run with the x87_regs_in_context tool, the tool will have replaced this
   function with a function that calls this original but first the tool 
   replacement function sets the x87 regs in the context used  
   in the PIN_CallApplicationFunction used to call this original function
*/
EXPORT_CSYM void ReplacedX87Regs()
{
    // verify and dump
    VerifyAndDumpX87RegsAtReplacedFunction();
}

void SetX87Regs(unsigned int val)
{
    static char buffer[2048 + 16];
    static char* aligned_bufp  = reinterpret_cast< char* >(((reinterpret_cast< ADDRINT >(buffer) + 16) >> 4) << 4);
    FPSTATE* fpVerboseContext1 = reinterpret_cast< FPSTATE* >(aligned_bufp);
    mmx_save(aligned_bufp);

    fpVerboseContext1->fxsave_legacy._mxcsr &= ~(0x200); // unmask divide by zero
    int i;
    for (i = 0; i < 8; i++)
    {
        fpVerboseContext1->fxsave_legacy._sts[i]._raw32._hi2 = val;
        fpVerboseContext1->fxsave_legacy._sts[i]._raw32._hi1 = val;
        fpVerboseContext1->fxsave_legacy._sts[i]._raw32._lo2 = val;
        fpVerboseContext1->fxsave_legacy._sts[i]._raw32._lo1 = val;
    }

    mmx_restore(aligned_bufp);
    for (i = 0; i < 8; i++)
    {
        fpVerboseContext1->fxsave_legacy._sts[i]._raw32._hi2 = 0;
        fpVerboseContext1->fxsave_legacy._sts[i]._raw32._hi1 = 0;
        fpVerboseContext1->fxsave_legacy._sts[i]._raw32._lo2 = 0;
        fpVerboseContext1->fxsave_legacy._sts[i]._raw32._lo1 = 0;
    }

    mmx_save(aligned_bufp);

    printf("_mxcsr %x\n", fpVerboseContext1->fxsave_legacy._mxcsr);
    if (fpVerboseContext1->fxsave_legacy._mxcsr &= 0x200)
    {
        printf("***Error divide by zero should be unmasked\n");
        exit(-1);
    }

    for (i = 0; i < 8; i++)
    {
        RAW32* ptr = reinterpret_cast< RAW32* >(&fpVerboseContext1->fxsave_legacy._sts[i]._raw);
        printf("_st[%d] %x %x %x %x\n", i, ptr->_hi2, ptr->_hi1, ptr->_lo2, ptr->_lo1);
        if (ptr->_hi2 != 0 && ptr->_hi1 != (val & 0xffff) && ptr->_lo2 != val && ptr->_lo1 != val)
        {
            printf("***Error in this _st\n");
            exit(-1);
        }
    }
}

void VerifyAndDumpX87RegsAtExecutedAtFunction()
{
    static char buffer[2048 + 16];
    static char* aligned_bufp  = reinterpret_cast< char* >(((reinterpret_cast< ADDRINT >(buffer) + 16) >> 4) << 4);
    FPSTATE* fpVerboseContext1 = reinterpret_cast< FPSTATE* >(aligned_bufp);
    mmx_save(aligned_bufp);
    printf("VerifyAndDumpX87RegsAtExecutedAtFunction\n");
    // tool set the fp regs, just before calling the function ExecutedAtFunc (using PIN_ExecuteAt) to:
    /*
   _mxcsr 1d80
    _sts[0] 0 cacd cacdcacd cacdcacd
    _sts[1] 0 cacd cacdcacd cacdcacd
    _sts[2] 0 cacd cacdcacd cacdcacd
    _sts[3] 0 cacd cacdcacd cacdcacd
    _sts[4] 0 cacd cacdcacd cacdcacd
    _sts[5] 0 cacd cacdcacd cacdcacd
    _sts[6] 0 cacd cacdcacd cacdcacd
    _sts[7] 0 cacd cacdcacd cacdcacd
    */
    printf("_mxcsr %x\n", fpVerboseContext1->fxsave_legacy._mxcsr);
    if (!(fpVerboseContext1->fxsave_legacy._mxcsr &= 0x200))
    {
        printf("***Error divide by zero should be masked\n");
        exit(-1);
    }
    int i;

    for (i = 0; i < 8; i++)
    {
        RAW32* ptr = reinterpret_cast< RAW32* >(&fpVerboseContext1->fxsave_legacy._sts[i]._raw);
        printf("_st[%d] %x %x %x %x\n", i, ptr->_hi2, ptr->_hi1, ptr->_lo2, ptr->_lo1);
        if (ptr->_hi2 != 0 && ptr->_hi1 != 0xcacd && ptr->_lo2 != 0xcacdcacd && ptr->_lo1 != 0xcacdcacd)
        {
            printf("***Error in this _st\n");
            exit(-1);
        }
    }
}

/* when run with the test_iarg_partial_context tool, the tool will call this
   function via the PIN_ExecuteAt, but first the tool will 
   sets the x87 regs in the context used to in the PIN_ExecuteAt call
*/
EXPORT_CSYM void ExecutedAtFunc()
{
    // verify and dump
    VerifyAndDumpX87RegsAtExecutedAtFunction();
}

int main()
{
    var1 = 2.0;
    var2 = 2.0;
    var3 = 2.0;

    // the test_iarg_partial_context tool will insert a call to an analysis routine
    // that changes the fp-stack contents and specifies to write it back to the
    // application context
    Fld1Fstp_a();

    // make sure the values were changed by the tool test_iarg_partial_context - since that tool
    // an IARG_PARTIAL_CONTEXT that writes the fp-stack
    if (var1 != 0.0 || var2 != 0.0 || var3 != 0.0)
    {
        printf("***Error in x87 values_1 var1 %f var2 %f var3 %f\n", var1, var2, var3);
        return (-1);
    }

    var1 = 2.0;
    var2 = 2.0;
    var3 = 2.0;

    // the test_iarg_partial_context tool will insert a call to an analysis routine
    // that changes the fp-stack contents and specifies to write it back to the
    // application context
    Fld1Fstp_b();

    // make sure the values were changed by the tool test_iarg_partial_context - since that tool
    // an IARG_PARTIAL_CONTEXT that writes the fp-stack
    if (var1 != 0.0 || var2 != 0.0 || var3 != 0.0)
    {
        printf("***Error in x87 values_2 var1 %f var2 %f var3 %f\n", var1, var2, var3);
        return (-1);
    }

    var1 = 2.0;
    var2 = 2.0;
    var3 = 2.0;

    // the test_iarg_partial_context tool will insert a call to an analysis routine
    // that changes the fp-stack contents and specifies to write it back to the
    // application context
    Fld1Fstp_c();

    // make sure the values were changed by the tool test_iarg_partial_context - since that tool
    // an IARG_PARTIAL_CONTEXT that writes the fp-stack
    if (var1 != 0.0 || var2 != 0.0 || var3 != 0.0)
    {
        printf("***Error in x87 values_3\n");
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

    // the test_iarg_partial_context tool will insert a call to an analysis routine
    // after the third fld1 in Fld1Fstp that adds values to the xmm registers
    Fld1Fstp_e();

    // make sure the values were changed by the tool test_iarg_partial_context - since that tool
    // specified that it's analysis routine writes back the xmm regs w
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        read_xmm_reg(i, xmm_regs[i]);
    }
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        if (!(xmm_regs[i].dword[0] == (0xdeadbeef + i + 1) && xmm_regs[i].dword[1] == 0xdeadbeef &&
              xmm_regs[i].dword[2] == 0xdeadbeef && xmm_regs[i].dword[3] == 0xdeadbeef))
        {
            printf("***Error in xmm regs values_1 xmm%d %x %x %x %x\n", i, xmm_regs[i].dword[3], xmm_regs[i].dword[2],
                   xmm_regs[i].dword[1], xmm_regs[i].dword[0]);
            return (-1);
        }
    }

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

    // the test_iarg_partial_context tool will insert a call to an analysis routine
    // after the third fld1 in Fld1Fstp that adds values to the xmm registers
    Fld1Fstp_f();

    // make sure the values were changed by the tool test_iarg_partial_context - since that tool
    // specified that it's analysis routine writes back the xmm regs
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        read_xmm_reg(i, xmm_regs[i]);
    }
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        if (!(xmm_regs[i].dword[0] == (0xdeadbeef + i + 1) && xmm_regs[i].dword[1] == 0xdeadbeef &&
              xmm_regs[i].dword[2] == 0xdeadbeef && xmm_regs[i].dword[3] == 0xdeadbeef))
        {
            printf("***Error in xmm regs values_1 xmm%d %x %x %x %x\n", i, xmm_regs[i].dword[3], xmm_regs[i].dword[2],
                   xmm_regs[i].dword[1], xmm_regs[i].dword[0]);
            return (-1);
        }
    }

    static char buffer[2048];
    static char* aligned_bufp  = reinterpret_cast< char* >(((reinterpret_cast< ADDRINT >(buffer) + 16) >> 4) << 4);
    FPSTATE* fpVerboseContext1 = reinterpret_cast< FPSTATE* >(aligned_bufp);

    // the test_iarg_partial_context tool will insert a call to an analysis routine
    // after the third fld1 that sets the rounding bits in the fpcw to 0
    Fld1_aa();

    mmx_save(aligned_bufp);

    Fstp3();

    if ((fpVerboseContext1->fxsave_legacy._fcw & 0x300) != 0)
    {
        printf("***Error1 in _fcw %x\n", fpVerboseContext1->fxsave_legacy._fcw);
        return (-1);
    }

    // the test_iarg_partial_context tool will insert a call to an analysis routine
    // after the third fld1 in rounding bits in the fpcw to 10 binary
    Fld1_ab();

    mmx_save(aligned_bufp);
    Fstp3();

    if ((fpVerboseContext1->fxsave_legacy._fcw & 0x300) != 0x200)
    {
        printf("***Error2 in _fcw %x\n", fpVerboseContext1->fxsave_legacy._fcw);
        return (-1);
    }

    // the test_iarg_partial_context tool will insert a call to an analysis routine
    // after the third fld1 that sets the rounding bits in the fpcw to 0
    Fld1_ac();

    mmx_save(aligned_bufp);
    Fstp3();

    if ((fpVerboseContext1->fxsave_legacy._fcw & 0x300) != 0)
    {
        printf("***Error3 in _fcw %x\n", fpVerboseContext1->fxsave_legacy._fcw);
        return (-1);
    }

    // the test_iarg_partial_context tool will insert a call to an analysis routine
    // after the third fld1 that sets the rounding bits in the fpcw to 10 binary
    Fld1_ad();

    mmx_save(aligned_bufp);
    Fstp3();

    if ((fpVerboseContext1->fxsave_legacy._fcw & 0x300) != 0x200)
    {
        printf("***Error4 in _fcw %x\n", fpVerboseContext1->fxsave_legacy._fcw);
        return (-1);
    }

    savedGax = get_gax(); // must be first
    savedGbx = get_gbx();
    savedGcx = get_gcx();
    savedGdx = get_gdx();
    savedGdi = get_gdi();
    savedGsi = get_gsi();
    savedGbp = get_gbp();
#ifdef TARGET_IA32E
    savedR8  = get_r8();
    savedR9  = get_r9();
    savedR10 = get_r10();
    savedR11 = get_r11();
    savedR12 = get_r12();
    savedR13 = get_r13();
    savedR14 = get_r14();
    savedR15 = get_r15();
#endif

    set_gax(0);
    set_gbx(1);
    set_gdx(3);
    set_gsi(5);
    //set_gbp(6);
#ifdef TARGET_IA32E
    set_r8(6);
    set_r9(7);
    set_r10(8);
    set_r11(9);
    set_r12(10);
    set_r13(11);
    set_r14(12);
    set_r15(13);
#endif
#if defined(__GNUC__)
    set_gcx(2);
    set_gdi(4);
#else
    set_gdi(4);
    set_gcx(2);
#endif

    // the test_iarg_partial_context tool will insert a call to an analysis routine
    // after the third fld1 that add 1 to each of the integer registers
    Fld1_ae();
    Fstp3();

    regGax = get_gax(); // must be first
    regGbx = get_gbx();
    regGcx = get_gcx();
    regGdx = get_gdx();
    regGdi = get_gdi();
    regGsi = get_gsi();
    regGbp = get_gbp();
#ifdef TARGET_IA32E
    regR8  = get_r8();
    regR9  = get_r9();
    regR10 = get_r10();
    regR11 = get_r11();
    regR12 = get_r12();
    regR13 = get_r13();
    regR14 = get_r14();
    regR15 = get_r15();
#endif

    if (regGax != 1 || regGbx != 2 || regGcx != 3 || regGdx != 4 || regGdi != 5 || regGsi != 6 //|| regGbp!=7
#ifdef TARGET_IA32E
        || regR8 == 7 || regR9 == 8 || regR10 == 9 || regR11 == 10 || regR12 == 11 || regR13 == 12 || regR14 == 13 || regR14 == 14
#endif
    )
    {
        printf("***Error in int scratch regs values regGax %x regGbx %x regGcx %x regGdx %x regGdi %x regGsi %x regGbp %x\n",
               regGax, regGbx, regGcx, regGdx, regGdi, regGsi, regGbp);
        return (-1);
    }

    set_gax(0);
    set_gbx(1);
    set_gdx(3);
    set_gsi(5);
    //set_gbp(6);
#ifdef TARGET_IA32E
    set_r8(6);
    set_r9(7);
    set_r10(8);
    set_r11(9);
    set_r12(10);
    set_r13(11);
    set_r14(12);
    set_r15(13);
#endif
#if defined(__GNUC__)
    set_gcx(2);
    set_gdi(4);
#else
    set_gdi(4);
    set_gcx(2);
#endif

    // the test_iarg_partial_context tool will insert a call to an analysis routine
    // after the third fld1 that add 1 to each of the integer registers
    Fld1_af();
    Fstp3();

    regGax = get_gax(); // must be first
    regGbx = get_gbx();
    regGcx = get_gcx();
    regGdx = get_gdx();
    regGdi = get_gdi();
    regGsi = get_gsi();
    regGbp = get_gbp();
#ifdef TARGET_IA32E
    regR8  = get_r8();
    regR9  = get_r9();
    regR10 = get_r10();
    regR11 = get_r11();
    regR12 = get_r12();
    regR13 = get_r13();
    regR14 = get_r14();
    regR15 = get_r15();
#endif

    if (regGax != 3 || regGbx != 4 || regGcx != 5 || regGdx != 6 || regGdi != 7 || regGsi != 8 //|| regGbp!=9
#ifdef TARGET_IA32E
        || regR8 == 10 || regR9 == 11 || regR10 == 12 || regR11 == 13 || regR12 == 14 || regR13 == 15 || regR14 == 16 ||
        regR14 == 17
#endif
    )
    {
        printf("***Error1 in int scratch regs values regGax %x regGbx %x regGcx %x regGdx %x regGdi %x regGsi %x regGbp %x\n",
               regGax, regGbx, regGcx, regGdx, regGdi, regGsi, regGbp);
        return (-1);
    }

    // reset the values to what they were
    set_gax(savedGax);
    set_gbx(savedGbx);
    set_gcx(savedGcx);
    set_gdx(savedGdx);
    set_gdi(savedGdi);
    set_gsi(savedGsi);
    set_gbp(savedGbp);
#ifdef TARGET_IA32E
    set_r8(savedR8);
    set_r9(savedR9);
    set_r10(savedR10);
    set_r11(savedR11);
    set_r12(savedR12);
    set_r13(savedR13);
    set_r14(savedR14);
    set_r15(savedR15);
#endif
#if defined(__GNUC__)
    set_gcx(savedGcx);
    set_gdi(savedGdi);
#else
    set_gdi(savedGdi);
    set_gcx(savedGcx);
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
    // the test_iarg_partial_context tool will insert a call to an analysis routine
    // after the third fld1 in Fld1Fstp that unmasks the divide by zero exception mask in mxcsr
    Fld1Fstp_j();

    // make sure the  divide by zero exception mask in mxcsr is not masked - since that tool
    // specified that mxcsr is writable in the context passed to the analysis routine
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

    // the test_iarg_partial_context tool will insert a call to an analysis routine
    // after the third fld1 in Fld1Fstp that unmasks the divide by zero exception mask in mxcsr
    Fld1Fstp_k();

    // make sure the  divide by zero exception mask in mxcsr is not masked - since the tool
    // specified that mxcsr is writable in the context passed to the analysis routine
    curMxcsr = GetMxcsr();
    if (curMxcsr & 0x200)
    {
        printf("***Error2 expected the divide by zero exception mask in mxcsr to have been unmasked by the tool\n");
        return (-1);
    }

    MaskZeroDivideInMxcsr();
    curMxcsr = GetMxcsr();
    if (!(curMxcsr & 0x200))
    {
        printf("***Error3 in masking the divide by zero exception mask in mxcsr\n");
        return (-1);
    }

    // the test_iarg_partial_context tool will insert a call to an analysis routine
    // after the third fld1 in Fld1Fstp that unmasks the divide by zero exception mask in mxcsr
    Fld1Fstp_l();

    /// make sure the  divide by zero exception mask in mxcsr is not masked - since the tool
    // specified that mxcsr is writable in the context passed to the analysis routine
    curMxcsr = GetMxcsr();
    if ((curMxcsr & 0x200))
    {
        printf("***Error3 expected the divide by zero exception mask in mxcsr to have been unmasked by the tool\n");
        return (-1);
    }

    MaskZeroDivideInMxcsr();
    curMxcsr = GetMxcsr();
    if (!(curMxcsr & 0x200))
    {
        printf("***Error3 in masking the divide by zero exception mask in mxcsr\n");
        return (-1);
    }

    // the test_iarg_partial_context tool will insert a call to an analysis routine
    // after the third fld1 in Fld1Fstp that unmasks the divide by zero exception mask in mxcsr
    Fld1Fstp_l1();

    /// make sure the  divide by zero exception mask in mxcsr is not masked - since the tool
    // specified that mxcsr is writable in the context passed to the analysis routine
    curMxcsr = GetMxcsr();
    if ((curMxcsr & 0x200))
    {
        printf("***Error3 expected the divide by zero exception mask in mxcsr to have been unmasked by the tool\n");
        return (-1);
    }

    MaskZeroDivideInMxcsr();
    curMxcsr = GetMxcsr();
    if (!(curMxcsr & 0x200))
    {
        printf("***Error3 in masking the divide by zero exception mask in mxcsr\n");
        return (-1);
    }

    // the test_iarg_partial_context tool will insert a call to an analysis routine
    // after the third fld1 in Fld1Fstp that unmasks the divide by zero exception mask in mxcsr
    Fld1Fstp_l2();

    /// make sure the  divide by zero exception mask in mxcsr is not masked - since the tool
    // specified that mxcsr is writable in the context passed to the analysis routine
    curMxcsr = GetMxcsr();
    if ((curMxcsr & 0x200))
    {
        printf("***Error3 expected the divide by zero exception mask in mxcsr to have been unmasked by the tool\n");
        return (-1);
    }

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

    // the test_iarg_partial_context tool will insert a call to an analysis routine
    // after the third fld1 in Fld1Fstp that adds values to the xmm registers
    Fld1Fstp_f1();

    // make sure the values were changed by the tool test_iarg_partial_context - since that tool
    // specified that it's analysis routine writes back the xmm regs
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        read_xmm_reg(i, xmm_regs[i]);
    }
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        if (!(xmm_regs[i].dword[0] == (0xdeadbeef + i + 1) && xmm_regs[i].dword[1] == 0xdeadbeef &&
              xmm_regs[i].dword[2] == 0xdeadbeef && xmm_regs[i].dword[3] == 0xdeadbeef))
        {
            printf("***Error in xmm regs values_2 xmm%d %x %x %x %x\n", i, xmm_regs[i].dword[3], xmm_regs[i].dword[2],
                   xmm_regs[i].dword[1], xmm_regs[i].dword[0]);
            return (-1);
        }
    }

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

    // the test_iarg_partial_context tool will insert a call to an analysis routine
    // after the third fld1 in Fld1Fstp that sets values to the xmm registers
    Fld1Fstp_f2();

    // make sure the values were changed by the tool test_iarg_partial_context - since that tool
    // specified that it's analysis routine writes back the xmm regs
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        read_xmm_reg(i, xmm_regs[i]);
    }
    for (UINT32 i = 0; i < NUM_XMM_SCRATCH_REGS; i++)
    {
        if (!(xmm_regs[i].dword[0] == (0xacdcacdc) && xmm_regs[i].dword[1] == 0xacdcacdc && xmm_regs[i].dword[2] == 0xacdcacdc &&
              xmm_regs[i].dword[3] == 0xacdcacdc))
        {
            printf("***Error in xmm regs values_2 xmm%d %x %x %x %x\n", i, xmm_regs[i].dword[3], xmm_regs[i].dword[2],
                   xmm_regs[i].dword[1], xmm_regs[i].dword[0]);
            return (-1);
        }
    }

    UINT32 savFlags = GetFlags();
    setFlagsX       = savFlags;
    setFlagsX &= 0xffff0026;
    setFlagsX |= 0xed5;
    SetFlags();

    // the test_iarg_partial_context tool will insert a call to an analysis routine
    // after the third fld1 in Fld1Fstp that sets values to the flags register
    Fld1_ag();
    Fstp3();

    UINT32 flagsAfterAnalysis = GetFlags();
    if ((flagsAfterAnalysis & 0xfff) != 0xad6)
    {
        printf("***Error in flags after analysis flagsAfterAnalysis %x\n", flagsAfterAnalysis);
        return (-1);
    }
    setFlagsX = savFlags;
    SetFlags();

    var1 = var2 = var3 = 3.0;

    Fld1Fstp_m(
        (TestIargPreserveInReplacement)); // this function calls TestIargPreserveInReplacement, which is replaced by the tool

    // make sure the values were changed by the tool test_iarg_partial_context - since that tool
    // specified that it's replacement routine writes back the x87 regs
    if (var1 != 0.0 || var2 != 0.0 || var3 != 0.0)
    {
        printf("***Error in x87 values_1a var1 %f var2 %f var3 %f\n", var1, var2, var3);
        return (-1);
    }

    SetX87Regs(0xa5a5a5a5);
    ReplacedX87Regs();

    return (0);
}
