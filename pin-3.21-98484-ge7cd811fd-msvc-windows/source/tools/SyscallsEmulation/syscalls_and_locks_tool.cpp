/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*!
 * This tool should run with the application "syscalls_and_locks_app.cpp". See full details in the application source code.
 */

#include "pin.H"
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

// This knob represents which type of lock should be acquired:
// 1 - PIN_LOCK
// 2 - PIN_RWMUTEX
// 3 - CLIENT LOCK
KNOB< UINT > KnobLockType(KNOB_MODE_WRITEONCE, "pintool", "lock_type", "0", "lock_type");

volatile bool isLockAcquired = false;

PIN_LOCK _lock;

PIN_RWMUTEX _rw_mutex;

VOID Replacement_AcquireAndReleaseLock()
{
    switch (KnobLockType)
    {
        case (1):
        {
            PIN_InitLock(&_lock);
            PIN_GetLock(&_lock, 1);
            break;
        }
        case (2):
        {
            PIN_RWMutexInit(&_rw_mutex);
            PIN_RWMutexWriteLock(&_rw_mutex);
            break;
        }
        case (3):
        {
            PIN_LockClient();
            break;
        }
        default:
        {
            break;
        }
    }

    // Notify the application that the lock has been acquired.
    isLockAcquired = true;

    // Sleep 20 seconds in order to let t1 to call to relevant system call/function
    // while t2 holds the lock.
    sleep(20);

    switch (KnobLockType)
    {
        case (1):
        {
            PIN_ReleaseLock(&_lock);
            break;
        }
        case (2):
        {
            PIN_RWMutexUnlock(&_rw_mutex);
            break;
        }
        case (3):
        {
            PIN_UnlockClient();
            break;
        }
        default:
            break;
    }
}

VOID Replacement_WaitThread2AcquireLock()
{
    while (!isLockAcquired)
        sched_yield();
}

VOID ImageLoad(IMG img, void* v)
{
    if (IMG_IsMainExecutable(img))
    {
        RTN rtn = RTN_Invalid();

        rtn = RTN_FindByName(img, "WaitThread2AcquireLock");
        if (RTN_Valid(rtn))
        {
            RTN_ReplaceProbed(rtn, AFUNPTR(Replacement_WaitThread2AcquireLock));
        }

        rtn = RTN_FindByName(img, "WaitUntilLockAcquiredAndReleased");
        if (RTN_Valid(rtn))
        {
            RTN_ReplaceProbed(rtn, AFUNPTR(Replacement_AcquireAndReleaseLock));
        }
    }
}
/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    PIN_StartProgramProbed();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
