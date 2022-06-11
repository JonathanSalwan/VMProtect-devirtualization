/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Pool of threads.
 */
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "../Utils/runnable.h"
#include "../Utils/threadlib.h"

/*!
 * Runnable object that exits the current thread.
 */
class EXIT_THREAD_OBJ : public RUNNABLE_OBJ
{
  public:
    void Run() { ExitCurrentThread(); }
};

/*!
 * Pool of threads that can be used to execute runnable objects.
 * External access to this singleton must be serialized. It is guaranteed
 * if the thread pool is managed by a single (main) thread of the process.
 */
class THREAD_POOL
{
  public:
    // Constructor
    THREAD_POOL() : m_numThreads(0) {}

    // Destructor
    ~THREAD_POOL() { TerminateAll(); }

    // Create specified number of new threads in the pool.
    // @return number of threads created successfully
    unsigned long Create(unsigned long numThreads);

    // Start running the specified object in the specified thread.
    // The function is non-blocking - it does not wait for the completion
    // of the RUNNABLE_OBJ::Run() function.
    // @return TRUE - success, FALSE - failure
    bool Start(unsigned long tid, RUNNABLE_OBJ* runObj);

    // Block the current thread until a runnable object, assigned to the specified thread,
    // completes its function,
    // @return runnable object assigned to the thread or NULL.
    RUNNABLE_OBJ* Wait(unsigned long tid);

    unsigned long NumThreads() const { return m_numThreads; }

    // Terminate all threads in the pool.
    void TerminateAll();

  private:
    unsigned long m_numThreads;

    struct TLS_ELEMENT
    {
        THREAD_HANDLE m_handle;
        RUNNABLE_OBJ* volatile m_runObj;
        volatile bool m_semaphore;

        // Initialize TLS element
        void Init()
        {
            m_handle    = 0;
            m_runObj    = 0;
            m_semaphore = false;
        }

        // Semaphore manipulation functions
        void SwitchSemaphore(bool semaphoreState);
        void WaitSemaphore(bool semaphoreState) const;
        bool CheckSemaphore() const;

    } m_tls[MAXTHREADS];

    //Disable copy constructor and assignment operator
    THREAD_POOL(const THREAD_POOL&);
    THREAD_POOL& operator=(const THREAD_POOL&);

    // Main routine of threads in the pool
    static void* ThreadRoutine(void* tlsArg);
};

#endif //THREAD_POOL_H

/* ===================================================================== */
/* eof */
/* ===================================================================== */
