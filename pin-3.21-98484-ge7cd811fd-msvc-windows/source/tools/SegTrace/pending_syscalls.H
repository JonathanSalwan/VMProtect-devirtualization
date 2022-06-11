/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <map>

/*!
 * Information about a "pending" system call.  (The tool has started processing it, but
 * not yet completed.)
 */
struct PENDING_SYSCALL
{
    PENDING_SYSCALL() : _number(0) {}
    PENDING_SYSCALL(ADDRINT fs, ADDRINT gs, ADDRINT num, ADDRINT a1, ADDRINT a2, ADDRINT a3, ADDRINT a4, ADDRINT a5)
        : _fs(fs), _gs(gs), _number(num), _arg1(a1), _arg2(a2), _arg3(a3), _arg4(a4), _arg5(a5)
    {}

    ADDRINT _fs;     // Value of FS prior to system call
    ADDRINT _gs;     // Value of GS prior to system call
    ADDRINT _number; // System call number
    ADDRINT _arg1;   // First 5 arguments to system call
    ADDRINT _arg2;
    ADDRINT _arg3;
    ADDRINT _arg4;
    ADDRINT _arg5;
};

/*!
 * A container which can hold one pending system call for each thread.
 */
class PENDING_SYSCALLS
{
  public:
    PENDING_SYSCALLS() { PIN_InitLock(&_lock); }

    VOID Add(THREADID tid, const PENDING_SYSCALL& pend)
    {
        PIN_GetLock(&_lock, 1);
        _map[tid] = pend;
        PIN_ReleaseLock(&_lock);
    }

    BOOL Remove(THREADID tid, PENDING_SYSCALL* pend)
    {
        PIN_GetLock(&_lock, 1);
        MAP::iterator it = _map.find(tid);
        if (it == _map.end())
        {
            PIN_ReleaseLock(&_lock);
            return FALSE;
        }

        *pend = (*it).second;
        _map.erase(it);
        PIN_ReleaseLock(&_lock);
        return TRUE;
    }

  private:
    PIN_LOCK _lock;

    typedef std::map< UINT32, PENDING_SYSCALL > MAP;
    MAP _map;
};
