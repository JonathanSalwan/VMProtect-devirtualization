/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Implementation of the threading API in Windows. 
 */

#include "threadlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <windows.h>

// Structure passed to the start routine in Windows
typedef struct WIN_THREAD_RTN_ARG_
{
    THREAD_RTN_PTR m_threadRtn;
    void* m_arg;
} WIN_THREAD_RTN_ARG;

// Thread start routine in Windows
static DWORD WINAPI WinThreadRtn(LPVOID arg)
{
    void* retval;
    WIN_THREAD_RTN_ARG winThreadRtnArg = *((WIN_THREAD_RTN_ARG*)arg);
    free(arg);
    retval = winThreadRtnArg.m_threadRtn(winThreadRtnArg.m_arg);
    return ((DWORD)retval);
}

BOOL CreateOneThread(THREAD_HANDLE* pThreadHandle, THREAD_RTN_PTR threadRtn, void* arg)
{
    HANDLE winThreadHandle;
    WIN_THREAD_RTN_ARG* pWinThreadRtnArg = (WIN_THREAD_RTN_ARG*)malloc(sizeof(WIN_THREAD_RTN_ARG));
    pWinThreadRtnArg->m_threadRtn        = threadRtn;
    pWinThreadRtnArg->m_arg              = arg;

    winThreadHandle = CreateThread(NULL, 0, WinThreadRtn, pWinThreadRtnArg, 0, NULL);
    if (winThreadHandle == NULL)
    {
        return FALSE;
    }

    *pThreadHandle = (THREAD_HANDLE)winThreadHandle;
    return TRUE;
}

BOOL JoinOneThread(THREAD_HANDLE threadHandle)
{
    HANDLE winThreadHandle = (HANDLE)threadHandle;
    DWORD waitStatus;
    waitStatus = WaitForSingleObject(winThreadHandle, INFINITE);
    return (waitStatus == WAIT_OBJECT_0);
}

void ExitCurrentThread() { ExitThread(0); }

void DelayCurrentThread(unsigned int millisec) { Sleep(millisec); }
