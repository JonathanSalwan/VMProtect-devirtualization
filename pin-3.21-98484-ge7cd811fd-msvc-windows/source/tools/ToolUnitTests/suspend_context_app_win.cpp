/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * This test application verifies that Pin on Windows correctly handles 
 * system calls that get/set context of a suspended thread.
 */

#define _WIN32_WINNT 0x0400 // Needed for SignalObjectAndWait()

#include <string>
#include <iostream>
#include <windows.h>

using std::cerr;
using std::endl;
using std::string;
typedef unsigned __int8 UINT8;
typedef unsigned __int16 UINT16;
typedef unsigned __int32 UINT32;
typedef unsigned __int64 UINT64;

// Auto-reset event. It is signaled when a thread is about to start.
HANDLE StartEvent;

// Manual-reset event. It is signaled when a thread is allowed to start.
HANDLE AllowStartEvent;

// Pointer to the NtTerminateThread function.
typedef LONG __stdcall NtTerminateThread_T(HANDLE threadHandle, LONG exitStatus);
NtTerminateThread_T* fpNtTerminateThread;

//==========================================================================
// Printing utilities
//==========================================================================
string UnitTestName("suspend_context_win");
string FunctionTestName;

static void StartFunctionTest(const string& functionTestName)
{
    if (FunctionTestName != "")
    {
        cerr << UnitTestName << "[" << FunctionTestName << "] Success" << endl;
    }
    FunctionTestName = functionTestName;
}

static void SkipFunctionTest(const string& msg)
{
    cerr << UnitTestName << "[" << FunctionTestName << "] Skip: " << msg << endl;
    FunctionTestName = "";
}

static void ExitUnitTest()
{
    if (FunctionTestName != "")
    {
        cerr << UnitTestName << "[" << FunctionTestName << "] Success" << endl;
    }
    cerr << UnitTestName << " : Completed successfully" << endl;
    exit(0);
}

static void Abort(const string& msg)
{
    cerr << UnitTestName << "[" << FunctionTestName << "] Failure: " << msg << endl;
    exit(1);
}

//==========================================================================
// Context manipulation utilities
//==========================================================================
#if defined(TARGET_IA32)

// IA-32 CONTEXT keeps XMM registers in the ExtendedRegisters[] array. This array
// has the FXSAVE structure.
struct FXSAVE
{
    UINT16 _fcw;
    UINT16 _fsw;
    UINT8 _ftw;
    UINT8 _pad1;
    UINT16 _fop;
    UINT32 _fpuip;
    UINT16 _cs;
    UINT16 _pad2;
    UINT32 _fpudp;
    UINT16 _ds;
    UINT16 _pad3;
    UINT32 _mxcsr;
    UINT32 _mxcsrmask;
    UINT8 _st[8 * 16];
    UINT8 _xmm[8 * 16];
    UINT8 _pad4[56 * 4];
};

const size_t XmmRegsOffset   = (offsetof(CONTEXT, ExtendedRegisters) + offsetof(FXSAVE, _xmm[0]));
const size_t XmmRegSize      = 16;
const size_t NumXmmRegs      = 8;
const DWORD XMM_CONTEXT_FLAG = CONTEXT_EXTENDED_REGISTERS;

#elif defined(TARGET_IA32E)

const size_t XmmRegsOffset   = offsetof(CONTEXT, FltSave.XmmRegisters);
const size_t XmmRegSize      = 16;
const size_t NumXmmRegs      = 16;
const DWORD XMM_CONTEXT_FLAG = CONTEXT_FLOATING_POINT;

#endif

/*!
 * Return TRUE if XMM registers in specified contexts are identical.
 */
static BOOL CompareXmmState(PCONTEXT pContext1, PCONTEXT pContext2)
{
    return (memcmp((unsigned char*)(pContext1) + XmmRegsOffset, (unsigned char*)(pContext2) + XmmRegsOffset,
                   NumXmmRegs * XmmRegSize) == 0);
}

/*!
 * Suspend specified threads and exchange their contexts.
 */
void ExchangeContexts(HANDLE hThread0, HANDLE hThread1)
{
    BOOL bStatus;
    DWORD suspendCount;

    CONTEXT ctxt0;
    CONTEXT ctxt1;
    ctxt0.ContextFlags = CONTEXT_ALL;
    ctxt1.ContextFlags = CONTEXT_ALL;

    suspendCount = SuspendThread(hThread0);
    if (suspendCount == DWORD(-1))
    {
        Abort("SuspendThread(0) failed");
    }
    suspendCount = SuspendThread(hThread1);
    if (suspendCount == DWORD(-1))
    {
        Abort("SuspendThread(1) failed");
    }

    bStatus = GetThreadContext(hThread0, &ctxt0);
    if (bStatus == 0)
    {
        Abort("GetThreadContext(0) failed");
    }
    bStatus = GetThreadContext(hThread1, &ctxt1);
    if (bStatus == 0)
    {
        Abort("GetThreadContext(1) failed");
    }

    bStatus = SetThreadContext(hThread0, &ctxt1);
    if (bStatus == 0)
    {
        Abort("SetThreadContext(0) failed");
    }
    bStatus = SetThreadContext(hThread1, &ctxt0);
    if (bStatus == 0)
    {
        Abort("SetThreadContext(1) failed");
    }
}

//==========================================================================
// Test A)
//==========================================================================
volatile DWORD PhaseOfThreadA = 0;

/*!
* The thread start routine.
*/
static DWORD WINAPI ThreadA(void* arg)
{
    SignalObjectAndWait(StartEvent, AllowStartEvent, INFINITE, FALSE);

    volatile DWORD* pTid = (volatile DWORD*)arg;

    {
        DWORD tid = GetCurrentThreadId();
        *pTid     = tid;
    }

    while (PhaseOfThreadA != 1)
    {
    }

    {
        DWORD tid = GetCurrentThreadId();
        *pTid     = tid;
    }

    while (PhaseOfThreadA != 2)
    {
    }

    return 0;
}

//==========================================================================
// Function test B)
//==========================================================================
/*!
 * The thread start routine.
 */
static DWORD WINAPI ThreadB(void* pContext)
{
    SignalObjectAndWait(StartEvent, AllowStartEvent, INFINITE, FALSE);
    return 0;
}

//==========================================================================
// Function test C)
//==========================================================================
/*!
 * Call NtTerminateThread for the current thread.
 */
static void TerminateThisThread()
{
    //exit with zero status
    fpNtTerminateThread((HANDLE)(LONG_PTR)(-2), 0);
}

/*!
 * The thread start routine.
 */
static DWORD WINAPI ThreadC(void*)
{
    SignalObjectAndWait(StartEvent, AllowStartEvent, INFINITE, FALSE);
    //exit with non-zero status
    return 1;
}

/*!
 * Given a handle to a thread suspended in the ThreadC function.
 * Change the context: IP register = TerminateThisThread entry.
 * Resume the thread and check the thread exit status.
 * @return 0 - the thread exited with an expected status.
 *         1 - the thread exited with an unexpected status.
 *        -1 - the test can not be performed in this configuration.
 */
static int TerminateThreadCAndCheckStatus(HANDLE hThread)
{
    int result = 0;
    BOOL bStatus;
    DWORD dwStatus;
    DWORD suspendCount;
    CONTEXT ctxt;
    ctxt.ContextFlags = CONTEXT_CONTROL;

    bStatus = GetThreadContext(hThread, &ctxt);
    if (bStatus == 0)
    {
        Abort("GetThreadContext(0) failed");
    }

    // Set IP = TerminateThisThread
#if defined(TARGET_IA32)
    ctxt.Eip = reinterpret_cast< DWORD >(TerminateThisThread);
#elif defined(TARGET_IA32E)
    // We do not want to change Rsp of the suspended system call.
    // It is not guarantied that system will accept this change.
    // On the other hand, we need Rsp properly aligned to execute
    // TerminateThisThread. So, we check alignment and it is not
    // good for us, just skip the test.
    if (ctxt.Rsp % 16 == 8)
    {
        ctxt.Rip = reinterpret_cast< DWORD64 >(TerminateThisThread);
    }
    else
    {
        result = -1;
    }
#endif

    bStatus = SetThreadContext(hThread, &ctxt);
    if (bStatus == 0)
    {
        Abort("SetThreadContext failed");
    }

    // Start the thread. It should exit with the zero status.
    suspendCount = ResumeThread(hThread);
    if (suspendCount == DWORD(-1))
    {
        Abort("ResumeThread failed");
    }

    // Wait for the thread exit and check the exit code.
    dwStatus = WaitForSingleObject(hThread, 60000);
    if (dwStatus != WAIT_OBJECT_0)
    {
        Abort("The thread did not exit");
    }

    if (result != -1)
    {
        DWORD exitCode = 1;
        bStatus        = GetExitCodeThread(hThread, &exitCode);
        if (bStatus == 0)
        {
            Abort("GetExitCodeThread failed");
        }

        result = ((exitCode == 0) ? 0 : 1);
    }
    return result;
}

/*!
 * The main procedure of the application.
 */
int main(int argc, char* argv[])
{
    BOOL bStatus;
    DWORD dwStatus;
    HANDLE hThread;
    DWORD suspendCount;

    StartEvent      = CreateEvent(0, FALSE, FALSE, 0);
    AllowStartEvent = CreateEvent(0, TRUE, FALSE, 0);

    fpNtTerminateThread = (NtTerminateThread_T*)GetProcAddress(GetModuleHandle("ntdll.dll"), "NtTerminateThread");
    if (fpNtTerminateThread == 0)
    {
        Abort("NtTerminateThread function is not found");
    }

    //============================================================================
    // A) Verify that Pin correctly gets/sets context for threads suspended in the
    //    code cache.
    //============================================================================
    StartFunctionTest("A");

    // Create two threads each of which assigns its own thread ID to the specified
    // variable: targ[0] = tid[0] and targ[1] = tid[1]. Suspend threads on entry
    // and exchange thread contexts. After both threads write their IDs, suspend
    // threads and exchange their contexts again. When threads exit, their IDs
    // should appear in opposite order: targ[0] = tid[1] and targ[1] = tid[0].

    {
        HANDLE hThread[2];
        DWORD tid[2];
        volatile DWORD targ[2] = {0, 0};

        ResetEvent(AllowStartEvent);

        hThread[0] = CreateThread(0, 0, ThreadA, (void*)&(targ[0]), 0, 0);
        if (hThread[0] == 0)
        {
            Abort("CreateThread[0] failed");
        }
        WaitForSingleObject(StartEvent, INFINITE);

        hThread[1] = CreateThread(0, 0, ThreadA, (void*)&(targ[1]), 0, 0);
        if (hThread[1] == 0)
        {
            Abort("CreateThread[1] failed");
        }
        WaitForSingleObject(StartEvent, INFINITE);

        // Start the threads
        SetEvent(AllowStartEvent);

        // Wait until both threads pass their IDs
        while ((targ[0] == 0) || (targ[1] == 0))
        {
            Sleep(5);
        }
        tid[0] = targ[0];
        tid[1] = targ[1];

        // Suspend threads and switch their contexts
        ExchangeContexts(hThread[0], hThread[1]);

        targ[0] = targ[1] = 0;
        PhaseOfThreadA    = 1;

        // Resume threads and wait until they pass IDs again
        suspendCount = ResumeThread(hThread[0]);
        if (suspendCount == DWORD(-1))
        {
            Abort("ResumeThread(0) failed");
        }
        suspendCount = ResumeThread(hThread[1]);
        if (suspendCount == DWORD(-1))
        {
            Abort("ResumeThread(1) failed");
        }

        while ((targ[0] == 0) || (targ[1] == 0))
        {
            Sleep(5);
        }

        // Suspend threads and switch their contexts back. This is needed
        // because the thread exit procedure may check consistency of the
        // stack register and the TEB stack bounds.
        ExchangeContexts(hThread[0], hThread[1]);

        // Allow both threads to exit
        PhaseOfThreadA = 2;

        // Resume threads to let them exit
        suspendCount = ResumeThread(hThread[0]);
        if (suspendCount == DWORD(-1))
        {
            Abort("ResumeThread(0) failed");
        }
        suspendCount = ResumeThread(hThread[1]);
        if (suspendCount == DWORD(-1))
        {
            Abort("ResumeThread(1) failed");
        }

        dwStatus = WaitForMultipleObjects(2, hThread, TRUE, 60000);
        if (dwStatus == WAIT_TIMEOUT)
        {
            Abort("At least one thread did not exit");
        }
        else if (dwStatus == WAIT_FAILED)
        {
            Abort("WaitForMultipleObjects failed");
        }

        CloseHandle(hThread[0]);
        CloseHandle(hThread[1]);

        // Check to see that <targ> and <tid> arrays have reverse order
        if ((targ[0] != tid[1]) || (targ[1] != tid[0]))
        {
            Abort("Context exchange failed");
        }
    }

#ifndef SKIP_XMM_TEST_CASE
    //============================================================================
    // B) Verify that Pin correctly reads/writes XMM state of threads suspended in
    //    a system call.
    //============================================================================
    StartFunctionTest("B");

    // Suspend a thread in a system call and set some predefined XMM state for
    // the suspended thread. Read the context and check the state of XMM registers.
    {
        // The "predefined" XMM state
        const size_t xmmStateSize = XmmRegSize * NumXmmRegs;
        char xmmState[xmmStateSize];
        memset(xmmState, 1, xmmStateSize);

        CONTEXT ctxt, ctxt1, ctxt2;

        // Create a thread and wait until it hangs in a system call.
        ResetEvent(AllowStartEvent);
        hThread = CreateThread(0, 0, ThreadB, 0, 0, 0);
        if (hThread == 0)
        {
            Abort("CreateThread failed");
        }
        WaitForSingleObject(StartEvent, INFINITE);

        // Suspend the thread in a system call.
        suspendCount = SuspendThread(hThread);
        if (suspendCount == DWORD(-1))
        {
            Abort("SuspendThread failed");
        }
        if (suspendCount != 0)
        {
            Abort("Unexpected suspend count in SuspendThread");
        }

        // Read the original XMM state
        ctxt.ContextFlags = XMM_CONTEXT_FLAG;
        bStatus           = GetThreadContext(hThread, &ctxt);
        if (bStatus == 0)
        {
            Abort("GetThreadContext failed");
        }

        // Set the "predefined" state for XMM registers
        ctxt1 = ctxt;
        memcpy((char*)(&ctxt1) + XmmRegsOffset, xmmState, xmmStateSize);
        bStatus = SetThreadContext(hThread, &ctxt1);
        if (bStatus == 0)
        {
            Abort("SetThreadContext failed");
        }

        // Read and check the state for XMM registers
        ctxt2.ContextFlags = XMM_CONTEXT_FLAG;
        bStatus            = GetThreadContext(hThread, &ctxt2);
        if (bStatus == 0)
        {
            Abort("GetThreadContext failed");
        }
        if (!CompareXmmState(&ctxt1, &ctxt2))
        {
            Abort("Mismatch in the XMM state");
        }

        // Restore the original XMM state
        bStatus = SetThreadContext(hThread, &ctxt);
        if (bStatus == 0)
        {
            Abort("SetThreadContext failed");
        }

        // Let the thread exit
        SetEvent(AllowStartEvent);
        suspendCount = ResumeThread(hThread);
        if (suspendCount == DWORD(-1))
        {
            Abort("ResumeThread failed");
        }
        dwStatus = WaitForSingleObject(hThread, 60000);
        if (dwStatus != WAIT_OBJECT_0)
        {
            Abort("The thread did not exit");
        }

        CloseHandle(hThread);
    }
#endif

    //============================================================================
    // C) Verify that Pin correctly changes IP register for threads suspended in the
    //    a system call.
    //============================================================================
    StartFunctionTest("C");

    // Create a thread that blocks itself in a system call. Suspend the thread and
    // change its IP register, so after resumption the thread executes a procedure
    // (TerminateThisThread) that terminates the thread. If context change
    // succeeded, the exit status of the thread should be zero. Otherwise, in case
    // of failure the thread returns with a non-zero status.

    {
        ResetEvent(AllowStartEvent);
        hThread = CreateThread(0, 0, ThreadC, 0, 0, 0);
        if (hThread == 0)
        {
            Abort("CreateThread failed");
        }

        // Wait until the new thread hangs in a system call
        WaitForSingleObject(StartEvent, INFINITE);

        // Suspend the thread in a system call.
        suspendCount = SuspendThread(hThread);
        if (suspendCount == DWORD(-1))
        {
            Abort("SuspendThread failed");
        }
        if (suspendCount != 0)
        {
            Abort("Unexpected suspend count in SuspendThread");
        }
        SetEvent(AllowStartEvent);

        int result = TerminateThreadCAndCheckStatus(hThread);
        if (result == 1)
        {
            Abort("Unexpected thread exit code");
        }
        if (result == -1)
        {
            SkipFunctionTest("The test can not be performed in this configuration");
        }
    }

    //============================================================================
    // D) Similar to C), but instead of suspension in a system call, create the
    //    thread in the suspended state (initially suspended).
    //============================================================================
    StartFunctionTest("D");

    {
        ResetEvent(AllowStartEvent);
        hThread = CreateThread(0, 0, ThreadC, 0, CREATE_SUSPENDED, 0);
        if (hThread == 0)
        {
            Abort("CreateThread failed");
        }

        int result = TerminateThreadCAndCheckStatus(hThread);
        if (result == 1)
        {
            Abort("Unexpected thread exit code");
        }
        if (result == -1)
        {
            SkipFunctionTest("The test can not be performed in this configuration");
        }
    }

    //============================================================================
    ExitUnitTest();

    return 0;
}
