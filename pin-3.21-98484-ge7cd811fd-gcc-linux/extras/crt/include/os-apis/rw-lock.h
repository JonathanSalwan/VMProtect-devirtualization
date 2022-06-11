/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

#ifndef OS_APIS_RW_LOCK_H__
#define OS_APIS_RW_LOCK_H__

#include "os-apis.h"

/*!
 * @defgroup OS_APIS_RW_LOCK Readers writers lock
 * @brief Implementation of readers writers lock.
 */

typedef enum
{
    OS_APIS_RW_LOCK_OWNER_TYPE_NONE,
    OS_APIS_RW_LOCK_OWNER_TYPE_WRITER,
    OS_APIS_RW_LOCK_OWNER_TYPE_READER
} OS_APIS_RW_LOCK_OWNER_TYPE;

/*!
 * This is an opaque struct for readers/writers lock.
 * This struct should be used with the functions declared in this header.
 */
typedef struct
{
    OS_MUTEX_TYPE_IMPL lock;
    OS_APIS_RW_LOCK_OWNER_TYPE owner_type;
    UINT32 readers;
    OS_EVENT no_readers_event;
    UINT32 waiters_count;
#ifndef TARGET_WINDOWS
    UINT32 spinlock_fork_count;
#endif
} OS_APIS_RW_LOCK_IMPL_T;

typedef PRE_ALIGNTO(CPU_MEMORY_CACHELINE_SIZE) union
{
    OS_APIS_RW_LOCK_IMPL_T impl;
    char reserved[2 * CPU_MEMORY_CACHELINE_SIZE];
} POST_ALIGNTO(CPU_MEMORY_CACHELINE_SIZE) OS_APIS_RW_LOCK_T;

/*!
 * Static initializer for OS_APIS_RW_LOCK_T type.
 */
#ifdef TARGET_WINDOWS
#define OS_APIS_RW_LOCK_INITIALIZER                                                                                        \
    {                                                                                                                      \
        {                                                                                                                  \
            OS_APIS_MUTEX_IMPL_DEPTH_SIMPLE_INITIALIZER, OS_APIS_RW_LOCK_OWNER_TYPE_NONE, (UINT32)0, OS_EVENT_INITIALIZER, \
                (UINT32)0                                                                                                  \
        }                                                                                                                  \
    }
#else
#define OS_APIS_RW_LOCK_INITIALIZER                                                                                        \
    {                                                                                                                      \
        {                                                                                                                  \
            OS_APIS_MUTEX_IMPL_DEPTH_SIMPLE_INITIALIZER, OS_APIS_RW_LOCK_OWNER_TYPE_NONE, (UINT32)0, OS_EVENT_INITIALIZER, \
                (UINT32)0, (UINT32)0                                                                                       \
        }                                                                                                                  \
    }
#endif

/*! @ingroup OS_APIS_RW_LOCK
 * Initializes a reader-writer lock.
 *
 * @param[in]  lock         The lock to initialize
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
void OS_RWLockInitialize(volatile OS_APIS_RW_LOCK_T* l);

/*! @ingroup OS_APIS_RW_LOCK
 * Destroy a reader-writer lock, freeing all exhausted resources associated with the lock.
 *
 * @param[in]  lock         The lock to destroy
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
void OS_RWLockDestroy(volatile OS_APIS_RW_LOCK_T* l);

/*! @ingroup OS_APIS_RW_LOCK
 * Acquires the lock for writer.
 * Blocks until the writer lock is acquired.
 *
 * @param[in]  lock         The lock to acquire
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
void OS_RWLockAcquireWrite(volatile OS_APIS_RW_LOCK_T* lock);

/*! @ingroup OS_APIS_RW_LOCK
 * Releases the lock for writer.
 *
 * @param[in]  lock         The lock to release
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
void OS_RWLockReleaseWrite(volatile OS_APIS_RW_LOCK_T* lock);

/*! @ingroup OS_APIS_RW_LOCK
 * Acquires the lock for reader.
 * Blocks until the reader lock is acquired.
 *
 * @param[in]  lock         The lock to acquire
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
void OS_RWLockAcquireRead(volatile OS_APIS_RW_LOCK_T* lock);

/*! @ingroup OS_APIS_RW_LOCK
 * Releases the lock for reader.
 *
 * @param[in]  lock         The lock to release
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
void OS_RWLockReleaseRead(volatile OS_APIS_RW_LOCK_T* lock);

/*! @ingroup OS_APIS_RW_LOCK
 * Releases the lock that was acquired.
 * The lock kind that was acquired (whether its read or write) is determined by this function.
 *
 * @param[in]  lock         The lock to release
 *
 * @return  TRUE if the lock was taken (for either read or write) and as a result of this call is unlocked, FALSE otherwise.
 *            If the reader lock was released, and a waiting writer was woken up and acquired the lock, the function will
 *            return TRUE.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
BOOL_T OS_RWLockRelease(volatile OS_APIS_RW_LOCK_T* l);

/*! @ingroup OS_APIS_RW_LOCK
 * Tries to Acquire the lock for writer.
 * This function returns immediately if the lock can't be acquired.
 *
 * @param[in]  lock         The lock to acquire
 *
 * @return     TRUE         If the writer lock was acquired.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
BOOL_T OS_RWLockTryAcquireWrite(volatile OS_APIS_RW_LOCK_T* l);

/*! @ingroup OS_APIS_RW_LOCK
 * Tries to Acquire the lock for reader.
 * This function returns immediately if the lock can't be acquired.
 *
 * @param[in]  lock         The lock to acquire
 *
 * @return     TRUE         If the reader lock was acquired.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
BOOL_T OS_RWLockTryAcquireRead(volatile OS_APIS_RW_LOCK_T* l);

#endif // OS_APIS_RW_LOCK_H__
