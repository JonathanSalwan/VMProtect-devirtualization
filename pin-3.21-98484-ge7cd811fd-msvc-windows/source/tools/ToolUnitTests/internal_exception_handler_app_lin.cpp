/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * This application used to verify that Pin tool can correctly handle internal exceptions
 */

#include <string>
#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
using std::cerr;
using std::endl;
using std::flush;
using std::hex;
using std::string;

#ifdef TARGET_IA32
struct fxsave
{
    unsigned short _fcw;
    unsigned short _fsw;
    unsigned char _ftw;
    unsigned char _pad1;
    unsigned short _fop;
    unsigned int _fpuip;
    unsigned short _cs;
    unsigned short _pad2;
    unsigned int _fpudp;
    unsigned short _ds;
    unsigned short _pad3;
    unsigned int _mxcsr;
    unsigned int _mxcsrmask;
    unsigned char _st[8 * 16];
    unsigned char _xmm[8 * 16];
    unsigned char _pad4[56 * 4];
};

struct KernelFpstate
{
    struct _libc_fpstate _fpregs_mem; // user-visible FP register state (_mcontext points to this)
    struct fxsave _fxsave;            // full FP state as saved by fxsave instruction
};
#else
struct fxsave
{
    unsigned short _cwd;
    unsigned short _swd;
    unsigned short _twd; /* Note this is not the same as the 32bit/x87/FSAVE twd */
    unsigned short _fop;
    unsigned long _rip;
    unsigned long _rdp;
    unsigned int _mxcsr;
    unsigned int _mxcsrmask;
    unsigned int _st[32];        /* 8*16 bytes for each FP-reg */
    unsigned char _xmm[16 * 16]; /* 16*16 bytes for each XMM-reg  */
    unsigned int _reserved2[24];
};

struct KernelFpstate
{
    struct fxsave _fxsave; // user-visible FP register state (_mcontext points to this)
};

#endif

//==========================================================================
// Printing utilities
//==========================================================================
string UnitTestName("internal_exception_handler_app_lin");

static void StartFunctionTest(const string& functionTestName)
{
    cerr << UnitTestName << " [ " << functionTestName << " ] Start" << endl << flush;
}

static void EndFunctionTest(const string& functionTestName)
{
    cerr << UnitTestName << " [ " << functionTestName << " ] Success" << endl << flush;
}

//================================================================
// Install signal handlers
//================================================================
void div0_signal_handler(int, siginfo_t*, void*);

void install_signal_handlers()
{
    int ret;
    struct sigaction sSigaction;

    /* Register the signal hander using the siginfo interface*/
    sSigaction.sa_sigaction = div0_signal_handler;
    sSigaction.sa_flags     = SA_SIGINFO;

    /* mask all other signals */
    sigfillset(&sSigaction.sa_mask);

    ret = sigaction(SIGFPE, &sSigaction, NULL);
    if (ret)
    {
        perror("ERROR, sigaction failed");
        exit(-1);
    }
}
//================================================================
// Define signal handlers
//================================================================

#define FCW_ZERO_DIVIDE 0x4
#define MXCSR_ZERO_DIVIDE 0x200
#ifdef TARGET_IA32
#define FCW_MASK_ZERO_DIVIDE (appFpState->_fxsave._fcw |= FCW_ZERO_DIVIDE)
#define FSW_RESET (appFpState->_fxsave._fsw = 0)
#define MSR_MASK_ZERO_DIVIDE (appFpState->_fxsave._mxcsr |= MXCSR_ZERO_DIVIDE)
#define REG_INST_PTR REG_EIP
#define REG_GBX REG_EBX
#else // not TARGET_IA32
#define FCW_MASK_ZERO_DIVIDE (appFpState->_fxsave._cwd |= FCW_ZERO_DIVIDE)
#define FSW_RESET (appFpState->_fxsave._swd = 0)
#define MSR_MASK_ZERO_DIVIDE (appFpState->_fxsave._mxcsr |= MXCSR_ZERO_DIVIDE)
#define REG_INST_PTR REG_RIP
#define REG_GBX REG_RBX
#endif // not TARGET_IA32
#define MCONTEXT_IP_REG uc_mcontext.gregs[REG_INST_PTR]

extern "C" void UnmaskFpZeroDivide();
extern "C" void UnmaskZeroDivideInMxcsr32();
extern "C" void MaskZeroDivideInMxcsr32();
extern "C" void UnmaskZeroDivideInMxcsr();
extern "C" void MaskZeroDivideInMxcsr();

void div0_signal_handler(int signum, siginfo_t* siginfo, void* uctxt)
{
    printf("Inside div0 handler\n");
    ucontext_t* frameContext = (ucontext_t*)uctxt;

    printf("signal %d, code %d (captured EIP: 0x%x)\n", signum, siginfo->si_code, frameContext->MCONTEXT_IP_REG);

    if (siginfo->si_code == FPE_INTDIV)
    {
        // Move IP to recovery code
        // Catch point is kept in %GBX
        frameContext->uc_mcontext.gregs[REG_INST_PTR] = frameContext->uc_mcontext.gregs[REG_GBX];
    }
    else if ((siginfo->si_code == FPE_FLTDIV))
    {
        // Mask "zero divide" exception in FPU Control Word register
        // Reset FPU Status Register
        fpregset_t fpState = frameContext->uc_mcontext.fpregs;
        /* Change application FP context */
        fpregset_t fpPtr          = frameContext->uc_mcontext.fpregs;
        KernelFpstate* appFpState = reinterpret_cast< KernelFpstate* >(fpPtr);
        FCW_MASK_ZERO_DIVIDE;
        FSW_RESET;
        MSR_MASK_ZERO_DIVIDE;
    }
}

// These numbers are for testing only
#define EXCEPTION_INT_DIVIDE_BY_ZERO 18
#define EXCEPTION_FLT_DIVIDE_BY_ZERO 19

static bool CheckExceptionCode(unsigned int exceptCode, unsigned int expectedExceptCode)
{
    if (exceptCode != expectedExceptCode)
    {
        cerr << "Unexpected exception code " << hex << exceptCode << ". Should be " << hex << expectedExceptCode << endl;
        return false;
    }
    return true;
}

extern "C" unsigned int RaiseIntDivideByZeroException(unsigned int (*)(), unsigned int);
extern "C" unsigned int CatchIntDivideByZeroException();

/*
 * Raise "int deivide by zero" exception
 */
void SafeExecuteIntDivideByZero()
{
    unsigned int exceptionCaught = RaiseIntDivideByZeroException(CatchIntDivideByZeroException, EXCEPTION_INT_DIVIDE_BY_ZERO);
    if (!CheckExceptionCode(exceptionCaught, EXCEPTION_INT_DIVIDE_BY_ZERO))
    {
        exit(-1);
    }
}

/*
 * Raise "X87 deivide by zero" exception
 */
extern "C" unsigned int RaiseFltDivideByZeroException(unsigned int exception_code)
{
    volatile float zero = 0.0;
    volatile float i    = 1.0 / zero;
    return exception_code;
}

void SafeExecuteFltDivideByZero()
{
    UnmaskFpZeroDivide();
#if defined(TARGET_IA32E)
    UnmaskZeroDivideInMxcsr();
#else
    UnmaskZeroDivideInMxcsr32();
#endif
    unsigned int exceptionCaught = RaiseFltDivideByZeroException(EXCEPTION_FLT_DIVIDE_BY_ZERO);
    if (!CheckExceptionCode(exceptionCaught, EXCEPTION_FLT_DIVIDE_BY_ZERO))
    {
        exit(-1);
    }
}

/*!
 * The main procedure of the application.
 */
int main(int argc, char* argv[])
{
    install_signal_handlers();
    StartFunctionTest("Raise int divide by zero in the tool");
    SafeExecuteIntDivideByZero();
    EndFunctionTest("Raise int divide by zero in the tool");

    StartFunctionTest("Raise FP divide by zero in the tool");
    SafeExecuteFltDivideByZero();
    EndFunctionTest("Raise FP divide by zero in the tool");
}
