/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This is a test framework for testing the locking primitives.  Use the
 * "-test" knob to select a specific test.  This tool is meant to be
 * run with "mt-worker-posix.cpp" or "mt-worker-windows.cpp".
 */

#include <iostream>
#include "pin.H"
#include "atomic.hpp"

#include <set>
using std::set;

// The test continues until all threads have run at least this many iterations
// trying to acquire / release the lock.
//
#if defined(TARGET_WINDOWS)
static const unsigned long ITERATION_COUNT = 1000;
#else
static const unsigned long ITERATION_COUNT = 100000;
#endif

// Some of the tests hold a lock over a short delay loop.  This is the number
// of iterations in that delay loop.
//
static const unsigned DELAY_COUNT = 16;

KNOB< std::string > KnobTest(KNOB_MODE_WRITEONCE, "pintool", "test", "",
                             "Name of the test to run [lock-integrity | lock-stress | "
                             "mutex-integrity | mutex-stress mutex-trystress | "
                             "writer-integrity | writer-stress | writer-trystress | "
                             "reader-stress | reader-trystress | "
                             "rw-integrity | rw-stress | rw-trystress | "
                             "semaphore | trylocks].");

enum TEST
{
    TEST_NONE,
    TEST_INVALID,
    TEST_LOCK_INTEGRITY,
    TEST_LOCK_STRESS,
    TEST_MUTEX_INTEGRITY,
    TEST_MUTEX_STRESS,
    TEST_MUTEX_TRYSTRESS,
    TEST_WRITER_INTEGRITY,
    TEST_WRITER_STRESS,
    TEST_WRITER_TRYSTRESS,
    TEST_READER_STRESS,
    TEST_READER_TRYSTRESS,
    TEST_RW_INTEGRITY,
    TEST_RW_STRESS,
    TEST_RW_TRYSTRESS,
    TEST_SEMAPHORE,
    TEST_TRYLOCKS
};
TEST TestType;

// The lock variables that we test.
//
PIN_LOCK Lock;
PIN_MUTEX Mutex;
PIN_RWMUTEX RWMutex;
PIN_SEMAPHORE Sem1;
PIN_SEMAPHORE Sem2;

PIN_LOCK pinLock;

THREADID HasLock = INVALID_THREADID;
REG RegThreadInfo;
BOOL FoundTestFunc           = FALSE;
BOOL FoundThreadCountFunc    = FALSE;
volatile int RunningWorkers  = 0;
volatile BOOL AllExit        = FALSE;
volatile int ActiveReaders   = 0;
volatile BOOL IsActiveWriter = FALSE;

struct THREAD_INFO
{
    THREAD_INFO(unsigned id) : _workerId(id), _count(0) {}
    unsigned _workerId;
    UINT64 _count;
};

static TEST GetTestType(const std::string&);
static VOID OnThreadFini(THREADID, const CONTEXT*, INT32, VOID*);
static void InstrumentRtn(RTN ins, VOID*);
static void OnExit(INT32, VOID*);
static void GetThreadCount(ADDRINT);
static void DoTestLockIntegrity(THREADID, THREAD_INFO*, UINT32*);
static void DoTestLockStress(THREADID, THREAD_INFO*, UINT32*);
static void DoTestMutexIntegrity(THREADID, THREAD_INFO*, UINT32*);
static void DoTestMutexStress(THREAD_INFO*, UINT32*);
static void DoTestMutexTryStress(THREAD_INFO*, UINT32*);
static void DoTestWriterIntegrity(THREADID, THREAD_INFO*, UINT32*);
static void DoTestWriterStress(THREAD_INFO*, UINT32*);
static void DoTestWriterTryStress(THREAD_INFO*, UINT32*);
static void DoTestReaderStress(THREAD_INFO*, UINT32*);
static void DoTestReaderTryStress(THREAD_INFO*, UINT32*);
static void DoTestReaderWriterIntegrity(THREAD_INFO*, UINT32*);
static void DoTestReaderWriterStress(THREAD_INFO*, UINT32*);
static void DoTestReaderWriterTryStress(THREAD_INFO*, UINT32*);
static void DoTestSemaphore(THREAD_INFO*, UINT32*);
static void DoTestTryLocks(UINT32*);
static bool CheckIfDone(THREAD_INFO*, UINT32*);

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    PIN_InitSymbols();

    PIN_InitLock(&pinLock);

    RegThreadInfo = PIN_ClaimToolRegister();
    if (RegThreadInfo == REG_INVALID())
    {
        std::cout << "Out of tool registers" << std::endl;
        PIN_ExitProcess(1);
    }

    // Get the test type and initialize the corresponding lock variable.
    //
    TestType = GetTestType(KnobTest.Value());
    switch (TestType)
    {
        case TEST_NONE:
            std::cout << "Must specify a test to run with the '-test' knob" << std::endl;
            PIN_ExitProcess(1);
            break;
        case TEST_INVALID:
            std::cout << "Invalid test name: " << KnobTest.Value() << std::endl;
            PIN_ExitProcess(1);
            break;
        case TEST_LOCK_INTEGRITY:
        case TEST_LOCK_STRESS:
            PIN_InitLock(&Lock);
            break;
        case TEST_MUTEX_INTEGRITY:
        case TEST_MUTEX_STRESS:
        case TEST_MUTEX_TRYSTRESS:
            PIN_MutexInit(&Mutex);
            break;
        case TEST_WRITER_INTEGRITY:
        case TEST_WRITER_STRESS:
        case TEST_WRITER_TRYSTRESS:
        case TEST_READER_STRESS:
        case TEST_READER_TRYSTRESS:
        case TEST_RW_INTEGRITY:
        case TEST_RW_STRESS:
        case TEST_RW_TRYSTRESS:
            PIN_RWMutexInit(&RWMutex);
            break;
        case TEST_SEMAPHORE:
            PIN_SemaphoreInit(&Sem1);
            PIN_SemaphoreInit(&Sem2);
            PIN_SemaphoreSet(&Sem1);
            PIN_MutexInit(&Mutex);
            break;
        case TEST_TRYLOCKS:
            PIN_MutexInit(&Mutex);
            PIN_RWMutexInit(&RWMutex);
            PIN_SemaphoreInit(&Sem1);
            break;
        default:
            ASSERTX(0);
    }

    PIN_AddThreadFiniFunction(OnThreadFini, NULL);
    RTN_AddInstrumentFunction(InstrumentRtn, NULL);
    PIN_AddFiniFunction(OnExit, NULL);
    PIN_StartProgram();
    return 1;
}

static TEST GetTestType(const std::string& name)
{
    if (name == "") return TEST_NONE;
    if (name == "lock-integrity") return TEST_LOCK_INTEGRITY;
    if (name == "lock-stress") return TEST_LOCK_STRESS;
    if (name == "mutex-integrity") return TEST_MUTEX_INTEGRITY;
    if (name == "mutex-stress") return TEST_MUTEX_STRESS;
    if (name == "mutex-trystress") return TEST_MUTEX_TRYSTRESS;
    if (name == "writer-integrity") return TEST_WRITER_INTEGRITY;
    if (name == "writer-stress") return TEST_WRITER_STRESS;
    if (name == "writer-trystress") return TEST_WRITER_TRYSTRESS;
    if (name == "reader-stress") return TEST_READER_STRESS;
    if (name == "reader-trystress") return TEST_READER_TRYSTRESS;
    if (name == "rw-integrity") return TEST_RW_INTEGRITY;
    if (name == "rw-stress") return TEST_RW_STRESS;
    if (name == "rw-trystress") return TEST_RW_TRYSTRESS;
    if (name == "semaphore") return TEST_SEMAPHORE;
    if (name == "trylocks") return TEST_TRYLOCKS;
    return TEST_INVALID;
}

set< THREADID > appThreads;

static VOID AppThreadStart(THREADID tid, PIN_REGISTER* regval)
{
    // Give each worker thread a unique, contiguous ID.
    //
    static unsigned workerCount = 0;
    regval->qword[0]            = (UINT64)(reinterpret_cast< ADDRINT >(new THREAD_INFO(workerCount++)));
    PIN_GetLock(&pinLock, PIN_GetTid());
    appThreads.insert(tid);
    PIN_ReleaseLock(&pinLock);
}

static VOID OnThreadFini(THREADID tid, const CONTEXT* ctxt, INT32, VOID*)
{
    if (appThreads.find(tid) != appThreads.end())
    {
        appThreads.erase(appThreads.find(tid));
        ADDRINT addrInfo  = PIN_GetContextReg(ctxt, RegThreadInfo);
        THREAD_INFO* info = reinterpret_cast< THREAD_INFO* >(addrInfo);
        delete info;
    }
}

static void InstrumentRtn(RTN rtn, VOID*)
{
    if (RTN_Name(rtn) == "TellPinThreadStart" || RTN_Name(rtn) == "_TellPinThreadStart")
    {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(AppThreadStart), IARG_THREAD_ID, IARG_REG_REFERENCE, RegThreadInfo, IARG_END);
        RTN_Close(rtn);
    }

    if (RTN_Name(rtn) == "TellPinThreadCount" || RTN_Name(rtn) == "_TellPinThreadCount")
    {
        FoundThreadCountFunc = TRUE;
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(GetThreadCount), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
        RTN_Close(rtn);
    }

    if (RTN_Name(rtn) == "InstrumentedWithPin" || RTN_Name(rtn) == "_InstrumentedWithPin")
    {
        FoundTestFunc = TRUE;
        RTN_Open(rtn);
        switch (TestType)
        {
            case TEST_LOCK_INTEGRITY:
                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DoTestLockIntegrity), IARG_THREAD_ID, IARG_REG_VALUE, RegThreadInfo,
                               IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
                break;
            case TEST_LOCK_STRESS:
                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DoTestLockStress), IARG_THREAD_ID, IARG_REG_VALUE, RegThreadInfo,
                               IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
                break;
            case TEST_MUTEX_INTEGRITY:
                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DoTestMutexIntegrity), IARG_THREAD_ID, IARG_REG_VALUE, RegThreadInfo,
                               IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
                break;
            case TEST_MUTEX_STRESS:
                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DoTestMutexStress), IARG_REG_VALUE, RegThreadInfo,
                               IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
                break;
            case TEST_MUTEX_TRYSTRESS:
                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DoTestMutexTryStress), IARG_REG_VALUE, RegThreadInfo,
                               IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
                break;
            case TEST_WRITER_INTEGRITY:
                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DoTestWriterIntegrity), IARG_THREAD_ID, IARG_REG_VALUE, RegThreadInfo,
                               IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
                break;
            case TEST_WRITER_STRESS:
                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DoTestWriterStress), IARG_REG_VALUE, RegThreadInfo,
                               IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
                break;
            case TEST_WRITER_TRYSTRESS:
                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DoTestWriterTryStress), IARG_REG_VALUE, RegThreadInfo,
                               IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
                break;
            case TEST_READER_STRESS:
                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DoTestReaderStress), IARG_REG_VALUE, RegThreadInfo,
                               IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
                break;
            case TEST_READER_TRYSTRESS:
                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DoTestReaderTryStress), IARG_REG_VALUE, RegThreadInfo,
                               IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
                break;
            case TEST_RW_INTEGRITY:
                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DoTestReaderWriterIntegrity), IARG_REG_VALUE, RegThreadInfo,
                               IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
                break;
            case TEST_RW_STRESS:
                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DoTestReaderWriterStress), IARG_REG_VALUE, RegThreadInfo,
                               IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
                break;
            case TEST_RW_TRYSTRESS:
                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DoTestReaderWriterTryStress), IARG_REG_VALUE, RegThreadInfo,
                               IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
                break;
            case TEST_SEMAPHORE:
                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DoTestSemaphore), IARG_REG_VALUE, RegThreadInfo,
                               IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
                break;
            case TEST_TRYLOCKS:
                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DoTestTryLocks), IARG_FUNCARG_ENTRYPOINT_VALUE, 0, IARG_END);
                break;
            default:
                ASSERTX(0);
        }
        RTN_Close(rtn);
    }
}

static void OnExit(INT32, VOID*)
{
    if (!FoundTestFunc || !FoundThreadCountFunc)
    {
        std::cout << "Couldn't find instrumentation routine(s)" << std::endl;
        PIN_ExitProcess(1);
    }

    // Destroy the lock variable.
    //
    switch (TestType)
    {
        case TEST_LOCK_INTEGRITY:
        case TEST_LOCK_STRESS:
            /* nothing to do */
            break;
        case TEST_MUTEX_INTEGRITY:
        case TEST_MUTEX_STRESS:
        case TEST_MUTEX_TRYSTRESS:
            break;
        case TEST_WRITER_INTEGRITY:
        case TEST_WRITER_STRESS:
        case TEST_WRITER_TRYSTRESS:
        case TEST_READER_STRESS:
        case TEST_READER_TRYSTRESS:
        case TEST_RW_INTEGRITY:
        case TEST_RW_STRESS:
        case TEST_RW_TRYSTRESS:
            break;
        case TEST_SEMAPHORE:
            PIN_SemaphoreFini(&Sem1);
            PIN_SemaphoreFini(&Sem2);
            break;
        case TEST_TRYLOCKS:
            PIN_SemaphoreFini(&Sem1);
            break;
        default:
            ASSERTX(0);
    }
}

static void GetThreadCount(ADDRINT threadCount)
{
    // Capture the number of worker threads in the application.
    // This is the number of threads that will execute DoLockTest().
    //
    RunningWorkers = threadCount;

    if (TestType == TEST_SEMAPHORE)
    {
        if (threadCount != 2)
        {
            std::cout << "Test 'semaphore' requires exactly two worker threads" << std::endl;
            PIN_ExitProcess(1);
        }
    }

    if (TestType == TEST_TRYLOCKS)
    {
        if (threadCount != 1)
        {
            std::cout << "Test 'trylocks' requires exactly one worker thread" << std::endl;
            PIN_ExitProcess(1);
        }
    }
}

// ----------------- Analysis routines that peform each test ----------------- //

static void DoTestLockIntegrity(THREADID tid, THREAD_INFO* info, UINT32* done)
{
    // This test checks to see if two threads can be in the PIN_LOCK mutex
    // simultaneously.

    PIN_GetLock(&Lock, tid);
    THREADID owner = HasLock;
    HasLock        = tid;

    if (owner != INVALID_THREADID)
    {
        std::cout << "Two theads in lock simultaneously: " << std::dec << tid << " and " << owner << std::endl;
        PIN_ExitProcess(1);
    }

    ATOMIC::OPS::Delay(DELAY_COUNT);

    HasLock      = INVALID_THREADID;
    THREADID ret = PIN_ReleaseLock(&Lock);

    if (ret != tid)
    {
        std::cout << "PIN_ReleaseLock returned unexpected value " << std::dec << ret << " (expected " << tid << ")" << std::endl;
        PIN_ExitProcess(1);
    }

    CheckIfDone(info, done);
}

static void DoTestLockStress(THREADID tid, THREAD_INFO* info, UINT32* done)
{
    // This test just tries to acquire and release PIN_LOCK as fast as possible
    // to see if we can provoke a deadlock due to missing a wakeup.

    PIN_GetLock(&Lock, tid);
    PIN_ReleaseLock(&Lock);
    CheckIfDone(info, done);
}

static void DoTestMutexIntegrity(THREADID tid, THREAD_INFO* info, UINT32* done)
{
    // This test checks to see if two threads can be in the PIN_MUTEX mutex
    // simultaneously.

    PIN_MutexLock(&Mutex);
    THREADID owner = HasLock;
    HasLock        = tid;

    if (owner != INVALID_THREADID)
    {
        std::cout << "Two theads in mutex simultaneously: " << std::dec << tid << " and " << owner << std::endl;
        PIN_ExitProcess(1);
    }

    ATOMIC::OPS::Delay(DELAY_COUNT);

    HasLock = INVALID_THREADID;
    PIN_MutexUnlock(&Mutex);

    CheckIfDone(info, done);
}

static void DoTestMutexStress(THREAD_INFO* info, UINT32* done)
{
    // This test just tries to acquire and release PIN_MUTEX as fast as possible
    // to see if we can provoke a deadlock due to missing a wakeup.

    PIN_MutexLock(&Mutex);
    PIN_MutexUnlock(&Mutex);
    CheckIfDone(info, done);
}

static void DoTestMutexTryStress(THREAD_INFO* info, UINT32* done)
{
    // Try to acquire / release PIN_MUTEX as fast as possible, using "try".

    if (PIN_MutexTryLock(&Mutex)) PIN_MutexUnlock(&Mutex);
    CheckIfDone(info, done);
}

static void DoTestWriterIntegrity(THREADID tid, THREAD_INFO* info, UINT32* done)
{
    // This test checks to see if two writer threads can be in the PIN_RWMUTEX mutex
    // simultaneously.

    PIN_RWMutexWriteLock(&RWMutex);
    THREADID owner = HasLock;
    HasLock        = tid;

    if (owner != INVALID_THREADID)
    {
        std::cout << "Two writer theads in rwmutex simultaneously: " << std::dec << tid << " and " << owner << std::endl;
        PIN_ExitProcess(1);
    }

    ATOMIC::OPS::Delay(DELAY_COUNT);

    HasLock = INVALID_THREADID;
    PIN_RWMutexUnlock(&RWMutex);

    CheckIfDone(info, done);
}

static void DoTestWriterStress(THREAD_INFO* info, UINT32* done)
{
    // This test just tries to acquire and release PIN_RWMUTEX as fast as possible
    // as a writer lock to see if we can provoke a deadlock due to missing a wakeup.

    PIN_RWMutexWriteLock(&RWMutex);
    PIN_RWMutexUnlock(&RWMutex);
    CheckIfDone(info, done);
}

static void DoTestWriterTryStress(THREAD_INFO* info, UINT32* done)
{
    // Try to acquire / release PIN_RWMUTEX as a writer using "try" as fast as possible.

    if (PIN_RWMutexTryWriteLock(&RWMutex)) PIN_RWMutexUnlock(&RWMutex);
    CheckIfDone(info, done);
}

static void DoTestReaderStress(THREAD_INFO* info, UINT32* done)
{
    // This test just tries to acquire and release PIN_RWMUTEX as fast as possible
    // as a reader lock.

    PIN_RWMutexReadLock(&RWMutex);
    PIN_RWMutexUnlock(&RWMutex);
    CheckIfDone(info, done);
}

static void DoTestReaderTryStress(THREAD_INFO* info, UINT32* done)
{
    // Try to acquire / release PIN_RWMUTEX as a reader using "try" as fast as possible.

    if (PIN_RWMutexTryReadLock(&RWMutex)) PIN_RWMutexUnlock(&RWMutex);
    CheckIfDone(info, done);
}

static void DoTestReaderWriterIntegrity(THREAD_INFO* info, UINT32* done)
{
    // This test checks that a "writer" thread can never hold the lock while
    // there is an active reader.

    if (info->_workerId & 1)
    {
        // Reader thread.
        //
        PIN_RWMutexReadLock(&RWMutex);
        ATOMIC::OPS::Increment(&ActiveReaders, 1);
        if (ATOMIC::OPS::Load(&IsActiveWriter))
        {
            std::cout << "Reader got lock while there is an active writer" << std::endl;
            PIN_ExitProcess(1);
        }

        ATOMIC::OPS::Delay(DELAY_COUNT);

        ATOMIC::OPS::Increment(&ActiveReaders, -1);
        PIN_RWMutexUnlock(&RWMutex);
    }
    else
    {
        // Writer thread.
        //
        PIN_RWMutexWriteLock(&RWMutex);
        ATOMIC::OPS::Store< BOOL >(&IsActiveWriter, TRUE);
        if (ATOMIC::OPS::Load(&ActiveReaders) != 0)
        {
            std::cout << "Writer has lock while there are active readers" << std::endl;
            PIN_ExitProcess(1);
        }

        ATOMIC::OPS::Delay(DELAY_COUNT);

        ATOMIC::OPS::Store< BOOL >(&IsActiveWriter, FALSE);
        PIN_RWMutexUnlock(&RWMutex);
    }
    CheckIfDone(info, done);
}

static void DoTestReaderWriterStress(THREAD_INFO* info, UINT32* done)
{
    // This test uses a mix of "reader" and "writer" threads to acquire and
    // release the PIN_RWMUTEX as fast as possible.

    if (info->_workerId & 1)
    {
        // Reader thread.
        //
        PIN_RWMutexReadLock(&RWMutex);
        PIN_RWMutexUnlock(&RWMutex);
    }
    else
    {
        // Writer thread.
        //
        PIN_RWMutexWriteLock(&RWMutex);
        PIN_RWMutexUnlock(&RWMutex);
    }
    CheckIfDone(info, done);
}

static void DoTestReaderWriterTryStress(THREAD_INFO* info, UINT32* done)
{
    // This test uses a mix of "reader" and "writer" threads to acquire and
    // release the PIN_RWMUTEX as fast as possible.  We acquire the lock using
    // "try".

    if (info->_workerId & 1)
    {
        // Reader thread.
        //
        if (PIN_RWMutexTryReadLock(&RWMutex)) PIN_RWMutexUnlock(&RWMutex);
    }
    else
    {
        // Writer thread.
        //
        if (PIN_RWMutexTryWriteLock(&RWMutex)) PIN_RWMutexUnlock(&RWMutex);
    }
    CheckIfDone(info, done);
}

static void DoTestSemaphore(THREAD_INFO* info, UINT32* done)
{
    // This test assumes exactly two threads.  The two threads take turns pinging
    // each other's semaphore.  We make sure that a thread does not wake up from
    // the semaphore unless it is set.  We also check for deadlocks due to missing
    // wakeups.
    if (info->_workerId == 0)
    {
        PIN_SemaphoreWait(&Sem1);

        if (!PIN_SemaphoreIsSet(&Sem1))
        {
            std::cout << "SemaphoreWait returned, but semaphore is not set" << std::endl;
            PIN_ExitProcess(1);
        }

        PIN_MutexLock(&Mutex);
        PIN_SemaphoreClear(&Sem1);
        PIN_SemaphoreSet(&Sem2);
        PIN_MutexUnlock(&Mutex);
    }
    else
    {
        PIN_SemaphoreWait(&Sem2);

        if (!PIN_SemaphoreIsSet(&Sem2))
        {
            std::cout << "SemaphoreWait returned, but semaphore is not set" << std::endl;
            PIN_ExitProcess(1);
        }

        PIN_MutexLock(&Mutex);
        PIN_SemaphoreClear(&Sem2);
        PIN_SemaphoreSet(&Sem1);
        PIN_MutexUnlock(&Mutex);
    }

    if (CheckIfDone(info, done))
    {
        PIN_SemaphoreSet(&Sem1);
        PIN_SemaphoreSet(&Sem2);
    }
}

static void DoTestTryLocks(UINT32* done)
{
    // This is a collection of single-thread tests that make sure that the
    // various "try" operations succeed or fail as expected.

    BOOL gotLock = PIN_MutexTryLock(&Mutex);
    if (!gotLock)
    {
        std::cout << "Failure on uncontended PIN_MutexTryLock" << std::endl;
        PIN_ExitProcess(1);
    }

    gotLock = PIN_MutexTryLock(&Mutex);
    if (gotLock)
    {
        std::cout << "PIN_MutexTryLock was able to get a lock twice" << std::endl;
        PIN_ExitProcess(1);
    }

    gotLock = PIN_RWMutexTryWriteLock(&RWMutex);
    if (!gotLock)
    {
        std::cout << "Failure on uncontended PIN_RWMutexTryWriteLock" << std::endl;
        PIN_ExitProcess(1);
    }

    gotLock = PIN_RWMutexTryWriteLock(&RWMutex);
    if (gotLock)
    {
        std::cout << "PIN_RWMutexTryWriteLock was able to get a lock twice" << std::endl;
        PIN_ExitProcess(1);
    }

    gotLock = PIN_RWMutexTryReadLock(&RWMutex);
    if (gotLock)
    {
        std::cout << "PIN_RWMutexTryReadLock was able to get a lock when writer owns it" << std::endl;
        PIN_ExitProcess(1);
    }

    PIN_RWMutexUnlock(&RWMutex);
    gotLock = PIN_RWMutexTryReadLock(&RWMutex);
    if (!gotLock)
    {
        std::cout << "Failure on uncontended PIN_RWMutexTryReadLock" << std::endl;
        PIN_ExitProcess(1);
    }

    gotLock = PIN_RWMutexTryReadLock(&RWMutex);
    if (!gotLock)
    {
        std::cout << "Unable to get a reader lock twice" << std::endl;
        PIN_ExitProcess(1);
    }

    PIN_SemaphoreSet(&Sem1);
    if (!PIN_SemaphoreIsSet(&Sem1))
    {
        std::cout << "Expected 'set' status from PIN_SemaphoreIsSet" << std::endl;
        PIN_ExitProcess(1);
    }

    PIN_SemaphoreClear(&Sem1);
    if (PIN_SemaphoreIsSet(&Sem1))
    {
        std::cout << "Expected 'clear' status from PIN_SemaphoreIsSet" << std::endl;
        PIN_ExitProcess(1);
    }

    if (PIN_SemaphoreTimedWait(&Sem1, 1))
    {
        std::cout << "Expected PIN_SemaphoreTimedWait to time-out" << std::endl;
        PIN_ExitProcess(1);
    }

    *done = 1;
}

// Common routine to check if the test is done.
//
static bool CheckIfDone(THREAD_INFO* info, UINT32* done)
{
    // Each thread exits after all threads have reached their iteration count.
    //
    if (info->_count++ == ITERATION_COUNT)
    {
        if (ATOMIC::OPS::Increment(&RunningWorkers, -1) <= 1) ATOMIC::OPS::Store< BOOL >(&AllExit, TRUE);
    }
    if (ATOMIC::OPS::Load(&AllExit))
    {
        *done = 1;
        return true;
    }
    return false;
}
