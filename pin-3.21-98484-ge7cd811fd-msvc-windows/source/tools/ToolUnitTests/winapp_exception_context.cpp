/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  An example of Windows application that raises exception and verifies
 *  the FP/XMM state. 
 */
#include <windows.h>
#include <string>
#include <iostream>
#include <memory.h>

using std::cerr;
using std::cout;
using std::endl;
using std::string;

typedef unsigned __int8 UINT8;
typedef unsigned __int16 UINT16;
typedef unsigned __int32 UINT32;
typedef unsigned __int64 UINT64;

#if defined(TARGET_IA32)

struct FXSAVE_STRUCT
{
    UINT16 _fcw;
    UINT16 _fsw;
    UINT8 _ftw;
    UINT8 _pad1;
    UINT16 _fop;
    UINT32 _fpuip;
    UINT16 _cs;
    UINT16 _pad2;
    UINT32 _fpudp;
    UINT16 _ds;
    UINT16 _pad3;
    UINT32 _mxcsr;
    UINT32 _mxcsrmask;
    UINT8 _st[8 * 16];
    UINT8 _xmm[8 * 16];
    UINT8 _pad4[56 * 4];
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

const size_t FpRegsOffset = offsetof(CONTEXT, FloatSave.RegisterArea);
const size_t FpRegSize    = 10;
const size_t NumFpRegs    = 8;

const size_t FpRegsOffset2 = (offsetof(CONTEXT, ExtendedRegisters) + offsetof(FPSTATE, fxsave_legacy._st[0]));
const size_t FpRegSize2    = 16;

const size_t XmmRegsOffset = (offsetof(CONTEXT, ExtendedRegisters) + offsetof(FPSTATE, fxsave_legacy._xmm[0]));
const size_t XmmRegSize    = 16;
const size_t NumXmmRegs    = 8;

#elif defined(TARGET_IA32E)

struct FXSAVE_STRUCT
{
    UINT16 _fcw;
    UINT16 _fsw;
    UINT8 _ftw;
    UINT8 _pad1;
    UINT16 _fop;
    UINT32 _fpuip;
    UINT16 _cs;
    UINT16 _pad2;
    UINT32 _fpudp;
    UINT16 _ds;
    UINT16 _pad3;
    UINT32 _mxcsr;
    UINT32 _mxcsrmask;
    UINT8 _st[8 * 16];
    UINT8 _xmm[16 * 16];
    UINT8 _pad4[24 * 4];
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
const size_t FpRegsOffset = offsetof(CONTEXT, FltSave.FloatRegisters);
const size_t FpRegSize    = 16;
const size_t NumFpRegs    = 8;

const size_t XmmRegsOffset = offsetof(CONTEXT, FltSave.XmmRegisters);
const size_t XmmRegSize    = 16;
const size_t NumXmmRegs    = 16;

#endif

/*!
 * Set some constant values in FP and XMM registers. We set these values before
 * raising the (second) exception and check them in the exception filter using the 
 * CheckMyFpContext() function.
 */
void SetMyFpContext(PCONTEXT pContext)
{
    memset((char*)(pContext) + FpRegsOffset, 0, NumFpRegs * FpRegSize);
    for (size_t i = 0; i < NumFpRegs; ++i)
    {
        *((unsigned char*)(pContext) + FpRegsOffset + (i * FpRegSize)) = (unsigned char)i;
    }

#if defined(TARGET_IA32)
    memset((char*)(pContext) + FpRegsOffset2, 0, NumFpRegs * FpRegSize2);
    for (size_t i = 0; i < NumFpRegs; ++i)
    {
        *((unsigned char*)(pContext) + FpRegsOffset2 + (i * FpRegSize2)) = (unsigned char)i;
    }
#endif

    memset((char*)(pContext) + XmmRegsOffset, 0, NumXmmRegs * XmmRegSize);
    for (size_t i = 0; i < NumXmmRegs; ++i)
    {
        *((unsigned char*)(pContext) + XmmRegsOffset + (i * XmmRegSize)) = (unsigned char)i;
    }
}

/*!
 * Check to see that values of FP/XMM registers in the specified context match
 * values assigned by SetMyFpContext().
 */
static bool CheckMyFpContext(PCONTEXT pContext)
{
    for (size_t i = 0; i < NumFpRegs * FpRegSize; ++i)
    {
        unsigned char regId = i / FpRegSize;

        if ((i % FpRegSize == 0) && (*((unsigned char*)(pContext) + FpRegsOffset + i) != regId))
        {
            return false;
        }
        if ((i % FpRegSize != 0) && (*((unsigned char*)(pContext) + FpRegsOffset + i) != 0))
        {
            return false;
        }
    }

    for (size_t i = 0; i < NumXmmRegs * XmmRegSize; ++i)
    {
        unsigned char regId = i / XmmRegSize;

        if ((i % XmmRegSize == 0) && (*((unsigned char*)(pContext) + XmmRegsOffset + i) != regId))
        {
            return false;
        }
        if ((i % XmmRegSize != 0) && (*((unsigned char*)(pContext) + XmmRegsOffset + i) != 0))
        {
            return false;
        }
    }
    return true;
}

/*!
 * Exception filter. It is invoked twice: 
 * On first exception we set some predefined values in FP and XMM registers and
 * re-raise the exception.
 * On second exception we just return to the exception handler which verifies the
 * the FP/XMM state.
 */
static int MyExceptionFilter(LPEXCEPTION_POINTERS exceptPtr, PCONTEXT pContext)
{
    static bool first = true;

    if (first)
    {
        first = false;
        SetMyFpContext(exceptPtr->ContextRecord);
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    else
    {
        *pContext = *(exceptPtr->ContextRecord);
        return EXCEPTION_EXECUTE_HANDLER;
    }
}

/*!
 * Exit with the specified error message
 */
static void Abort(const char* msg)
{
    cerr << msg << endl;
    exit(1);
}

/*!
 * The main procedure of the application.
 */
int main(int argc, char* argv[])
{
    CONTEXT context;

    __try
    {
        char* p = 0;
        p++;
        *p = 0;
    }
    __except (MyExceptionFilter(GetExceptionInformation(), &context))
    {
        if (!CheckMyFpContext(&context))
        {
            Abort("Mismatch in the FP context");
        }
    }

    cerr << "Success" << endl;
    return 0;
}
