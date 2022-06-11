/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Runnable and function objects.
 */
#include "runnable.h"

#if defined(TARGET_WINDOWS)
#include <windows.h>
#elif defined(TARGET_LINUX)

// If __USE_GNU is defined, we don't need to do anything.
// If we defined it ourselves, we need to undefine it later.
#ifndef __USE_GNU
#define __USE_GNU
#define APP_UNDEF_USE_GNU
#endif

#include <ucontext.h>

// If we defined __USE_GNU ourselves, we need to undefine it here.
#ifdef APP_UNDEF_USE_GNU
#undef __USE_GNU
#undef APP_UNDEF_USE_GNU
#endif

#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include <setjmp.h>
#endif

//=======================================================================
// Implementation of the FUNC_OBJ::ExecuteSafe function
//=======================================================================

#if defined(TARGET_WINDOWS)

/*!
 * Given exception context and record, retrieve the exception address.
 */
static void* GetExceptIp(LPEXCEPTION_POINTERS exceptPtr)
{
#if defined(TARGET_IA32)
    return reinterpret_cast< void* >(exceptPtr->ContextRecord->Eip);
#elif defined(TARGET_IA32E)
    return reinterpret_cast< void* >(exceptPtr->ContextRecord->Rip);
#endif
}

FUNC_OBJ& FUNC_OBJ::ExecuteSafe()
{
    void* exceptIp;

    __try
    {
        FUNC_OBJ& obj = Execute();
        return obj;
    }
    __except ((exceptIp = GetExceptIp(GetExceptionInformation())), EXCEPTION_EXECUTE_HANDLER)
    {
        return HandleException(exceptIp);
    }
}

#elif defined(TARGET_LINUX)

static sigjmp_buf jumpBuffer;
static void* exceptIp;

/*!
 * Signal handler: stores exception IP in a global variable and returns with siglongjmp.
 */
static void HandleSignal(int sig, siginfo_t* i, void* vctxt)
{
    ucontext_t* ctxt = (ucontext_t*)vctxt;

#if defined(TARGET_IA32)
    exceptIp = reinterpret_cast< void* >(ctxt->uc_mcontext.gregs[REG_EIP]);
#elif defined(TARGET_IA32E)
    exceptIp = reinterpret_cast< void* >(ctxt->uc_mcontext.gregs[REG_RIP]);
#endif

    siglongjmp(jumpBuffer, 1);
}

/*!
 * Set up the SIGSEGV signal handler.
 */
static bool SetupSignalHandler()
{
    struct sigaction sigact;

    sigact.sa_sigaction = HandleSignal;
    sigact.sa_flags     = SA_SIGINFO;
    sigemptyset(&sigact.sa_mask);
    if (sigaction(SIGSEGV, &sigact, 0) == -1)
    {
        cerr << "Unable to set up handler" << endl;
        return false;
    }
    return true;
}

FUNC_OBJ& FUNC_OBJ::ExecuteSafe()
{
    static bool first = true;
    if (first)
    {
        SetupSignalHandler();
        first = false;
    }

    if (sigsetjmp(jumpBuffer, 1) == 0)
    {
        return Execute();
    }
    else
    {
        return HandleException(exceptIp);
    }
}

#else // macOS*

FUNC_OBJ& FUNC_OBJ::ExecuteSafe()
{
    // Exception handling is not supported on macOS*
    return Execute();
}

#endif

/* ===================================================================== */
/* eof */
/* ===================================================================== */
