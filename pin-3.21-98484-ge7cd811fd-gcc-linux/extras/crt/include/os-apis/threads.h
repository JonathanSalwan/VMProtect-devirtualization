/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header
/// @file threads.h

/*!
 * @defgroup OS_APIS_THREAD Threads
 * @brief Contains thread-related os apis
 */

#ifndef OS_APIS_THREAD_H
#define OS_APIS_THREAD_H

#define OS_APIS_TLS_SLOT_TLS_AND_STACK_START_ADDRESS 4
#define OS_APIS_TLS_SLOT_TLS_AND_STACK_SIZE 5

/*! @ingroup OS_APIS_THREAD
 * Create a new thread in the current process.
 *
 * @param[in]   ThreadMainFunction Thread start address
 * @param[in]   ThreadParam        Thread optional parameter
 * @param[in]   stackBottom        The base address of the thread's stack.
 *                                 This argument can be NULL. In that case this function will
 *                                 allocate the stack according to the @b stackSize argument.
 * @param[in]   stackSize          The size of the thread's stack.
 * @param[out]  td                 The descriptor of newly created thread.
 *
 * @retval      OS_RETURN_CODE_NO_ERROR             If the operation succeeded
 * @retval      OS_RETURN_CODE_THREAD_CREATE_FAILED If the operation Failed
 * @return      Operation status code.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_CreateThread(VOID (*ThreadMainFunction)(VOID*), VOID* ThreadParam, VOID* stackBottom, ADDRINT stackSize,
                               NATIVE_TID* td);

/*! @ingroup OS_APIS_THREAD
 * Causes a running thread to exit.
 *
 * @param[in]  td                  Thread descriptor
 *
 * @retval      OS_RETURN_CODE_NO_ERROR             If the operation succeeded
 * @retval      OS_RETURN_CODE_THREAD_EXIT_FAILED   If the operation Failed
 * @return      Operation status code.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_ExitThread(NATIVE_TID td);

/*! @ingroup OS_APIS_THREAD
 *
 * This function clears a word and then exits the calling thread.  It guaranteess that it will use only
 * register state (no memory) once \a word is cleared.
 *
 *  @param[in] code          The thread exit code as passed from the caller
 *  @param[out] dwordToReset This memory location is cleared (set to zero).
 *
 * @retval      OS_RETURN_CODE_THREAD_EXIT_FAILED   If the operation Failed
 * @return      Operation status code.
 *
 * @par Availability:
 *   @b O/S:   Linux & macOS*\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_ThreadExitAndClear(void* stackAddr, ADDRINT stackSize, INT32* dwordToReset);

/*! @ingroup OS_APIS_THREAD
 * Suspends a running thread.
 *
 * @param[in]  td                  Thread descriptor
 *
 * @retval      OS_RETURN_CODE_NO_ERROR                 If the operation succeeded
 * @retval      OS_RETURN_CODE_THREAD_SUSPEND_FAILED    If the operation Failed
 * @return      Operation status code.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_SuspendThread(NATIVE_TID td);

/*! @ingroup OS_APIS_THREAD
 * Resumes running thread.
 *
 * @param[in]  td                  Thread descriptor
 *
 * @retval      OS_RETURN_CODE_NO_ERROR             If the operation succeeded
 * @retval      OS_RETURN_CODE_THREAD_RESUME_FAILED If the operation Failed
 * @return      Operation status code.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_ResumeThread(NATIVE_TID td);

/*! @ingroup OS_APIS_THREAD
 * Sends thread to sleep.
 *
 * @param[in]  interval            Interval to sleep in milli-seconds.
 *
 * @retval      OS_RETURN_CODE_NO_ERROR             If the operation succeeded
 * @retval      OS_RETURN_CODE_THREAD_SLEEP_FAILED  If the operation Failed
 * @return      Operation status code.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_Sleep(INT interval);

/*! @ingroup OS_APIS_THREAD
 * Retrieves the current thread's handle.
 *
 * @param[out]  id                 Address of handle container variable
 *
 * @retval      OS_RETURN_CODE_NO_ERROR             If the operation succeeded
 * @retval      OS_RETURN_CODE_THREAD_QUERY_FAILED  If the operation Failed
 * @return      Operation status code.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_GetTid(NATIVE_TID* id);

/*! @ingroup OS_APIS_THREAD
 * Pre-empts the thread specified by the running thread.
 *
 * @retval      OS_RETURN_CODE_NO_ERROR             If the operation succeeded
 * @retval      OS_RETURN_CODE_THREAD_SLEEP_FAILED  If the operation Failed
 * @return      Operation status code.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_Yield(void);

/*! @ingroup OS_APIS_THREAD
 * Raise an exception.
 *
 * @param[in]  td   Thread descriptor
 * @param[in]  code Exception type
 *
 * @retval      OS_RETURN_CODE_NO_ERROR             If the operation succeeded
 * @retval      OS_RETURN_CODE_THREAD_SLEEP_FAILED  If the operation Failed
 * @return      Operation status code.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_RaiseException(NATIVE_TID td, UINT32 code);

#endif // file guard
