/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  The test verifies that FP state of thread is not changed by Pin injection
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string>
#include <list>
#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <stdint.h>
#include "../Utils/threadlib.h"
using std::list;
using std::string;

struct ThreadLock
{
    unsigned long _tid;
};

extern "C" void InitLock(ThreadLock* lock);
extern "C" void GetLock(ThreadLock* lock, unsigned long tid);
extern "C" void ReleaseLock(ThreadLock* lock);

/* 
 * The total number of threads that should run in this process
 * The number may be changed in command line with -th_num
 */
unsigned int numOfSecondaryThreads = 4;

// Used to sync between parent and child process: When to start the injection
int syncPipe[2];

#define FP_STATE_SIZE 512
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
    unsigned char _reserved[56 * 4];
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
    unsigned int _reserved[24];
};

#endif

struct KernelFpstate
{
    struct fxsave _fxsave; // user-visible FP register state (_mcontext points to this)
};

extern "C" void ReadFpContext(unsigned char* buf);

int CompareFpStates(unsigned char* fpBuf1, unsigned char* fpBuf2)
{
    KernelFpstate* fpState1 = reinterpret_cast< KernelFpstate* >(fpBuf1);
    KernelFpstate* fpState2 = reinterpret_cast< KernelFpstate* >(fpBuf2);
#if defined(TARGET_LINUX) && defined(TARGET_IA32)
    // On some Linux kernels, sysenter zeroes the last CS and DS selector in the FPU.
    // This is not a bug in Pin - so we ignore these values (i.e.: always set them to zero).
    fpState1->_fxsave._cs = fpState1->_fxsave._ds = 0;
    fpState2->_fxsave._cs = fpState2->_fxsave._ds = 0;
#endif
    return memcmp(&(fpState1->_fxsave), &(fpState2->_fxsave), sizeof(fpState1->_fxsave) - sizeof(fpState1->_fxsave._reserved));
}

void PrintFpState(unsigned long tid, unsigned char* fpBuf)
{
    KernelFpstate* fpState = reinterpret_cast< KernelFpstate* >(fpBuf);
    unsigned int* buf      = (unsigned int*)&(fpState->_fxsave);
    unsigned int bufSize =
        sizeof(struct fxsave) - sizeof(fpState->_fxsave._reserved) - sizeof(fpState->_fxsave._xmm) - sizeof(fpState->_fxsave._st);
    for (unsigned int i = 0; i < bufSize / sizeof(unsigned int); i++)
    {
        fprintf(stderr, "%08x ", buf[i]);
    }
    fprintf(stderr, "\n");
    // print fp registers
    buf     = (unsigned int*)&(fpState->_fxsave._st);
    bufSize = sizeof(fpState->_fxsave._st);

    for (unsigned int i = 0; i < bufSize / 16; i++)
    {
        fprintf(stderr, "st%d = %08x %08x %08x %08x ", i, buf[i * 4 + 0], buf[i * 4 + 1], buf[i * 4 + 2], buf[i * 4 + 3]);
    }
    fprintf(stderr, "\n");
    // print xmm registers
    buf     = (unsigned int*)&(fpState->_fxsave._xmm);
    bufSize = sizeof(fpState->_fxsave._xmm);

    for (unsigned int i = 0; i < bufSize / 16; i++)
    {
        fprintf(stderr, "xmm%d = %08x %08x %08x %08x ", i, buf[i * 4 + 0], buf[i * 4 + 1], buf[i * 4 + 2], buf[i * 4 + 3]);
    }
    fprintf(stderr, "\n");
}

bool waitForPin                        = true;
volatile unsigned int secThreadStarted = 0;
ThreadLock mutex;

void* ThreadFunc(void* arg)
{
    unsigned char* buf1 = new unsigned char[FP_STATE_SIZE + 16];
    // align 16
    unsigned char* fpstateBuf1 = (unsigned char*)((((long)buf1 + 16) >> 4) << 4);
    memset(fpstateBuf1, 0, FP_STATE_SIZE);

    unsigned char* buf2 = new unsigned char[FP_STATE_SIZE + 16];
    // align 16
    unsigned char* fpstateBuf2 = (unsigned char*)((((long)buf2 + 16) >> 4) << 4);
    memset(fpstateBuf2, 0, FP_STATE_SIZE);

    // Do not call any routine that can change FP state
    // between two reads:

    // This is the first read
    ReadFpContext(fpstateBuf1);

    GetLock(&mutex, GetTid());
    ++secThreadStarted;
    ReleaseLock(&mutex);

    while (waitForPin)
    {
        sched_yield();
    }

    // This is the second read
    ReadFpContext(fpstateBuf2);

    unsigned long res;
    GetLock(&mutex, GetTid());
    if (CompareFpStates(fpstateBuf1, fpstateBuf2))
    {
        printf("Fp state was changed in thread %lu\n", GetTid());
        fprintf(stderr, "\n\nApplication FP state for thread %lu:\n", GetTid());
        PrintFpState(GetTid(), fpstateBuf1);

        fprintf(stderr, "\n\nApplication+Pin FP state for thread %lu:\n", GetTid());
        PrintFpState(GetTid(), fpstateBuf2);
        res = 0;
    }
    else
    {
        res = 1;
    }
    ReleaseLock(&mutex);
    delete[] buf1;
    delete[] buf2;
    return (void*)res;
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
void AttachAndInstrument(list< string >* pinArgs)
{
    int res;
    list< string >::iterator pinArgIt = pinArgs->begin();

    string pinBinary = *pinArgIt;
    pinArgIt++;

    pid_t parent_pid = getppid();

    char** inArgv = new char*[pinArgs->size() + 10];

    unsigned int idx = 0;
    inArgv[idx++]    = (char*)pinBinary.c_str();
    inArgv[idx++]    = (char*)"-pid";
    inArgv[idx]      = (char*)malloc(10);
    sprintf(inArgv[idx++], "%d", parent_pid);

    for (; pinArgIt != pinArgs->end(); pinArgIt++)
    {
        inArgv[idx++] = (char*)pinArgIt->c_str();
    }
    inArgv[idx] = 0;

    // Wait for parent to notify injection can start
    do
    {
        char dummy;
        res = read(syncPipe[0], &dummy, sizeof(dummy));
    }
    while (res < 0 && errno == EINTR);
    assert(res == 0);

    PrintArguments(inArgv);

    execvp(inArgv[0], inArgv);
    fprintf(stderr, "ERROR: execv %s failed\n", inArgv[0]);
    kill(parent_pid, 9);
    return;
}

/*
 * Expected command line: <this exe> [-th_num NUM] -pin $PIN -pinarg <pin args > -t tool <tool args>
 */

void ParseCommandLine(int argc, char* argv[], list< string >* pinArgs)
{
    string pinBinary;
    for (int i = 1; i < argc; i++)
    {
        string arg = string(argv[i]);
        if (arg == "-th_num")
        {
            numOfSecondaryThreads = atoi(argv[++i]) - 1;
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
    if (pinBinary.empty())
    {
        fprintf(stderr, "-pin parameter should be specified\n");
    }
    else
    {
        pinArgs->push_front(pinBinary);
    }
}

pthread_t* thHandle;
// This function should be replaced by Pin tool.
extern "C" int ThreadsReady(unsigned int numOfThreads)
{
    assert(numOfThreads == numOfSecondaryThreads + 1);
    return 0;
}

int main(int argc, char* argv[])
{
    list< string > pinArgs;
    ParseCommandLine(argc, argv, &pinArgs);

    // initialize a mutex that will be used by threads
    InitLock(&mutex);

    thHandle = new pthread_t[numOfSecondaryThreads];

    // start all secondary threads
    for (uintptr_t i = 0; i < (uintptr_t)numOfSecondaryThreads; i++)
    {
        pthread_create(&thHandle[i], 0, ThreadFunc, (void*)i);
    }

    while (secThreadStarted < numOfSecondaryThreads)
    {
        sched_yield();
    }

    /*
     * Allocate 2 buffers for FP state. The first buffer is filled before attach.
     * The second is just after.
     * We should avoid any operation that change XMM registers between two reads
    */
    float a = 3.5;
    float b = a / 5.003;

    char fpbuf[100];
    sprintf(fpbuf, "%f", b);

    unsigned char* buf1 = new unsigned char[FP_STATE_SIZE + 16];
    // align 16
    unsigned char* fpstateBuf1 = (unsigned char*)((((long)buf1 + 16) >> 4) << 4);
    memset(fpstateBuf1, 0, FP_STATE_SIZE);

    unsigned char* buf2 = new unsigned char[FP_STATE_SIZE + 16];
    // align 16
    unsigned char* fpstateBuf2 = (unsigned char*)((((long)buf2 + 16) >> 4) << 4);
    memset(fpstateBuf2, 0, FP_STATE_SIZE);

    int res = pipe(syncPipe);
    assert(res >= 0);

    pid_t child = fork();

    if (child == 0)
    {
        // Child

        close(syncPipe[1]);
        AttachAndInstrument(&pinArgs);
    }
    else
    {
        // Parent

        close(syncPipe[0]);
        // === First read is here
        ReadFpContext(fpstateBuf1);

        // Assumption: From this point until ReadFpContext() is called again XMM registers are not changed

        // Notify child process that it can start the injection of PIN after we saved the FP state.
        // Note! Using fork instead of this pipe mechanism is not good since when using fork() we must first
        // save FP state and then do fork(), fork() may change XMM state (been seen on some machines),
        // in that case FP state compare will fail on main thread (even though injecting didn't change FP state).
        close(syncPipe[1]);
    }

    // Give enough time for all threads to get started
    while (!ThreadsReady(numOfSecondaryThreads + 1))
    {
        sched_yield();
    }

    // === Second read is here
    ReadFpContext(fpstateBuf2);

    // tell other threads that Pin is attached
    waitForPin = false;

    // now all secondary threads should exit
    // the returned value is not 0 if FP state wasn't correctly set
    // after Pin attach

    bool result = true;
    for (unsigned int i = 0; i < numOfSecondaryThreads; i++)
    {
        void* threadRetVal;
        pthread_join(thHandle[i], &threadRetVal);
        if (threadRetVal != (void*)1)
        {
            result = false;
        }
    }
    if (!result)
    {
        printf("ERROR: FP registers are changed after Pin attach\n");
        return -1;
    }

    // Check the main thread

    if (CompareFpStates(fpstateBuf1, fpstateBuf2))
    {
        printf("Fp state was changed in the main thread %lu\n", GetTid());
        fprintf(stderr, "\n\nApplication FP state for thread %lu:\n", GetTid());
        PrintFpState(GetTid(), fpstateBuf1);
        fprintf(stderr, "\n\nApplication+Pin FP state for thread %lu:\n", GetTid());
        PrintFpState(GetTid(), fpstateBuf2);
        return -1;
    }

    delete[] buf1;
    delete[] buf2;

    printf("SUCCESS: FP registers are preserved after Pin attach\n");
    return 0;
}
