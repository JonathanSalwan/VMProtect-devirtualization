/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_PIN_CLIENT_PH
#error duplicate inclusion of pin_client
#else
#define PIN_G_PIN_CLIENT_PH
/*! @file
 *
 * Client side interface to pin vm.
 */

#define EnterPinClientSlaveMode() CheckPinClientLock(__FUNCTION__)
#define ExitPinClientSlaveMode() CheckPinClientLock(__FUNCTION__)
#ifndef PIN_GENERATED_MESSAGE_PIN_H
#define LOG_IMAGE(s) \
    if (ClientInt()->_VmLog) ClientInt()->_VmLog("IMAGE", s);
#define LOG_SYM(s) \
    if (ClientInt()->_VmLog) ClientInt()->_VmLog("SYMBOLS", s);
#define LOG_PROBE(s) \
    if (ClientInt()->_VmLog) ClientInt()->_VmLog("PROBE", s);
#define LOG_FETCH_RTN_INS(s) \
    if (ClientInt()->_VmLog) ClientInt()->_VmLog("FETCH_RTN_INS", s);
#define LOG_UNWIND(s) \
    if (ClientInt()->_VmLog) ClientInt()->_VmLog("UNWIND", s);
#define LOG_BUFFER(s) \
    if (ClientInt()->_VmLog) ClientInt()->_VmLog("BUFFER", s);
#define LOG_JIT_PROFILING(s) \
    if (ClientInt()->_VmLog) ClientInt()->_VmLog("VTUNE_JIT_API", s);
// When LOG()/WARNING() is called from Pin client library it will be printed to pintool.log
// If one wants to print a general log/warning message into pin.log,
// then LOG_VMLOG()/LOG_VMWARNING() macro should be used.
#define LOG_VMLOG(s) \
    if (ClientInt()->_VmLog) ClientInt()->_VmLog("LOG", s);
#define LOG_VMWARNING(s) \
    if (ClientInt()->_VmLog) ClientInt()->_VmLog("WARNING", s);
#endif
/*! @ingroup TRACE
  Opaque type for pin client
*/
class TRACE_CLASS;

/*! @ingroup TRACE
  Container for a trace
*/
typedef TRACE_CLASS* TRACE;

/*! @ingroup PIN_CONTROL
  Global configuration for Pin.
  Currently, this configuration includes whether Pin should serialize calls
  to some callbacks (for backward compatibility) or if it should concurrently
  call the callbacks (better for multi-threaded performance).

  See also:
  @ref PIN_SetAllowedConcurrentCallbacks()

  @note A new instance of PIN_CONFIGURATION_INFO with default values
  should be created with @ref PIN_CreateDefaultConfigurationInfo()

*/
typedef VOID* PIN_CONFIGURATION_INFO;

/*! @ingroup PIN_CALLBACKS
  Opaque type for pin callback
*/
class COMPLEX_CALLBACKVAL_BASE;

/*! @ingroup PIN_CALLBACKS
  Container for a PIN callback
*/
typedef COMPLEX_CALLBACKVAL_BASE* PIN_CALLBACK;

/*! @ingroup PIN_CALLBACKS
  Invalid value for PIN callback
*/
const PIN_CALLBACK PIN_CALLBACK_INVALID(0);

/*! @ingroup PIN_CONTROL
 * Call back function when Pin removes all old instrumented code from its cache
 *
 * @param[in] v             The tool's call-back value.
 */
typedef VOID (*REMOVE_INSTRUMENTATION_CALLBACK)(VOID* v);

/*! @ingroup PIN_CONTROL
 * Call back function when Pin detaches from application in JIT mode
 *
 * @param[in] v             The tool's call-back value.
 */
typedef VOID (*DETACH_CALLBACK)(VOID* v);

/*! @ingroup PIN_CONTROL
 * Call back function when Pin detaches from application in Probe mode
 *
 * @param[in] v             The tool's call-back value.
 */
typedef VOID (*DETACH_PROBED_CALLBACK)(VOID* v);

/*! @ingroup PIN_CONTROL
 * Call back function when Pin attaches to application in Probe mode
 *
 * @param[in] v             The tool's call-back value.
 */
typedef VOID (*ATTACH_PROBED_CALLBACK)(VOID* v);

/*! @ingroup PIN_CONTROL
 * Call back function when Pin attaches to application in JIT mode
 *
 * @param[in] v             The tool's call-back value.
 */
typedef VOID (*ATTACH_CALLBACK)(VOID* v);

/*! @ingroup PIN_CONTROL
 * Call back function when pin initialization is finished.
 *
 * @param[in] v             The tool's call-back value.
 */
typedef VOID (*APPLICATION_START_CALLBACK)(VOID* v);

/*! @ingroup PIN_CONTROL
 * Call back function invoked when Pin is about to detach from terminating process.
 * The function that is registered by this callback may gracefully finish
 * all the internal threads of the tool, e.g.
 * - Force all the internal threads of the tool to finish using return from
 *   the main function of the thread or calling PIN_ExitThread.
 * - Wait until all the internal threads are finished before it will continue
 *   to run.
 * @Note: normal run of the internal threads after exit
 * from all registered call back functions of this type is not guaranteed.
 * @Note: If current thread is an internal thread, do not finish it in this callback!
 *
 * This function is invoked in thread that initiated process termination.
 * This function is registered via @ref PIN_AddPrepareForFiniFunction().
 *
 *  @param[in] v      The tool's call-back value
 */
typedef VOID (*PREPARE_FOR_FINI_CALLBACK)(VOID* v);

/*! @ingroup PIN_CONTROL
 * Call back function when application exits.
 * It is called after all the per thread fini functions were called
 * (registered via @ref PIN_AddThreadFiniFunction).
 * This function may be called on any application thread or internal thread
 * spawned by the tool.
 *
 * @Note    It is prohibited to access the TLS in this callback.
 *
 * @param[in] code          O/S specific termination code for the application.
 * @param[in] v             The tool's call-back value.
 */
typedef VOID (*FINI_CALLBACK)(INT32 code, VOID* v);

/*! @ingroup PIN_CONTROL
 * Call back function that Pin will use to fetch code bytes to be jitted.
 *
 * @param[out] buf          A buffer which receives the fetched instruction bytes.
 *                           This buffer is at least \a size bytes long.
 * @param[in] addr          Starting address from which instruction bytes should
 *                            be fetched.
 * @param[in] size          The number of bytes that should be fetched.
 * @param[out] pExceptInfo  If less than \a size bytes are fetched, \a pExceptInfo
 *                           receives an exception that describes why more bytes
 *                           could not be fetched.  If Pin needs more bytes to
 *                           decode the next instruction, this exception will be
 *                           delivered to the application.
 * @param[in] v             The tool's call-back value.
 *
 * @return The number of bytes sucessfully fetched into \a buf. This may be
 *       less than \a size (or even zero) if Pin attempts to fetch instructions from an
 *       invalid address.
 */
typedef size_t (*FETCH_CALLBACK)(void* buf, ADDRINT addr, size_t size, EXCEPTION_INFO* pExceptInfo, VOID* v);

/*! @ingroup PIN_CONTROL
 * Call back function when pin/tool is out of memory.
 * This function can be registered via @ref PIN_AddOutOfMemoryFunction().
 *
 *  @param[in] size   Size of the failed allocation
 *  @param[in] v      The tool's call-back value
 */
typedef VOID (*OUT_OF_MEMORY_CALLBACK)(size_t size, VOID* v);

/*! @ingroup PIN_CONTROL
 * Call back function for pin/tool to modify memory address arguments in analysis routines.
 * This feature allows users to register for a callback immediately prior to an analysis routine
 * which gets one of the four IARG_MEMORY*_PTR as a parameter.
 * The callback enables the user to manipulate the memory address pointed to by the aforementioned parameter.
 * This function can be registered via @ref PIN_AddMemoryAddressTransFunction().
 * This function is called from the jitted code, so calls to this function are not serialized.
 *
 *  @param[in] memTransInfo     Memory address data structure
 *  @param[in] v                The tool's call-back value
 */
typedef ADDRINT(PIN_FAST_ANALYSIS_CALL* MEMORY_ADDR_TRANS_CALLBACK)(PIN_MEM_TRANS_INFO* memTransInfo, VOID* v);

/*! @ingroup TRACE
 * Call back function when SMC is detected. This function can be registered via TRACE_AddSmcDetectedFunction.
 * The callback delivers the start and end addresses of the TRACE containing the SMC.
 * Using this function can potentially cause Pin to use unlimited memory due to SMC tracking.
 *
 *  @param[in] traceStartAddress The start address of the trace in which SMC is detected
 *  @param[in] traceEndAddress   The end address of the trace in which SMC is detected
 *  @param[in] v                 The tool's call-back value
 */
typedef VOID (*SMC_CALLBACK)(ADDRINT traceStartAddress, ADDRINT traceEndAddress, VOID* v);

/*! @ingroup PIN_CONTROL
 * Call-back function when the application forks.
 *
 *    @param[in] threadId  Thread index
 *    @param[in] ctxt      Register state immediately before/after the fork.
 *    @param[in] v         The tool's call-back value.
*/
typedef VOID (*FORK_CALLBACK)(THREADID threadid, const CONTEXT* ctxt, VOID* v);

/*! @ingroup PIN_CONTROL
 * Call-back function when thread begins.
 *
 *  @param[in] threadIndex  The Pin thread ID of the new thread.
 *  @param[in] ctxt         Initial register state for the new thread.  The tool may change this.
 *  @param[in] flags        O/S specific flags for the new thread.
 *  @param[in] v            The tool's call-back value.
 */
typedef VOID (*THREAD_START_CALLBACK)(THREADID threadIndex, CONTEXT* ctxt, INT32 flags, VOID* v);

/*! @ingroup PIN_CONTROL
  Call-back function when an attached thread starts to run under Pin in probe mode.
  The tool receives the signal mask at the attach point and can modify it.
  The changes to this value will be applied when the thread continues to run.
  Pin does not validate the new signal mask so tools must take extra care
  when modifying this value.

  @param[in, out] sigmask The thread's signal mask at the attach point.
                          The tool may change this and affect the signal mask once
                          the thread starts running again.
  @param[in] v            The tool's call-back value.
 */
typedef VOID (*THREAD_ATTACH_PROBED_CALLBACK)(VOID* sigmask, VOID* v);

/*! @ingroup PIN_CONTROL
 Call-back function before an application thread is being detached from Pin in probe mode.
 The callback is received in the thread native context before probes removal.

 @param[in] v             The tool's call-back value.
 */
typedef VOID (*THREAD_DETACH_PROBED_CALLBACK)(VOID* v);

/*! @ingroup PIN_CONTROL
  Call-back function when an attached thread starts to run under Pin in JIT mode.

  @param[in] threadIndex  The Pin thread ID of the terminating thread.
  @param[in] ctxt         Register state of the thread.
  @param[in] v            The tool's call-back value.
 */
typedef VOID (*THREAD_ATTACH_CALLBACK)(THREADID threadIndex, CONTEXT* ctxt, VOID* v);

/*! @ingroup PIN_CONTROL
 Call-back function before an application thread is detached from Pin in JIT mode.
 The callback is received in the thread native context before Pin detach.

 *  @param[in] threadIndex  The Pin thread ID of the terminating thread.
 *  @param[in] ctxt         Register state of the thread immediately before it detaches.
 *  @param[in] v            The tool's call-back value.
 */
typedef VOID (*THREAD_DETACH_CALLBACK)(THREADID threadIndex, const CONTEXT* ctxt, VOID* v);

/*! @ingroup PIN_CONTROL
 * Call-back function when thread ends.
 *
 * @Note    This function may be called on a different thread than the given threadIndex.
 *
 *  @param[in] threadIndex  The Pin thread ID of the terminating thread.
 *  @param[in] ctxt         Register state of the thread immediately before it terminates.
 *  @param[in] code         O/S specific termination code for the thread.
 *  @param[in] v            The tool's call-back value.
 */
typedef VOID (*THREAD_FINI_CALLBACK)(THREADID threadIndex, const CONTEXT* ctxt, INT32 code, VOID* v);

/*! @ingroup PIN_CONTROL
  Call-back function when application changes context.

  @param[in] threadIndex   The Pin thread ID of the thread that changes contexts.
  @param[in] reason        Cause of the context change.
  @param[in] from          Application's register state prior to the context change (NULL if
                           \a reason is CONTEXT_CHANGE_REASON_CALLBACK).
  @param[in] to            Application's register state after context change (NULL if \a reason
                           is CONTEXT_CHANGE_REASON_FATALSIGNAL).  The tool may change this
                           and affect the new register state.
  @param[in] info          Additional information (depends on \a reason):
                               \li CONTEXT_CHANGE_REASON_FATALSIGNAL: The Unix signal number.
                               \li CONTEXT_CHANGE_REASON_SIGNAL: The Unix signal number.
                               \li CONTEXT_CHANGE_REASON_EXCEPTION: The Windows exception code.
  @param[in] v             The tool's call-back value.
*/
typedef VOID (*CONTEXT_CHANGE_CALLBACK)(THREADID threadIndex, CONTEXT_CHANGE_REASON reason, const CONTEXT* from, CONTEXT* to,
                                        INT32 info, VOID* v);

/*! @ingroup SYSCALL
  Call-back function before execution of a system call.

  @param[in]      threadIndex   The Pin thread ID of the thread that executes the system call.
  @param[in, out] ctxt          Application's register state immediately before execution of
                                the system call. The tool may change this and affect the new
                                register state.
  @param[in]      std           The system calling standard.
  @param[in]      v             The tool's call-back value.
*/
typedef VOID (*SYSCALL_ENTRY_CALLBACK)(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v);

/*! @ingroup  SYSCALL
  Call-back function after execution of a system call.

  @param[in]      threadIndex   The Pin thread ID of the thread that executed the system call.
  @param[in, out] ctxt          Application's register state immediately after execution of
                                the system call.The tool may change this and affect the new
                                register state.
  @param[in]      std           The system calling standard.
  @param[in]      v             The tool's call-back value.
*/
typedef VOID (*SYSCALL_EXIT_CALLBACK)(THREADID threadIndex, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v);

/*! @ingroup PIN_CONTROL
 * Call back function when the tool intercepts a signal with PIN_InterceptSignal().
 *
 *  @param[in] tid          The Pin thread ID of the thread that handled the signal.
 *  @param[in] sig          The signal number.
 *  @param[in,out] ctxt     The application's register state when it was interrupted by the signal.
 *                           The tool may change this context if desired.  If so, the application
 *                           continues at the modified context.  Or, if the application's
 *                           signal handler is invoked, the handler is passed the modified context
 *                           for its return context.
 *  @param[in] hasHandler   TRUE if the application has a handler registered for this signal.
 *  @param[in] pExceptInfo  If the signal represents an exception, \a pExceptInfo points to a
 *                           description of the exception.  If the signal is not an exception,
 *                           \a pExceptInfo is NULL.
 *  @param[in] v            The tool's call-back value.
 *
 * @return   Returning TRUE tells Pin to pass the signal on to the application.  Returning FALSE
 *            tells Pin to squash the signal and execution resumes at \a ctxt.
 */
typedef BOOL (*INTERCEPT_SIGNAL_CALLBACK)(THREADID tid, INT32 sig, CONTEXT* ctxt, BOOL hasHandler,
                                          const EXCEPTION_INFO* pExceptInfo, VOID* v);

/*! @ingroup BUFFER
 *
 * A call-back function which Pin calls whenever the tools needs to consume a trace buffer
 * (e.g., the trace buffer is full).
 *
 * @Note    This function may be called on a different thread than the given threadIndex.
 *
 * @param[in] id             The ID of the trace buffer.
 * @param[in] tid            The ID of the thread owning this buffer.
 * @param[in] buf            Pointer to the start of the buffer.
 * @param[in] numElements    The number of elements collected into the buffer which need to be consumed.
 * @param[in] v              The tool's call-back value.
 *
 * @return A pointer to the buffer to use when the thread resumes.  Typically, this is \a buf, but
 *         see also @ref PIN_AllocateBuffer().
 */
typedef VOID* (*TRACE_BUFFER_CALLBACK)(BUFFER_ID id, THREADID tid, const CONTEXT* ctxt, VOID* buf, UINT64 numElements, VOID* v);

/*! @ingroup INS_INSTRUMENTATION
 *
 * @anchor INS_INSTRUMENT_CALLBACK
 *
 * Call back function used to instrument instructions
 */
typedef VOID (*INS_INSTRUMENT_CALLBACK)(INS ins, VOID* v);

/*! @ingroup TRACE
  Call back function used to instrument traces
*/
typedef VOID (*TRACE_INSTRUMENT_CALLBACK)(TRACE trace, VOID* v);

/*! @ingroup RTN
  Call back function used to instrument routines
*/
typedef VOID (*RTN_INSTRUMENT_CALLBACK)(RTN rtn, VOID* v);

/*! @ingroup REPLAY PIN_CONTROL
  Flags passed to PIN_SetReplayMode.
*/
enum REPLAY_MODE
{
    REPLAY_MODE_NONE     = 0x00,
    REPLAY_MODE_IMAGEOPS = 0x01, ///< Replayer will handle image ops
    REPLAY_MODE_ALL      = REPLAY_MODE_IMAGEOPS
};

/*! @ingroup REPLAY PIN_CONTROL
  Tell Pin to execute different sets of operations in replay mode.

  The only flag currently supported is REPLAY_MODE_IMAGEOPS which disables Pin's normal detection of
  image load and unload events, which are expected to be replayed through the @ref
  IMG_ReplayImageLoad and @ref IMG_ReplayImageUnload calls.  It also avoids Pin loading
  information about the initial image given on the command line.

  Other flags may be added as required in the future.

  Must be called before @ref PIN_StartProgram

  @param flags  A mask created from the PIN_REPLAY_MODE_FLAGS enum describing the operations
                the replayer will take control of.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux (others later)\n
  \b CPU:   All\n
*/
extern VOID PIN_SetReplayMode(UINT32 flags);

/*! @ingroup REPLAY THREADS
 *
 * Start a new thread which will begin to execute with the register
 * state from the context. At least the flags register in the context
 * must be sensible (even if all of the thread's registers will be set in the
 * thread start callback).
 *
 * This function requires the VM lock to operate, so will acquire and release it
 * if it is not already held.
 *
 * @param[in]   ctxt   context for the new thread.
 * @return      TRUE if a new thread was sucessfully spawned, FALSE if not.
 *
 * @note The vm lock is obtained during the call of this API.
 * @note This function must be used on applications that are running with the (newer)
 *       NPTL version of pthread library. Running this function on an application that
 *       is running with the (older) LinuxThreads version of pthread can prevent PIN
 *       from shuting down the VM when the application exits.
 *
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern BOOL PIN_SpawnApplicationThread(const CONTEXT* ctxt);

/*! @ingroup PIN_CONTROL
  Some Pin API functions may only be called when the thread is holding this lock. See also @ref PIN_UnlockClient.
  The client lock is recursive, so can be claimed again by a thread if it already holds it.
  PIN_LockClient and PIN_UnlockClient calls must be matched, since the lock implementation maintains
  a depth count.\n
  This function can be used in any thread, including internal threads spawned by the tool.

  @par Availability:
  \b Mode:  JIT & Probe\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID PIN_LockClient();

/*! @ingroup PIN_CONTROL
  Some Pin API functions may be called only when the thread is holding this lock. See also @ref PIN_LockClient.\n
  This function can be used in any thread, including any internal thread spawned by the tool.

  @par Availability:
  \b Mode:  JIT & Probe\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID PIN_UnlockClient();

/*! @ingroup  ERROR_FILE
  Writes the given error message to the Pin error file.  The error file name
  is specified using the -error_file switch.

  This API allows tool writer to write a tool-specified error message to the
  Pin error file.  Pin reserves the error type enums from 0-999.  Pin will exit
  after printing a fatal message.\n
  This function can be used in any thread, including any internal thread spawned by the tool.

  @param     msg           a description of the error
  @param     type          an enum associated with the error.  type >= 1000.
  @param     severity      0=fatal, 1=non-fatal.
  @param     num           number of arguments following this parameter
  @param     ...           arguments. must be of type "char *".

  @par Availability:
  \b Mode:  JIT, Probe\n
  \b O/S:   Linux, Windows\n
  \b CPU:   All.\n
*/
extern VOID PIN_WriteErrorMessage(const char* msg, INT32 type, PIN_ERR_SEVERITY_TYPE severity, INT32 num, ...);

/*! @ingroup RTN
 * Replace a routine in the application (\a replacedRtn) by another function defined
 * in the Pin tool (\a replacementFun).  The replacement function is not instrumented.
 * The signature of the replacement function must be exactly the same as the replaced
 * application routine.  However, see @ref RTN_ReplaceSignature(), which allows you to
 * have a different signature.
 *
 * This API returns a function pointer to the replaced application routine's entry point,
 * which allows the replacement function to call back to the original routine.  If you
 * do this, be sure to call the original routine via @ref PIN_CallApplicationFunction().
 * Directly calling the application's function pointer from the replacement function may
 * result in a crash.
 *
 * This API works in JIT mode, so you must start the application with @ref PIN_StartProgram().
 *
 *  @param[in] replacedRtn      The application routine to be replaced.
 *  @param[in] replacementFun   The replacement function.
 *
 * @return  A function pointer to the replaced application routine's entry point.
 *
 * @note The pin client lock is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT only.\n
 * \b O/S:   Linux & Windows\n
 * \b CPU:   All.\n
 * NOTE: Pin does not support calling this function from either the TRACE or INS InstrumentationFunction callback
 */
extern AFUNPTR RTN_Replace(RTN replacedRtn, AFUNPTR replacementFun);

/*! @ingroup RTN
 * Replace a routine in the application (\a replacedRtn) by another function defined
 * in the Pin tool (\a replacementFun).  The replacement function is not instrumented.
 * The signature of the replacement function can be different from the replaced
 * application routine's signature, which allows the tool to pass more (or fewer)
 * parameters than were passed to the original application routine.
 *
 * The @ref IARG_ORIG_FUNCPTR argument is especially useful because it allows the
 * replacement function to call back to the original routine.  Other useful arguments
 * are @ref IARG_FUNCARG_ENTRYPOINT_VALUE, which allows you to pass a parameter from
 * the original routine to the replacement function, and @ref IARG_PROTOTYPE, which
 * allows you define the prototype of the original routine.  The IARG_PROTOTYPE argument
 * is recommended whenever you use IARG_FUNCARG_ENTRYPOINT_VALUE.  It is required if
 * the original routine has any parameters that are not simple integral or pointer
 * values.
 *
 * If your replacement function calls back to the original application routine, be
 * sure to do so via @ref PIN_CallApplicationFunction().  Directly calling the application's
 * function pointer from the replacement function may result in a crash.
 *
 * This API works in JIT mode, so you must start the application with @ref PIN_StartProgram().
 *
 *  @param[in] replacedRtn      The application routine to be replaced.
 *  @param[in] replacementFun   The replacement function.
 *  @param[in] ...              Any additional arguments define parameters that are
 *                               passed to the replacement function, see @ref IARG_TYPE.
 *                               This list must end with @ref IARG_END.
 *
 * @return  A function pointer to the replaced application routine's entry point.
 *           The replacement function can use this pointer to call back to the
 *           original application routine, or it can use IARG_ORIG_FUNCPTR for
 *           this purpose.
 *
 * @note The pin client lock is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT only.\n
 * \b O/S:   Linux & Windows\n
 * \b CPU:   All.\n
 * NOTE: Pin does not support calling this function from either the TRACE or INS InstrumentationFunction callback
 */
extern AFUNPTR RTN_ReplaceSignature(RTN replacedRtn, AFUNPTR replacementFun, ...);

/*! @ingroup RTN
 * PROBE_MODE enumerator allows user to set probe mode instrumentation other than
 * default for a particular function. Usually, non-default mode is used when
 * Pin can't instrument a routine in a regular way.
 * A non default mode is usually less safe and tool-writer takes responsibility
 * for correctness in this case.
 *
 * PROBE_MODE_ALLOW_RELOCATION
 *
 * Doing probed instrumentation Pin inserts a jumper in the first bytes of the
 * instrumented routine. If the first basic block calculated within static discovery is
 * not long enough, Pin can't insert a jumper and the instrumentation request
 * is rejected.
 * One more chance to insert a jumper in such case is to relocate the whole
 * routine. It is not always possible, of course. The routine can be relocated
 * by Pin if
 *  - the size is known
 *  - there is no jumps outside function and
 *  - the routine does not contain indirect jumps
 *
 * The routine relocation may destabilize the application since ability to
 * propagate exceptions is not preserved. Doing static analysis Pin also does
 * not see additional entry points in the routine code.
 *
 * In PROBE_MODE_ALLOW_RELOCATION mode Pin tries to keep the instrumented routine
 * in place, and considers relocation when "in-place" instrumentation is impossible.
 * In PROBE_MODE_DEFAULT the relocation is not allowed.
 * Routine relocation is not supported on Windows.
 */
enum PROBE_MODE
{
    PROBE_MODE_DEFAULT          = 0,
    PROBE_MODE_ALLOW_RELOCATION = (1 << 0)
};

/*! @ingroup RTN
  Replace a routine in the application (replacedRtn) by another function defined in
  the Pintool (replacementFun) using probes. The replacement function is not instrumented.
  The signature of the replacement function must be the same as the replaced application
  routine.
  Replacement functions typically need to call the replaced routines. However, calls to
  RTN_Funptr(replacedRtn) will be redirected to replacementFun. Replacement functions
  must instead call or jump to the returned function pointer, which is a copy of the entry point
  that is not redirected.

  @param     replacedRtn           the application routine to be replaced.
  @param     replacementFun        the replacement function
  @return    a function pointer to replaced application routine's entry point.
             This allows the replacement function to execute the replaced routine.

  @ref PIN_StartProgramProbed() must be used when using this API.

  Use @ref RTN_IsSafeForProbedReplacement() to determine if a function is a
  suitable candidate for probed function replacement.

  @note This API assumes that the application stack (which is used by replacementFun) is
        aligned according to calling conventions when calling to replacement function, but that
        is not always guaranteed in Linux 32-bit. In case the stack is not aligned as
        expected then it may result in segmentation fault.
        To avoid this, add 'force_align_arg_pointer' function attribute to replacementFun or use
        @ref RTN_ReplaceSignatureProbed() instead.

  @note The pin client lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  Probe\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern AFUNPTR RTN_ReplaceProbed(RTN replacedRtn, AFUNPTR replacementFun);

/*! @ingroup RTN
 Replace a routine in the application (replacedRtn) by another function defined in
 the Pintool (replacementFun) using probes. This API is an analog to
 @ref RTN_ReplaceProbed.

 @param     replacedRtn           the application routine to be replaced.
 @param     mode                  instrumentation mode, see @ref PROBE_MODE
 @param     replacementFun        the replacement function
 @return    a function pointer to replaced application routine's entry point.

 @ref PIN_StartProgramProbedEx() must be used when using this API.

 Use @ref RTN_IsSafeForProbedReplacementEx(rtn, mode) to determine if a function is a
 suitable candidate for probed function replacement.

 @par Availability:
 \b Mode:  Probe\n
 \b O/S:   Linux & Windows\n
 \b CPU:   All\n
*/
extern AFUNPTR RTN_ReplaceProbedEx(RTN replacedRtn, PROBE_MODE mode, AFUNPTR replacementFun);

/*! @ingroup PIN_CONTROL
  This API allows a tool to call a function inside the application.
  The function is executed under control of Pin's JIT compiler, and
  the application code is instrumented normally.  Tools should not
  make direct calls to application functions when Pin is in JIT mode.

  PIN_CallApplicationFunction() can be called from a replacement routine or
  an analysis routine, but not from a call-back.  Note that this API
  can result in recursive execution in the tool since the called application
  function is also instrumented.

  One usage model for this API is to "wrap" an application function,
  executing analysis code before the function is called and after it
  returns.  To do this, use \ref RTN_ReplaceSignature() to replace the
  application function, use \ref IARG_ORIG_FUNCPTR to pass the original
  function pointer to the replacement routine, and use \ref IARG_CONST_CONTEXT
  to pass the register state.  You can then call back on the original function
  from your replacement routine with PIN_CallApplicationFunction().  Note,
  however, that the overhead of PIN_CallApplicationFunction() can be high, so
  it is more efficient to use *_InsertCall() instead if possible.

  @param[in] ctxt           The application register state to use when calling the
                             function.  Pin changes the register state in order to pass
                             arguments to the function, but the \a ctxt parameter
                             isn't changed.
  @param[in] tid            The Pin id of the current thread (see @ref IARG_THREAD_ID).
  @param[in] cstype         The calling standard used for calling this function
                             (CALLINGSTD_DEFAULT recommended).
  @param[in] origFunPtr     The entry point of the application function.
  @param[in] param          Additional parameters to define the exact behavior of this
                             function invocation (or NULL for using the defaults).
                             See @ref CALL_APPLICATION_FUNCTION_PARAM .
  @param[in] ...            List of function argument type and size.

  See @ref PIN_PARG for the recommended method of passing function arguments.
  @ref PIN_PARG_END() must end the argument list, even if the function has no arguments.
  If the application function has a return value, it must appear first in the argument list.

  @note The vm lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  JIT only\n
  \b O/S:   Linux and Windows\n
  \b CPU:   IA-32 and Intel(R) 64 architectures\n
*/
extern VOID PIN_CallApplicationFunction(const CONTEXT* ctxt, THREADID tid, CALLINGSTD_TYPE cstype, AFUNPTR origFunPtr,
                                        CALL_APPLICATION_FUNCTION_PARAM* param, ...);

/*! @ingroup INS_INSPECTION
  Return TRUE if ins is not an original instruction in the application, but is an
  instruction added by Pin for doing function replacement. Pin allows you to even
  instrument these non-original instructions in JIT mode.

  @par Availability:
  \b Mode:  JIT & Probe\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern BOOL INS_IsAddedForFunctionReplacement(INS ins);

/*! @ingroup PIN_CALLBACKS
  Sets the execution order of a registered callback function. See @ref CALL_ORDER.
  The the lower the order, the sooner it will be called relative to other
  similar callbacks.
  @param callback  Registered Callback function returned from *_Add*Function()
  @param order  New order to assign to the callback (relative to the other callbacks). See @ref CALL_ORDER for usage details.\n

  @note The pin client lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID CALLBACK_SetExecutionOrder(PIN_CALLBACK callback, CALL_ORDER order);

/*! @ingroup PIN_CALLBACKS
  Gets the execution order of a registered callback function. See @ref CALL_ORDER.
  The the lower the order, the sooner it will be called relative to other
  similar callbacks
  @param callback  Registered Callback function returned from *_Add*Function()

  @return CALL_ORDER The current order value of the callback

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern CALL_ORDER CALLBACK_GetExecutionOrder(PIN_CALLBACK callback);

/*! @ingroup TRACE
  Add a function used to instrument at trace granularity
  @param fun Instrumentation function for traces
  @param val passed as the second argument to the instrumentation function

  @return PIN_CALLBACK A handle to a callback that can be used to further modify this callback's properties

  @note The pin client lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern PIN_CALLBACK TRACE_AddInstrumentFunction(TRACE_INSTRUMENT_CALLBACK fun, VOID* val);

/*! @ingroup INS_INSTRUMENTATION
  Add a function used to instrument at instruction granularity
  @param fun Instrumentation function for instructions
  @param val passed as the second argument to the instrumentation function

  @return PIN_CALLBACK A handle to a callback that can be used to further modify this callback's properties

  @note The pin client lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern PIN_CALLBACK INS_AddInstrumentFunction(INS_INSTRUMENT_CALLBACK fun, VOID* val);

/*! @ingroup PIN_CONTROL
  Call func immediately before the application exits. The function is not an
  instrumentation function--it cannot insert instrumentation. There can be
  more than one Fini function.

  @param fun Call back function is passed exit code and val
  @param val Value to be passed to fun when it is called

  @return PIN_CALLBACK A handle to a callback that can be used to further modify this callback's properties

  @note  The pin client lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern PIN_CALLBACK PIN_AddFiniFunction(FINI_CALLBACK fun, VOID* val);

/*! @ingroup PIN_CONTROL
 * Register a function to be called when the application is about to exit.
 * The registered function will be executed in a thread that initiated the exit and
 * currently does not hold any thread synchronization lock in Pin. It means that this
 * callback function can be executed concurrently with other Pin callbacks and APIs.\n
 * All callbacks registered by this function will be executed before any callback
 * registered by the @ref PIN_AddFiniFunction() function.
 *
 * @param fun   Callback function to be registered
 * @param val   Value to be passed to fun when it is called
 *
 * @return PIN_CALLBACK A handle to a callback that can be used to further modify this callback's properties
 *
 * @note Function registered by this API after application exit is started, may not be executed.
 *
 * @note The pin client lock is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   All\n
 */
extern PIN_CALLBACK PIN_AddPrepareForFiniFunction(PREPARE_FOR_FINI_CALLBACK fun, VOID* val);

/*! @ingroup PIN_CONTROL
  Called to register a function That will become the function that Pin calls to fetch application
  instructions. Pin sometimes attempt to fetch instructions from an invalid address, and the
  call-back needs to handle this correctly.
  If multiple functions are registered using this API, only the last one registered is used

  Pin cannot automatically detect self-modifying code (SMC) when a tool uses
  this API. Therefore, such tools take over responsibility for detecting SMC
  and must explicitly regenerate all traces within a modified code range if such was detected using
  PIN_RemoveInstrumentationInRange.

  @param[in] fun The tool's call-back function.
  @param[in] val The tool's call-back value.

  @note  The pin client lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID PIN_AddFetchFunction(FETCH_CALLBACK fun, VOID* val);

/*! @ingroup PIN_CONTROL
  A function to fetch the code raw bytes.

  This function should be used when the tool needs the raw bytes of instructions
  and the tool is using a code fetcher function that translates the original code
  address to an actual code address.

  @param[out] copyBuf      destination buffer
  @param[in]  address      address to copy from
  @param[in]  maxSize      number of bytes to copy
  @param[out] pExceptInfo  pointer to the structure that receives the exception information

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern size_t PIN_FetchCode(void* copyBuf, const VOID* address, size_t maxSize, EXCEPTION_INFO* pExceptInfo);

/*! @ingroup PIN_CONTROL
 * Register a notification function that is called when pin/tool is out
 * of memory.\n Try to reduce memory consumption when this notification function
 * is called.\n Any implicit or explicit usage of malloc()/realloc() etc. in
 * this notification function might cause an infinite recursion.\n
 * The call to the notification function is NOT serialized by pin.
 * The user can't assume anything about the locks state while the
 * notification is delivered (as out of memory condition can occur anywhere).
 * Avoid dead-locks by careful design of the notification function.\n
 *
 * A tool can register only one notification function.\n
 * A new notification function registration overwrites any previous registration.\n
 * To disable the notification function, pass a NULL function pointer.\n
 *
 * @param fun Call back function is passed the size of the failing allocation
 * @param val Value to be passed to fun when it is called
 *
 * @note  The pin client lock is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT & PROBE\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   All\n
 */
extern VOID PIN_AddOutOfMemoryFunction(OUT_OF_MEMORY_CALLBACK fun, VOID* val);

/*! @ingroup PIN_CONTROL
 * Register a callback for modifying memory address arguments that are passed to analysis routines.
 * See @ref MEMORY_ADDR_TRANS_CALLBACK for usage details. \n
 *
 * Only one such callback may be registered. \n
 *
 * @param fun Call back function is passed the thread id, address, size, read\write
 * @param val Value to be passed to fun when it is called
 *
 * @note  The pin client lock is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT \n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   All\n
 */
extern VOID PIN_AddMemoryAddressTransFunction(MEMORY_ADDR_TRANS_CALLBACK fun, VOID* val);

/*! @ingroup TRACE
 * Register a call back to be called when Pin detects a self modification of code in the application.
 * See @ref SMC_CALLBACK for usage details. \n
 *
 * @param fun The call back function that is to be called
 * @param val Value to be passed to fun when it is called
 *
 * @note  The pin client lock is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT \n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   All\n
 */
extern VOID TRACE_AddSmcDetectedFunction(SMC_CALLBACK fun, VOID* val);

/*! @ingroup PIN_CONTROL
 * Get the Registered notification function that is called when tool needs to modify
 * memory address in PIN.\n
 *
 * @param fun Call back function has no parameters
 * @param val Value to be passed to fun when it is called
 * @par Availability:
 * \b Mode:  JIT & PROBE\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   All\n
 */
extern MEMORY_ADDR_TRANS_CALLBACK PIN_GetMemoryAddressTransFunction();

/*! @ingroup PIN_CONTROL
  Call func immediately before Pin relinquishes control of the application through
  @ref PIN_Detach().

  At this stage no more instrumentation/analysis callbacks will be called

  This function is called one time for the process. It may be called by
  any thread.
  Pin memory is not freed at this moment.

  @param fun Call back function to execute before detaching
  @param val Value to be passed to fun when it is called

  @return PIN_CALLBACK A handle to a callback that can be used to further modify this callback's properties

  @note The pin client lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, macOS*\n
  \b CPU:   All\n
*/
extern PIN_CALLBACK PIN_AddDetachFunction(DETACH_CALLBACK fun, VOID* val);

/*! @ingroup PIN_CONTROL
 * Register a notification function that is called immediately after Pin relinquishes control
 * of the application through @ref PIN_DetachProbed(). There can be more than one Detach
 * callback function.\n When this callback function is called, it is guaranteed that Pin
 * removed all probes from the application code and that no other callback function will be
 * called afterwards (except from additional Detach callback functions).\n
 *
 * @param fun Call back function to execute before detaching
 * @param val Value to be passed to fun when it is called
 *
 * @return PIN_CALLBACK A handle to a callback that can be used to further modify this callback's properties
 *
 * @note The pin client lock is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  Probe\n
 * \b O/S:   Windows, Linux\n
 * \b CPU:   All\n
 */
extern PIN_CALLBACK PIN_AddDetachFunctionProbed(DETACH_PROBED_CALLBACK fun, VOID* val);

/*! @ingroup PIN_CONTROL
  Register a notification function that is called when a thread starts executing in the application.
  The call-back happens even for the application's root (initial) thread.

  @param[in] fun    Call-back function to execute for the new thread.
  @param[in] val    Value to pass to the function.

  @return PIN_CALLBACK A handle to a callback that can be used to further modify this callback's properties

  @note  The pin client lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern PIN_CALLBACK PIN_AddThreadStartFunction(THREAD_START_CALLBACK fun, VOID* val);

/*! @ingroup PIN_CONTROL
 This API is useful when Pin is run in probe mode and attaches to a running process.
 Tools may use this API to register a notification that is called by each application
 thread.

 After Pin attaches to the process, Pin first notifies the tool of any images that
 are loaded (see @ref IMG_AddInstrumentFunction()) and then notifies the tool that
 initialization is complete (see @ref PIN_AddApplicationStartFunction()).  After that,
 each thread in the application calls the notification set up by
 PIN_AddThreadAttachProbedFunction().  After each thread returns from this notification,
 it resumes execution in the application code.

 Note, this notification happens only for threads that exist in the process at the time
 that Pin attaches.  It does not happen for threads that are created in the process
 after Pin is already attached.

 @param[in] fun    Call-back function to execute for the attached thread.
 @param[in] val    Value to pass to the function.

 @return PIN_CALLBACK A handle to a callback that can be used to further modify this callback's properties

 @note  The pin client lock is obtained during the call of this API.

 @par Availability:
 \b Mode:  Probe\n
 \b O/S:   Linux & macOS*\n
 \b CPU:   All\n
*/
extern PIN_CALLBACK PIN_AddThreadAttachProbedFunction(THREAD_ATTACH_PROBED_CALLBACK fun, VOID* val);

/*! @ingroup PIN_CONTROL
 This API is useful when Pin is run in JIT mode and attaches to a running process.
 Tools may use this API to register a notification that is called by each application
 thread.

 After Pin attaches to the process, Pin first notifies the tool of any images that
 are loaded (see @ref IMG_AddInstrumentFunction()) and then notifies the tool that
 initialization is complete (see @ref PIN_AddApplicationStartFunction()).  After that,
 each thread in the application calls the notification set up by
 PIN_AddThreadAttachFunction().  After each thread returns from this notification,
 it resumes execution in the application code.

 Note, this notification happens only for threads that exist in the process at the time
 that Pin attaches.

 It does not happen for the following threads:
 1. Threads that are created in the process after Pin is already attached.
 2. Internal threads spawned by Pin tool when Pin re-attaches to the application.

 @param[in] fun    Call-back function to execute for the attached thread.
 @param[in] val    Value to pass to the function.

 @return PIN_CALLBACK A handle to a callback that can be used to further modify this callback's properties

 @note  The pin client lock is obtained during the call of this API.

 @par Availability:
 \b Mode:  JIT\n
 \b O/S:   Linux & macOS*\n
 \b CPU:   All\n
*/
extern PIN_CALLBACK PIN_AddThreadAttachFunction(THREAD_ATTACH_CALLBACK fun, VOID* val);

/*! @ingroup PIN_CONTROL
 This API is useful when Pin runs in probe mode and detaches from a running process.
 Tools may use this API to register a notification that is called by each application
 thread before any probes removal.
 This notification is given in the thread native context.

 Note, this notification happens only for threads that exist in the process at the time
 that Pin detaches.  It does not happen for threads that are created in the process
 after Pin is already detached.

 @param[in] fun    Call-back function to execute by the detaching thread.
 @param[in] val    Value to pass to the function.

 @return PIN_CALLBACK A handle to a callback that can be used to further modify this callback's properties

 @note  The pin client lock is obtained during the call of this API.

 @par Availability:
 \b Mode:  Probe\n
 \b O/S:   Linux & macOS*\n
 \b CPU:   All\n
 */
extern PIN_CALLBACK PIN_AddThreadDetachProbedFunction(THREAD_DETACH_PROBED_CALLBACK fun, VOID* val);

/*! @ingroup PIN_CONTROL
 This API is useful when Pin runs in JIT mode and detaches from a running process.
 Tools may use this API to register a notification that is called by each application
 thread before Pin detaches from the thread.
 This notification is given in the thread native context.

 Note, this notification happens only for application threads that exist in the process at the time
 that Pin detaches.
 It does not happen for the following threads:
 1. Threads that are created in the process after Pin is already detached.
 2. Internal threads spawned by Pin tool.


 @param[in] fun    Call-back function to execute by the detaching thread.
 @param[in] val    Value to pass to the function.

 @return PIN_CALLBACK A handle to a callback that can be used to further modify this callback's properties

 @note  The pin client lock is obtained during the call of this API.

 @par Availability:
 \b Mode:  JIT\n
 \b O/S:   Linux\n
 \b CPU:   All\n
 */
extern PIN_CALLBACK PIN_AddThreadDetachFunction(THREAD_DETACH_CALLBACK fun, VOID* val);

/*! @ingroup PIN_CONTROL
  Register a notification function that is called after pin initialization is finished.
  This notification function is available when pin launches the application and when pin
  attaches to a running process.

  On Unix (Linux & macOS*):
    This notification function is called on the application's main thread
    (both launch & attach mode).
    This notification is called right before Pin resume the application.

  On Windows:
    When pin launches the application, this notification function is called on the
    application's main thread.
    When pin attaches to a running process, this notification function is called on a
    dedicated thread which is not part of the application's threads.\n
    When it is called, all the application's threads are paused by Pin and are about to
    be released.\n

  No new threads can be created by the application at this time.

  When Pin starts running a program, it calls into the tool several times in the
  following order:\n
  1. Calls the tool's main() function.\n
  2. If the tool set up a notification with IMG_AddInstrumentFunction(), Pin calls
     this call-back for each image that is already loaded in the application's address
     space at the point when Pin attaches to it.\n
  3. Calls any notification call-backs set up with PIN_AddApplicationStartFunction().\n

  @param[in] fun     Function to be called at application start.
  @param[in] val     Value to pass to the function.

  @return PIN_CALLBACK A handle to a callback that can be used to further modify this callback's properties

  @note  The pin client lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  JIT & Probe\n
  \b O/S:   Windows, Linux, macOS*\n
  \b CPU:   IA-32, Intel(R) 64\n
*/
extern PIN_CALLBACK PIN_AddApplicationStartFunction(APPLICATION_START_CALLBACK fun, VOID* val);

/*! @ingroup PIN_CONTROL
  Register a notification function that is called when an application thread terminates.
  The call-back happens even for the application's root (initial) thread.

  The notification call-back will often happen on the physical thread that is exiting,
  but under some circumstances it will be called on a different physical thread.

  @param[in] fun    Call-back function to execute for the terminating thread.
  @param[in] val    Value to pass to the function.

  @return PIN_CALLBACK A handle to a callback that can be used to further modify this callback's properties

  @note The pin client lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern PIN_CALLBACK PIN_AddThreadFiniFunction(THREAD_FINI_CALLBACK fun, VOID* val);

/*! @ingroup PIN_CONTROL
  Register a notification function that is called immediately before the application changes context
  due to receipt of an asynchronous event such as Unix signal or Windows APC.

  @param[in] fun    Call-back function to execute immediately before the application changes context.
  @param[in] val    Value to pass to the function.

  @return PIN_CALLBACK A handle to a callback that can be used to further modify this callback's properties

  @note  The pin client lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows\n
  \b CPU:   All\n
*/
extern PIN_CALLBACK PIN_AddContextChangeFunction(CONTEXT_CHANGE_CALLBACK fun, VOID* val);

/*! @ingroup  SYSCALL
  Register a notification function that is called immediately before execution of a
  system call.

  @param[in] fun     Function to be called immediately before execution of a system call.
  @param[in] val     Value to pass to the function.

  @return PIN_CALLBACK A handle to a callback that can be used to further modify this callback's properties

  @note  The pin client lock is obtained during the call of this API..
  @note  In Linux systems with kernel 5.3 and above, callbacks will be called also when branching to vsyscall area.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows\n
  \b CPU:   All\n
*/
extern PIN_CALLBACK PIN_AddSyscallEntryFunction(SYSCALL_ENTRY_CALLBACK fun, VOID* val);

/*! @ingroup  SYSCALL
  Register a notification function that is called immediately after execution of a
  system call. \n
  The notification is called for each system call when it returns to the
  application, even if the system call changes control flow and does not fall
  through to the next instruction. \n

  @param[in] fun     Function to be called immediately after execution of a system call.
  @param[in] val    Value to pass to the function.

  @note
  In order to monitor all possible control transfers from the operating system to the user mode
  application, the tool has to register both a SYSCALL_EXIT_CALLBACK and a CONTEXT_CHANGE_CALLBACK
  callback functions. This can be done using the corresponding registration functions:
  PIN_AddSyscallExitFunction() and PIN_AddContextChangeFunction().\n
  When a system call returns to the application, the tool receives the SYSCALL_EXIT_CALLBACK
  notification. Usually, this callback immediately follows the corresponding SYSCALL_ENTRY_CALLBACK
  notification and there is no application code executed between these two events. However, some
  (Windows) system calls can be interrupted by a system event (APC, Windows callback, exception)
  before they return to the application. If this happens, the tool receives the corresponding
  CONTEXT_CHANGE_CALLBACK notification just before the (user mode) handler of the system event
  gets executed. Eventually, when the event handler and the interrupted system call are completed,
  the SYSCALL_EXIT_CALLBACK notification is delivered to the tool.

  @return PIN_CALLBACK A handle to a callback that can be used to further modify this callback's properties

  @note  The pin client lock is obtained during the call of this API.
  @note  In Linux systems with kernel 5.3 and above, callbacks will be called also when branching to vsyscall area.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows\n
  \b CPU:   All\n
*/
extern PIN_CALLBACK PIN_AddSyscallExitFunction(SYSCALL_EXIT_CALLBACK fun, VOID* val);

/*! @ingroup PIN_CONTROL
 * Establish an interceptor function for signals delivered to the application.
 * Tools should never call sigaction() directly to handle signals.  The interceptor
 * function is called whenever the application receives the requested signal,
 * regardless of whether the application has a handler for that signal.  The
 * tool's handler can then decide whether the signal should be forwarded to the
 * application.
 *
 * There are two intended usage models for this API, which are both advanced
 * features.  If your tool merely needs to be notified of signals that are
 * sent to the application, use PIN_AddContextChangeFunction() instead.
 *
 * In one usage model, a tool can take over ownership of a signal in order to
 * use the signal as an asynchronous communication mechanism to the outside
 * world.  For example, if a tool intercepts SIGUSR1, a user of the tool could
 * send this signal and tell the tool to do something.  In this usage model,
 * the tool may want to call PIN_UnblockSignal() so that it will receive the
 * signal even if the application attempts to block it.
 *
 * In the second usage model, the tool may need to "squash" certain signals that
 * the application generates.  For example, a tool that forces speculative
 * execution in the application may want to intercept and squash exceptions
 * generated in the speculative code.
 *
 * A tool can set only one "intercept" handler for a particular signal, so a
 * new handler overwrites any previous handler for the same signal.  To disable
 * a handler, pass a NULL function pointer.
 *
 *  @param[in] sig      The signal number to intercept, which may not be SIGKILL
 *                       or SIGSTOP.
 *  @param[in] fun      The tool's intercept function, or NULL.
 *  @param[in] val      Value to pass to the intercept function.
 *
 * @return  TRUE on success (always FALSE on Windows).
 *
 * @note  The pin client lock is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux\n
 * \b CPU:   All\n
 */
extern BOOL PIN_InterceptSignal(INT32 sig, INTERCEPT_SIGNAL_CALLBACK fun, VOID* val);

/*! @ingroup PIN_CONTROL
 * This API is normally used in conjunction with PIN_InterceptSignal() to prevent the
 * application from blocking a signal that the tool intercepts.  This is useful if the
 * tool wants to ensure that the application will not prevent the tool from receiving
 * the intercepted signal.
 *
 * When this API is called before PIN_StartProgram(), it affects all threads in the
 * application.  However, if it is called after PIN_StartProgram(), it only affects the
 * calling thread.  If the application spawns a thread, the child thread inherits the
 * signal blocking state from its parent.
 *
 * Tools should take care when using this API because it can adversely affect the
 * application if it also uses the signal \a sig.
 *
 *  @param[in] sig      The signal number, which may not be SIGKILL or SIGSTOP.
 *  @param[in] enable   If TRUE, the application is prevented from blocking \a sig.
 *                       If FALSE, the application may block or unblock \a sig at will.
 *
 * @return  TRUE on success (always FALSE on Windows).
 *
 * @note  The pin client lock is obtained during the call of this API.
 *
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux\n
 * \b CPU:   All\n
 */
extern BOOL PIN_UnblockSignal(INT32 sig, BOOL enable);

/*! @ingroup PIN_CONTROL
  * Enum for controlling Pin's SMC detection
 */
typedef enum
{
    SMC_ENABLE,
    SMC_DISABLE
} SMC_ENABLE_DISABLE_TYPE;

/*! @ingroup PIN_CONTROL
 * Enable/Disable Pin's support of self modifying code
 * When enabled, Pin will detect self modification of code and will deliver a callback, if one is registered,
 * when such an event happens.
 * When disabled and if SMC occurs, the SMC will not be detected and the original code will continue to execute.
 * This function takes precedence over the specification of smc_support in the invocation line.
 * This function must be called before PIN_StartProgram.
 *
 * @param[in] enable    SMC_ENABLE, or SMC_DISABLE
 *
 * SMC support is enabled by default
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Windows, Linux & macOS*\n
 * \b CPU:   All\n
 */
extern VOID PIN_SetSmcSupport(SMC_ENABLE_DISABLE_TYPE enable_disable);

/*! @ingroup PIN_CONTROL
 * time of callback notification
 */
enum FPOINT
{
    FPOINT_BEFORE,          ///< Call-back in parent, just before fork
    FPOINT_AFTER_IN_PARENT, ///< Call-back in parent, immediately after fork
    FPOINT_AFTER_IN_CHILD   ///< Call-back in child, immediately after fork
};

/*! @ingroup PIN_CONTROL
 * Register a notification handler that is called when the application
 * forks a new process.
 *
 *    @param[in] where       Tells when to make the call-back and tells
 *                            whether the parent or child makes the call-back.
 *    @param[in] fun         Call-back function to execute.
 *    @param[in] v           Value passed as final parameter to call-back.
 *
 *    @return PIN_CALLBACK A handle to a callback that can be used to further modify this callback's properties
 *
 * If the fork function fails, the FPOINT_AFTER_IN_PARENT function, if it exists,
 * is still called.
 *
 * @note  The pin client is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, macOS*\n
 * \b CPU:   All\n
*/
extern PIN_CALLBACK PIN_AddForkFunction(FPOINT point, FORK_CALLBACK fun, VOID* val);

/*! @ingroup PIN_CONTROL
  All instrumentation is removed. When application code is executed
  the instrumentation routines will be called to reinstrument all code.
  Similarly, any ahead-of-time instrumentation will be reapplied if the
  relevant code is re-executed.

  @note  The vm lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID PIN_RemoveInstrumentation();

/*! @ingroup PIN_CONTROL
  Removes all instrumentation from the given range [start,end]. If the application reaches this range again,
  the code will be reinstrumented.
  Can be called from an analysis function or a callback.
  Note: Some code beyond the start and end margins may be reinstrumented as well.

  @param start The starting address of the range
  @param end   The ending address (inclusive) of the range

  @note The vm lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID PIN_RemoveInstrumentationInRange(ADDRINT start, ADDRINT end);

/*! @ingroup PIN_CONTROL
  Invalidates all the Fini callback functions registered via @ref PIN_AddFiniFunction;
  Fini callbacks will no longer be called before the application exits.

  Must be called prior to invoking @ref PIN_Detach if the tool has previously
  registered Fini callbacks and libc.so is not yet loaded into memory yet for a
  dynamically linked binary.

  @note  The pin client lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID PIN_RemoveFiniFunctions();

/*! @ingroup PIN_CONTROL

  Pin relinquishes control of the application and the original un-instrumented
  code is executed.

  On Linux, the function may be invoked from analysis, instrumentation or callback routines.
  It may be executed by any thread, including threads spawned by the Pintool which
  are invisible to the Pin VM.

  On Windows and macOS*, the function must be invoked from an analysis routine.
  Invoking the function from instrumentation routines could cause Pin to misbehave.
  See Tests/detach.C for an example of its use.

  The request for detach is considered by Pin as asynchronous. This means that Pin will
  perform the detach procedure at a point which it considers convenient to do so.
  From the user's perspective, instrumentation, analysis or callback routines might still be
  executed after calling PIN_Detach().
  In practice, PIN_Detach() simply registers the request to the Pin VM and exits immediately
  i.e. the function does not wait for detach to complete.

  Once Pin starts the detach procedure, it can't be blocked, and will not run forever.
  It is not possible to detach one thread. All threads will run natively after PIN_Detach().
  During detach, Pin doesn't free memory used for Pin objects. This memory will be used
  during re-attach.

  During the detach procedure Pin runs an auxiliary process, which communicates with the
  running application process via debug interface. The auxiliary process is temporary and
  will exit immediately when the detach procedure is completed. For the user, this means that
  a debugger cannot be attached to the application process during detach.

  Use PIN_AddDetachFunction() to receive a notification just prior to detach.
  The callback will be received after all the threads are stopped inside the VM and
  no more instrumentation functions, analysis routines or callbacks will be called.\n
  Note: at this point, the auxiliary process mentioned above is still attached to the
  application process.

  @note  The vm and client locks are obtained during the call of this API.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows, & macOS*\n
  \b CPU:   All\n
*/
extern VOID PIN_Detach();

/*! @ingroup PIN_CONTROL
 * Return values of PIN_Attach() and PIN_AttachProbed() which describe the status of the attach
 * request.
 */
enum ATTACH_STATUS
{
    ATTACH_INITIATED,    ///< Attach request was delivered successfully to PIN
    ATTACH_FAILED_DETACH ///< Attach request was not delivered to Pin
                         ///< since detach session hasn't completed yet.
};

/*! @ingroup PIN_CONTROL
 * Initiate Pin attach request in JIT mode. If Pin was not detached
 * (using @ref PIN_DetachProbed()) this function will be ignored.\n
 * The given notification function will be called immediately after Pin regains control
 * of the application. It is expected that in the scope of this callback, the tool will
 * register callbacks (e.g. @ref IMG_AddInstrumentFunction()) and will reconfigure itself
 * (if needed). In order to change tool's KNOB value, use @ref KNOBS
 *
 * @note Pin's state can't be reconfigured in the scope of the callback function
 * (don't use @ref PIN_Init(), @ref PIN_InitSymbols(), @ref PIN_StartProgramProbed() etc.)
 *
 * @param fun Call back function to execute when starting to attach
 * @param val Value to be passed to fun when it is called
 *
 * @return status of attach request (ATTACH_STATUS)
 *
 * @note  The vm and pin client locks are obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Windows, Linux\n
 * \b CPU:   IA32, Intel64\n
 */
extern ATTACH_STATUS PIN_Attach(ATTACH_CALLBACK fun, VOID* val);

/*! @ingroup PIN_CONTROL
 * Initiate Pin detach request in probe mode.
 * In the scope of this operation, Pin removes all probes set by Pin & tool,
 * The user can get a notification when the detach operation has been completed
 * (via callback registration using @ref PIN_AddDetachFunctionProbed()).
 * On Linux, the user can get additional per-thread callbacks just prior to probes removal.
 * (via callback registration using @ref PIN_AddThreadDetachProbedFunction()).These callbacks will be given for each
 * thread in its native context.
 * If Pin has already detached (using @ref PIN_DetachProbed()), or if Pin has not fully
 * started yet (the notification function registered by @ref PIN_AddApplicationStartFunction()
 * was not yet delivered to the tool), this function will be ignored.\n
 *
 * @note  Even though Pin removes all probes, replacement functions can still be active
 *        (for example, if the replacement function is still located on the call stack of one
 *        of the threads). It is the tool responsibility to handle these scenarios.\n
 *
 *
 * @note  The vm and pin client locks are obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  Probe\n
 * \b O/S:   Linux, Windows, & macOS*\n
 * \b CPU:   IA32, Intel64\n
 */
extern VOID PIN_DetachProbed();

/*! @ingroup PIN_CONTROL
 * Initiate Pin attach request in probe mode. If Pin was not detached
 * (using @ref PIN_DetachProbed()) this function will be ignored.\n
 * The given notification function will be called immediately after Pin regain control
 * of the application. It is expected that in the scope of this callback, the tool will
 * register callbacks (e.g. @ref IMG_AddInstrumentFunction()) and will reconfigure itself
 * (if needed). In order to change tool's KNOB value, use @ref KNOBS
 *
 * @note Pin's state can't be reconfigured in the scope of the callback function
 * (don't use @ref PIN_Init(), @ref PIN_InitSymbols(), @ref PIN_StartProgramProbed() etc.)
 *
 * @param fun Call back function to execute when starting to attach
 * @param val Value to be passed to fun when it is called
 *
 * @return status of attach request (ATTACH_STATUS)
 *
 * @note  The vm and pin client locks are obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  Probe\n
 * \b O/S:   Windows, Linux\n
 * \b CPU:   IA32, Intel64\n
 */
extern ATTACH_STATUS PIN_AttachProbed(ATTACH_PROBED_CALLBACK fun, VOID* val);

/*! @ingroup INS_INSTRUMENTATION

  See @ref INS_InsertCall. When the instruction has a predicate and the predicate is false, the analysis
  function is not called.

  On the IA-32 and Intel 64 architectures the only instructions
  treated as predicated are CMOVcc, FCMOVcc and REPped string ops.
  For the conditional moves, the predicate is based on the condition
  codes tested by the instruction. For the REPped string ops the
  predicate is that the execution count is not zero. For all other
  instructions the predicate is always true, so
  INS_InsertPredicatedCall is identical to @ref INS_InsertCall.

  If you want to test both your own condition, and the predicate, you can use @ref
  INS_InsertThenPredicatedCall, or use @ref IARG_EXECUTING to pass the predicate value to an @ref
  INS_InsertIfCall.

  If more than one call is inserted for the same instruction, the order is determined by @ref IARG_CALL_ORDER.
  For more information, see @ref CALL_ORDER.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID INS_InsertPredicatedCall(INS ins, IPOINT ipoint, AFUNPTR funptr, ...);

/*! @ingroup INS_INSTRUMENTATION
  Insert a call to funptr relative to instruction ins.
  @param ins Instruction to instrument
  @param action Specifies before, after, etc.
  \n IPOINT_BEFORE is always valid for all instructions.
  \n IPOINT_AFTER is valid only when a fall-through exists (i.e. Calls and unconditional branches will fail).
  It is only allowed when INS_IsValidForIpointAfter(ins) is true.
  \n IPOINT_TAKEN_BRANCH is invalid for non-branches. It is only allowed when INS_IsValidForIpointTakenBranch is true.
  @param funptr Insert a call to funptr
  @param ... List of arguments to pass funptr. See @ref IARG_TYPE, terminated with IARG_END

  If more than one call is inserted for the same instruction, the order is determined by @ref IARG_CALL_ORDER.
  For more information, see @ref CALL_ORDER.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID INS_InsertCall(INS ins, IPOINT action, AFUNPTR funptr, ...);

/*! @ingroup INS_INSTRUMENTATION

  Insert a call to funptr relative to an INS. If funptr returns a
  non-zero ADDRINT, then the immediately following "then" analysis call is
  executed.

  @param ins Instruction to instrument
  @param action Specifies before, after, etc.
  \n IPOINT_BEFORE is always valid for all instructions.
  \n IPOINT_AFTER is valid only when a fall-through exists (i.e. Calls and unconditional branches will fail).
  It is only allowed when INS_IsValidForIpointAfter(ins) is true.
  \n IPOINT_TAKEN_BRANCH is invalid for non-branches. It is only allowed when INS_IsValidForIpointTakenBranch is true.
  \n IPOINT_ANYWHERE is not supported and will result an error.
  \n action value must be identical to the value passed to the corresponding @ref INS_InsertThenCall.
  @param funptr Insert a call to funptr. Its return type must be ADDRINT
  @param ... List of arguments to pass funptr. See @ref IARG_TYPE, terminated with IARG_END

  If more than one call is inserted for the same instruction, the order is determined by @ref IARG_CALL_ORDER.
  For more information, see @ref CALL_ORDER.
  Note that if @ref CALL_ORDER is used, Both "if" and "then" analysis calls must have the same order.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID INS_InsertIfCall(INS ins, IPOINT action, AFUNPTR funptr, ...);

/*! @ingroup INS_INSTRUMENTATION

  Insert a call to funptr relative to an INS. The function is
  called only if the immediately preceding "if" analysis call returns a
  non-zero value.

  @param ins Instruction to instrument
  @param action Specifies before, after, etc.
  \n IPOINT_BEFORE is always valid for all instructions.
  \n IPOINT_AFTER is valid only when a fall-through exists (i.e. Calls and unconditional branches will fail).
  It is only allowed when INS_IsValidForIpointAfter(ins) is true.
  \n IPOINT_TAKEN_BRANCH is invalid for non-branches. It is only allowed when INS_IsValidForIpointTakenBranch is true.
  \n IPOINT_ANYWHERE is not supported and will result an error.
  \n action value must be identical to the value passed to the corresponding @ref INS_InsertIfCall.
  @param funptr Insert a call to funptr
  @param ... List of arguments to pass funptr. See @ref IARG_TYPE, terminated with IARG_END

  If more than one call is inserted for the same instruction, the order is determined by @ref IARG_CALL_ORDER.
  For more information, see @ref CALL_ORDER.
  Note that if @ref CALL_ORDER is used, Both "if" and "then" analysis calls must have the same order.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID INS_InsertThenCall(INS ins, IPOINT action, AFUNPTR funptr, ...);

/*! @ingroup INS_INSTRUMENTATION

  Insert a call to funptr relative to an INS. If funptr returns a
  non-zero ADDRINT and the instruction has a true predicate, then the
  immediately following "then" analysis call is executed. If the
  instruction is not predicated, then this function is identical to
  @ref INS_InsertIfCall.

  Note that funptr \e may be called even if the predicate is false,
  the predicate is only defined to guard the execution of the
  following THEN function. (So if the function inserted here modifies
  the machine state it might affect the value of the predicate. Best
  practice is not to modify machine state here!)

  On IA32 and Intel64, the sequences
  @code
     INS_InsertIfCall(...);
     INS_InsertThenPredicatedCall(...);
  @endcode
  @code
     INS_InsertIfPredicatedCall(...);
     INS_InsertThenCall(...);
  @endcode
  and
  @code
     INS_InsertIfPredicatedCall(...);
     INS_InsertThenPredicatedCall(...);
  @endcode

  produce identical results. They all generate code which can be
  represented like this
  @code
      if (UsersIfFunction(...))
          if (predicate)
              UsersThenFunction(...);
  @endcode
  However on other architectures the behavior may be different, and
  the "IF" call need not always be called.

  This means that on IA32 and Intel64 the user's IF code is always called, however on these architectures
  that is very likely to generate faster code, since predicated instructions are rare. The dynamically
  most important predicated instructions are almost certainly REP MOVS, which have a very low (~80ppm)
  probablity of being predicated false. Unless the user's IF code is very large, or has a lower
  probability of filtering the execution, it is always better to use the test of the user condition to
  prevent execution of the predicate test. Of course, the code included in a user IF call is expected to
  be small, since the objective of INS_InsertIfCall, INS_InsertThenCall is to allow the code in the IF to
  be inlined.

  If you need to know the instruction predicate inside the IF call, you should pass IARG_EXECUTING as an
  argument and test it.

  @param ins Instruction to instrument
  @param action Specifies before, after, etc.
  \n @ref IPOINT_BEFORE is always valid for all instructions.
  \n @ref IPOINT_AFTER is valid only when a fall-through exists (i.e. Calls and unconditional branches will fail).
  It is only allowed when INS_IsValidForIpointAfter(ins) is true.
  \n @ref IPOINT_TAKEN_BRANCH is invalid for non-branches. It is only allowed when INS_IsValidForIpointTakenBranch is true.
  @param funptr Insert a call to funptr. Its return type must be ADDRINT
  @param ... List of arguments to pass funptr. See @ref IARG_TYPE, terminated with IARG_END

  If more than one call is inserted for the same instruction, the order is determined by @ref IARG_CALL_ORDER.
  For more information, see @ref CALL_ORDER.
  Note that if @ref CALL_ORDER is used, Both "if" and "then" analysis calls must have the same order.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID INS_InsertIfPredicatedCall(INS ins, IPOINT action, AFUNPTR funptr, ...);

/*! @ingroup INS_INSTRUMENTATION

  Insert a call to funptr relative to an INS. The function is called only if the immediately preceding
  "if" analysis call returns a non-zero value and the instruction's predicate is true. See @ref
  INS_InsertIfPredicatedCall for details of the semantics of mixing @ref INS_InsertThenPredicatedCall
  with @ref INS_InsertIfCall (and all the other possibilities).

  @param ins Instruction to instrument
  @param action Specifies before, after, etc.
  \n IPOINT_BEFORE is always valid for all instructions.
  \n IPOINT_AFTER is valid only when a fall-through exists (i.e. Calls and unconditional branches will fail).
  It is only allowed when INS_IsValidForIpointAfter(ins) is true.
  \n @ref IPOINT_TAKEN_BRANCH is invalid for non-branches. It is only allowed when INS_IsValidForIpointTakenBranch is true.
  @param funptr Insert a call to funptr
  @param ... List of arguments to pass funptr. See @ref IARG_TYPE, terminated with IARG_END

  If more than one call is inserted for the same instruction, the order is determined by @ref IARG_CALL_ORDER.
  For more information, see @ref CALL_ORDER.
  Note that if @ref CALL_ORDER is used, Both "if" and "then" analysis calls must have the same order.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID INS_InsertThenPredicatedCall(INS ins, IPOINT action, AFUNPTR funptr, ...);

/*! @ingroup PIN_CONTROL
  Creates an instance of PIN_CONFIGURATION_INFO with default values.
  This instance can be passed to @ref PIN_StartProgram() in order
  to take effect on Pin.

  The default values are:
  - Callback types which are allowed to run concurrently - None (PIN_CALLBACK_TYPE_NONE)

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern PIN_CONFIGURATION_INFO PIN_CreateDefaultConfigurationInfo();

/*! @ingroup PIN_CONTROL
  Enables/disables concurrent delivery of certain callbacks in a specific
  PIN_CONFIGURATION_INFO instance.

  @param options                    The @ref PIN_CONFIGURATION_INFO instance to modify.
  @param callbackType
      Specifies which Pin callbacks are allowed to be delivered concurrently:\n\n
      In the case where a callback should be delivered to several threads at a given moment,
      delivering a callback concurrently means that each thread is free to deliver its the respective callbacks
      without waiting for the other threads.
      This means that Pin doesn't acquire any exclusive internal lock in order to deliver a concurrent callback\n\n
      In constrast, delivering a callback in a serialized way means that, in a given moment, only one threads is
      allowed to deliver any type of callback.
      Serializing the callback might hurt the performance of your Pin tool, but it will protect the tool
      against possibly race conditions in your tool.
      When a callback is considered "serialized", Pin will acquire an exclusive internal lock
      before delivering the callback to the tool.

  @note In order for this option to take effect the @ref PIN_CONFIGURATION_INFO must be passed
  to @ref PIN_StartProgram().

  @note Currently, other types of callbacks (which don't have a respective enum entry in @ref PIN_CALLBACK_TYPE)
  are serialized by Pin with no option to change that.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux\n
  \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID PIN_SetAllowedConcurrentCallbacks(PIN_CONFIGURATION_INFO options, PIN_CALLBACK_TYPE callbackType);

/*! @ingroup PIN_CONTROL
  Starts executing the application, when Pin is in JIT mode, which is the default.
  Note that @ref PIN_Init() must be called before PIN_StartProgram().

  @param options   Pin global configuration to use in this run of Pin.
                   These configuration options are set only once (during the call
                   of PIN_StartProgram()), and they are affecting the entire execution
                   of Pin.

  @note The PIN_StartProgram() function never returns.  It also unwinds the tool's
  stack, so any local (stack based) variables are lost.

  @note The vm and pin client locks are obtained during the call of this API.

  See also: @ref PIN_CreateDefaultConfigurationInfo().

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID PIN_StartProgram(PIN_CONFIGURATION_INFO options = PIN_CreateDefaultConfigurationInfo());

/*! @ingroup PIN_CONTROL
*/
extern VOID PIN_StartProgram(PIN_CONFIGURATION_INFO options);

/*! @ingroup PIN_CONTROL
  Starts executing the application, when Pin is in Probe mode.
  Note that @ref PIN_Init() must be called before PIN_StartProgramProbed().

  The PIN_StartProgramProbed() function never returns.  It also unwinds the tool's
  stack, so any local (stack based) variables are lost.

  @note The vm and pin client locks are obtained during the call of this API.

  @par Availability:
  \b Mode:  Probe\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID PIN_StartProgramProbed();

/*! @ingroup PIN_CONTROL
 * Returns a fully descriptive Pin version string, including the Intel copyright notice. The returned version is the Pin version
 * that the tool was linked with.
 */
extern const std::string& PIN_Version();

/*! @ingroup PIN_CONTROL
  Initialize Pin system. Must be called before @ref PIN_StartProgram
  @param argc argc value passed to main
  @param argv argv value passed to main, encoded in UTF8 (a superset of ASCII)

  @return true if an error was detected parsing the command line

  @note The pin client lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  JIT & Probe\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern BOOL PIN_Init(INT32 argc, CHAR** argv);

/*! @ingroup TRACE
  Insert one or more analysis calls in a trace.

  @param[in] trace      The trace to instrument.
  @param[in] action     Specifies when the analysis call executes:
    \li \c IPOINT_BEFORE inserts the call before the first instruction in the trace.
    \li \c IPOINT_AFTER inserts the call after the last instruction in the trace.  This call
        will only execute if execution falls-through (i.e. does not branch).  You may
        only use IPOINT_AFTER if the last instruction in the trace has a fall-through
        path, which you can find out by using @ref TRACE_HasFallThrough().
    \li \c IPOINT_ANYWHERE is like IPOINT_BEFORE, but may put the call on a different instruction
        for better performance.
    \li \c IPOINT_TAKEN_BRANCH inserts a call after each branch in the trace.  The call only
        executes if the trace exits with a taken branch.

  @param[in] funptr     The analysis function to call.
  @param[in] ...        @ref IARG_TYPE.  Arguments to pass to \a funptr.

  @note  The pin client lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID TRACE_InsertCall(TRACE trace, IPOINT action, AFUNPTR funptr, ...);

/*! @ingroup TRACE

  Insert one or more analysis calls in a trace.  If \a funptr returns a
  non-zero ADDRINT, then the immediately following "then" analysis call is
  executed.
  Note that if @ref CALL_ORDER is used, Both "if" and "then" analysis calls must have the same order.

  @param[in] trace      The trace to instrument.
  @param[in] action     Specifies when the analysis call executes.  See the documentation
                         in @ref TRACE_InsertCall().
  @param[in] funptr     The analysis function to call. Its return type must be ADDRINT.
  @param[in] ...        @ref IARG_TYPE.  Arguments to pass to \a funptr.

  @note  The pin client lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID TRACE_InsertIfCall(TRACE trace, IPOINT action, AFUNPTR funptr, ...);

/*! @ingroup TRACE

  Insert one or more analysis calls in a trace.  The functions are called
  only if the immediately preceding "if" analysis call returns a non-zero
  value.
  Note that if @ref CALL_ORDER is used, Both "if" and "then" analysis calls must have the same order.

  @param[in] trace      The trace to instrument.
  @param[in] action     Specifies when the analysis call executes.  See the documentation
                         in @ref TRACE_InsertCall().
  @param[in] funptr     The analysis function to call.
  @param[in] ...        @ref IARG_TYPE.  Arguments to pass to \a funptr.

  @note  The pin client lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID TRACE_InsertThenCall(TRACE trace, IPOINT action, AFUNPTR funptr, ...);

/*! @ingroup TRACE
  @return first bbl of trace

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern BBL TRACE_BblHead(TRACE trace);

/*! @ingroup TRACE
  @return last bbl of trace

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern BBL TRACE_BblTail(TRACE trace);

/*! @ingroup TRACE
  @return Application address of a trace

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern ADDRINT TRACE_Address(TRACE trace);

/*! @ingroup TRACE
  Traces represent contiguous segments of the original code.  This function returns
  the original source footprint of the given trace (not the corresponding
  post-instrumentation footprint in the code cache).
  @return Original application code size of a trace

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern USIZE TRACE_Size(TRACE trace);

/*! @ingroup TRACE
  @return RTN that contains first instruction of trace

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern RTN TRACE_Rtn(TRACE trace);

/*! @ingroup TRACE
  Tells if the last instructon in the trace has a fall-through path.

  @param[in] trace  The trace.
  @return TRUE if the last instruction in the trace has a fall-through path.
  see @ref INS_HasFallThrough for more details.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern BOOL TRACE_HasFallThrough(TRACE trace);

/*! @ingroup TRACE
  @return Number of BBLs in trace

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern UINT32 TRACE_NumBbl(TRACE trace);

/*! @ingroup TRACE
  @return Number of instructions in trace

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern UINT32 TRACE_NumIns(TRACE trace);

/*! @ingroup BBL
  Insert call relative to a bbl.
  @param bbl @ref BBL to instrument
  @param action Specifies before, after, etc.
  \n @ref IPOINT_BEFORE is always valid for all BBLs.
  \n @ref IPOINT_AFTER is valid only when a fall-through exists. Can only be used if @ref BBL_HasFallThrough is true.
  \n @ref IPOINT_ANYWHERE will put the instrumentation at a place inside the bbl for best performance.
  \n @ref IPOINT_TAKEN_BRANCH is valid only when the BBL ends with a control-flow instruction. Can only be used if
  INS_IsValidForIpointTakenBranch(BBL_ins_tail(bbl)) is true.
  @param funptr Analysis function to call
  @param ... @ref IARG_TYPE.  Arguments to pass to funptr

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID BBL_InsertCall(BBL bbl, IPOINT action, AFUNPTR funptr, ...);

/*! @ingroup BBL

  Insert a call to funptr relative to a BBL. If funptr returns a non-zero
  ADDRINT, then the immediately following "then" analysis call is executed.
  Note that if @ref CALL_ORDER is used, Both "if" and "then" analysis calls must have the same order.

  @param bbl BBL to instrument
  @param action Specifies when the analysis call executes.  See the documentation in @ref BBL_InsertCall().
  @param funptr Analysis function to call. Its return type must be ADDRINT
  @param ... @ref IARG_TYPE.  Arguments to pass to funptr

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID BBL_InsertIfCall(BBL bbl, IPOINT action, AFUNPTR funptr, ...);

/*! @ingroup BBL

  Insert a call to funptr relative to a BBL. The function is
  called only if the immediately preceding "if" analysis call returns a
  non-zero value.
  Note that if @ref CALL_ORDER is used, Both "if" and "then" analysis calls must have the same order.

  @param bbl BBL to instrument
  @param action Specifies when the analysis call executes.  See the documentation in @ref BBL_InsertCall().
  @param funptr Analysis function to call
  @param ... @ref IARG_TYPE.  Arguments to pass to funptr

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
 */
extern VOID BBL_InsertThenCall(BBL bbl, IPOINT action, AFUNPTR funptr, ...);

/*! @ingroup BBL
  Refer to @ref INS_HasFallThrough for checking if an instruction could
  have a fall-through path based on the opcode.
  @return TRUE if the BBL has a fall-through bbl within the trace, else returns FALSE.

  @par Availability:
  \b Mode:  JIT & Probe\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern BOOL BBL_HasFallThrough(BBL bbl);

/*! @ingroup CONTEXT
 * Query if Pin supports the specified PROCESSOR_STATE.
 *
 * @param[in]       state   the PROCESSOR_STATE being queried
 * @return   TRUE if the PROCESSOR_STATE specified by state is supported by Pin
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern BOOL PIN_SupportsProcessorState(PROCESSOR_STATE state);

/*! @ingroup CONTEXT
 * Query if the specified CONTEXT contains the specified PROCESSOR_STATE.
 *
 * @param[in]       ctxt   context to be examined
 * @param[in]       state  the PROCESSOR_STATE being queried
 * @return   TRUE if the PROCESSOR_STATE specified by state is contained in the ctxt
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern BOOL PIN_ContextContainsState(CONTEXT* ctxt, PROCESSOR_STATE state);

/*! @ingroup CONTEXT
 * Set the given value for the requested register in the context. This function is applicable
 * for all context registers (integer, fp etc.).
 * When this function is used from within a callback, e.g. THREAD_START_CALLBACK, the new context
 * will take effect when the application continues. If this function is called from within an analysis
 * routine, in order for the new context to take effect when the application continues one must do one
 * of the following: Either use @ref IARG_PARTIAL_CONTEXT and specify the registers which should be
 * modified in the "out" REGSET. Or, use the IARG_CONTEXT and call @ref PIN_ExecuteAt (don't forget to
 * set REG_INST_PTR appropriately).
 *
 * @param[in,out]   ctxt   context whose register value is to be set
 * @param[in]       reg    register whose value is to be set
 * @param[in]       val    buffer holding the new value of the register, see note below
 *
 * @note It is the user's responsibility to allocate and free the val buffer. The buffer size should be
 *       at least as wide as the register. The register size can be obtained using @ref REG_Size.\n
 *       It is recommended to use a @ref PIN_REGISTER object as the buffer (and pass a pointer to it).
 *       This will ensure that the buffer is large enough for any architectural register.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID PIN_SetContextRegval(CONTEXT* ctxt, REG reg, const UINT8* val);

/*! @ingroup CONTEXT
 * Get the value of the requested register from the context. This function is applicable
 * for all context registers (integer, fp etc).
 *
 * @param[in]       ctxt   context whose register value is requested
 * @param[in]       reg    register whose value is requested
 * @param[out]      val    an empty buffer which will hold the register value, see note below
 *
 * @note It is the user's responsibility to allocate and free the val buffer. The buffer size should be
 *       at least as wide as the register. The register size can be obtained using @ref REG_Size.\n
 *       It is recommended to use a @ref PIN_REGISTER object as the buffer (and pass a pointer to it).
 *       This will ensure that the buffer is large enough for any architectural register.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID PIN_GetContextRegval(const CONTEXT* ctxt, REG reg, UINT8* val);

/*! @ingroup CONTEXT
 * Set the given value for the integer register or fp status/control register in the specified context.
 * Note that fp registers such as fp-stack regs, xmm regs, ymm regs (when running with Intel(R) AVX support), can
 * only be set using @ref PIN_SetContextRegval.
 *
 * @param[in,out]   ctxt   context whose register value is to be set
 * @param[in]       reg    register whose value is to be set. This could be an integer register
 *                         or a floating point register with integer values, like FP status/control register.
 * @param[in]       val    new value of the register
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID PIN_SetContextReg(CONTEXT* ctxt, REG reg, ADDRINT val);

/*! @ingroup CONTEXT
 * Get the value of the integer register or fp status/control register in the specified context.
 * Note that fp registers such as fp-stack regs, xmm regs, ymm regs (when running with Intel(R) AVX support), can
 * only be read using @ref PIN_GetContextRegval.
 *
 * @param[in]       ctxt   context whose register value is requested
 * @param[in]       reg    register whose value is requested. This could be an integer register
 *                         or a floating point register with integer values, like FP status/control register.
 * @return   value of the register in the specified context
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern ADDRINT PIN_GetContextReg(const CONTEXT* ctxt, REG reg);

/*! @ingroup CONTEXT
 * Set the given floating point state in the specified context.
 *
 * @param[in,out]   ctxt        context whose floating point state is to be set
 * @param[in]       fpstate     new floating point state of the context.
 *                              the FPSTATE type is exposed by Pin.
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID PIN_SetContextFPState(CONTEXT* ctxt, const FPSTATE* fpstate);

/*! @ingroup CONTEXT
 * Get the floating point state of the specified context.
 *
 * @param[in]       ctxt        context whose floating point state is retrieved
 * @param[in]       fpstate     pointer to the buffer that receives floating point
 *                              state of the context.
 *                              the FPSTATE type is exposed by Pin.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID PIN_GetContextFPState(const CONTEXT* ctxt, FPSTATE* fpstate);

/*! @ingroup CONTEXT
 * Get full REGSET.\n
 * When using IARG_PARTIAL_CONTEXT such a regset for both in and out REGSET
 * will provide access to all registers including the ability to update them.
 * Such a usage will provide better performance than using IARG_CONTEXT combined with
 * PIN_ExecuteAt().
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern REGSET PIN_GetFullContextRegsSet();

/*! @ingroup CONTEXT
 * Copy the CONTEXT structure.\n
 * CONTEXT structures supplied by Pin to the instrumentation callbacks are not
 * "byte-copyable". The tool should use this function to copy the context provided
 * by Pin and must not attempt to move it as raw memory bytes.
 *
 * @param[in]    ctxtFrom   CONTEXT structure to copy from
 * @param[out]   ctxtTo     CONTEXT structure to copy into
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID PIN_SaveContext(const CONTEXT* ctxtFrom, CONTEXT* ctxtTo);

/*! @ingroup CONTEXT
 * A tool can call this API to abandon the current analysis function and resume execution
 * of the calling thread at a new application register state.  Note that this API does not
 * return back to the caller's analysis function.
 *
 * This API can be called from an analysis function or a replacement routine, but not from
 * a callback.
 *
 * When this API is called from an analysis function or replacement function, 
 * and if they execute the current routine or instruction being analyzed, 
 * then execution will resume at the instrumented routine or instruction 
 * and the analysis function will be called again.
 * It is the pintool's responsibility to avoid going into an infinite loop of 
 * calls to the analysis function.
 *
 *  @param[in] ctxt     New application register state for the calling thread.
 *
 * @return  This API never returns.
 *
 * @note The vm lock is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID PIN_ExecuteAt(const CONTEXT* ctxt);

/*! @ingroup  SYSCALL
 * Set the given value for the argument of the system call to be executed in the
 * specified context.\n
 * It is a user's responsibility to make sure that the specified context and the
 * current memory content represent the state of a system call before execution.
 * For example, this function can be safely used in the scope of @ref SYSCALL_ENTRY_CALLBACK.
 * Applying this function to an inappropriate context results in undefined behavior
 * and even may cause crash on systems in which system call arguments are located in memory.
 *
 * @param[in,out]   ctxt    context before the system call execution
 * @param[in]       std     system calling standard
 * @param[in]       argNum  ordinal number of the argument whose value is to be set.
 *                          Ordinal numbers start from zero for the first argument
 * @param[in]       val     new value of the argument
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID PIN_SetSyscallArgument(CONTEXT* ctxt, SYSCALL_STANDARD std, UINT32 argNum, ADDRINT val);

/*! @ingroup  SYSCALL
 * Get the value of the argument of the system call to be executed in the specified
 * context.\n
 * It is a user's responsibility to make sure that the specified context and the
 * current memory content represent the state of a system call before execution.
 * For example, this function can be safely used in the scope of @ref SYSCALL_ENTRY_CALLBACK.
 * Applying this function to an inappropriate context results in undefined behavior
 * and even may cause crash on systems in which system call arguments are located in memory.
 *
 * @param[in]       ctxt    context before the system call execution
 * @param[in]       std     system calling standard
 * @param[in]       argNum  ordinal number of the argument whose value is requested.
 *                          Ordinal numbers start from zero for the first argument
 * @return          value of the argument
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern ADDRINT PIN_GetSyscallArgument(const CONTEXT* ctxt, SYSCALL_STANDARD std, UINT32 argNum);

/*! @ingroup SYSCALL
 * Set the number (ID) of the system call to be executed in the specified context.\n
 * It is a user's responsibility to make sure that the specified context represents
 * the state of a system call before execution.
 * For example, this function can be safely used in the scope of @ref SYSCALL_ENTRY_CALLBACK.
 * Applying this function to an inappropriate context results in undefined behavior.
 *
 * @param[in]       ctxt    context before the system call execution
 * @param[in]       std     system calling standard
 * @param[in]       val     new system call number
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID PIN_SetSyscallNumber(CONTEXT* ctxt, SYSCALL_STANDARD std, ADDRINT val);

/*! @ingroup  SYSCALL
 * Get the number (ID) of the system call to be executed in the specified context.\n
 * It is a user's responsibility to make sure that the specified context represents
 * the state of a system call before execution.
 * For example, this function can be safely used in the scope of @ref SYSCALL_ENTRY_CALLBACK, but
 * not in a SYSCALL_EXIT_CALLBACK.
 * Applying this function to an inappropriate context results in undefined behavior.
 *
 * If you need the system call number in a SYSCALL_EXIT_CALLBACK, you
 * should use a SYSCALL_ENTRY_CALLBACK to save the system call number
 * into thread local storage so that you can get it from there in your
 * SYSCALL_EXIT_CALLBACK. Beware, though of interruptable system
 * calls, to handle those you will need to worry about other context
 * changes (see the discussion in @ref PIN_AddSyscallExitFunction ).
 *
 * In Linux systems with kernel 5.3 and above, calling this API while jumping to vsyscall area
 * will return special number - VSYSCALL_NR.
 *
 * @param[in]       ctxt    context before the system call execution
 * @param[in]       std     system calling standard
 * @return          system call number
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern ADDRINT PIN_GetSyscallNumber(const CONTEXT* ctxt, SYSCALL_STANDARD std);

/*! @ingroup SYSCALL
 * Get the return value of the system call which has just returned with the specified
 * context.\n
 * It is a user's responsibility to make sure that the specified context represents
 * the state of a system call after execution.
 * For example, this function can be safely used in the scope of @ref SYSCALL_EXIT_CALLBACK.
 * Applying this function to an inappropriate context results in undefined behavior.
 *
 * @param[in]       ctxt    context after the system call execution
 * @param[in]       std     system calling standard
 * @return          return value of the system call. \n
 *                  On Linux and macOS* the function returns -1 if the system call failed
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern ADDRINT PIN_GetSyscallReturn(const CONTEXT* ctxt, SYSCALL_STANDARD std);

/*! @ingroup SYSCALL
 * Get the error code of the system call which is just returned with the specified
 * context.\n
 * It is a user's responsibility to make sure that the specified context represents
 * the state of a system call after execution.
 * For example, this function can be safely used in the scope of @ref SYSCALL_EXIT_CALLBACK.
 * Applying this function to an inappropriate context results in undefined behavior.
 *
 * @param[in]       ctxt    context after the system call execution
 * @param[in]       std     system calling standard
 * @return          error code, if the system call failed;\n
 *                  zero, if the system call succeeded
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern ADDRINT PIN_GetSyscallErrno(const CONTEXT* ctxt, SYSCALL_STANDARD std);

/*! @ingroup PHYSICAL_CONTEXT
 * Set the given value for the integer register in the specified physical context.
 *
 * @param[in,out]   pPhysCtxt  physical context whose register value is to be set
 * @param[in]       reg        physical register whose value is to be set (up to REG_PHYSICAL_INTEGER_END)
 * @param[in]       val        new value of the register
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID PIN_SetPhysicalContextReg(PHYSICAL_CONTEXT* pPhysCtxt, REG reg, ADDRINT val);

/*! @ingroup PHYSICAL_CONTEXT
 * Get the value of the integer register in the specified physical context.
 *
 * @param[in]       pPhysCtxt  physical context whose register value is requested
 * @param[in]       reg        physical register whose value is requested (up to REG_PHYSICAL_INTEGER_END)
 * @return   value of the register in the specified context
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern ADDRINT PIN_GetPhysicalContextReg(const PHYSICAL_CONTEXT* pPhysCtxt, REG reg);

/*! @ingroup PHYSICAL_CONTEXT
 * Set the given floating point state in the specified physical context.
 *
 * @param[in,out]   pPhysCtxt       physical context whose floating point state is to be set
 * @param[in]       fpstate         new floating point state of the context. It must
 *                                  have fxsave format in IA-32 and Intel(R) 64
 *                                  architectures.
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID PIN_SetPhysicalContextFPState(PHYSICAL_CONTEXT* pPhysCtxt, const VOID* fpstate);

/*! @ingroup PHYSICAL_CONTEXT
 * Get the floating point state of the specified physical context.
 *
 * @param[in]       pPhysCtxt       physical context whose floating point state is retrieved
 * @param[in]       fpstate         pointer to the buffer that receives floating point
 *                                  state of the context. The size of the buffer must
 *                                  be at least FPSTATE_SIZE bytes. The returned state
 *                                  has fxsave format in IA-32 and Intel(R) 64
 *                                  architectures.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID PIN_GetPhysicalContextFPState(const PHYSICAL_CONTEXT* pPhysCtxt, VOID* fpstate);

/*! @ingroup EXCEPTION
 * A tool can call this API to raise an emulated exception in the application's calling
 * thread.  Calling this API abandons the current analysis function and immediately starts
 * executing at the application's exception handler (if there is one) or terminates the
 * application (if there is no handler).  This API does not return back to the calling
 * analysis function.
 *
 * When raising a floating point exception, this method merely raises the exception.  It
 * does not set any bits in the floating point status register, nor does it check that
 * the exception is enabled via the exception mask.  If the tool wishes to examine or
 * change the floating point status register bits, it may do so via the \a ctxt parameter.
 *
 * This function can be called from an analysis function or a replacement routine, but
 * not from an instrumentation callback.
 *
 * @param[in]   ctxt            Register state to be reported as the application's context
 *                              at the exception point.
 * @param[in]   tid             Id of the calling thread.
 * @param[in]   pExceptInfo     Pointer to the @ref EXCEPTION_INFO structure that describes
 *                              the exception. The exception information should be properly
 *                              initialized.
 *
 * @return  This API never returns.
 * @pre     All locks must be released before calling to this function.
 *
 * @note  The vm lock is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID PIN_RaiseException(const CONTEXT* ctxt, THREADID tid, const EXCEPTION_INFO* pExceptInfo);

/*! @ingroup INS_MODIFICATION
 Change this memory access instruction to reference the virtual
 memory location contained in the given register.

 @param[in] ins       input instruction
 @param[in] memopIdx  controls which memory operand to rewrite (0,1,...)
 @param[in] newBase   register containing the base address of the new operand which will
 normally be a scratch register allocated via PIN_ClaimToolRegister()

 On IA-32 and Intel64, the modified operand uses only base register
 addressing with the new base register \a newBase.  Any index, scale,
 or offset fields from that operand in the original instruction are
 removed.  In addition, if the original instruction's operand uses a
 segment override, the instruction is changed to use the default
 segment.

 This function can be used to rewrite memory operands even when they are implicit
 (for instance call, ret, push, pop), though in this case the instruction may
 ultimately be replaced by a sequence of instructions which achieve the same effect.
 (This is transparent to instrumentation, which continues to see the original
 instruction).

 The only instruction which cannot be rewritten is \b enter with a second operand > 0.

 Note that the address in \a newBase is always the lowest address
 which will be accessed by this operand. This is consistent with the
 way in which Pin returns addresses in IARG_*_EA, but means that if
 the operand is modified by the instruction before the memory access occurs
 (for instance a \b push instruction), the value in \a newBase will not be
 the value in the stack pointer, but the address of the memory which is accessed
 by the instruction.

 This can also be confusing for xlat; where the value of \a newBase is
 the address from which data is loaded, not the address of the base of
 the translation table. (Again, this is consistent with the IARG_*_EA
 which Pin will report for an xlat operation).

 Similarly for the bt,btc,btr and bts insructions, if the bit index is
 larger than the operand size (so that parts of the bit index affect
 the EA), they are included in Pin's normal EA calculation. In this
 case, Pin automatically masks the bit index operand so that it only
 includes the index within the addressed unit of memory. This ensures
 that your address manipulation function need only consider the
 translation of the EA, it does not have to worry about additional
 offsets generated by the bit index operand of these
 instructions. (This is equivalent to saying that if you replace all
 memory operands, but use an address computation function that simply
 returns the original EA, the code will continue to execute
 correctly).

 The canonical instrumentation code for memory address rewriting now looks something like this
 @code
 // Map the originalEa to a translated address.
 static ADDRINT ProcessAddress(ADDRINT originalEa, ADDRINT size, UINT32 access);
 ...
    for (UINT32 op = 0; op<INS_MemoryOperandCount(ins); op++)
    {
        UINT32 access = (INS_MemoryOperandIsRead(ins,op)    ? 1 : 0) |
                        (INS_MemoryOperandIsWritten(ins,op) ? 2 : 0);

        INS_InsertCall(ins, IPOINT_BEFORE,
                       AFUNPTR(ProcessAddress),
                       IARG_MEMORYOP_EA,   op,
                       IARG_MEMORYOP_SIZE, op,
                       IARG_UINT32,        access,
                       IARG_RETURN_REGS,   REG_INST_G0+i,
                       IARG_END);

        INS_RewriteMemoryOperand(ins, i, REG(REG_INST_G0+i));
    }
 @endcode
 There is no need to handle any instructions specially.

 @par Availability:
 \b Mode:  JIT\n
 \b O/S:   Linux, Windows & macOS*\n
 \b CPU:   IA-32 and Intel(R) 64 architectures\n
*/
extern VOID INS_RewriteMemoryOperand(INS ins, UINT32 memindex, REG reg);

/*! @ingroup INS_MODIFICATION

  Insert an indirect jump instruction relative to the given instruction.
  When used with INS_Delete it can be used to emulate control transfer
  instructions.

  @param[in] ins       input instruction
  @param[in] ipoint    location relative to ins (only IPOINT_BEFORE and IPOINT_AFTER are supported)
  @param[in] reg       register holding the target

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   IA-32 and Intel(R) 64 architectures\n
*/
extern VOID INS_InsertIndirectJump(INS ins, IPOINT ipoint, REG reg);

/*! @ingroup INS_MODIFICATION

  Insert a direct jump instruction relative to the given instruction
  When used with INS_Delete it can be used to emulate control transfer
  instructions.

  @param[in] ins       input instruction
  @param[in] ipoint    location relative to ins (only IPOINT_BEFORE and IPOINT_AFTER are supported)
  @param[in] tgt       absolute address of the target

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   IA-32 and Intel(R) 64 architectures\n
*/
extern VOID INS_InsertDirectJump(INS ins, IPOINT ipoint, ADDRINT tgt);

/*! @ingroup REG
   Claim a PIN scratch register for use by this tool.

   By using this function to claim scratch registers tools can avoid
   contention if a tool is composed from different components each of
   which uses scratch registers.  Using this function is to be preferred to
   directly using the REG_INST_Gn register enumerations.

   Note that although this function is available in probe mode, use of
   Pin scratch registers is not currently supported in probe mode, so
   this function is useless there.

   @return A register to be used, or REG_INVALID() if
           all of the scratch registers available for tool use have been allocated.

  @par Availability:
  \b Mode:  JIT & Probe\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern REG PIN_ClaimToolRegister();

/*! @ingroup PIN_CONTROL
  Returns full path of Pin binary invoked, encoded in UTF8 (superset of ASCII),
  this is supported for Linux (only for locales encoded in UTF8).

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux & macOS*\n
  \b CPU:   All\n
 */
extern CHAR* PIN_VmFullPath();

/*! @ingroup PIN_CONTROL
  Returns full path of main Pin tool binary image, encoded in UTF8 (superset of ASCII).

  @par Availability:
  \b Mode:  JIT & Probe\n
  \b O/S:   Windows, Linux\n
  \b CPU:   All\n
 */
extern const CHAR* PIN_ToolFullPath();

/*! @ingroup PIN_PROCESS
 * Terminate the current process immediately, without calling any
 * thread or process fini callbacks that may be registered. (See @ref
 * PIN_ExitApplication, if you want fini callbacks to be called).
 *
 * This function should be used only for abnormal termination of the
 * instrumented process. Normally, the process is terminated when Pin
 * executes a process termination system call on behalf of the
 * application.
 *
 * @param[in]  exitCode     exit code of the process to be reported to the operating system.
 * @return  the function never returns.
 *
 * @par Availability:
 * \b Mode:  JIT & Probe\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   All\n
 */
extern NORETURN VOID PIN_ExitProcess(INT32 exitCode);

/*! @ingroup PIN_PROCESS
 * Returns current process id

 @par Availability:
 \b Mode:  JIT & Probe\n
 \b O/S:   Linux, Windows & macOS*\n
 \b CPU:   All\n
*/
extern INT PIN_GetPid();

/*! @ingroup INS_MODIFICATION
  Delete the instruction

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID INS_Delete(INS ins);

/*! @ingroup INST_ARGS
  Add an argument to an IARGLIST.

  @par Availability:
  \b Mode:  JIT & Probe\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID IARGLIST_AddArguments(IARGLIST args, ...);

/*! @ingroup INST_ARGS
  Allocate a container to hold instrumentation arguments. Pass to *_InsertCall by using IARG_LIST

  @par Availability:
  \b Mode:  JIT & Probe\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern IARGLIST IARGLIST_Alloc();

/*! @ingroup INST_ARGS
  Free a container to hold instrumentation arguments. Pass to *_InsertCall by using IARG_LIST

  @par Availability:
  \b Mode:  JIT & Probe\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID IARGLIST_Free(IARGLIST args);

/*! @ingroup PIN_CONTROL
 * Copy the specified number of bytes from a source memory region to a destination memory region.
 * The function guarantees safe return to the caller even if the source or destination
 * regions are inaccessible (entirely or partially).
 *
 * Tools should use this function to ensure safe access to the original content of the
 * application's memory. For example, on Windows, Pin replaces certain TEB fields when running
 * analysis routines in the tool. If the tool accesses these fields directly, it would see the
 * values assigned by Pin rather than the original ones. On the contrary, PIN_SafeCopy() always
 * reads and modifies the original application's values of these fields.
 *
 * This function can be used in any thread, including any internal thread spawned by the tool.
 *
 * @param[out] dst      destination region
 * @param[in]  src      region to copy from
 * @param[in]  size     number of bytes to copy
 *
 * @return  Number of bytes successfully copied from the source to the destination region.
 *
 * @note PIN_SafeCopy() should not be called before the application has been started.  For instance,
 * it cannot be called in an Image load callback. Unexpected results will occur.
 *
  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows\n
  \b CPU:   All\n
 */
extern size_t PIN_SafeCopy(VOID* dst, const VOID* src, size_t size);

/*! @ingroup PIN_CONTROL
 * Copy the specified number of bytes from a source memory region to a destination memory region.
 * The function guarantees safe return to the caller even if the source or destination
 * regions are inaccessible (entirely or partially).
 *
 * In addition to the @ref PIN_SafeCopy functionality, this function allows the tool to obtain
 * detailed exception information in case of failure.
 *
 * This function can be used in any thread, including any internal thread spawned by the tool.
 *
 * @param[out] dst          destination region
 * @param[in]  src          region to copy from
 * @param[in]  size         number of bytes to copy
 * @param[out] pExceptInfo  pointer to the structure that receives the exception information
 *                          in case of failure.
 *
 * @return  Number of bytes successfully copied from the source to the destination region.
 *          If the function fails to copy all bytes, it provides detailed description of
 *          the failure in the \a <pExceptInfo> structure.
 *
 * @note The exception address in the returned exception information always has zero value.
 *       If the tool wants to raise this exception on behalf of the application, it should use
 *       @ref PIN_SetExceptionAddress function to set an appropriate exception address
 *       before passing the exception information to the @ref PIN_RaiseException function.
 *
 * @note PIN_SafeCopyEx() should not be called before the application has been started.  For instance,
 * it cannot be called in an Image load callback. Unexpected results will occur.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows\n
 * \b CPU:   All\n
 */
extern size_t PIN_SafeCopyEx(VOID* dst, const VOID* src, size_t size, EXCEPTION_INFO* pExceptInfo);

/*! @ingroup INS_INSTRUMENTATION
 *
 * Insert analysis code to fill one record in a trace buffer whenever an application instruction executes.
 *
 * @param[in] ins           The application instruction.
 * @param[in] action        Tells whether the record is filled before or after the instruction.
 * @param[in] id            The ID of the buffer whose record is filled.
 * @param[in] ...           @ref IARG_TYPE.  Additional arguments to specify the fields of the trace buffer.
 *                          These additional arguments take the form:
 *                             IARG_TYPE arg, [optional IARG parameters], size_t offset, ..., IARG_END
 *                          The \i arg argument specifies the value to write to the trace record field.
 *                          The \i offset argument specifies the offset (in bytes) from the start of the
 *                          trace record to this field.  Typically, you would use "offsetof()" for this.
 *                          if \i arg requires additional parameters, they come before \i offset.
 *
 * Certain IARG_TYPEs cannot be  used with the *_InsertFillBuffer APIs.
 * The unsupported IARG_TYPEs are: IARG_CONTEXT, IARG_REG_REFERENCE,
 * IARG_REG_CONST_REFERENCE and IARG_MULTI_ELEMENT_OPERAND.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux & Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID INS_InsertFillBuffer(INS ins, IPOINT action, BUFFER_ID id, ...);

/*! @ingroup INS_INSTRUMENTATION
 *
 * Insert analysis code to fill one record in a trace buffer whenever an application
 * instruction executes, based on that instruction's predicate.
 *
 * @param[in] ins           The application instruction
 * @param[in] action        Whether the record is filled before or  after the instruction
 * @param[in] id            The ID of the buffer whose record is filled
 * @param[in] ...           @ref IARG_TYPE.  Additional arguments to specify the fields of the trace buffer.
 *
 * Certain IARG_TYPEs cannot be  used with the *_InsertFillBuffer APIs.
 * The unsupported IARG_TYPEs are: IARG_CONTEXT, IARG_REG_REFERENCE,
 * IARG_REG_CONST_REFERENCE and IARG_MULTI_ELEMENT_OPERAND.
 *
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux & Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID INS_InsertFillBufferPredicated(INS ins, IPOINT action, BUFFER_ID id, ...);

/*! @ingroup INS_INSTRUMENTATION
 *
 * Insert analysis code to fill one record in a trace buffer whenever an application
 * instruction executes.  The record is only inserted if the preceding "if" analysis
 * call returns a non-zero value.
 *
 * @param[in] ins           The application instruction
 * @param[in] action        Whether the record is filled before or after the instruction
 * @param[in] id            The ID of the buffer whose record is to filled
 * @param[in] ...           @ref IARG_TYPE.  Additional arguments to specify the fields of the trace buffer.
 *
 * Certain IARG_TYPEs cannot be  used with the *_InsertFillBuffer APIs.
 * The unsupported IARG_TYPEs are: IARG_CONTEXT, IARG_REG_REFERENCE,
 * IARG_REG_CONST_REFERENCE and IARG_MULTI_ELEMENT_OPERAND.
 *
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux & Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID INS_InsertFillBufferThen(INS ins, IPOINT action, BUFFER_ID id, ...);

/*! @ingroup BUFFER
 *
 * Define a trace buffer to use with the Pin trace buffer API.  This function defines
 * the shape of the buffer, but doesn't allocate the buffer itself.  Each thread
 * implicitly creates its first buffer on start-up.  Additional buffers may then be created
 * using \ref PIN_AllocateBuffer, but this is only needed by tools using "double buffering".
 *
 * Upon exit, the notification callback may be called on a different physical thread than
 * the one that is exiting.
 *
 * @param[in] recordSize    Size (bytes) of each record in the buffer.  This size must be
 *                          less than the size of an OS page.
 * @param[in] numPages      The number of OS pages to allocate for each buffer.  This size
 *                          does not have to be an even multiple of \a recordSize.
 * @param[in] fun           A call-back function that is called whenever the buffer is full,
 *                          or when the thread exits with a partially-full buffer.
 *                          Note that when called for a full buffer, not during thread exit,
 *                          this function is called WITHOUT holding any Pin locks. So that
 *                          multiple threads may be executing the function simultaneously.
 *                          It is the tool's responsibility to take care of the multi-thread safety
 *                          of this function, and any functions called by it.
 * @param[in] val           Passed as the last argument to \a fun.
 *
 * @return On success, a BUFFER_ID.  On error (e.g., maximum number of trace buffers exceeded,)
 *         returns BUFFER_ID_INVALID.
 *
 * @note The vm and pin client locks are obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux & Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern BUFFER_ID PIN_DefineTraceBuffer(size_t recordSize, UINT32 numPages, TRACE_BUFFER_CALLBACK fun, VOID* val);

/*! @ingroup BUFFER
 *
 * Explicitly allocate a trace buffer.  This is only needed for tools which use a "double buffering"
 * technique.  When used, the buffer pointer should be returned from the TRACE_BUFFER_CALLBACK call-back.
 *
 * @param[in] id             The ID of the trace buffer to allocate.
 *
 * @return A pointer to the new buffer.]
 *
 * @note  The pin client lock is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux & Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID* PIN_AllocateBuffer(BUFFER_ID id);

/*! @ingroup BUFFER
 *
 * Explicitly deallocate a trace buffer.  This is only needed by tools using a "double buffering" technique,
 * where it is used to deallocate buffers allocated via \ref PIN_AllocateBuffer().  However, it may be
 * safely called (with no effect) for a thread's implicit initial buffer.
 *
 * @param[in] id             The ID of the trace buffer.
 * @param[in] buf            Pointer to the start of the buffer.
 *
 * @note  The pin client lock is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux & Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 *
 */
extern VOID PIN_DeallocateBuffer(BUFFER_ID id, VOID* buf);

/*! @ingroup PIN_CONTROL
 * This function is useful when an analysis routine might block for an indefinite period of time.
 * In such cases, the analysis code can periodically call PIN_IsActionPending() to see if the
 * thread has some pending action that needs to be handled in the Pin VM.  If so, the analysis
 * routine should return back to the VM, either by returning or by calling PIN_ExecuteAt().
 *
 *  @param[in] tid  The ID of the calling thread.
 *
 * @return  TRUE if the thread has a pending action in the Pin VM.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern BOOL PIN_IsActionPending(THREADID tid);

/*! @ingroup PIN_CONTROL
 * This function is useful when the tool that use attach is doing some initial work
 * that needs the number of threads at the time of the attach.
 *
 * @return  Number of threads at the time of the attach, in all other cases
 *          it will return one. It is not implemented for Windows with attach.
 *
 * @par Availability:
 * \b Mode:  JIT/Probe\n
 * \b O/S:   Linux\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern UINT32 PIN_GetInitialThreadCount();

/*! @ingroup BUFFER
 *
 * Returns the address of the current position in the buffer. Needs a
 * CONTEXT that was passed in as a call back argument or IARG_CONTEXT
 *
 * @param[in] id             The ID of the trace buffer.
 * @param[in] ctxt           CONTEXT
 *
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux & Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID* PIN_GetBufferPointer(CONTEXT* const ctxt, BUFFER_ID id);

/*! @ingroup PIN_CONTROL
 * Given an address, this API checks whether the memory page which contains this address has a read access protection.
 *
 * @param[in]  addr     memory address
 *
 * @return     True if the memory page which contains the given address has a read access protection, false otherwise.
 *
 * @note PIN_CheckReadAccess() should not be called before the application has been started. For instance,
 * it cannot be called in an Image load callback. Unexpected results will occur.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern BOOL PIN_CheckReadAccess(VOID* addr);

/*! @ingroup PIN_CONTROL
 * Given an address, this API checks whether the memory page which contains this address has a write access protection.
 *
 * @param[in]  addr     memory address
 *
 * @return     True if the memory page which contains the given address has a write access protection, false otherwise.
 *
 * @note PIN_CheckWriteAccess() should not be called before the application has been started. For instance,
 * it cannot be called in an Image load callback. Unexpected results will occur.
 *
 * @note PIN_CheckWriteAccess assume that if a memory page has a write access protection it also has a read
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern BOOL PIN_CheckWriteAccess(VOID* addr);

/*! @ingroup PIN_CONTROL
 * Given an aux vector entry type, returns the entry's value.
 *
 * @param[in]  type    The type of the desired auxv entry.
 * @param[out] found   True if the entry was found, false otherwise.
 *
 * @return     The value of the aux vector entry, undefined if .
 *
 * @par Availability:
 * \b Mode:  JIT/Probe\n
 * \b O/S:   Linux
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern ADDRINT PIN_GetAuxVectorValue(ADDRINT type, bool* found);

#endif // PIN_G_PIN_CLIENT_PH
