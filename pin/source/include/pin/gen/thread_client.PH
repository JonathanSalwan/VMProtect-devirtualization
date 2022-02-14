/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_THREAD_CLIENT_PH
#error duplicate inclusion of thread_client
#else
#define PIN_G_THREAD_CLIENT_PH
/*! @file
 *
 * Threading API for clients
 */
/*! @ingroup THREADS
 *  Get system identifier of the current thread.
 *  @return system ID of the current thread.
 *
 * @par Availability:
 * \b Mode:  JIT & Probe\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   All\n
 */
extern OS_THREAD_ID PIN_GetTid();

/*! @ingroup THREADS
 *  Get identifier of the current thread in Pin.
 *  @return ID of the current thread in Pin or INVALID_THREADID upon failure.
 *          Usually, the failure means that the function is called in a private
 *          tool's thread which is created by a direct call to a system service
 *          and not via the @ref PIN_SpawnInternalThread() function.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   All\n
 */
extern THREADID PIN_ThreadId();

/*! @ingroup THREADS
 *  Get unique identifier of the current thread in Pin.
 *  @return Unique ID of the current thread in Pin or INVALID_PIN_THREAD_UID upon failure.
 *          Usually, the failure means that the function is called in a private
 *          tool's thread which is created by a direct call to a system service
 *          and not via the @ref PIN_SpawnInternalThread() function.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   All\n
 */
extern PIN_THREAD_UID PIN_ThreadUid();

/*! @ingroup THREADS
 *  Get system identifier of the parent thread, if known.
 *  @return  system ID of the parent thread or INVALID_OS_THREAD_ID if the
 *           parent thread is unknown.
 *           On Windows the result is always INVALID_OS_THREAD_ID, since there is,
 *           in general, no well defined parent child relationship between threads.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   All\n
 */
extern OS_THREAD_ID PIN_GetParentTid();

/*! @ingroup THREADS
 * Delay execution of the current thread for the specified time interval.
 * @param[in]  milliseconds     time interval, in milliseconds.
 *
 * @par Availability:
 * \b Mode:  JIT & Probe\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   All\n
 */
extern VOID PIN_Sleep(UINT32 milliseconds);

/*! @ingroup THREADS
 * Yield the processor to another thread.
 *
 * @par Availability:
 * \b Mode:  JIT & Probe\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   All\n
 */
extern VOID PIN_Yield();

/*! @ingroup THREADS
 * Create a new tool internal thread in the current process.
 *
 * It is safe to create internal threads in the tool's main procedure and spawn new
 * internal threads from existing ones. However new internal threads cannot be created
 * in any other places, like Pin callbacks and analysis routines in application threads.\n
 *
 * In order to ensure graceful termination of internal threads on the application's exit,
 * the tool can use the following recommended method:
 *      - The tool uses the @ref PIN_AddPrepareForFiniFunction() function to register a
 *        @ref PREPARE_FOR_FINI_CALLBACK callback. When the registered function is called
 *        in an "unlocked" thread, the tool requests each other internal thread to exit
 *        and waits until the @ref PIN_WaitForThreadTermination() function returns.
 *      @Note: Pin doesn't wait for internal threads termination after the callbacks.
 *
 * Many of Pin's APIs, that are primarily intended for application threads, are also available in
 * internal threads. Look at the API's description ( \b Availability paragraph) or the description
 * of the corresponding group of APIs to check whether a specific API is available in internal threads.\n
 *
 * @param[in]  pThreadFunc  main (starting) function of the thread
 * @param[in]  arg          argument of the main thread function
 * @param[in]  stackSize    size of the thread's stack, in bytes. The function rounds
 *                          this value up to the page size. If this parameter is zero,
 *                          DEFAULT_THREAD_STACK_SIZE bytes will be allocated for the stack.
 * @param[out] pThreadUid   pointer to a variable that receives the unique identifier of the
 *                          new thread in Pin. This identifier can be used in the
 *                          PIN_WaitForThreadTermination() function to monitor the thread's state.
 *                          If the caller specifies NULL for this parameter, the unique thread ID
 *                          is not returned.
 *
 * @return  ID of the new thread in Pin or INVALID_THREADID if the thread creation failed.
 *
 * @note The @ref PIN_SpawnInternalThread() API is the only way for tools to create a private thread
 *       in the Pin-controlled process. System services, like CreateThread() in Windows or
 *       clone() in Linux should not be used for this purpose.
 * @par
 *       Pin makes an effort to hide internal threads from the application so, usually, a tool's
 *       threads do not interfere with the application. However, the complete transparency of
 *       internal threads is not guaranteed, so tools should only use them when their instrumentation
 *       tasks cannot be done (effectively) by analysis routines within application threads.
 *       For example, a need to execute Windows services (Win32 APIs) may be a reason for creating
 *       a private thread in the tool. All Win32 APIs that do not modify the application's resources
 *       can be freely used in internal threads. In application threads, on the contrary, using
 *       Win32 APIs in analysis routines and Pin callbacks is not supported due to possible
 *       reentrancy and isolation problems.
 * @par
 *       Internal threads remain blocked inside Pin until @ref PIN_StartProgram() is called and Pin
 *       completes some initialization.  On Linux, internal threads start running \a pThreadFunc
 *       before Pin executes the first application instruction.  On Windows all threads, including
 *       Pin internal threads, start executing from the system runtime before they execute the
 *       \a pThreadFunc function.  However, the system runtime blocks threads until the application
 *       has finished initializing its DLL's (i.e. until the application releases the internal
 *       "loader lock").  As a result, Pin internal threads on Windows do not execute \a pThreadFunc
 *       until after the application finishes executing the DLL initialization code.
 *       On macOS* in launch mode internal threads start running \a pThreadFun only after the application
 *       loader initialize the main executable. Hence don't expect the internal thread to start running after
 *       calling this function from the tool main() function in launch mode.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows, macOS*\n
 * \b CPU:   All\n
 */
extern THREADID PIN_SpawnInternalThread(ROOT_THREAD_FUNC* pThreadFunc, VOID* arg, size_t stackSize, PIN_THREAD_UID* pThreadUid);

/*! @ingroup THREADS
 * Terminate the current thread.
 *
 * This function is intended for threads created by the tool (see @ref PIN_SpawnInternalThread())
 * and is not normally used for threads created by the application, since application threads
 * exit automatically when Pin executes a thread termination system call on their behalf.
 *
 * If this call is made on an application thread, Pin will make any callbacks registered for
 * thread exit before the thread is terminated.
 *
 * @param[in] exitCode  exit code of the thread to be returned by the
 *                      PIN_WaitForThreadTermination() function.
 * @return  the function never returns.
 *
 * @note The vm lock is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows\n
 * \b CPU:   All\n
 */
extern VOID PIN_ExitThread(INT32 exitCode);

/*! @ingroup THREADS
 * Check to see if the current thread is created by the application or it is
 * an internal thread spawned by the tool or Pin itself (see @ref PIN_SpawnInternalThread()).
 *
 * @return  TRUE, if this function is called in a thread created by the application;
 *          FALSE, if this function is called in an internal thread spawned by the tool or Pin.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows\n
 * \b CPU:   All\n
 */
extern BOOL PIN_IsApplicationThread();

/*! @ingroup THREADS
 * Delay the current thread until the specified thread is terminated or the time-out
 * interval elapses.
 *
 * To avoid deadlocks, the caller should not hold any lock that the target thread may try
 * to acquire. For example, this function can be safely used in the @ref PREPARE_FOR_FINI_CALLBACK
 * callback if it is registered by the @ref PIN_AddPrepareForFiniFunction(). However, it is
 * not recommended to use this function in other Pin callbacks if the target thread is
 * an application thread or an internal thread that uses Pin APIs. This is because most of Pin
 * APIs and callbacks are serialized by the same internal lock.\n
 *
 * This function can not be used to wait for the termination of the calling thread.
 *
 * @param[in]  threadUid        unique identifier of the thread to be waited for termination,
 *                              provided by @ref PIN_SpawnInternalThread() or @ref PIN_ThreadUid().
 * @param[in]  milliseconds     time-out interval, in milliseconds. If this parameter
 *                              is zero, the function tests the thread's state and
 *                              returns immediately. If this parameter is PIN_INFINITE_TIMEOUT,
 *                              the time-out interval never elapses.
 * @param[out] pExitCode        optional pointer to a variable that receives exit code of
 *                              the thread. If this pointer is NULL or the thread has not
 *                              terminated, the exit code is not returned.
 * @return  TRUE  - the thread has terminated,
 *          FALSE - the specified time-out interval elapsed
 *                  or threadUid is not valid or corresponds to current thread
 *                  or specified thread is not yet started when application shutdown begins.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows\n
 * \b CPU:   All\n
 */
extern BOOL PIN_WaitForThreadTermination(const PIN_THREAD_UID& threadUid, UINT32 milliseconds, INT32* pExitCode);

#endif // PIN_G_THREAD_CLIENT_PH
