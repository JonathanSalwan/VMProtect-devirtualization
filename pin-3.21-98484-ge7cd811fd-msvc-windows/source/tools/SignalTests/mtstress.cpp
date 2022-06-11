/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This is a stress test for handling signals in a multi-threaded application.
 * The root thread creates two classes of worker threads: computers and blockers.
 * Computers execute a small loop that should run entirely from the Pin code cache
 * once it is compiled.  Blockers iteratively execute a blocking system call.  The
 * root thread randomly sends signals to each of the works as fast as possible.
 */

#include <cstdlib>
#include <iostream>
#include <pthread.h>
#include <cassert>
#include <signal.h>
#include <time.h>

const unsigned NUM_BLOCKERS  = 8;
const unsigned NUM_COMPUTERS = 8;
const unsigned NUM_SIGNALS   = 1000;

// The total number of signals received by all threads.
//
volatile unsigned NumSignalsReceived = 0;
pthread_mutex_t SignalReceivedLock   = PTHREAD_MUTEX_INITIALIZER;

// Information for each worker thread.
//
struct THREAD_INFO
{
    THREAD_INFO() : _ready(false)
    {
        pthread_mutex_init(&_readyLock, 0);
        pthread_cond_init(&_readyCond, 0);
    }

    ~THREAD_INFO()
    {
        pthread_cond_destroy(&_readyCond);
        pthread_mutex_destroy(&_readyLock);
    }

    pthread_t _tid;

    // This is set TRUE when the thread is ready to receive a signal.
    //
    bool _ready;
    pthread_mutex_t _readyLock;
    pthread_cond_t _readyCond;
};

THREAD_INFO* ThreadInfos;

// Each thread uses this key to find its own THREAD_INFO.
//
pthread_key_t MyInfoKey;

static void* BlockerRoot(void*);
static void* ComputerRoot(void*);
static void SetMyInfo(THREAD_INFO*);
static void Handle(int);

int main()
{
    struct sigaction act;
    act.sa_handler = Handle;
    act.sa_flags   = 0;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGUSR1, &act, 0) != 0)
    {
        std::cerr << "Unable to set SIGUSR1 handler\n";
        return 1;
    }

    if (pthread_key_create(&MyInfoKey, 0) != 0)
    {
        std::cerr << "Unable to create key\n";
        return 1;
    }

    // Create the worker threads.
    //
    ThreadInfos = new THREAD_INFO[NUM_BLOCKERS + NUM_COMPUTERS];
    for (unsigned i = 0; i < NUM_BLOCKERS; i++)
    {
        THREAD_INFO* info = &ThreadInfos[i];
        if (pthread_create(&info->_tid, 0, BlockerRoot, info) != 0)
        {
            std::cerr << "Unable to create blocker thread\n";
            return 1;
        }
    }
    for (unsigned i = 0; i < NUM_COMPUTERS; i++)
    {
        THREAD_INFO* info = &ThreadInfos[NUM_BLOCKERS + i];
        if (pthread_create(&info->_tid, 0, ComputerRoot, info) != 0)
        {
            std::cerr << "Unable to create computer thread\n";
            return 1;
        }
    }

    // Randomly send signals to the workers.
    //
    for (unsigned i = 0; i < NUM_SIGNALS; i++)
    {
        unsigned index    = std::rand() % (NUM_BLOCKERS + NUM_COMPUTERS);
        THREAD_INFO* info = &ThreadInfos[index];

        // Wait for the worker to be ready to handle a signal.
        //
        pthread_mutex_lock(&info->_readyLock);
        while (!info->_ready)
            pthread_cond_wait(&info->_readyCond, &info->_readyLock);
        info->_ready = false;
        pthread_mutex_unlock(&info->_readyLock);

        std::cout << "Sending signal " << std::dec << i << "\n";
        if (pthread_kill(info->_tid, SIGUSR1) != 0)
        {
            std::cerr << "Unable to send SIGUSR1 to thread index " << std::dec << index << "\n";
            return 1;
        }
    }

    // Wait for all the workers to terminate.
    //
    for (unsigned i = 0; i < NUM_BLOCKERS + NUM_COMPUTERS; i++)
    {
        if (pthread_join(ThreadInfos[i]._tid, 0) != 0)
        {
            std::cerr << "Unable to wait for thread index " << std::dec << i << "\n";
            return 1;
        }
    }

    delete[] ThreadInfos;
    pthread_key_delete(MyInfoKey);
    return 0;
}

static void* BlockerRoot(void* vinfo)
{
    SetMyInfo(static_cast< THREAD_INFO* >(vinfo));

    while (NumSignalsReceived < NUM_SIGNALS)
    {
        struct timespec tv;
        tv.tv_sec  = 10;
        tv.tv_nsec = 0;
        nanosleep(&tv, 0);
    }
    return 0;
}

static void* ComputerRoot(void* vinfo)
{
    SetMyInfo(static_cast< THREAD_INFO* >(vinfo));

    volatile double x[100];
    while (NumSignalsReceived < NUM_SIGNALS)
    {
        for (unsigned i = 0; i < 100; i++)
            x[i] = (double)(i + 1);
        for (unsigned i = 2; i < 100; i++)
            x[i] = x[i] / x[i - 1] * x[i - 2] + x[i];
    }
}

static void SetMyInfo(THREAD_INFO* info)
{
    // Initialize the worker's thread-private data to point to it's THREAD_INFO.
    // Once we do this, we are ready to handle a signal.
    //
    pthread_mutex_lock(&info->_readyLock);
    pthread_setspecific(MyInfoKey, info);
    info->_ready = true;
    pthread_cond_signal(&info->_readyCond);
    pthread_mutex_unlock(&info->_readyLock);
}

static void Handle(int)
{
    // Count this signal.
    //
    pthread_mutex_lock(&SignalReceivedLock);
    NumSignalsReceived++;
    pthread_mutex_unlock(&SignalReceivedLock);

    // Once we count the signal, this thread is ready to handle another signal.
    //
    THREAD_INFO* info = static_cast< THREAD_INFO* >(pthread_getspecific(MyInfoKey));
    pthread_mutex_lock(&info->_readyLock);
    assert(!info->_ready);
    info->_ready = true;
    pthread_cond_signal(&info->_readyCond);
    pthread_mutex_unlock(&info->_readyLock);
}
