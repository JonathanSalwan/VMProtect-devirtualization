/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Pool of threads.
 */

#include "thread_pool.h"

//=======================================================================
// Implementation of the THREAD_POOL class
//=======================================================================

unsigned long THREAD_POOL::Create(unsigned long numThreads)
{
    unsigned long count;
    for (count = 0; (count < numThreads) && (m_numThreads < MAXTHREADS); ++count, ++m_numThreads)
    {
        TLS_ELEMENT* tls = &(m_tls[m_numThreads]);
        tls->Init();
        BOOL created = CreateOneThread(&(tls->m_handle), ThreadRoutine, tls);
        if (!created)
        {
            break;
        }
    }
    return count;
}

void THREAD_POOL::TerminateAll()
{
    EXIT_THREAD_OBJ exitObj;
    for (unsigned long tid = 0; tid < m_numThreads; ++tid)
    {
        Wait(tid);
        Start(tid, &exitObj); // run EXIT_THREAD_OBJ in the target thread to exit the thread
        JoinOneThread(m_tls[tid].m_handle);
    }
    m_numThreads = 0;
}

bool THREAD_POOL::Start(unsigned long tid, RUNNABLE_OBJ* runObj)
{
    if (tid >= m_numThreads)
    {
        return false;
    }

    TLS_ELEMENT* tls = &(m_tls[tid]);
    if (tls->CheckSemaphore() == true)
    {
        return false; // can not start a new task until a previous one is not yet completed
    }

    // switch control to the specified thread in the pool
    tls->m_runObj = runObj;
    tls->SwitchSemaphore(true);
    return true;
}

RUNNABLE_OBJ* THREAD_POOL::Wait(unsigned long tid)
{
    if (tid >= m_numThreads)
    {
        return 0;
    }

    TLS_ELEMENT* tls = &(m_tls[tid]);
    tls->WaitSemaphore(false);
    return tls->m_runObj;
}

void* THREAD_POOL::ThreadRoutine(void* tlsArg)
{
    TLS_ELEMENT* tls = static_cast< TLS_ELEMENT* >(tlsArg);
    while (true)
    {
        tls->WaitSemaphore(true);
        tls->m_runObj->Run();
        // switch control back to the managing thread
        tls->SwitchSemaphore(false);
    }
    return 0;
}

void THREAD_POOL::TLS_ELEMENT::SwitchSemaphore(bool semaphoreState)
{
    DelayCurrentThread(0); // we use this system call as a memory fence
    m_semaphore = semaphoreState;
}

void THREAD_POOL::TLS_ELEMENT::WaitSemaphore(bool semaphoreState) const
{
    while (m_semaphore != semaphoreState)
    {
        DelayCurrentThread(0);
    }
    DelayCurrentThread(0); // we use this system call as a memory fence
}

bool THREAD_POOL::TLS_ELEMENT::CheckSemaphore() const
{
    bool semaphoreState = m_semaphore;
    DelayCurrentThread(0); // we use this system call as a memory fence
    return semaphoreState;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
