/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  This application attaches Pin in the middle of signal handler.
 * The test verifies RT signal frame save/restore inside Pin
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
using std::cerr;
using std::cout;
using std::endl;
using std::hex;
using std::list;
using std::string;
#if defined(TARGET_MAC)
#include <sys/ucontext.h>
#else
#include <ucontext.h>
#endif
#include <sched.h>
#include <sys/utsname.h>
#include <signal.h>
#include <stdint.h>
#include "../Utils/threadlib.h"

list< string > pinArgs;

volatile bool sigHandled = false;

const long xmm1_app = 0x1111;
const long xmm2_app = 0x2222;
const long xmm3_app = 0x3333;

const long xmm1_sig = 0x4444;
const long xmm2_sig = 0x5555;
const long xmm3_sig = 0x6666;

extern "C" void SetXmmRegs(long v1, long v2, long v3);
extern "C" void GetXmmRegs(long* v1, long* v2, long* v3);

void AttachAndInstrument();
extern "C" int ThreadsReady(unsigned int numOfThreads) { return 0; }

#ifdef TARGET_LINUX

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

long GetXmmRegFromMctxt(mcontext_t* mctxt, int xmmIdx)
{
    long out;
    KernelFpstate* appFpState = reinterpret_cast< KernelFpstate* >(mctxt->fpregs);
    memcpy(&out, appFpState->_fxsave._xmm + 16 * xmmIdx, sizeof(out));
    return out;
}

void SetXmmRegInMctxt(mcontext_t* mctxt, int xmmIdx, long val)
{
    KernelFpstate* appFpState = reinterpret_cast< KernelFpstate* >(mctxt->fpregs);
    memcpy(appFpState->_fxsave._xmm + 16 * xmmIdx, &val, sizeof(val));
}
#elif defined(TARGET_MAC)
long GetXmmRegFromMctxt(mcontext_t* mctxt, int xmmIdx)
{
    long out;
    memcpy(&out, ((char*)&(*mctxt)->__fs.__fpu_xmm0) + 16 * xmmIdx, sizeof(out));
    return out;
}

void SetXmmRegInMctxt(mcontext_t* mctxt, int xmmIdx, long val)
{
    memcpy(((char*)&(*mctxt)->__fs.__fpu_xmm0) + 16 * xmmIdx, &val, sizeof(val));
}
#endif

void SigUsr1Handler(int signum, siginfo_t* siginfo, void* uctxt)
{
    AttachAndInstrument();

    // Give enough time for all threads to get started
    while (!ThreadsReady(1))
    {
        sched_yield();
    }
    mcontext_t* mContext = &reinterpret_cast< ucontext_t* >(uctxt)->uc_mcontext;

    long appContextXmm1 = GetXmmRegFromMctxt(mContext, 1);
    long appContextXmm2 = GetXmmRegFromMctxt(mContext, 2);
    long appContextXmm3 = GetXmmRegFromMctxt(mContext, 3);

    if ((appContextXmm1 != xmm1_app) || (appContextXmm2 != xmm2_app) || (appContextXmm3 != xmm3_app))
    {
        cerr << "Unexpected xmm values in signal handler: " << hex << endl;
        cerr << "xmm1 = " << appContextXmm1 << ", Expected " << xmm1_app << endl;
        cerr << "xmm2 = " << appContextXmm2 << ", Expected " << xmm1_app << endl;
        cerr << "xmm3 = " << appContextXmm3 << ", Expected " << xmm1_app << endl;
        exit(-1);
    }

    SetXmmRegInMctxt(mContext, 1, xmm1_sig);
    SetXmmRegInMctxt(mContext, 2, xmm2_sig);
    SetXmmRegInMctxt(mContext, 3, xmm3_sig);

    sigHandled = true;
}

void SigUsr2Handler(int signum)
{
    AttachAndInstrument();

    // Give enough time for all threads to get started
    while (!ThreadsReady(1))
    {
        sched_yield();
    }
    sigHandled = true;
}

#define DECSTR(buf, num)         \
    {                            \
        buf = (char*)malloc(10); \
        sprintf(buf, "%d", num); \
    }

inline void PrintArguments(char** inArgv)
{
    fprintf(stderr, "Going to run: ");
    for (unsigned int i = 0; inArgv[i] != 0; ++i)
    {
        fprintf(stderr, "%s ", inArgv[i]);
    }
    fprintf(stderr, "\n");
}

/* AttachAndInstrument()
 * a special routine that runs $PIN
 */
void AttachAndInstrument()
{
    list< string >::iterator pinArgIt = pinArgs.begin();

    string pinBinary = *pinArgIt;
    pinArgIt++;

    pid_t parent_pid = getpid();

    pid_t child = fork();

    if (child)
    {
        fprintf(stderr, "Pin injector pid %d\n", child);
        // inside parent
        return;
    }
    else
    {
        // inside child

        char** inArgv = new char*[pinArgs.size() + 10];

        unsigned int idx = 0;
        inArgv[idx++]    = (char*)pinBinary.c_str();
        inArgv[idx++]    = (char*)"-pid";
        inArgv[idx]      = (char*)malloc(10);
        sprintf(inArgv[idx++], "%d", parent_pid);

        for (; pinArgIt != pinArgs.end(); pinArgIt++)
        {
            inArgv[idx++] = (char*)pinArgIt->c_str();
        }
        inArgv[idx] = 0;

        PrintArguments(inArgv);

        execvp(inArgv[0], inArgv);
        fprintf(stderr, "ERROR: execv %s failed\n", inArgv[0]);
        kill(parent_pid, 9);
        return;
    }
}

/*
 * Expected command line: <this exe> [-test NUM] -pin $PIN -pinarg <pin args > -t tool <tool args>
 */

void ParseCommandLine(int argc, char* argv[], list< string >* pinArgs, unsigned int* testNo)
{
    string pinBinary;
    for (int i = 1; i < argc; i++)
    {
        string arg = string(argv[i]);
        if (arg == "-test")
        {
            *testNo = atoi(argv[++i]);
        }
        else if (arg == "-pin")
        {
            pinBinary = argv[++i];
        }
        else if (arg == "-pinarg")
        {
            for (int parg = ++i; parg < argc; parg++)
            {
                pinArgs->push_back(string(argv[parg]));
                ++i;
            }
        }
    }
    assert(!pinBinary.empty());
    pinArgs->push_front(pinBinary);
}

int TestRtSigframe();
int TestSigframe();

int main(int argc, char* argv[])
{
    unsigned int testNo = 0;
    ParseCommandLine(argc, argv, &pinArgs, &testNo);

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
