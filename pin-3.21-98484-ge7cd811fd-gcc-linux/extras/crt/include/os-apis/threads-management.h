/*
 * Copyright (C) 2016-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

#ifndef OS_APIS_THREAD_MANAGEMENT_H_INCLUDED__
#define OS_APIS_THREAD_MANAGEMENT_H_INCLUDED__

#include "os-apis.h"

/*!
 * @defgroup OS_APIS_THREAD_MANAGEMENT Threads database
 * @brief Contains API for thread tracking
 */

/*! @ingroup OS_APIS_THREAD_MANAGEMENT
 * Deregister a thread from the threads database and release all the resources
 * used to track this thread (including TLS).
 *
 * @param[in]  ntid               OS thread ID of the thread to deregister.
 *
 * @par Availability:
 *   - @b O/S:   Windows, Linux & macOS*
 *   - @b CPU:   All
 */
VOID OS_DeregisterThread(NATIVE_TID ntid);

/*! @ingroup OS_APIS_THREAD_MANAGEMENT
 * Deregister the current thread from the threads database and release all the resources
 * used to track this thread (including TLS).
 *
 * @par Availability:
 *   - @b O/S:   Windows, Linux & macOS*
 *   - @b CPU:   All
 */
VOID OS_DeregisterCurrentThread(void);

#endif // OS_APIS_THREAD_MANAGEMENT_H_INCLUDED__
