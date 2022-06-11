/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_LOCK_PH
#error duplicate inclusion of lock
#else
#define PIN_G_LOCK_PH
/*! @file
  This file contains lock primitives
 */
/*! @ingroup LOCK
 * A simple non-recursive lock.
 */
struct PIN_LOCK
{
    PINVM::PINSYNC_SAFEPOD_LOCK _lock; ///< Implements the lock.
    INT32 _owner;                      ///< Used for debugging, typically the ID of the lock owner.
};

/*! @ingroup LOCK
 * A simple non-recursive lock.  PIN_MUTEX is different from PIN_LOCK because
 * it provides just mutex locking without an extra "lock owner" parameter.
 *
 * @note For performance reasons this type request through compiler attributes to be
 * aligned to a memory cache line (64  bytes). For statically allocated objects of this
 * type it is enough. However, when allocating it dynamically it is advised to use
 * allocation operation that enforces this alignment (e.g., memalign)
 */
typedef PINVM::PINSYNC_POD_LOCK PIN_MUTEX;

/*! @ingroup LOCK
 * A non-recursive multiple-reader / single-writer lock.  Use this lock when
 * multiple "reader" threads can simultaneously access a shared resource, but
 * "writer" threads need to have exclusive access.  This is a write-biased
 * lock: if a writer thread blocks on the lock because there are active readers,
 * new readers are prevented from acquiring the lock until the writer gets
 * access.  This prevents starvation of writer threads.
 *
 * @note For performance reasons this type request through compiler attributes to be
 * aligned to a memory cache line (64  bytes). For statically allocated objects of this
 * type it is enough. However, when allocating it dynamically it is advised to use
 * allocation operation that enforces this alignment (e.g., memalign)
 *
 */
typedef PINVM::PINSYNC_POD_RWLOCK PIN_RWMUTEX;

/*! @ingroup LOCK
 * A binary semaphore synchronization object.  You can use this synchronization
 * when one thread needs to wait for some condition to become true.  A binary
 * semaphore has exactly two states: "set" and "clear".  It is possible for
 * one or more threads to wait for the semaphore to become "set".  Those
 * threads resume when some other thread changes the state to "set".
 *
 * Note that it is generally not safe to wait on a PIN_SEMAPHORE from an
 * analysis routine or from a call-back function.  Most Pin call-back functions
 * are called while Pin holds an internal lock (the VM lock).  Therefore, if you
 * wait on a PIN_SEMAPHORE from a call-back, you will prevent any other thread
 * from entering any call-back function (because the waiting thread also holds
 * the VM lock).  There is also a danger when waiting on a PIN_SEMAPHORE from
 * an analysis routine.  If a thread waits on a semaphore from an analysis
 * routine, the application may also hold some locks of its own.  Thus, you
 * can cause a deadlock in the application if you wait on a semaphore while the
 * application holds its own lock.
 *
 * The dangers listed above do not exist if you wait on a PIN_SEMAPHORE from a
 * Pin internal thread (see \ref PIN_SpawnInternalThread()).  Also, it is safe
 * to set, clear, or test a semaphore from any thread, even when executing an
 * analysis routine or call-back function.
 *
 * @note For performance reasons this type request through compiler attributes to be
 * aligned to a memory cache line (64  bytes). For statically allocated objects of this
 * type it is enough. However, when allocating it dynamically it is advised to use
 * allocation operation that enforces this alignment (e.g., memalign)
 */
typedef PINVM::PINSYNC_POD_SEMAPHORE PIN_SEMAPHORE;

/*! @ingroup LOCK
 * Initialize the lock as free
 *
 *  @param[in] lock     The lock variable to initialize.
 */
extern VOID PIN_InitLock(PIN_LOCK* lock);

/*! @ingroup LOCK
 * Acquire the lock.
 *
 *  @param[in] lock     The lock variable.
 *  @param[in] val      Used for debugging.  Typically, this is the ID of the calling thread.
 *                       See the \e _owner field of \ref PIN_LOCK.
 */
extern VOID PIN_GetLock(PIN_LOCK* lock, INT32 val);

/*! @ingroup LOCK
 * Release the lock.
 *
 *  @param[in] lock     The lock variable.
 *
 * @return  The \e val parameter that was passed to PIN_GetLock() when the lock was
 *           acquired.  Typically, this is the ID of the thread that owned the lock.
 */
extern INT32 PIN_ReleaseLock(PIN_LOCK* lock);

/*! @ingroup LOCK
 * This function must be called to initialize a PIN_MUTEX before it is used.
 *
 *  @param[in] lock     The lock variable.
 *
 * @return  TRUE on successful initialization.  If FALSE is returned,
 *           initialization failed, and the PIN_MUTEX may not be used.
 */
extern BOOL PIN_MutexInit(PIN_MUTEX* lock);

/*! @ingroup LOCK
 * Destroy the PIN_MUTEX and deallocate resources.  If you want to use the
 * lock object again later, you must call PIN_MutexInit() again.
 *
 *  @param[in] lock     The lock variable.
 */
extern VOID PIN_MutexFini(PIN_MUTEX* lock);

/*! @ingroup LOCK
 * Block the caller until the lock can be acquired.
 *
 *  @param[in] lock     The lock variable.
 */
extern VOID PIN_MutexLock(PIN_MUTEX* lock);

/*! @ingroup LOCK
 * Release the lock.
 *
 *  @param[in] lock     The lock variable.
 */
extern VOID PIN_MutexUnlock(PIN_MUTEX* lock);

/*! @ingroup LOCK
 * Try to acquire the lock, but do not block the caller.
 *
 *  @param[in] lock     The lock variable.
 *
 * @return  TRUE if the lock is acquired, FALSE if not.
 */
extern BOOL PIN_MutexTryLock(PIN_MUTEX* lock);

/*! @ingroup LOCK
 * This function must be called to initialize a PIN_RWMUTEX before it is used.
 *
 *  @param[in] lock     The lock variable.
 *
 * @return  TRUE on successful initialization.  If FALSE is returned,
 *           initialization failed, and the PIN_RWMUTEX may not be used.
 */
extern BOOL PIN_RWMutexInit(PIN_RWMUTEX* lock);

/*! @ingroup LOCK
 * Destroy the PIN_RWMUTEX and deallocate resources.  If you want to use the
 * lock object again later, you must call PIN_RWMutexInit() again.
 *
 *  @param[in] lock     The lock variable.
 */
extern VOID PIN_RWMutexFini(PIN_RWMUTEX* lock);

/*! @ingroup LOCK
 * Acquire the lock for "read" access, blocking if necessary.  Multiple readers
 * may simultaneously hold the same lock.
 *
 *  @param[in] lock     The lock variable.
 */
extern VOID PIN_RWMutexReadLock(PIN_RWMUTEX* lock);

/*! @ingroup LOCK
 * Acquire the lock for "write" access, blocking if necessary.  A writer has
 * exclusive ownership of the lock, not shared with any other readers or
 * writers.
 *
 *  @param[in] lock     The lock variable.
 */
extern VOID PIN_RWMutexWriteLock(PIN_RWMUTEX* lock);

/*! @ingroup LOCK
 * Release the lock.  Used for both "readers" and "writers".
 *
 *  @param[in] lock     The lock variable.
 */
extern VOID PIN_RWMutexUnlock(PIN_RWMUTEX* lock);

/*! @ingroup LOCK
 * Attempts to acquire the lock as a reader, but does not block the caller.
 *
 *  @param[in] lock     The lock variable.
 *
 * @return  TRUE if the lock is acquired, FALSE if not.
 */
extern BOOL PIN_RWMutexTryReadLock(PIN_RWMUTEX* lock);

/*! @ingroup LOCK
 * Attempts to acquire the lock as a writer, but does not block the caller.
 *
 *  @param[in] lock     The lock variable.
 *
 * @return  TRUE if the lock is acquired, FALSE if not.
 */
extern BOOL PIN_RWMutexTryWriteLock(PIN_RWMUTEX* lock);

/*! @ingroup LOCK
 * This function must be called to initialize a PIN_SEMAPHORE before it is used.
 *
 *  @param[in] sem  The semaphore variable.
 *
 * @return  TRUE on successful initialization.  If FALSE is returned,
 *           initialization failed, and the PIN_SEMAPHORE may not be used.
 */
extern BOOL PIN_SemaphoreInit(PIN_SEMAPHORE* sem);

/*! @ingroup LOCK
 * Destroy the PIN_SEMAPHORE and deallocate resources.  If you want to use the
 * lock object again later, you must call PIN_SemaphoreInit() again.
 *
 *  @param[in] sem  The semaphore variable.
 */
extern VOID PIN_SemaphoreFini(PIN_SEMAPHORE* sem);

/*! @ingroup LOCK
 * Change the semaphore's state to "set" and tell any threads waiting on the
 * semaphore to wake up.  Note that threads waiting on the semaphore may not
 * resume running right away, and they are guaranteed to resume only if the
 * semaphore's state is still "set" when they actually do resume.
 *
 *  @param[in] sem  The semaphore variable.
 */
extern VOID PIN_SemaphoreSet(PIN_SEMAPHORE* sem);

/*! @ingroup LOCK
 * Change the semaphore's state to "clear".  This has no effect on any threads
 * waiting on the semaphore.
 *
 *  @param[in] sem  The semaphore variable.
 */
extern VOID PIN_SemaphoreClear(PIN_SEMAPHORE* sem);

/*! @ingroup LOCK
 * Check whether the semaphore's state is "set", but do not block.
 *
 *  @param[in] sem  The semaphore variable.
 *
 * @return  TRUE if the semaphore's state is "set".
 */
extern BOOL PIN_SemaphoreIsSet(PIN_SEMAPHORE* sem);

/*! @ingroup LOCK
 * Block the calling thread until the semaphore's state is "set".  The
 * calling thread resumes immediately if the state is already "set".
 *
 *  @param[in] sem  The semaphore variable.
 */
extern VOID PIN_SemaphoreWait(PIN_SEMAPHORE* sem);

/*! @ingroup LOCK
 * Block the calling thread until the semaphore's state is "set" or until
 * a timeout expires.  The calling thread resumes immediately if the state
 * is already "set".
 *
 *  @param[in] sem      The semaphore variable.
 *  @param[in] timeout  The timeout period (milliseconds).
 *
 * @return  TRUE if the semaphore's state is "set", FALSE if this method
 *           returns due to the timeout.
 */
extern BOOL PIN_SemaphoreTimedWait(PIN_SEMAPHORE* sem, unsigned timeout);

#endif // PIN_G_LOCK_PH
