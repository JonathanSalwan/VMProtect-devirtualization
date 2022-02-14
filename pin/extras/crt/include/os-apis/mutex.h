/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

#ifndef OS_APIS_MUTEX_H_INCLUDED_
#define OS_APIS_MUTEX_H_INCLUDED_

/*!
 * @defgroup OS_APIS_MUTEX Mutex
 * @brief Implementation of mutex lock.
 */

typedef ADDRINT OS_SPINLOCK_TYPE;

typedef enum _OS_APIS_MUTEX_KIND
{
    OS_MUTEX_DEPTH_SIMPLE = 0, //!< This lock can only be acquired once, no matter if the same thread tried to acquire it twice
    OS_MUTEX_DEPTH_RECURSIVE   //!< Recursive lock that can be acquired recursively by the same thread.
} OS_MUTEX_DEPTH;

/*! @ingroup OS_APIS_MUTEX
 * This type holds a representation of a mutex.
 *
 * @par Availability:
 *   - @b O/S:   Windows, Linux & macOS*
 *   - @b CPU:   All
 */
typedef struct _OS_APIS_MUTEX_TYPE
{
    OS_MUTEX_DEPTH kind;
    OS_SPINLOCK_TYPE spinlock;
    OS_EVENT event;
    UINT32 num_waiters;
    UINT32 depth;
#ifndef TARGET_WINDOWS
    UINT32 spinlock_fork_count;
#endif
} OS_MUTEX_TYPE_IMPL;

typedef PRE_ALIGNTO(CPU_MEMORY_CACHELINE_SIZE) union
{
    OS_MUTEX_TYPE_IMPL impl;
    char reserved[2 * CPU_MEMORY_CACHELINE_SIZE];
} POST_ALIGNTO(CPU_MEMORY_CACHELINE_SIZE) OS_MUTEX_TYPE;

#ifdef TARGET_WINDOWS
#define OS_APIS_MUTEX_IMPL_DEPTH_SIMPLE_INITIALIZER                                            \
    {                                                                                          \
        OS_MUTEX_DEPTH_SIMPLE, (OS_SPINLOCK_TYPE)0, OS_EVENT_INITIALIZER, (UINT32)0, (UINT32)0 \
    }
#else
#define OS_APIS_MUTEX_IMPL_DEPTH_SIMPLE_INITIALIZER                                                       \
    {                                                                                                     \
        OS_MUTEX_DEPTH_SIMPLE, (OS_SPINLOCK_TYPE)0, OS_EVENT_INITIALIZER, (UINT32)0, (UINT32)0, (UINT32)0 \
    }
#endif

/*! @ingroup OS_APIS_MUTEX
 * Static initializer for a mutex.
 * It is guaranteed that a (simple) mutex initialized this way:
 *
 * OS_MUTEX_TYPE mutex = OS_APIS_MUTEX_DEPTH_SIMPLE_INITIALIZER;
 *
 * Will be initialized before any constuctor will be called.
 * Also, the static initializer for a simple mutex must be all zeros.
 * This is because we want mutex that is initialized in a default way
 * (all zeros according to the C++ standard) will be initialized correctly
 * to a simple mutex.
 *
 * @par Availability:
 *   - @b O/S:   Windows, Linux & macOS*
 *   - @b CPU:   All
 */
#define OS_APIS_MUTEX_DEPTH_SIMPLE_INITIALIZER      \
    {                                               \
        OS_APIS_MUTEX_IMPL_DEPTH_SIMPLE_INITIALIZER \
    }

#ifdef TARGET_WINDOWS
#define OS_APIS_MUTEX_IMPL_DEPTH_RECURSIVE_INITIALIZER                                            \
    {                                                                                             \
        OS_MUTEX_DEPTH_RECURSIVE, (OS_SPINLOCK_TYPE)0, OS_EVENT_INITIALIZER, (UINT32)0, (UINT32)0 \
    }
#else
#define OS_APIS_MUTEX_IMPL_DEPTH_RECURSIVE_INITIALIZER                                                       \
    {                                                                                                        \
        OS_MUTEX_DEPTH_RECURSIVE, (OS_SPINLOCK_TYPE)0, OS_EVENT_INITIALIZER, (UINT32)0, (UINT32)0, (UINT32)0 \
    }
#endif

/*! @ingroup OS_APIS_MUTEX
 * Static initializer for a mutex.
 * It is guaranteed that a (recursive) mutex initialized this way:
 *
 * OS_MUTEX_TYPE mutex = OS_APIS_MUTEX_DEPTH_SIMPLE_INITIALIZER;
 *
 * Will be initialized before any constuctor will be called.
 *
 * @par Availability:
 *   - @b O/S:   Windows, Linux & macOS*
 *   - @b CPU:   All
 */
#define OS_APIS_MUTEX_DEPTH_RECURSIVE_INITIALIZER      \
    {                                                  \
        OS_APIS_MUTEX_IMPL_DEPTH_RECURSIVE_INITIALIZER \
    }

/*! @ingroup OS_APIS_MUTEX
 * Initialize a mutex.
 * A mutex must be initialized before being used.
 * Use this function to initialize the mutex or use one of the static initializers.
 *
 * @param[in]  lock               The mutex to initialize.
 *
 * @par Availability:
 *   - @b O/S:   Windows, Linux & macOS*
 *   - @b CPU:   All
 */
void OS_MutexInit(volatile OS_MUTEX_TYPE* lock);

/*! @ingroup OS_APIS_MUTEX
 * Initialize a recursive mutex.
 * A mutex must be initialized before being used.
 * Use this function to initialize a recursive mutex or use one of the static initializers.
 *
 * @param[in]  lock               The mutex to initialize.
 *
 * @par Availability:
 *   - @b O/S:   Windows, Linux & macOS*
 *   - @b CPU:   All
 */
void OS_MutexRecursiveInit(volatile OS_MUTEX_TYPE* lock);

/*! @ingroup OS_APIS_MUTEX
 * Destroys a mutex after it is no longer in use.
 *
 * @note The behavior of a mutex after it was destroyed is undefined.
 * It is the responsibility of the user to verify that no other thread
 * is using the mutex when it comes to destroy it.
 *
 * @param[in]  lock               The mutex to destroy.
 *
 * @par Availability:
 *   - @b O/S:   Windows, Linux & macOS*
 *   - @b CPU:   All
 */
void OS_MutexDestroy(volatile OS_MUTEX_TYPE* lock);

/*! @ingroup OS_APIS_MUTEX
 * Aquire a mutex, blocks until the mutex becomes available (according to the mutex's semantics).
 *
 * @param[in]  lock               The mutex to acquire.
 *
 * @par Availability:
 *   - @b O/S:   Windows, Linux & macOS*
 *   - @b CPU:   All
 */
void OS_MutexLock(volatile OS_MUTEX_TYPE* lock);

/*! @ingroup OS_APIS_MUTEX
 * Aquire a mutex, blocks until the mutex becomes available (according to the mutex's semantics).
 * This function is similar to OS_MutexLock() except that it accept as an argument the thread ID
 * of the thread that want to acquire the lock (which is usually the current thread).
 *
 * @param[in]  lock               The mutex to acquire.
 * @param[in]  tid                The thread ID of the thread that wants to acquire the mutex.
 *                                This argument can be INVALID_NATIVE_TID if the thread ID is
 *                                not known (e.g. in a mutex of type OS_MUTEX_DEPTH_SIMPLE).
 *
 * @par Availability:
 *   - @b O/S:   Windows, Linux & macOS*
 *   - @b CPU:   All
 */
void OS_MutexLockTid(volatile OS_MUTEX_TYPE* lock, NATIVE_TID myTid);

/*! @ingroup OS_APIS_MUTEX
 * Tries to aquire a mutex:\n
 * - If the mutex is available, acquire it and return true.\n
 * - Otherwise, return false.
 *
 * @param[in]  lock               The mutex to acquire.
 *
 * @retval     TRUE               If the mutex was acquired.
 *
 * @par Availability:
 *   - @b O/S:   Windows, Linux & macOS*
 *   - @b CPU:   All
 */
BOOL_T OS_MutexTryLock(volatile OS_MUTEX_TYPE* lock);

/*! @ingroup OS_APIS_MUTEX
 * Tries to aquire a mutex:\n
 * - If the mutex is available, acquire it and return true.\n
 * - Otherwise, return false.
 * This function is similar to OS_MutexTryLock() except that it accept as an argument the thread ID
 * of the thread that want to acquire the lock (which is usually the current thread).
 *
 * @param[in]  lock               The mutex to acquire.
 * @param[in]  tid                The thread ID of the thread that wants to acquire the mutex.
 *                                This argument can be INVALID_NATIVE_TID if the thread ID is
 *                                not known (e.g. in a mutex of type OS_MUTEX_DEPTH_SIMPLE).
 *
 * @retval     TRUE               If the mutex was acquired.
 *
 * @par Availability:
 *   - @b O/S:   Windows, Linux & macOS*
 *   - @b CPU:   All
 */
BOOL_T OS_MutexTryLockTid(volatile OS_MUTEX_TYPE* lock0, NATIVE_TID myTid);

/*! @ingroup OS_APIS_MUTEX
 * Checks whether a mutex state is locked.\n
 * Doesn't affect the mutex state and doesn't block.
 *
 * @param[in]  lock               The mutex to check.
 *
 * @retval     TRUE               If the mutex is locked.
 *
 * @par Availability:
 *   - @b O/S:   Windows, Linux & macOS*
 *   - @b CPU:   All
 */
BOOL_T OS_MutexIsLocked(volatile OS_MUTEX_TYPE* lock);

/*! @ingroup OS_APIS_MUTEX
 * Aquire a mutex, blocks until the mutex becomes available (according to the mutex's semantics).
 * or 'timeoutMillis' milli seconds passed.
 * When 'timeoutMillis' is zero, this function is identical to @ref OS_MutexTryLock().
 *
 * @param[in]  lock               The mutex to acquire.
 * @param[in]  timeoutMillis      The timeout to block.
 *
 * @retval     TRUE               If the mutex is locked.
 *             FALSE              If the timeout was expired and the mutex can't be acquired during that time.
 *
 * @par Availability:
 *   - @b O/S:   Windows, Linux & macOS*
 *   - @b CPU:   All
 */
BOOL_T OS_MutexTimedLock(volatile OS_MUTEX_TYPE* lock, UINT32 timeoutMillis);

/*! @ingroup OS_APIS_MUTEX
 * Aquire a mutex, blocks until the mutex becomes available (according to the mutex's semantics).
 * or 'timeoutMillis' milli seconds passed.
 * When 'timeoutMillis' is zero, this function is identical to @ref OS_MutexTryLock().
 * This function is similar to OS_MutexTimedLock() except that it accept as an argument the thread ID
 * of the thread that want to acquire the lock (which is usually the current thread).
 *
 * @param[in]  lock               The mutex to acquire.
 * @param[in]  timeoutMillis      The timeout to block.
 * @param[in]  tid                The thread ID of the thread that wants to acquire the mutex.
 *                                This argument can be INVALID_NATIVE_TID if the thread ID is
 *                                not known (e.g. in a mutex of type OS_MUTEX_DEPTH_SIMPLE).
 *
 * @retval     TRUE               If the mutex is locked.
 *             FALSE              If the timeout was expired and the mutex can't be acquired during that time.
 *
 * @par Availability:
 *   - @b O/S:   Windows, Linux & macOS*
 *   - @b CPU:   All
 */
BOOL_T OS_MutexTimedLockTid(volatile OS_MUTEX_TYPE* lock0, NATIVE_TID myTid, UINT32 timeoutMillis);

/*! @ingroup OS_APIS_MUTEX
 * Queries the owner of a recursive mutex.
 *
 * @param[in]  lock               The mutex to query.
 *
 * @retval     NATIVE_TID         The owner of the mutex of INVALID_NATIVE_TID is the mutex is not locked.
 *
 * @note The return value of this function is undefined for non-recursive (simple) mutex.
 *
 * @par Availability:
 *   - @b O/S:   Windows, Linux & macOS*
 *   - @b CPU:   All
 */
NATIVE_TID OS_MutexGetOwner(volatile OS_MUTEX_TYPE* lock);

/*! @ingroup OS_APIS_MUTEX
 * Queries the recursion of a recursive mutex. I.e. the number of times that the unlock functions needs to be
 * called before the mutex can be acquired by other thread.
 *
 * @param[in]  lock               The mutex to query.
 *
 * @retval     UINT32             The recursion level of the mutex.
 *
 * @note For non-recursive (simple) mutex, the return value of this function is 1 if the mutex is lock,
 *       or 0 if the mutex is unlocked.
 *
 * @par Availability:
 *   - @b O/S:   Windows, Linux & macOS*
 *   - @b CPU:   All
 */
UINT32 OS_MutexGetRecursionLevel(volatile OS_MUTEX_TYPE* lock);

/*! @ingroup OS_APIS_MUTEX
 * Releases a mutex.
 *
 * @param[in]  lock               The mutex to release.
 *
 * @retval     TRUE               If the mutex was locked and as a result of this call was unlocked.
 *
 * @par Availability:
 *   - @b O/S:   Windows, Linux & macOS*
 *   - @b CPU:   All
 */
BOOL_T OS_MutexUnlock(volatile OS_MUTEX_TYPE* lock);

#endif // OS_APIS_MUTEX_H_INCLUDED_
