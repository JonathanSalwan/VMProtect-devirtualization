/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: pinvm
// <FILE-TYPE>: public header

#ifndef PINSYNC_HPP
#define PINSYNC_HPP

#include "os-apis.h"
#include <map>
#include <string>

namespace PINVM
{
/*!
 * Interface for generic lock
 */
class ILOCK /*<INTERFACE>*/
{
  public:
    /*!
     * Destructor
     */
    virtual ~ILOCK() {}

    /*!
     * Blocks the caller until the lock can be acquired.
     */
    virtual void Lock() = 0;

    /*!
     * Releases the lock.
     */
    virtual void Unlock() = 0;

    /*!
     * Attempts to acquire the lock, but does not block the caller.
     *
     * @return  Returns TRUE if the lock is acquired, FALSE if not.
     */
    virtual bool TryLock() = 0;
};

/*!
 * Basic non-recursive lock.
 */
class PINSYNC_LOCK /*<UTILITY>*/
{
  public:
    /*!
     * The initial sate of the lock is "not locked".
     */
    PINSYNC_LOCK() { OS_MutexInit(&_impl); }

    /*!
     * It is not necessary to call this method.  It is provided only for symmetry.
     *
     * @return  Always returns TRUE.
     */
    bool Initialize()
    {
        OS_MutexInit(&_impl);
        return true;
    }

    /*!
     * Destroy a mutex
     */
    void Destroy() { OS_MutexDestroy(&_impl); }

    /*!
     * Set the state of the lock to "not locked", even if the calling thread
     * does not own the lock.
     */
    void Reset() { OS_MutexUnlock(&_impl); }

    /*!
     * Blocks the caller until the lock can be acquired.
     */
    void Lock() { OS_MutexLock(&_impl); }

    /*!
     * Releases the lock.
     */
    void Unlock() { OS_MutexUnlock(&_impl); }

    /*!
     * Attempts to acquire the lock, but does not block the caller.
     *
     * @return  Returns TRUE if the lock is acquired, FALSE if not.
     */
    bool TryLock() { return OS_MutexTryLock(&_impl); }

  private:
    OS_MUTEX_TYPE _impl;
};

/*!
 * Basic non-recursive lock with POD semantics.
 */
typedef struct
{
  public:
    /*!
     * It is not necessary to call this method.  It is provided only for symmetry.
     *
     * @return  Always returns TRUE.
     */
    bool Initialize()
    {
        OS_MutexInit(&_impl);
        return true;
    }

    /*!
     * Destroy a mutex
     */
    void Destroy() { OS_MutexDestroy(&_impl); }

    /*!
     * Set the state of the lock to "not locked", even if the calling thread
     * does not own the lock.
     */
    void Reset() { OS_MutexUnlock(&_impl); }

    /*!
     * Blocks the caller until the lock can be acquired.
     */
    void Lock() { OS_MutexLock(&_impl); }

    /*!
     * Releases the lock.
     */
    void Unlock() { OS_MutexUnlock(&_impl); }

    /*!
     * Attempts to acquire the lock, but does not block the caller.
     *
     * @return  Returns TRUE if the lock is acquired, FALSE if not.
     */
    bool TryLock() { return OS_MutexTryLock(&_impl); }

    OS_MUTEX_TYPE _impl;
} PINSYNC_POD_LOCK;

/*!
 * Basic non-recursive lock with SAFEPOD semantics.
 */
typedef PINSYNC_POD_LOCK PINSYNC_SAFEPOD_LOCK;

/*!
 * Read-writer lock.
 */
class PINSYNC_RWLOCK /*<UTILITY>*/
{
  public:
    /*!
     * The new lock is initially not acquired
     */
    PINSYNC_RWLOCK() { OS_RWLockInitialize(&_impl); }

    /*!
     * Destructor
     */
    ~PINSYNC_RWLOCK() { Destroy(); }

    /*!
     * It is not necessary to call this method.  It is provided only for symmetry.
     *
     * @return  Always returns TRUE.
     */
    bool Initialize()
    {
        OS_RWLockInitialize(&_impl);
        return true;
    }

    /*!
     * Destroy a read-writer lock
     */
    void Destroy() { OS_RWLockDestroy(&_impl); }

    /*!
     * Set the state of the lock to "not locked", even if the calling thread
     * does not own the lock.
     */
    void Reset()
    {
        Destroy();
        Initialize();
    }

    /*!
     * Acquire the lock for "read" access.  Multiple "reader" threads may
     * simultaneously acquire the lock.
     */
    void ReadLock() { OS_RWLockAcquireRead(&_impl); }

    /*!
     * Acquire the lock for exclusive "write" access.  A "writer" thread has
     * exclusive ownership of the lock, not shared by any other "reader" or
     * "writer" threads.
     */
    void WriteLock() { OS_RWLockAcquireWrite(&_impl); }

    /*!
     * Release the lock.  Used for both "readers" and "writers".
     */
    void Unlock() { OS_RWLockRelease(&_impl); }

    /*!
     * Attempts to acquire the lock as a "reader" thread, but does not
     * block the caller.
     *
     * @return  Returns TRUE if the lock is acquired, FALSE if not.
     */
    bool TryReadLock() { return OS_RWLockTryAcquireRead(&_impl); }

    /*!
     * Attempts to acquire the lock as an exclusive "writer" thread, but
     * does not block the caller.
     *
     * @return  Returns TRUE if the lock is acquired, FALSE if not.
     */
    bool TryWriteLock() { return OS_RWLockTryAcquireWrite(&_impl); }

  private:
    OS_APIS_RW_LOCK_T _impl;
};

/*!
 * Read-writer lock with POD semantics.
 */
typedef struct
{
  public:
    /*!
     * It is not necessary to call this method.  It is provided only for symmetry.
     *
     * @return  Always returns TRUE.
     */
    bool Initialize()
    {
        OS_RWLockInitialize(&_impl);
        return true;
    }

    /*!
     * Destroy a read-writer lock
     */
    void Destroy() { OS_RWLockDestroy(&_impl); }

    /*!
     * Set the state of the lock to "not locked", even if the calling thread
     * does not own the lock.
     */
    void Reset() { OS_RWLockRelease(&_impl); }

    /*!
     * Acquire the lock for "read" access.  Multiple "reader" threads may
     * simultaneously acquire the lock.
     */
    void ReadLock() { OS_RWLockAcquireRead(&_impl); }

    /*!
     * Acquire the lock for exclusive "write" access.  A "writer" thread has
     * exclusive ownership of the lock, not shared by any other "reader" or
     * "writer" threads.
     */
    void WriteLock() { OS_RWLockAcquireWrite(&_impl); }

    /*!
     * Release the lock.  Used for both "readers" and "writers".
     */
    void Unlock() { OS_RWLockRelease(&_impl); }

    /*!
     * Attempts to acquire the lock as a "reader" thread, but does not
     * block the caller.
     *
     * @return  Returns TRUE if the lock is acquired, FALSE if not.
     */
    bool TryReadLock() { return OS_RWLockTryAcquireRead(&_impl); }

    /*!
     * Attempts to acquire the lock as an exclusive "writer" thread, but
     * does not block the caller.
     *
     * @return  Returns TRUE if the lock is acquired, FALSE if not.
     */
    bool TryWriteLock() { return OS_RWLockTryAcquireWrite(&_impl); }

    OS_APIS_RW_LOCK_T _impl;
} PINSYNC_POD_RWLOCK;

#ifdef CC_FAST_LOOKUP
/*!
 * Recursive Read-writer lock.
 */
template< int MaxThreads > class PINSYNC_RECURSIVE_RWLOCK /*<UTILITY>*/
{
  public:
    /*!
     * The new lock is initially not acquired
     */
    PINSYNC_RECURSIVE_RWLOCK()
    {
        _writer_tid            = INVALID_NATIVE_TID;
        _write_recursion_level = 0;
        _read_recursion_level.ClearNonAtomic();
        _impl.Initialize();
        _mutex.Initialize();
        _snapshots.clear();
        _pausedInAtLeastOneThread = false;
        _dependent_lock           = NULL;
    }

    virtual ~PINSYNC_RECURSIVE_RWLOCK()
    {
        _impl.Destroy();
        _mutex.Destroy();
    }

    /*!
     * Define a hierarchy between this lock and another lock.
     * This lock must not be acquired if the dependent lock is already locked.
     */
    void SetDependentLock(PINSYNC_RECURSIVE_RWLOCK* dependent_lock) { _dependent_lock = dependent_lock; }

    /*!
     * Set the state of the lock to "not locked", even if the calling thread
     * does not own the lock.
      */
    void Reset()
    {
        _writer_tid            = INVALID_NATIVE_TID;
        _write_recursion_level = 0;
        _read_recursion_level.ClearNonAtomic();
        _impl.Reset();
        _mutex.Reset();
        _snapshots.clear();
        _pausedInAtLeastOneThread = false;
        _dependent_lock           = NULL;
    }

    /*!
     * Acquire the lock for "read" access.  Multiple "reader" threads may
     * simultaneously acquire the lock.
     * To prevent a thread from deadlocking on itself, only nested Read locks are allowed.
     */
    virtual void ReadLock(NATIVE_TID tid)
    {
        ASSERTXSLOW(IsPaused(tid) == false);
        ASSERTXSLOW(IsLegalLockChain(tid));
        int recursion_level;
        bool mayAcquireReadLock = MayAcquireReadLock(tid, &recursion_level);
        ASSERTX(mayAcquireReadLock);
        if (recursion_level == 0) _impl.ReadLock();
        RegisterReadLock(tid);
    }

    /*!
     * Acquire the lock for exclusive "write" access.  A "writer" thread has
     * exclusive ownership of the lock, not shared by any other "reader" or
     * "writer" threads.
     * To prevent a thread from deadlocking on itself, once a thread took a Write lock,
     * and for as long as it is locked, it may not take another lock, neither Read nor Write.
     */
    virtual void WriteLock(NATIVE_TID tid)
    {
        ASSERTXSLOW(IsPaused(tid) == false);
        ASSERTXSLOW(IsLegalLockChain(tid));
        int recursion_level;
        bool mayAcquireWriteLock = MayAcquireWriteLock(tid, &recursion_level);
        ASSERTX(mayAcquireWriteLock);
        if (recursion_level == 0) _impl.WriteLock();
        RegisterWriteLock(tid);
    }

    /*!
     * Release the lock.  Used for both "readers" and "writers".
     */
    virtual void Unlock(NATIVE_TID tid)
    {
        ASSERTXSLOW(IsPaused(tid) == false);
        ASSERTXSLOW(IsLegalLockChain(tid));
        Unlock(tid, true);
    }

    /*!
     * Release the lock if it is currently locked.  Used for both "readers" and "writers".
     */
    virtual void UnlockIfLocked(NATIVE_TID tid)
    {
        ASSERTXSLOW(IsPaused(tid) == false);
        ASSERTXSLOW(IsLegalLockChain(tid));
        Unlock(tid, false);
    }

    /*!
     * Save the state of all locks (either reader and writer locks) acquired by the current thread,
       then release these locks so other threads can acquire the lock.
       The state of all locks is saved in a way that allows the current thread to later
       re-claim the exact locks when calling ResumeLock().
     */
    virtual void PauseLock(NATIVE_TID tid)
    {
        ASSERTXSLOW(IsPaused(tid) == false);
        ASSERTXSLOW(IsLegalLockChain(tid));
        int* rd_recursion_ptr = _read_recursion_level.Find(tid);
        int rd_recursion      = (NULL == rd_recursion_ptr) ? 0 : *rd_recursion_ptr;
        int wr_recursion      = (_writer_tid == tid) ? _write_recursion_level : 0;
        if ((rd_recursion == 0) && (wr_recursion == 0))
        {
            return;
        }
        _mutex.Lock();
        typename SNAPSHOTS_MAP::iterator it_snp = _snapshots.find(tid);
        ASSERTX(it_snp == _snapshots.end());
        _snapshots.insert(std::make_pair(tid, new Snapshot(rd_recursion, wr_recursion)));
        _pausedInAtLeastOneThread = true;
        if (wr_recursion >= 1)
        {
            _write_recursion_level = 1;
            if (rd_recursion_ptr != NULL) *rd_recursion_ptr = 0;
        }
        else
        {
            if (rd_recursion_ptr != NULL) *rd_recursion_ptr = 1;
        }
        _mutex.Unlock();
        Unlock(tid);
    }

    /*!
     * Restore the lock state saved by the call to PauseLock() and acquire the lock.
     * If the lock had both active Write locks and Read locks then the Write lock takes precedence
     * since only Write can be at the top of the lock chain.
     */
    virtual void ResumeLock(NATIVE_TID tid)
    {
        ASSERTXSLOW(IsLegalLockChain(tid));
        _mutex.Lock();
        typename SNAPSHOTS_MAP::iterator it_snp = _snapshots.find(tid);
        if (it_snp == _snapshots.end())
        {
            _mutex.Unlock();
            return;
        }
        int rd_recursion = (it_snp->second)->read_recursion_level;
        int wr_recursion = (it_snp->second)->write_recursion_level;
        _snapshots.erase(it_snp);
        _pausedInAtLeastOneThread = _snapshots.size() > 0;
        _mutex.Unlock();

        if (wr_recursion)
        {
            WriteLock(tid);
            wr_recursion--;
        }
        else if (rd_recursion)
        {
            ReadLock(tid);
            rd_recursion--;
        }

        for (int i = 0; i < wr_recursion; i++)
            RegisterWriteLock(tid);

        for (int i = 0; i < rd_recursion; i++)
            RegisterReadLock(tid);
    }

    /*!
     * Attempts to acquire the lock as a "reader" thread, but does not
     * block the caller.
     *
     * @return  Returns TRUE if the lock is acquired, FALSE if not.
     */
    virtual bool TryReadLock(NATIVE_TID tid)
    {
        ASSERTXSLOW(IsPaused(tid) == false);
        ASSERTXSLOW(IsLegalLockChain(tid));
        int recursion_level;
        bool mayAcquireReadLock = MayAcquireReadLock(tid, &recursion_level);
        ASSERTX(mayAcquireReadLock);
        if (recursion_level == 0)
        {
            if (!_impl.TryReadLock()) return false;
        }

        RegisterReadLock(tid);
        return true;
    }

    /*!
     * Attempts to acquire the lock as an exclusive "writer" thread, but
     * does not block the caller.
     *
     * @return  Returns TRUE if the lock is acquired, FALSE if not.
     */
    virtual bool TryWriteLock(NATIVE_TID tid)
    {
        ASSERTXSLOW(IsPaused(tid) == false);
        ASSERTXSLOW(IsLegalLockChain(tid));
        int recursion_level;
        bool mayAcquireWriteLock = MayAcquireWriteLock(tid, &recursion_level);
        ASSERTX(mayAcquireWriteLock);
        if (recursion_level == 0)
        {
            if (!_impl.TryWriteLock()) return false;
        }

        RegisterWriteLock(tid);
        return true;
    }

    /*!
     * Returns TRUE if the thread 'tid' has acquired either a Read lock or Write lock.
     *
     * @return  Returns TRUE if a Read/Write lock is acquired, FALSE if not.
     */
    virtual bool IsLockedByThread(NATIVE_TID tid)
    {
        if (_writer_tid == tid)
        {
            return true;
        }
        else
        {
            int* recursion_level_ptr = _read_recursion_level.Find(tid);
            return (NULL != recursion_level_ptr) && (*recursion_level_ptr > 0);
        }
    }

  private:
    void Unlock(NATIVE_TID tid, bool assert_is_locked)
    {
        int recursion_level = 0;
        // Unregister Read locks first because only Write can be at the top of the lock chain
        bool lock_unregistered = UnregisterReadLock(tid, &recursion_level);
        if (lock_unregistered)
        {
            if (recursion_level == 0) recursion_level = GetWriteRecursionLevel(tid);
        }
        else
        {
            lock_unregistered = UnregisterWriteLock(tid, &recursion_level);
        }
        ASSERTX((assert_is_locked == false) || lock_unregistered);
        if (lock_unregistered && (recursion_level == 0)) _impl.Unlock();
    }

    /*!
     * Checks the conditions for whether it is legal to take a Read lock for this thread.
     * Taking a Read lock is always legal.
     * Only the first lock in the recursion chain for the current thread will be physically taken.
     * If the current thread has an active Write lock then implicitly no other thread will modify
     * the data so the requirement for the Read lock is met.
     * If the current thread has an active Read lock then it is legal to take another Read lock.
     *
     * @param[in]   tid                     The thread Id
     * @param[out] recursionLevel     The current recursion level, read + write.

     * @return  Returns TRUE if may acquire Read lock, FALSE if not.
     */
    bool MayAcquireReadLock(NATIVE_TID tid, int* recursionLevel)
    {
        int* recursion_level_ptr = _read_recursion_level.Find(tid);
        *recursionLevel          = (NULL == recursion_level_ptr) ? 0 : *recursion_level_ptr;
        if (tid == _writer_tid) *recursionLevel += _write_recursion_level;
        return true;
    }

    /*!
     * Checks the conditions for whether it is legal to take a Write lock for this thread.
     * Taking a Write lock is legal only if this thread does not have an active Read physical lock.
     * If the thread has an active Write lock, it is legal to take another Write lock.
     *
     * @param[in]   tid                     The thread Id
     * @param[out] recursionLevel     The current recursion level, read + write.

     * @return  Returns TRUE if may acquire Write lock, FALSE if not.
     */
    bool MayAcquireWriteLock(NATIVE_TID tid, int* recursionLevel)
    {
        *recursionLevel          = 0;
        int* recursion_level_ptr = _read_recursion_level.Find(tid);
        if ((tid == _writer_tid) || (NULL == recursion_level_ptr) || (*recursion_level_ptr == 0))
        {
            if (tid == _writer_tid) *recursionLevel += _write_recursion_level;
            if (NULL != recursion_level_ptr) *recursionLevel += *recursion_level_ptr;
            return true;
        }
        return false;
    }

    /*!
     * Register a Read lock.
     */
    void RegisterReadLock(NATIVE_TID tid)
    {
        int* recursion_level_ptr = _read_recursion_level.Find(tid);
        if (NULL != recursion_level_ptr)
            ATOMIC::OPS::Increment(recursion_level_ptr, 1);
        else
            _read_recursion_level.Add(tid, 1);
    }

    /*!
     * Unregister a Read lock, if one exists for tid.
     *
     * @param[in]   tid                     The thread Id
     * @param[out] recursionLevel      The Read recursion level after unregister, or 0 if there was no Read lock for tid.
     *
     * @return  Return true if this tid had an active Read lock, false otherwise.
     */
    bool UnregisterReadLock(NATIVE_TID tid, int* recursionLevel)
    {
        *recursionLevel          = 0;
        int* recursion_level_ptr = _read_recursion_level.Find(tid);
        if (NULL != recursion_level_ptr)
        {
            *recursionLevel = ATOMIC::OPS::Increment(recursion_level_ptr, -1) - 1;
            ASSERTX(*recursionLevel >= 0);
            if (*recursionLevel == 0) _read_recursion_level.Remove(tid);
            return true;
        }
        return false;
    }

    /*!
     * Register a Write lock.
     */
    void RegisterWriteLock(NATIVE_TID tid)
    {
        _writer_tid = tid;
        ATOMIC::OPS::Increment(&_write_recursion_level, 1);
    }

    /*!
     * Unregister a Write lock, if one exists for tid.
     *
     * @param[in]   tid                     The thread Id
     * @param[out] recursionLevel      The Write recursion level after unregister, or 0 if there was no Write lock for tid.
     *
     * @return  Return true if this tid had an active Write lock, false otherwise.
     */
    bool UnregisterWriteLock(NATIVE_TID tid, int* recursionLevel)
    {
        *recursionLevel = 0;
        if (_writer_tid == tid)
        {
            *recursionLevel = ATOMIC::OPS::Increment(&_write_recursion_level, -1) - 1;

            if (_write_recursion_level == 0)
            {
                _writer_tid = INVALID_NATIVE_TID;
            }
            return true;
        }
        return false;
    }

    /*!
     * Get the current write recursion level for tid
     *
     * @param[in]   tid                     The thread Id
     *
     * @return  Return 0 if tid has no active write lock, otherwise returns the write lock recursion level.
     */
    int GetWriteRecursionLevel(NATIVE_TID tid) { return (_writer_tid == tid) ? _write_recursion_level : 0; }

    /*!
     * Checks whether the lock for tid is in paused state.
     *
     * @return  Returns TRUE if the lock for tid is in paused state, FALSE otherwise.
     */
    bool IsPaused(NATIVE_TID tid)
    {
        if (!_pausedInAtLeastOneThread) return false;

        _mutex.Lock();
        typename SNAPSHOTS_MAP::const_iterator it = _snapshots.find(tid);
        bool is_paused                            = (it != _snapshots.end());
        _mutex.Unlock();
        return is_paused;
    }

    /*!
     * Checks whether the lock chain is legal.
     * Conditions for a legal lock chain:
     * 1. There is no dependency on another lock.
     * 2. This lock is already locked. In this case it doesn't matter whether the dependent
     *     lock is locked or not because we know that we will not take another physical lock.
     * 3. The dependent lock is not in locked state.
     *
     * @return  Returns TRUE if the lock chain is legal, FALSE otherwise.
     */
    bool IsLegalLockChain(NATIVE_TID tid)
    {
        if (_dependent_lock == NULL) return true;
        if (IsLockedByThread(tid)) return true;
        if (_dependent_lock->IsLockedByThread(tid) == false) return true;
        return false;
    }

    PINSYNC_POD_RWLOCK _impl;
    NATIVE_TID _writer_tid;
    int _write_recursion_level;
    typedef ATOMIC::FIXED_MULTIMAP< NATIVE_TID, int, INVALID_NATIVE_TID, NATIVE_TID_CURRENT, MaxThreads > READERS_MAP;
    READERS_MAP _read_recursion_level;
    PINSYNC_POD_LOCK _mutex;
    struct Snapshot
    {
        int read_recursion_level;
        int write_recursion_level;
        Snapshot(int rd_rec, int wr_rec) : read_recursion_level(rd_rec), write_recursion_level(wr_rec) {}
    };
    typedef std::map< NATIVE_TID, Snapshot* > SNAPSHOTS_MAP;
    SNAPSHOTS_MAP _snapshots;
    bool _pausedInAtLeastOneThread;
    PINSYNC_RECURSIVE_RWLOCK* _dependent_lock;
};
#endif

/*!
 * Binary semaphore.
 * This synchronization object works as a barrier.
 */
class PINSYNC_SEMAPHORE /*<UTILITY>*/
{
  public:
    /*!
     * The initial state of the semaphore is "clear".
     */
    PINSYNC_SEMAPHORE()
    {
        OS_MutexInit(&_impl);
        Clear();
    }

    /*!
     * Destructor
     */
    ~PINSYNC_SEMAPHORE() { Destroy(); }

    /*!
     * It is not necessary to call this method.  It is provided only for symmetry.
     *
     * @return  Always returns TRUE.
     */
    bool Initialize()
    {
        OS_MutexInit(&_impl);
        Clear();
        return true;
    }

    /*!
     * Destroy a semaphore
     */
    void Destroy() { OS_MutexDestroy(&_impl); }

    /*!
     * Change the semaphore to "set" state and tell any waiters in Wait() or
     * TimedWait() to resume.  Those threads are guaranteed to return from
     * Wait() or TimedWait() only if the semaphore is still "set" when they
     * actually do resume running.
     */
    void Set()
    {
        _isSet = true;
        OS_MutexUnlock(&_impl);
    }

    /*!
     * Change the semaphore to "clear" state.
     */
    void Clear()
    {
        _isSet = false;
        OS_MutexTryLock(&_impl);
    }

    /*!
     * Check whether the semaphore's state is "set".  This method always returns
     * immediately.
     *
     * @return  TRUE if the state is "set".
     */
    bool IsSet() { return _isSet; }

    /*!
     * Block the calling thread until the semaphore's state is "set".  This
     * method returns immediately if the state is already "set".
     */
    void Wait()
    {
        OS_MutexLock(&_impl);
        OS_MutexUnlock(&_impl);
    }

    /*!
     * Block the calling thread until the semaphore's state is "set" or until
     * a timeout expires.  This method returns immediately if the state is
     * already "set".
     *
     *  @param[in] timeout  Maximum number of milliseconds to wait.
     *
     * @return  TRUE if the semaphore is in "set" state.  FALSE if this method
     *           returns instead due to the timeout.
     */
    bool TimedWait(unsigned timeout)
    {
        bool res = OS_MutexTimedLock(&_impl, timeout);
        if (res) OS_MutexUnlock(&_impl);
        return res;
    }

  private:
    OS_MUTEX_TYPE _impl;

    /*!
     * This member field was introduced only to be used in the IsSet() method.
     * Because the implementation of Wait() on a "set" PINSYNC_SEMAPHORE is to immediately lock
     * and unlock the '_impl' mutex, IsSet() might return 'false' in the short time between
     * the lock and unlock.
     * Using this member field as a return values from IsSet() will return the right result in this case.
     */
    volatile bool _isSet;
};

/*!
 * Binary semaphore with POD semantics.
 * This synchronization object works as a barrier.
 */
typedef struct
{
  public:
    /*!
     * Initialize a semaphore
     *
     * @return  Always returns TRUE.
     */
    bool Initialize()
    {
        OS_MutexInit(&_impl);
        Clear();
        return true;
    }

    /*!
     * Destroy a semaphore
     */
    void Destroy()
    {
        OS_MutexDestroy(&_impl);
        Clear();
    }

    /*!
     * Change the semaphore to "set" state and tell any waiters in Wait() or
     * TimedWait() to resume.  Those threads are guaranteed to return from
     * Wait() or TimedWait() only if the semaphore is still "set" when they
     * actually do resume running.
     */
    void Set() { OS_MutexUnlock(&_impl); }

    /*!
     * Change the semaphore to "clear" state.
     */
    void Clear() { OS_MutexTryLock(&_impl); }

    /*!
     * Check whether the semaphore's state is "set".  This method always returns
     * immediately.
     *
     * @return  TRUE if the state is "set".
     */
    bool IsSet() { return !OS_MutexIsLocked(&_impl); }

    /*!
     * Block the calling thread until the semaphore's state is "set".  This
     * method returns immediately if the state is already "set".
     */
    void Wait()
    {
        OS_MutexLock(&_impl);
        OS_MutexUnlock(&_impl);
    }

    /*!
     * Block the calling thread until the semaphore's state is "set" or until
     * a timeout expires.  This method returns immediately if the state is
     * already "set".
     *
     *  @param[in] timeout  Maximum number of milliseconds to wait.
     *
     * @return  TRUE if the semaphore is in "set" state.  FALSE if this method
     *           returns instead due to the timeout.
     */
    bool TimedWait(unsigned timeout)
    {
        bool res = OS_MutexTimedLock(&_impl, timeout);
        if (res) OS_MutexUnlock(&_impl);
        return res;
    }

    OS_MUTEX_TYPE _impl;
} PINSYNC_POD_SEMAPHORE;

} // namespace PINVM

#endif // file guard
