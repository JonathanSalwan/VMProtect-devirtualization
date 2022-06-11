/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_PIN_CLIENT_REPLAY_PH
#error duplicate inclusion of pin_client_replay
#else
#define PIN_G_PIN_CLIENT_REPLAY_PH
/*! @file
 *
 * Additional client side interface to pin vm containing the interfaces
 * useful for a replay tool.
 * These mainly allow a PinTool to request that Pin execute callbacks
 * associated with specific events.
 */

/*! @ingroup REPLAY SYSCALL
 * This API is intended for use by tools that replay the execution of
 * an application and need to behave as if a system call has been
 * made, calling all callbacks registered with PIN_AddSysCallEntry function. 
 * However, no system call is actually performed.
 *
 * This API can be called from an analysis routine or a replacement routine,
 * but not from a call-back.
 *
 * @param[in] tid       The ID of the thread in which the system call appears 
 *                      to execute.
 * @param[in] from      The register state at the point when the system call 
 *                      appears to execute.
 * @param[in] std       The calling standard of the system call.
 *
 * @note The vm and pin client locks are obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux,Windows,BSD\n
 * \b CPU:   All\n
 */
extern VOID PIN_ReplaySyscallEntry(THREADID tid, CONTEXT* ctxt, SYSCALL_STANDARD std);

/*! @ingroup REPLAY SYSCALL
 * This API is intended for use by tools that replay the execution of
 * an application and need to behave as if a system call has been
 * made, calling all callbacks registered with PIN_AddSysCallExit function. 
 * However, no system call is actually performed.
 *
 * This API can be called from an analysis routine or a replacement routine,
 * but not from a call-back.
 *
 * @param[in] tid       The ID of the thread in which the system call appears 
 *                      to execute.
 * @param[in] from      The register state at the point after the system call 
 *                      has executed.
 * @param[in] std       The calling standard of the system call.
 *
 * @note The vm and pin client locks are obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux,Windows,BSD\n
 * \b CPU:   All\n
 */
extern VOID PIN_ReplaySyscallExit(THREADID tid, CONTEXT* ctxt, SYSCALL_STANDARD std);

/*! @ingroup PIN_CONTROL REPLAY 
 * This API is intended for use by tools that replay the execution of
 * an application and need to replay an asynchronous context change
 * that was received by the application at record time.  This API is
 * not intended as a general way to send a forced context change
 * (signal, exception, APC) to the application. To do that, see
 * @ref PIN_RaiseException().
 *
 * Calling this API causes Pin to call back on the tool's
 * context-changed notification function if one was set up with
 * PIN_AddContextChangeFunction().  Thus, the tool is notified about
 * the context change as though the application really received
 * it. Calling this API may also notify an external debugger if one is
 * attached to Pin via the -appdebug switch, which gives the debugger
 * a chance to stop execution of the application when it receives the
 * replayed context change. (Whether this happens depends on the 
 * CONTEXT_CHANGE_REASON, and the expected behavior of the debugger
 * on different platforms).
 *
 * PIN_ReplayContextChange() does not return to its caller. If the \a
 * to parameter is not NULL, execution resumes at that context after
 * it has been potentially modified by the tool's context-changed
 * call-back. If \a reason is CONTEXT_CHANGE_REASON_FATALSIGNAL, Pin
 * emulates a fatal signal and execution terminates gracefully after
 * calling the tool's Fini functions.
 *
 * This API can be called from an analysis routine or a replacement routine,
 * but not from a callback.
 *
 * @param[in] tid       The ID of the calling thread.
 * @param[in] from      The register state prior to the context change (e.g. the
 *                       register state at the point when the signal is received
 *                       for CONTEXT_CHANGE_REASON_SIGNAL).
 * @param[in] to        The register state immediately after the context change.
 *                       This should be NULL for CONTEXT_CHANGE_REASON_FATALSIGNAL.
 * @param[in] reason    The type of the context change.                 
 * @param[in] info      Additional information, depending on \a reason.  (Ignored for
 *                       \a reason codes not listed below.)
 *                       \li CONTEXT_CHANGE_REASON_FATALSIGNAL: The Unix signal number.
 *                       \li CONTEXT_CHANGE_REASON_SIGNAL: The Unix signal number.
 *                       \li CONTEXT_CHANGE_REASON_EXCEPTION: The Windows exception code.
 * @return This function does not return.
 *
 * @note The vm and pin client locks are obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux & Windows\n
 * \b CPU:   All\n
 */
extern VOID PIN_ReplayContextChange(THREADID tid, const CONTEXT* from, CONTEXT* to, CONTEXT_CHANGE_REASON reason, INT32 info);

/*! @ingroup IMG REPLAY
  Ask Pin to perform all the functions it would normally do when it detects that a shared library
  has been unloaded. This is intended for use in a replay system, where the replay tool has recorded
  library unload operations, and now wants to replay them.

  Note that a replay system need not explicitly call this function to cause image unload callbacks at
  the end of process execution. Pin will generate those callbacks itself provided that images were
  created using the @ref IMG_ReplayImageLoad call. However, this function is required to handle explicit
  image unloads (for instance those caused by \c dlclose() )

  IMG_ReplayImageUnload can be called from analysis or callback routines. If called from an analysis
  routine, the Pin client lock must be claimed before calling IMG_ReplayImageUnload (the Pin client lock
  is already held when in a callback routine, so there is no need to claim it again). 

  @param img        the IMG to be unloaded

  @note The pin client lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux (others to follow)\n
  \b CPU:   All\n
*/
extern VOID IMG_ReplayImageUnload(IMG img);

/*! @ingroup PIN_CONTROL REPLAY
  Return TRUE if PIN was successfully reserved the neccessary memory at application load
  time (specified on the -reserve_memory knob), and was successfully restricted the memory
  from the loader (specified on the -restrict_memory knob).
  It is guaranteed this is the return value is FALSE then none the memory specified in the
  mentioned knobs was reserved/restricted

  @return            TRUE if all the memory was reserved/restricted

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux\n
  \b CPU:   All\n
*/
extern BOOL PIN_WasMemoryReservedInLoadTime();

/*! @ingroup PIN_PROCESS
 *
 * Terminate the current process after calling the relevant thread and
 * process exit callbacks. This call behaves "as if" the application
 * process itself had executed a process termination system call. It
 * therefore \e does perform all the finalization actions Pin normally
 * takes on application process exit, including calling registered
 * thread and process fini-functions, unlike @ref PIN_ExitProcess,
 * which exits immediately.
 *
 * This API may be called from an instrumentation callback function or an
 * analysis routine. However, it should not be called from any @ref FINI_CALLBACK
 * or @ref THREAD_FINI_CALLBACK routines. It may be called by an application thread
 * or a tool internal thread (that was created via @ref PIN_SpawnInternalThread).
 *
 * Be aware, that when called from an internal thread, the calling thread
 * will not exit until after the tool's fini callbacks return. Therefore,
 * to avoid deadlock, your fini callbacks should not wait for that internal
 * thread to exit.
 *
 * @param[in] status    The result argument to exit
 * @return This function does not return.
 *
 * @note The vm and pin client locks are obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux,Windows,BSD\n
 * \b CPU:   All\n
 */
extern NORETURN VOID PIN_ExitApplication(INT32 status);

#endif // PIN_G_PIN_CLIENT_REPLAY_PH
