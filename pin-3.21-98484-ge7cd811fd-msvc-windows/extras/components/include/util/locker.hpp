/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: util
// <FILE-TYPE>: component public header

#ifndef UTIL_LOCKER_HPP
#define UTIL_LOCKER_HPP

namespace UTIL
{
/*!
 * A simple utility that manages a mutex lock.  The lock is automatically acquired on
 * entry to the scope and released on exit.
 *
 *  @param LOCK     Any type that supports Lock() and Unlock() methods with the usual semantics.
 */
template< class LOCK > class /*<UTILITY>*/ LOCKER
{
  public:
    /*!
     *  @param[in] lock     The lock to acquire.
     *  @param[in] acquire  If TRUE, the constructor acquires the lock.
     */
    LOCKER(LOCK* lock, bool acquire = true) : _lock(lock), _isLocked(acquire)
    {
        if (acquire) _lock->Lock();
    }

    /*!
     * The destructor releases the lock unless it was manually released via Unlock().
     */
    ~LOCKER()
    {
        if (_isLocked) _lock->Unlock();
    }

    /*!
     * Manually release the lock.
     */
    void Unlock()
    {
        _lock->Unlock();
        _isLocked = false;
    }

    /*!
     * Manually acquire the lock.  This only makes sense if the lock was manually released
     * via Unlock().  No error checking is done to prevent self-deadlock.
     */
    void Lock()
    {
        _lock->Lock();
        _isLocked = true;
    }

    /*!
     * Indicate that the lock has been acquired or released outside of this wrapper.
     *
     *  @param[in] isLocked     TRUE if the lock has been acquired.
     */
    void SetLocked(bool isLocked) { _isLocked = isLocked; }

  private:
    LOCK* _lock;

    // _lock might be dead/deallocated while we are inside a scope, we don't want to unlock it in this case (will cause exception).
    // _isLocked gives us a way out in such a case (See SetLocked()).
    bool _isLocked;
};

/*!
 * A simple utility that manages a RW mutex lock for READ lock oprtations.  The lock is automatically acquired on
 * entry to the scope and released on exit.
 *
 *  @param LOCK     Any type that supports ReadLock(), WriteLock() and Unlock() methods with the usual semantics.
 */
template< class LOCK > class /*<UTILITY>*/ RWLOCKER
{
  public:
    enum LockType
    {
        READ,
        WRITE
    };

    /*!
     *  @param[in] lock     The lock to acquire.
     *  @param[in] acquire  If TRUE, the constructor acquires the lock.
     */
    RWLOCKER(LOCK* lock, LockType lock_type, bool acquire = true) : _lock(lock), _isLocked(false)
    {
        if (acquire)
        {
            Lock(lock_type);
        }
    }

    /*!
     * The destructor releases the lock unless it was manually released via Unlock().
     */
    ~RWLOCKER() { Unlock(); }

    /*!
     * Manually release the lock.
     */
    void Unlock()
    {
        if (_isLocked)
        {
            if (NULL != _lock)
            {
                _lock->Unlock();
            }
            _isLocked = false;
        }
    }

    /*!
     * Manually acquire the lock.  This only makes sense if the lock was manually released
     * via Unlock().  No error checking is done to prevent self-deadlock.
     */
    void Lock(LockType lock_type)
    {
        if (NULL != _lock)
        {
            switch (lock_type)
            {
                case RWLOCKER::READ:
                    _lock->ReadLock();
                    break;
                case RWLOCKER::WRITE:
                    _lock->WriteLock();
                    break;
            }
        }
        _isLocked = true;
    }

  private:
    LOCK* _lock;
    bool _isLocked;
};

#ifdef CC_FAST_LOOKUP
/*!
 * A simple utility that manages a RW mutex lock for READ lock oprtations.  The lock is automatically acquired on
 * entry to the scope and released on exit.
 *
 *  @param LOCK     Any type that supports ReadLock(), WriteLock() and Unlock() methods with the usual semantics.
 */
template< class LOCK, NATIVE_TID (*GetTidFn)() > class /*<UTILITY>*/ RWLOCKER_WITH_TID
{
  public:
    enum LockType
    {
        READ,
        WRITE
    };

    /*!
     *  @param[in] lock     The lock to acquire.
     *  @param[in] acquire  If TRUE, the constructor acquires the lock.
     */
    RWLOCKER_WITH_TID(LOCK* lock, LockType lock_type, bool acquire = true) : _lock(lock), _tid(INVALID_NATIVE_TID)
    {
        if (acquire)
        {
            Lock(lock_type);
        }
    }

    /*!
     * The destructor releases the lock unless it was manually released via Unlock().
     */
    ~RWLOCKER_WITH_TID() { Unlock(); }

    /*!
     * Manually release the lock.
     */
    void Unlock()
    {
        if (_tid != INVALID_NATIVE_TID && NULL != _lock)
        {
            _lock->Unlock(_tid);
            _tid = INVALID_NATIVE_TID;
        }
    }

    /*!
     * Manually acquire the lock.  This only makes sense if the lock was manually released
     * via Unlock().  No error checking is done to prevent self-deadlock.
     */
    void Lock(LockType lock_type)
    {
        if (_lock == NULL) return;
        _tid = GetTidFn();
        switch (lock_type)
        {
            case RWLOCKER_WITH_TID< LOCK, GetTidFn >::READ:
                _lock->ReadLock(_tid);
                break;
            case RWLOCKER_WITH_TID< LOCK, GetTidFn >::WRITE:
                _lock->WriteLock(_tid);
                break;
        }
    }

  private:
    LOCK* _lock;
    NATIVE_TID _tid;
};
#endif

/*!
 * A simple utility that manages a Microsoft CRITICAL_SECTION.  The critical section
 * is automatically acquired on entry to the scope and released on exit.
 *
 *  @param CSTYPE   Usually this is the Microsoft CRITICAL_SECTION type, but you can
 *                   use any type that works with functions named EnterCriticalSection()
 *                   and LeaveCriticalSection().
 */
template< typename CSTYPE > class /*<UTILITY>*/ SCOPED_CRITICAL_SECTION
{
  public:
    /*!
     *  @param[in] cs       The critical section to acquire.
     *  @param[in] acquire  If TRUE, the constructor acquires the critical section.
     */
    SCOPED_CRITICAL_SECTION(CSTYPE* cs, bool acquire = true) : _cs(cs), _isLocked(acquire)
    {
        if (acquire) EnterCriticalSection(cs);
    }

    /*!
     * The destructor releases the critical section unless it was manually released via Unlock().
     */
    ~SCOPED_CRITICAL_SECTION()
    {
        if (_isLocked) LeaveCriticalSection(_cs);
    }

    /*!
     * Manually release the critical section.
     */
    void Unlock()
    {
        LeaveCriticalSection(_cs);
        _isLocked = false;
    }

    /*!
     * Manually acquire the critical section.  This only makes sense if the critical section
     * was manually released via Unlock().  No error checking is done to prevent self-deadlock.
     */
    void Lock()
    {
        EnterCriticalSection(_cs);
        _isLocked = true;
    }

    /*!
     * Indicate that the critical section has been acquired or released outside of this wrapper.
     *
     *  @param[in] isLocked     TRUE if the critical section has been acquired.
     */
    void SetLocked(bool isLocked) { _isLocked = isLocked; }

  private:
    CSTYPE* _cs;
    bool _isLocked;
};

} // namespace UTIL
#endif // file guard
