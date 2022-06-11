/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This application detaches Pin in the middle of signal handler.
 * The test verifies signal frame after Pin detach
 */
#include <assert.h>
#include <stdio.h>
#include <dlfcn.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <list>
#include <ucontext.h>
using std::cerr;
using std::cout;
using std::endl;
using std::hex;
using std::string;

#include <sched.h>
#include <sys/utsname.h>

volatile bool sigHandled = false;

const long xmm1_app = 0x1111;
const long xmm2_app = 0x2222;
const long xmm3_app = 0x3333;

const long xmm1_sig = 0x4444;
const long xmm2_sig = 0x5555;
const long xmm3_sig = 0x6666;

extern "C" void SetXmmRegs(long v1, long v2, long v3);
extern "C" void GetXmmRegs(long* v1, long* v2, long* v3);

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

/* This function should be replaced by Pin tool */
extern "C" int ThreadHoldByPin() { return 0; }

/* This function should be replaced by Pin tool */
extern "C" void DetachPin()
{
    fprintf(stderr, "This function shouldn't be called form application\n");
    assert(false);
    return;
}

/*
 * A signal handler for canceling all threads
 */
void SigUsr1Handler(int signum, siginfo_t* siginfo, void* uctxt)
{
    DetachPin();

    // Wait until Pin is detached
    while (ThreadHoldByPin())
    {
        sched_yield();
    }

    ucontext_t* frameContext = (ucontext_t*)uctxt;

    /* Change application FP context */
    fpregset_t fpPtr = frameContext->uc_mcontext.fpregs;

    KernelFpstate* appFpState = reinterpret_cast< KernelFpstate* >(fpPtr);

    long appContextXmm1 = 0;
    memcpy(&appContextXmm1, appFpState->_fxsave._xmm + 16, sizeof(appContextXmm1));
    long appContextXmm2 = 0;
    memcpy(&appContextXmm2, appFpState->_fxsave._xmm + 32, sizeof(appContextXmm2));
    long appContextXmm3 = 0;
    memcpy(&appContextXmm3, appFpState->_fxsave._xmm + 48, sizeof(appContextXmm3));

    if ((appContextXmm1 != xmm1_app) || (appContextXmm2 != xmm2_app) || (appContextXmm3 != xmm3_app))
    {
        cerr << "Unexpected xmm values in signal handler: " << hex << endl;
        cerr << "xmm1 = " << appContextXmm1 << ", Expected " << xmm1_app << endl;
        cerr << "xmm2 = " << appContextXmm2 << ", Expected " << xmm2_app << endl;
        cerr << "xmm3 = " << appContextXmm3 << ", Expected " << xmm3_app << endl;
        exit(-1);
    }

    memcpy(appFpState->_fxsave._xmm + 16, &xmm1_sig, sizeof(xmm1_sig));
    memcpy(appFpState->_fxsave._xmm + 32, &xmm2_sig, sizeof(xmm2_sig));
    memcpy(appFpState->_fxsave._xmm + 48, &xmm3_sig, sizeof(xmm3_sig));

    sigHandled = true;
}

void SigUsr2Handler(int signum)
{
    DetachPin();

    // Wait until Pin is detached
    while (ThreadHoldByPin())
    {
        sched_yield();
    }

    sigHandled = true;
}

/*
 * Expected command line: <this exe> [-test NUM]
 */

void ParseCommandLine(int argc, char* argv[], unsigned int* testNo)
{
    for (int i = 1; i < argc; i++)
    {
        string arg = string(argv[i]);
        if (arg == "-test")
        {
            *testNo = atoi(argv[++i]);
        }
    }
}

/*
 * 2 tests: for default frame and for RT frame
 */

int TestRtSigframe();
int TestSigframe();

int main(int argc, char* argv[])
{
    unsigned int testNo = 0;
    ParseCommandLine(argc, argv, &testNo);

    if (testNo == 0)
    {
        return TestRtSigframe();
    }
    else
    {
        return TestSigframe();
    }
    return 0;
}

int TestRtSigframe()
{
    struct sigaction sSigaction;

    /* Register the signal hander using the siginfo interface*/
    sSigaction.sa_sigaction = SigUsr1Handler;
    sSigaction.sa_flags     = SA_SIGINFO;

    /* mask all other signals */
    sigfillset(&sSigaction.sa_mask);

    int ret = sigaction(SIGUSR1, &sSigaction, NULL);
    if (ret)
    {
        perror("ERROR, sigaction failed");
        exit(-1);
    }

    SetXmmRegs(xmm1_app, xmm2_app, xmm3_app);
    kill(getpid(), SIGUSR1);

    while (!sigHandled)
    {
        sched_yield();
    }

    long xmm1, xmm2, xmm3;
    GetXmmRegs(&xmm1, &xmm2, &xmm3);

    if ((xmm1 != xmm1_sig) || (xmm2 != xmm2_sig) || (xmm3 != xmm3_sig))
    {
        cerr << "Unexpected xmm values after return from signal handler: " << hex << endl;
        cerr << "xmm1 = " << xmm1 << ", Expected " << xmm1_sig << endl;
        cerr << "xmm2 = " << xmm2 << ", Expected " << xmm2_sig << endl;
        cerr << "xmm3 = " << xmm3 << ", Expected " << xmm3_sig << endl;
        return -1;
    }
    cout << "All xmm values are correct" << endl;

    return 0;
}

int TestSigframe()
{
    signal(SIGUSR2, SigUsr2Handler);

    SetXmmRegs(xmm1_app, xmm2_app, xmm3_app);
    kill(getpid(), SIGUSR2);

    while (!sigHandled)
    {
        sched_yield();
    }
    long xmm1, xmm2, xmm3;
    GetXmmRegs(&xmm1, &xmm2, &xmm3);

    if ((xmm1 != xmm1_app) || (xmm2 != xmm2_app) || (xmm3 != xmm3_app))
    {
        cerr << "Unexpected xmm values after return from signal handler: " << hex << endl;
        cerr << "xmm1 = " << xmm1 << ", Expected " << xmm1_app << endl;
        cerr << "xmm2 = " << xmm2 << ", Expected " << xmm2_app << endl;
        cerr << "xmm3 = " << xmm3 << ", Expected " << xmm3_app << endl;
        return -1;
    }

    cout << "All xmm values are correct" << endl;

    return 0;
}
