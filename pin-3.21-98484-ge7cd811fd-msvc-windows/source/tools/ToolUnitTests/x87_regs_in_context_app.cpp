/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// this application is for use in the x87_regs_in_context.test, in conjunction with the
// x87_regs_in_context tool
#if defined(TARGET_WINDOWS)
#include <windows.h>
#include <string>
#include <iostream>
#include <memory.h>

#define EXPORT_CSYM extern "C" __declspec(dllexport)

#else // not TARGET_WINDOWS

#include <ucontext.h>
#include <signal.h>
#include <stdio.h>
#include <setjmp.h>
#include <assert.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <memory.h>

#define EXPORT_CSYM extern "C"
#endif

#include <stdio.h>
#include "../Utils/threadlib.h"

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

#define ALIGN16 __declspec(align(16))
#define ALIGN8 __declspec(align(8))

#else
#error Expect usage of either GNU or MS compiler.
#endif

extern "C" void Fld1_a();
extern "C" void Fld1_b();
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

/*
 Retrieve the X87 registers and print their contents
 */
void DumpX87Regs()
{
    static char buffer[2048 + 16];
    static char* aligned_bufp  = reinterpret_cast< char* >(((reinterpret_cast< ADDRINT >(buffer) + 16) >> 4) << 4);
    FPSTATE* fpVerboseContext1 = reinterpret_cast< FPSTATE* >(aligned_bufp);
    mmx_save(aligned_bufp);

    printf("_mxcsr %x\n", fpVerboseContext1->fxsave_legacy._mxcsr);

    int i;
    for (i = 0; i < 8; i++)
    {
        printf("_sts[%d] %x %x %x %x\n", i, fpVerboseContext1->fxsave_legacy._sts[i]._raw32._hi2,
               fpVerboseContext1->fxsave_legacy._sts[i]._raw32._hi1, fpVerboseContext1->fxsave_legacy._sts[i]._raw32._lo2,
               fpVerboseContext1->fxsave_legacy._sts[i]._raw32._lo1);
    }
    fflush(stdout);
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

/* when run with the set_fp_context_xmm_regs tool, the tool will call this
   function via the PIN_ExecuteAt, but first the tool will
   sets the x87 regs in the context used to in the PIN_ExecuteAt call
*/
EXPORT_CSYM void ExecutedAtFunc()
{
    // verify and dump
    VerifyAndDumpX87RegsAtExecutedAtFunction();
}

void VerifyAndDumpX87RegsAtFunctionCalledFromToolExceptionCatcher()
{
    static char buffer[2048 + 16];
    static char* aligned_bufp  = reinterpret_cast< char* >(((reinterpret_cast< ADDRINT >(buffer) + 16) >> 4) << 4);
    FPSTATE* fpVerboseContext1 = reinterpret_cast< FPSTATE* >(aligned_bufp);
    mmx_save(aligned_bufp);
    printf("VerifyAndDumpX87RegsAtFunctionCalledFromToolExceptionCatcher\n");
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

// the tool's OnException function directs execution to here after changing the values in the xmm registers
EXPORT_CSYM void DumpX87RegsAtException()
{
    VerifyAndDumpX87RegsAtExecutedAtFunction();
    // and exit OK
    exit(0);
}

#if !defined(TARGET_WINDOWS)

// Linux

// the segv exception handler will never be reached, because the tool OnException function redirects the
// continuation to the DumpX87RegsAtException
void handle(int sig, siginfo_t* info, void* vctxt)
{
    printf("Exception ocurred. Now in handler\n");
    fflush(stdout);
}
#endif

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

extern "C" ADDRINT setFlagsX = 0;

int main()
{
    int i;
    printf("x87 regs at app main\n");
    DumpX87Regs();
    printf("x87 regs as set to values just before app calls the function replaced by the tool\n");
    SetX87Regs(0xa5a5a5a5);
    Fld1_a(); // tool will insert analysis function to cause the x87 state to
              // be in the spill area at the time of the call to the function
    ReplacedX87Regs();

#if !defined(TARGET_WINDOWS)
    // Linux
    // define a handler so that the tool gets the  context change callback with the CONTEXT_CHANGE_REASON_SIGNAL
    // and a valid ctxtTo
    struct sigaction sigact;

    sigact.sa_sigaction = handle;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &sigact, 0) == -1)
    {
        fprintf(stderr, "Unable to handle SIGSEGV signal\n");
        exit(-1);
    }
#endif

    char* p = 0;
    p++;
    printf("x87 regs as set to values just before app causes exception\n");
    SetX87Regs(0x5a5a5a5a);
    Fld1_b(); // tool will insert analysis function to cause the x87 state to
              // be in the spill area at the time of the following exception
    // the gpf here will cause the invocation of the tool's OnException function
    *p = 0;
}
