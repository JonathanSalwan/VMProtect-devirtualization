/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Generic threading API. 
 */

#ifndef THREADLIB_H
#define THREADLIB_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef int BOOL;
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define MAXTHREADS 1000

    typedef void* THREAD_HANDLE;
    typedef void* THREAD_RTN(void* arg);
    typedef THREAD_RTN* THREAD_RTN_PTR;

    BOOL CreateOneThread(THREAD_HANDLE* pThreadHandle, THREAD_RTN_PTR threadRtn, void* arg);

    BOOL JoinOneThread(THREAD_HANDLE threadHandle);

    void ExitCurrentThread();

    void DelayCurrentThread(unsigned int millisec);

    unsigned long GetTid();

#ifdef __cplusplus
}
#endif

#endif // #ifndef THREADLIB_H
