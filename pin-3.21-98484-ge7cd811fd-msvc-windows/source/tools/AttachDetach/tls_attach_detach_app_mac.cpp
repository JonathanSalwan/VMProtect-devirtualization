/*
 * Copyright (C) 2017-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
*  Test TLS value before attach, after attach and after detach
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <string>
#include <list>
#include <sstream>
#include <sys/utsname.h>
#include <mach/mach.h>

using std::endl;
using std::list;
using std::ostringstream;
using std::string;

#define NTHREADS 4

int syncPipe[2];
semaphore_t thread_created_sem;
pthread_mutex_t gMutex                  = PTHREAD_MUTEX_INITIALIZER;
unsigned int numOfThreadsReadyForDetach = 0;
volatile unsigned long pinDetached      = false;

extern "C" void TellPinToDetach(volatile unsigned long* updateWhenReady) { return; }

extern "C" int PinAttached() { return 0; }

void* ReadTLSBase()
{
    void* tlsBase;
#if defined(TARGET_IA32)
    asm("mov %%gs:0, %%eax\n"
        "mov %%eax, %0"
        : "=r"(tlsBase)
        :
        : "eax");
#else
    asm("mov %%gs:0, %%rax\n"
        "mov %%rax, %0"
        : "=r"(tlsBase)
        :
        : "rax");
#endif
    return tlsBase;
}

/*
 * Compare TLS_BASE values before and after detach.
 */
void* thread_func(void* arg)
{
    unsigned long thread_no = (unsigned long)arg + 1;

    void* tlsBase = 0;
    tlsBase       = ReadTLSBase();
    fprintf(stderr, "tls base in thread %lu: %p\n", thread_no, tlsBase);

    kern_return_t kret = semaphore_signal(thread_created_sem);
    assert(KERN_SUCCESS == kret);

    while (!PinAttached())
    {
        sleep(1);
    }

    void* tlsBaseAfterAttach = 0;
    tlsBaseAfterAttach       = ReadTLSBase();
    if (tlsBase != tlsBaseAfterAttach)
    {
        fprintf(stderr, "ERROR in thread %lu: GTLSBASE before attach %p; after attach %p\n", thread_no, tlsBase,
                tlsBaseAfterAttach);
        return (void*)1;
    }

    pthread_mutex_lock(&gMutex);
    numOfThreadsReadyForDetach++;
    pthread_mutex_unlock(&gMutex);

    while (!pinDetached)
    {
        sched_yield();
    }

    void* tlsBaseAfterDetach = 0;
    tlsBaseAfterDetach       = ReadTLSBase();
    if (tlsBase != tlsBaseAfterDetach)
    {
        fprintf(stderr, "ERROR in thread %lu: GTLSBASE before detach %p; after detach %p\n", thread_no, tlsBase,
                tlsBaseAfterDetach);
        return (void*)1;
    }
    return 0;
}

void PrintArguments(const char** inArgv)
{
    printf("Going to run: ");
    for (unsigned int i = 0; inArgv[i] != 0; ++i)
    {
        printf("%s ", inArgv[i]);
    }
    printf("\n");
}

/*
 * Expected command line: <this exe> -pin $PIN -pinarg <pin args > -t tool <tool args>
 */

void ParseCommandLine(int argc, char* argv[], list< string >* pinArgs)
{
    string pinBinary;
    bool slow_asserts = false;
    for (int i = 1; i < argc; i++)
    {
        string arg = string(argv[i]);
        if (arg == "-pin")
        {
            pinBinary = argv[++i];
        }
        else if (arg == "-slow_asserts")
        {
            slow_asserts = true;
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
    if (slow_asserts) pinArgs->push_front("-slow_asserts");
    pinArgs->push_front(pinBinary);
}

template< class T > string my_to_string(const T& v)
{
    ostringstream oss;
    oss << v;
    return oss.str();
}

void StartPin(list< string >* pinArgs)
{
    pid_t appPid = getpid();
    int res      = pipe(syncPipe);
    assert(res >= 0);
    pid_t child = fork();
    assert(child >= 0);
    if (child != 0)
    {
        close(syncPipe[0]);
        return;
    }

    /* here is the child */
    close(syncPipe[1]);

    // start Pin from child
    const char** inArgv = new const char*[pinArgs->size() + 10];

    // Pin binary in the first
    list< string >::iterator pinArgIt = pinArgs->begin();
    string pinBinary                  = *pinArgIt;
    string pidString                  = my_to_string(appPid);
    pinArgIt++;

    // build pin arguments:
    unsigned int idx = 0;
    inArgv[idx++]    = (char*)pinBinary.c_str();
    inArgv[idx++]    = (char*)"-pid";
    inArgv[idx++]    = pidString.c_str();

    for (; pinArgIt != pinArgs->end(); pinArgIt++)
    {
        inArgv[idx++] = (char*)pinArgIt->c_str();
    }
    inArgv[idx] = 0;

    do
    {
        char dummy;
        res = read(syncPipe[0], &dummy, sizeof(dummy));
    }
    while (res < 0 && errno == EINTR);
    assert(res == 0);

    PrintArguments(inArgv);

    execvp(inArgv[0], (char* const*)inArgv);
    printf("ERROR: execv %s failed\n", inArgv[0]);

    exit(1);
}

int main(int argc, char* argv[])
{
    kern_return_t kret;
    pthread_t h[NTHREADS];
    list< string > pinArgs;

    kret = semaphore_create(mach_task_self(), &thread_created_sem, SYNC_POLICY_FIFO, 0);
    assert(KERN_SUCCESS == kret);

    ParseCommandLine(argc, argv, &pinArgs);

    StartPin(&pinArgs);

    // Save tls base before attach

    void* tlsBase = 0;
    tlsBase       = ReadTLSBase();
    fprintf(stderr, "tls base in main thread: %p\n", tlsBase);

    // Create threads

    for (unsigned long i = 0; i < NTHREADS; i++)
    {
        pthread_create(&h[i], 0, thread_func, (void*)i);
        kret = semaphore_wait(thread_created_sem);
        assert(KERN_SUCCESS == kret);
    }

    //
    // Attach
    //

    // Signal child process to attach Pin to the current process
    close(syncPipe[1]);
    printf("Attaching Pin\n");

    while (!PinAttached())
    {
        sleep(1);
    }
    printf("Pin attached\n");

    // Check tls base after attach

    void* tlsBaseAfterAttach = 0;
    tlsBaseAfterAttach       = ReadTLSBase();
    if (tlsBase != tlsBaseAfterAttach)
    {
        fprintf(stderr, "ERROR in the main thread: TLS_BASE before attach %p; after attach %p\n", tlsBase, tlsBaseAfterAttach);
        return -1;
    }

    // Wait for all threads to be ready for detach

    bool readyForDetach = false;
    do
    {
        unsigned int numOfThreadsReady = 0;
        pthread_mutex_lock(&gMutex);
        numOfThreadsReady = numOfThreadsReadyForDetach;
        pthread_mutex_unlock(&gMutex);
        if (numOfThreadsReady == NTHREADS)
        {
            readyForDetach = true;
        }
        else
        {
            sleep(1);
        }
    }
    while (!readyForDetach);

    printf("Tls base of all threads remains the same after attach\n");

    //
    // Detach
    //

    printf("Requesting Pin to detach\n");
    TellPinToDetach(&pinDetached);

    while (!pinDetached)
    {
        sleep(2);
    }
    printf("Detach completed\n");
    usleep(500000); // 500ms

    //
    // Pin detached from application
    //

    // Check tls base after detach

    void* tlsBaseAfterDetach = 0;
    tlsBaseAfterDetach       = ReadTLSBase();
    if (tlsBase != tlsBaseAfterDetach)
    {
        fprintf(stderr, "ERROR in the main thread: TLS_BASE before detach %p; after detach %p\n", tlsBase, tlsBaseAfterDetach);
        return -1;
    }

    // Wait for threads to exit

    void* result[NTHREADS];
    for (unsigned long i = 0; i < NTHREADS; i++)
    {
        pthread_join(h[i], &(result[i]));
    }
    for (unsigned long i = 0; i < NTHREADS; i++)
    {
        if (result[i] != 0)
        {
            fprintf(stderr, "TEST FAILED\n");
            return -1;
        }
    }

    printf("Tls base of all threads remains the same after detach\n");

    printf("All threads exited. The test PASSED\n");
    return 0;
}
