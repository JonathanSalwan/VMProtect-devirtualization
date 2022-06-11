/*
 * Copyright (C) 2012-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_STOPPED_THREAD_CLIENT_PH
#error duplicate inclusion of stopped_thread_client
#else
#define PIN_G_STOPPED_THREAD_CLIENT_PH
/*! @file
 *
 * Pin client functions to temporarily stop all application threads, examine
 * their state and resume.
 *
 */
/*! @ingroup THREADS
 * This function may be called by either an application thread or by a Pin
 * internal thread to stop all other application threads at a "safe point".
 * Threads that are stopped at a safe point are always stopped in between
 * traces, so the caller is guaranteed that they are not stopped in the middle
 * of any analysis functions or call-back functions. Once stopped, the
 * calling thread can examine and modify the registers of the stopped threads.
 *
 * If this function is called by an internal thread, it stops all application
 * threads. If it is called by an application thread, it stops all other
 * application threads. When called by an application thread, this function
 * may be called from an analysis function, but not from a call-back function.
 *
 * Since this function blocks until other application threads finish their
 * current trace, the caller must not hold any locks that the other threads
 * might try to acquire. Doing so could result in a deadlock.
 *
 * @param[in] tid The Pin thread ID of the calling thread.
 *
 * @return TRUE if the target threads are successfully stopped. FALSE
 * indicates that some other thread is trying to stop the
 * calling thread. In such a case, the caller should return
 * from its analysis function to avoid a deadlock.
 *
 * @note The vm lock is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
*/
extern BOOL PIN_StopApplicationThreads(THREADID tid);

/*! @ingroup THREADS
 * This function returns true if the thread with denoted by 'tid'
 * given by its arguments was stopped in the debugger.
 * Note: If there is no thread with ID 'tid', this function
 * returns false.
 *
 * @param[in] tid The Pin thread ID to check
 *
 * @return TRUE if the specified thread was stopped in the debugger.
 * FALSE indicates that the thread is either running, or doesn't exist
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
*/
extern BOOL PIN_IsThreadStoppedInDebugger(THREADID tid);

/*! @ingroup THREADS
 * This function may be called after a successful call to
 * PIN_StopApplicationThreads() in order to resume execution of the stopped
 * application threads. If a thread's CONTEXT was changed while it was
 * stopped, it resumes with the new CONTEXT.
 *
 * @Note
 * When used in application thread, it is highly recommended to call it
 * in the same analysis function that called PIN_StopApplicationThreads().
 * Deferring the call could result in a deadlock when the thread later tries
 * to acquire a lock held by other application thread it suspended.
 *
 * @param[in] tid The Pin thread ID of the calling thread. Should be called in
 *                the same thread as corresponding PIN_StopApplicationThreads()
 *
 * @note The vm lock is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID PIN_ResumeApplicationThreads(THREADID tid);

/*! @ingroup THREADS
 * This function can be called in two scenarios. First, it may be called
 * after stopping threads with PIN_StopApplicationThreads(), in which case
 * it tells the number of application threads that were stopped with that
 * API.
 *
 * Alternatively, this function may be called from a call-back that is
 * registered via PIN_AddDebugInterpreter(). In this case, it tells the
 * number of application threads that are stopped in the debugger.
 *
 * @return The number of stopped application threads. Returns zero if threads
 * are not currently stopped.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern UINT32 PIN_GetStoppedThreadCount();

/*! @ingroup THREADS
 * This function returns the Pin thread ID of a stopped application thread.
 *
 * @param[in] i    An index in the range [0, n-1], where \a n is the value
 *                 returned by PIN_GetStoppedThreadCount().
 *
 * @return  The ID of the indexed thread, which is currently stopped.
 *          Returns INVALID_THREADID if \a i is out of range.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern THREADID PIN_GetStoppedThreadId(UINT32 i);

/*! @ingroup THREADS
 * This function returns the CONTEXT (register state) of a stopped application
 * thread. The caller can inspect the stopped thread's register state via
 * PIN_GetContextReg() and related API's.
 *
 * @param[in] tid Pin ID of a stopped thread.
 *
 * @return The CONTEXT for thread \a tid or NULL if that thread is not stopped.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern const CONTEXT* PIN_GetStoppedThreadContext(THREADID tid);

/*! @ingroup THREADS
 * This function is similar to PIN_GetStoppedThreadContext(), but it returns a
 * CONTEXT that may be modified. The caller may modify the stopped thread's
 * register state via PIN_SetContextReg() and related API's. The stopped thread
 * uses the new register state when it resumes.
 *
 * @param[in] tid Pin ID of a stopped thread.
 *
 * @return The CONTEXT for thread \a tid or NULL if that thread is not stopped.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern CONTEXT* PIN_GetStoppedThreadWriteableContext(THREADID tid);

#endif // PIN_G_STOPPED_THREAD_CLIENT_PH
