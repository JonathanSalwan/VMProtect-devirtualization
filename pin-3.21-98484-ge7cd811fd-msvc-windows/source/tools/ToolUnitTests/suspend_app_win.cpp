/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 * This test application verifies that Pin on Windows correctly handles
 * thread suspension and thread termination system calls. The application must
 * be run with the "suspend_win" tool.
 */

#define _WIN32_WINNT 0x0400 // Needed for SignalObjectAndWait()

#include <string>
#include <iostream>
#include <windows.h>
using std::cerr;
using std::endl;
using std::string;
// Auto-reset event. It is signaled when a thread is about to start.
HANDLE StartEvent;

// Manual-reset event. It is signaled when a thread is allowed to start.
HANDLE AllowStartEvent;

//==========================================================================
// Printing utilities
//==========================================================================
string UnitTestName("suspend_win");
string FunctionTestName;

static void StartFunctionTest(const string& functionTestName)
{
    if (FunctionTestName != "")
    {
        cerr << UnitTestName << "[" << FunctionTestName << "] Success" << endl;
    }
    FunctionTestName = functionTestName;
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
// Check the tool presence
//==========================================================================
typedef int (*FP_IsToolPresent)(volatile int*);
extern "C" __declspec(dllexport) int IsToolPresent(volatile int*);
volatile FP_IsToolPresent fpIsToolPresent = IsToolPresent;
volatile int IsTool                       = 0;

/*!
 * The tool intercepts this routine and sets <isTool> to 1.
 */
static int IsToolPresent(volatile int* pIsTool) { return *pIsTool; }

//==========================================================================
// Function test A)
//==========================================================================
typedef void (*FP_SleepInTool)(volatile int*);
extern "C" __declspec(dllexport) void SleepInTool(volatile int*);
volatile FP_SleepInTool fpSleepInTool = SleepInTool;
volatile int InTool                   = 0;

/*!
 * The tool intercepts this routine, sets <InTool> to 1 and sleeps some time
 * on entry to this function.
 */
static void SleepInTool(volatile int* pInTool) { *pInTool = 0; }

/*!
 * The thread start routine.
 */
static DWORD WINAPI ThreadA(void*)
{
    SetEvent(StartEvent);
    while (TRUE)
    {
        // Call this dummy function through a volatile pointer to ensure the
        // compiler doesn't inline it.
        fpSleepInTool(&InTool);
    }
    return 0;
}

//==========================================================================
// Function test C)
//==========================================================================
extern "C" __declspec(dllexport) void DoFlush();

typedef void (*FP_CheckFlush)(volatile int*);
extern "C" __declspec(dllexport) void CheckFlush(volatile int*);
volatile FP_CheckFlush fpCheckFlush = CheckFlush;

volatile int InLoop           = 0;
volatile int CodeCacheFlushed = 0;

void DoFlush()
{
    // The tool intercepts this routine to flush the code cache.
}

void CheckFlush(volatile int*)
{
    // The tool intercepts this routine and sets <CodeCacheFlushed> to 1 if
    // the code cache was flushed.
}

/*!
 * The thread start routine.
 */
static DWORD WINAPI ThreadC(void*)
{
    SetEvent(StartEvent);
    while (TRUE)
    {
        InLoop = 1;
    }
    return 0;
}

//==========================================================================
// Function test F)
//==========================================================================
/*!
 * The thread start routine.
 */
static DWORD WINAPI ThreadF(void*)
{
    SignalObjectAndWait(StartEvent, AllowStartEvent, INFINITE, FALSE);
    return 0;
}

//==========================================================================
// Function test G)
//==========================================================================
/*!
 * The thread start routine.
 */
static DWORD WINAPI ThreadG(void*)
{
    SetEvent(StartEvent);
    SuspendThread(GetCurrentThread());
    return 0;
}

//==========================================================================
// Function test M)
//==========================================================================
volatile LONG ThreadMEntryCount = 0;
volatile LONG ThreadMExitCount  = 0;

/*!
 * The thread start routine.
 */
static DWORD WINAPI ThreadM(void* pTargetThread)
{
    SignalObjectAndWait(StartEvent, AllowStartEvent, INFINITE, FALSE);

    const char* err      = 0;
    HANDLE hTargetThread = *(HANDLE*)pTargetThread;

    // Wait for other ThreadM to enter the loop
    if (InterlockedIncrement(&ThreadMEntryCount) != 2)
    {
        while (ThreadMEntryCount != 2)
        {
            ;
        }
    }

    for (int i = 0; i < 3; i++)
    {
        DWORD suspendCount;
        suspendCount = SuspendThread(hTargetThread);
        if (suspendCount == DWORD(-1))
        {
            err = "SuspendThread failed";
            break;
        }
        if (suspendCount != 0)
        {
            err = "Unexpected suspend count in SuspendThread";
            break;
        }

        suspendCount = ResumeThread(hTargetThread);
        if (suspendCount == DWORD(-1))
        {
            err = "ResumeThread failed";
            break;
        }
        if (suspendCount != 1)
        {
            err = "Unexpected suspend count in ResumeThread";
            break;
        }
    }

    // Wait for other ThreadM to complete the loop
    if (InterlockedIncrement(&ThreadMExitCount) != 2)
    {
        while (ThreadMExitCount != 2)
        {
            ;
        }
    }

    if (err != 0)
    {
        Abort(err);
    }
    return 0;
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

    // Check to see if the tool is present
    IsToolPresent(&IsTool);

    //============================================================================
    // A) Verify that Pin does not suspend threads in analysis routines
    //============================================================================
    StartFunctionTest("A");

    // Create a thread that calls to a dummy function in an infinite loop.
    // The function is instrumented by the tool, that sleeps some time on each
    // function call. Try to suspend the thread while it is sleeping in the tool.
    // Pin should postpone suspension till the thread returns from the tool's
    // routine.
    hThread = CreateThread(0, 0, ThreadA, 0, 0, 0);
    if (hThread == 0)
    {
        Abort("CreateThread failed");
    }

    // Wait until the new thread starts running
    WaitForSingleObject(StartEvent, INFINITE);

    for (int i = 0; i < 3; i++)
    {
        // Wait until the thread enters the sleeping routine in the tool and then
        // suspend the thread.

        while (IsTool && (InTool == 0))
        {
            Sleep(0);
        }

        suspendCount = SuspendThread(hThread);
        if (suspendCount == DWORD(-1))
        {
            Abort("SuspendThread failed");
        }
        if (suspendCount != 0)
        {
            Abort("Unexpected suspend count in SuspendThread");
        }
        if (InTool != 0)
        {
            Abort("The thread is suspended in the tool");
        }

        // Resume the thread and check the suspension count
        suspendCount = ResumeThread(hThread);
        if (suspendCount == DWORD(-1))
        {
            Abort("ResumeThread failed");
        }
        if (suspendCount != 1)
        {
            Abort("Unexpected suspend count in ResumeThread");
        }
    }

    //============================================================================
    // B) Verify that Pin does not terminate threads in analysis routines
    //============================================================================
    StartFunctionTest("B");

    while (IsTool && (InTool == 0))
    {
        Sleep(0);
    }
    bStatus = TerminateThread(hThread, 0);
    if (bStatus == 0)
    {
        Abort("TerminateThread failed");
    }
    if (InTool != 0)
    {
        Abort("The thread is terminated in the tool");
    }

    // Wait some time before checking the thread state. TerminateThread() kills
    // threads asynchronously.
    dwStatus = WaitForSingleObject(hThread, 5000);
    if (dwStatus != WAIT_OBJECT_0)
    {
        Abort("The thread is not terminated");
    }

    CloseHandle(hThread);

    //============================================================================
    // C) Verify that Pin does not suspend threads in the code cache and the
    // CC flush is possible when a thread is suspended.
    //============================================================================
    StartFunctionTest("C");

    // Create a thread that loops infinitely in the code cache and suspend
    // the thread somewhere in the middle.
    // Cause the tool to flush the code cache and then check if the flush happened.
    // Pin pokes the thread out of the code cache before suspension and removes the
    // thread from generation counts. This should allow CC flush during the thread
    // suspension.

    hThread = CreateThread(0, 0, ThreadC, 0, 0, 0);
    if (hThread == 0)
    {
        Abort("CreateThread failed");
    }

    // Wait until the new thread enters the loop
    WaitForSingleObject(StartEvent, INFINITE);
    while (!InLoop)
    {
        Sleep(0);
    }

    // Suspend the thread
    suspendCount = SuspendThread(hThread);
    if (suspendCount == DWORD(-1))
    {
        Abort("SuspendThread failed");
    }
    if (suspendCount != 0)
    {
        Abort("Unexpected suspend count in SuspendThread");
    }

    // Run for (at most) three iterations to ensure steady state - no more jitting of the DoFlush function.
    // The tool will remove instrumentation only after reaching steady state.
    // The first two calls may be jitted, but by the third we expect the tool to remove instrumentation.
    // The next (fourth at most) call will cause the code to be rejitted due to the cache flush and that will set
    // CodeCacheFlushed. Eventually, the cache will only be flushed once.
    for (unsigned int i = 0; i < 4; ++i)
    {
        DoFlush();

        // Call this dummy function through a volatile pointer to ensure the compiler doesn't inline it.
        fpCheckFlush(&CodeCacheFlushed);
        if (CodeCacheFlushed) break;
    }
    if (IsTool && (CodeCacheFlushed == 0))
    {
        Abort("The code cache was not flushed");
    }

    //============================================================================
    // D) Verify that Pin correctly terminates suspended threads
    //============================================================================
    StartFunctionTest("D");

    bStatus = TerminateThread(hThread, 0);
    if (bStatus == 0)
    {
        Abort("TerminateThread failed");
    }

    // Wait some time before checking the thread state. TerminateThread() kills
    // threads asynchronously.
    dwStatus = WaitForSingleObject(hThread, 5000);
    if (dwStatus != WAIT_OBJECT_0)
    {
        Abort("The thread is not terminated");
    }

    CloseHandle(hThread);

    //============================================================================
    // E) Verify that Pin correctly suspends and terminates threads that are not
    //    yet started.
    //============================================================================
    StartFunctionTest("E");

    // Create a thread in the suspended state
    hThread = CreateThread(0, 0, ThreadC, 0, CREATE_SUSPENDED, 0);
    if (hThread == 0)
    {
        Abort("CreateThread failed");
    }

    // Suspend the thread one more time and check the suspension count
    suspendCount = SuspendThread(hThread);
    if (suspendCount == DWORD(-1))
    {
        Abort("SuspendThread failed");
    }
    if (suspendCount != 1)
    {
        Abort("Unexpected suspend count in SuspendThread");
    }

    // Terminate the thread that has not run yet
    bStatus = TerminateThread(hThread, 0);
    if (bStatus == 0)
    {
        Abort("TerminateThread failed");
    }

    dwStatus = WaitForSingleObject(hThread, 5000);
    if (dwStatus != WAIT_OBJECT_0)
    {
        Abort("The thread is not terminated");
    }

    CloseHandle(hThread);

    //============================================================================
    // F) Verify that Pin correctly suspends and terminates threads that are
    //    blocked in a system call.
    //============================================================================
    StartFunctionTest("F");

    // Create a thread that blocks itself in a system call. Suspend and terminate
    // the blocked thread.

    ResetEvent(AllowStartEvent);
    hThread = CreateThread(0, 0, ThreadF, 0, 0, 0);
    if (hThread == 0)
    {
        Abort("CreateThread failed");
    }

    // Wait until the new thread hangs in a system call
    WaitForSingleObject(StartEvent, INFINITE);

    // Suspend the thread
    suspendCount = SuspendThread(hThread);
    if (suspendCount == DWORD(-1))
    {
        Abort("SuspendThread failed");
    }
    if (suspendCount != 0)
    {
        Abort("Unexpected suspend count in SuspendThread");
    }

    // Resume the thread. It is still blocked on AllowStartEvent!
    suspendCount = ResumeThread(hThread);
    if (suspendCount == DWORD(-1))
    {
        Abort("ResumeThread failed");
    }
    if (suspendCount != 1)
    {
        Abort("Unexpected suspend count in ResumeThread");
    }

    // Terminate the thread that is blocked in a system call
    bStatus = TerminateThread(hThread, 0);
    if (bStatus == 0)
    {
        Abort("TerminateThread failed");
    }

    dwStatus = WaitForSingleObject(hThread, 5000);
    if (dwStatus != WAIT_OBJECT_0)
    {
        Abort("The thread is not terminated");
    }

    CloseHandle(hThread);

    //============================================================================
    // G) Verify that Pin correctly handles the thread self-suspension.
    //============================================================================
    StartFunctionTest("G");

    // Create a thread and wait until it suspends itself. Then, resume the thread.
    hThread = CreateThread(0, 0, ThreadG, 0, 0, 0);
    if (hThread == 0)
    {
        Abort("CreateThread failed");
    }

    WaitForSingleObject(StartEvent, INFINITE);
    while (TRUE)
    {
        Sleep(5);
        suspendCount = SuspendThread(hThread);
        if (suspendCount == DWORD(-1))
        {
            Abort("SuspendThread failed");
        }
        if (suspendCount != 0)
        {
            if (suspendCount != 1)
            {
                Abort("Unexpected suspend count in SuspendThread");
            }
            break;
        }

        suspendCount = ResumeThread(hThread);
        if (suspendCount == DWORD(-1))
        {
            Abort("ResumeThread failed");
        }
    }
    suspendCount = ResumeThread(hThread);
    if (suspendCount == DWORD(-1))
    {
        Abort("ResumeThread#1 failed");
    }
    if (suspendCount != 2)
    {
        Abort("Unexpected suspend count in ResumeThread");
    }

    suspendCount = ResumeThread(hThread);
    if (suspendCount == DWORD(-1))
    {
        Abort("ResumeThread#2 failed");
    }
    if (suspendCount != 1)
    {
        Abort("Unexpected suspend count in ResumeThread");
    }

    dwStatus = WaitForSingleObject(hThread, 60000);
    if (dwStatus != WAIT_OBJECT_0)
    {
        Abort("The thread is not resumed");
    }

    CloseHandle(hThread);

    //============================================================================
    // M) Verify that mutual suspension does not cause deadlock.
    //============================================================================

    // Apparently, this test works fine under Pin but causes deadlock when
    // executed natively.
    // It seems that Windows kernel suspends both threads if they try to suspend
    // each other and issue SuspendThread system calls simultaneously.
    // Leave this test for internal Pin debugging only.

    if ((argc == 2) && (string(argv[1]) == "-dbg"))
    {
        StartFunctionTest("M");
        // Create two threads that simultaneously attempt to suspend/resume each
        // other. Both threads should exit successfully if not deadlocked.

        HANDLE hThread[2];
        ResetEvent(AllowStartEvent);

        hThread[0] = CreateThread(0, 0, ThreadM, &(hThread[1]), 0, 0);
        if (hThread[0] == 0)
        {
            Abort("CreateThread[0] failed");
        }
        WaitForSingleObject(StartEvent, INFINITE);

        hThread[1] = CreateThread(0, 0, ThreadM, &(hThread[0]), 0, 0);
        if (hThread[1] == 0)
        {
            Abort("CreateThread[1] failed");
        }
        WaitForSingleObject(StartEvent, INFINITE);

        // Start the threads
        SetEvent(AllowStartEvent);

        // Wait 1 minute or until both threads exit
        dwStatus = WaitForMultipleObjects(2, hThread, TRUE, 60000);
        if (dwStatus == WAIT_TIMEOUT)
        {
            Abort("The threads appear to be deadlocked");
        }
        else if (dwStatus == WAIT_FAILED)
        {
            Abort("WaitForMultipleObjects failed");
        }

        CloseHandle(hThread[0]);
        CloseHandle(hThread[1]);
    }

    //============================================================================
    ExitUnitTest();

    return 0;
}
