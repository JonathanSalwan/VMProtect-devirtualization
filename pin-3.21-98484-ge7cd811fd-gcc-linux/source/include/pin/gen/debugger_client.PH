/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_DEBUGGER_CLIENT_PH
#error duplicate inclusion of debugger_client
#else
#define PIN_G_DEBUGGER_CLIENT_PH
/*! @file
 *
 * Pin client functions for the application-level debugger.
 */
/*! @ingroup APPDEBUG
 * Call-back function to handle commands from a debugger.
 *
 * The format of the \a cmd and \a reply strings may vary depending on which debugger is connected to
 * Pin.  Tools can call PIN_GetDebuggerType() to tell which debugger is connected.
 *
 * When the debugger is GDB, \a cmd is the text string that the user types after the "monitor" command
 * and \a reply is displayed verbatim as a response to the command.
 *
 * Pin reserves all command strings that start with the prefix "pin " or "pin:".  Pin interprets these
 * commands itself and does not pass them on to the tool.
 *
 *  @param[in] threadIndex  The Pin thread ID of the debugger's "focus" thread.
 *  @param[in] ctxt         Application register state of the debugger's "focus" thread.  The interpreter can change
 *                           this state if it handles \a cmd.  When the debugger resumes this thread, it will
 *                           use the new register state in \a ctxt.
 *  @param[in] cmd          The debugger command.
 *  @param[out] reply       Receives the reply to the command, if the interpreter understands \a cmd.
 *  @param[in] v            The tool's call-back value.
 *
 * @return  TRUE if this interpreter function understands \a cmd.  FALSE if it does not.
 *           If FALSE is returned, Pin will call the next registered interpreter to see
 *           if it understands \a cmd.
 */
typedef BOOL (*DEBUG_INTERPRETER_CALLBACK)(THREADID threadIndex, CONTEXT* ctxt, const std::string& cmd, std::string* reply,
                                           VOID* v);

/*! @ingroup APPDEBUG
 * Call-back function to handle breakpoint set requests from a debugger.
 *
 * Once a callback accepted control over a breakpoint at the given address, Pin and PinADX will not stop at that
 * address. The stop responsibility is handled solely by the tool.
 *
 * However, PinADX does keep track of the breakpoint, in case the tool wants to "reset" the breakpoint and give the
 * control back to PinADX.
 *
 *  @param[in] addr         The address of the requested breakpoint.
 *  @param[in] size         The size of the breakpoint (HW/SW)
 *  @param[in] insert       Whether if this is a breakpoint insertion or deletion.
 *  @param[in] v            The tool's call-back value.
 *
 * @return  TRUE if this callback function takes control over the breakpoint.  FALSE if it does not.
 *           If FALSE is returned, Pin will call the next registered interpreter to see
 *           if it understands \a cmd.
 */
typedef BOOL (*DEBUG_BREAKPOINT_CALLBACK)(ADDRINT addr, UINT size, BOOL insert, VOID* v);

/*! @ingroup APPDEBUG
 * Call-back function when the tool intercepts a debugging event with PIN_InterceptDebuggingEvent().
 *
 *  @param[in] tid          The Pin thread ID of the thread that received the debugging event.
 *  @param[in] eventType    Tells the debugging event.
 *  @param[in,out] ctxt     On input, gives the register state at the point the thread received
 *                           the event. The tool may change \a ctxt.  If the event is passed on to
 *                           the debugger, the debugger sees the modified register state.  If the
 *                           event is not passed on to the debugger, the thread resumes execution
 *                           at the new register state.
 *  @param[in] arg          The tool's call-back value.
 *
 * @return  Returning TRUE tells Pin to pass the debugging event on to the debugger.  Returning
 *           FALSE tells Pin to squash the event and the thread resumes without stopping in the
 *           debugger.
 *
 * The following scenarios are not allowed:
 *
 *  - The call-back may not return FALSE for DEBUGGING_EVENT_ASYNC_BREAK.
 *  - If the call-back returns TRUE for DEBUGGING_EVENT_BREAKPOINT or DEBUGGING_EVENT_SINGLE_STEP,
 *    it may not change the value of REG_INST_PC in \a ctxt.  This restriction exists because debuggers
 *    typically make assumptions on the PC value when these events trigger.
 */
typedef BOOL (*INTERCEPT_DEBUGGING_EVENT_CALLBACK)(THREADID tid, DEBUGGING_EVENT eventType, CONTEXT* ctxt, VOID* arg);

/*! @ingroup APPDEBUG
 * Call-back function that Pin calls to get the value of a register emulated by
 * the tool.
 *
 *  @param[in] toolRegId    Identifies the emulated register (from the \e registerDescriptions
 *                           parameter to PIN_AddDebuggerRegisterEmulator()).
 *  @param[in] tid          Identifies the thread whose register is read.
 *  @param[in] ctxt         Architected register state for the thread.
 *  @param[out] data        Points to a buffer that receives the value of the register.  The
 *                           value must be stored in little-endian format (least significant bytes
 *                           first).  If the register size is not an even multiple of bytes, the
 *                           upper bits of the last byte (most significant byte) are unused.  If
 *                           the register size is 2, 4, or 8 bytes, \a data is naturally aligned,
 *                           so it may be cast to a pointer of the appropriate type.
 *  @param[in] v            The tool's call-back value.
 */
typedef VOID (*GET_EMULATED_REGISTER_CALLBACK)(unsigned toolRegId, THREADID tid, CONTEXT* ctxt, VOID* data, VOID* v);

/*! @ingroup APPDEBUG
 * Call-back function that Pin calls to set the value of a register emulated by
 * the tool.
 *
 *  @param[in] toolRegId    Identifies the emulated register (from the \e registerDescriptions
 *                           parameter to PIN_AddDebuggerRegisterEmulator()).
 *  @param[in] tid          Identifies the thread whose register is written.
 *  @param[in] ctxt         Architected register state for the thread.
 *  @param[in] data         Points to the new value for the register.  The value is stored in
 *                           the same format as described in GET_EMULATED_REGISTER_CALLBACK.
 *  @param[in] v            The tool's call-back value.
 */
typedef VOID (*SET_EMULATED_REGISTER_CALLBACK)(unsigned toolRegId, THREADID tid, CONTEXT* ctxt, const VOID* data, VOID* v);

/*! @ingroup APPDEBUG
 * Call-back function that Pin calls to get the content of a document that describes an
 * emulated target processor to an application-level debugger.  This is useful, for example, when
 * a Pin tool emulates additional registers beyond those defined by the native CPU.  A debugger
 * can use this document to understand the emulated registers and how to display them.
 *
 * The format of the returned document varies depending on the debugger that is connected to
 * Pin.  Tools can use PIN_GetDebuggerType() to tell which debugger is connected.
 *
 * When used with GDB, the description should be an XML "target feature" document, as described
 * in the GDB user manual, "Debugging With GDB".  See the appendix titled "Target Descriptions"
 * for details of the XML document format.  GDB starts by asking for a document titled
 * "target.xml".  However, this document may reference other documents via "include" statements.
 * If so, GDB will ask for those additional documents by their names.
 *
 *  @param[in] name     The name of the requested document.
 *  @param[in] size     Size (bytes) of the \a buf buffer.
 *  @param[out] buf     Points to a buffer that receives the content of the document.  If the
 *                       document requires more than \a size bytes, the tool need not write
 *                       anything into \a buf.  Instead, the tool should return the required size.
 *  @param[in] v        The tool's call-back value.
 *
 * @return  If the tool knows how to provide the document named \a name, it returns the size
 *           (bytes) of that document.  If that size is less than or equal to \a size, the tool
 *           should also write the content of the document to \a buf.  If the tool does not know
 *           how to provide this document, it should return zero.
 */
typedef USIZE (*GET_TARGET_DESCRIPTION_CALLBACK)(const std::string& name, USIZE size, VOID* buf, VOID* v);

/*! @ingroup APPDEBUG
 * Establish an interceptor function for debugging events that Pin sends to an external
 * debugger. This API allows a tool to filter these events before they are visible to
 * the debugger.
 *
 * A tool can set only one "intercept" function for a particular event, so a new
 * function overwrites any previous one for the same event. To disable an interceptor,
 * pass a NULL function pointer.
 *
 *  @param[in] eventType    Tells the type of events to intercept.
 *  @param[in] fun          The tool's interceptor function, or NULL.
 *  @param[in] val          Value to pass to the interceptor function.
 *
 * @note The pin client lock is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID PIN_InterceptDebuggingEvent(DEBUGGING_EVENT eventType, INTERCEPT_DEBUGGING_EVENT_CALLBACK fun, VOID* arg);

/*! @ingroup APPDEBUG
 * This API is useful for Pin tools that emulate registers that do not exist in the host
 * machine's ISA.  If an application debugger is attached to Pin, this API informs
 * the debugger about the extended registers, allowing users to display and manipulate the
 * emulated registers as though they were native registers.  Of course, not all debuggers
 * have this capability.  Calling this API for such a non-conforming debugger is legal,
 * but ineffective.  A non-conforming debugger will ignore the emulated registers and just
 * display the native ones.
 *
 * When using this API, the set of registers presented to the debugger is specified in
 * two redundant ways, and it is the tool's responsibility to ensure that they are
 * consistent.  One specification is a text document that the debugger reads.  This
 * document can have any format that the tool and the debugger agree upon, and could
 * convey information about the registers, how they should be displayed in the debugger,
 * etc.  The tool provides this document via the \a getDescriptionFun call-back.  The
 * second register specification is through the \a registerDescriptions parameter, which
 * Pin uses when communicating the register values to the debugger.
 *
 * If this API is called, it must be called before calling PIN_StartProgram().  Also,
 * a tool can install only one debugger register emulator, so a call to
 * PIN_AddDebuggerRegisterEmulator() will overwrite any previous call.
 *
 * When used with GDB, this API is effective only for versions of GDB that support
 * register extensions in the XML "feature document".  This includes GDB versions 7.2
 * and later, as well as some distributions of earlier GDB versions.
 *
 *  @param[in] numRegisters         The number of entries in \a registerDescriptions.
 *  @param[in] registerDescriptions An array describing each register that the debugger will
 *                                   know about.  This includes both native registers and
 *                                   emulated registers.
 *  @param[in] getFun               Call-back function that Pin calls to get the value of an
 *                                   emulated register.
 *  @param[in] setFun               Call-back function that Pin calls to set the value of an
 *                                   emulated register.
 *  @param[in] getDescriptionFun    Call-back function that Pin calls to get the content of
 *                                   a text document that tells the debugger about the registers
 *                                   defined in \a registerDescriptions.
 *  @param[in] val                  Value passed to the call-back functions.
 *
 * @note The pin client lock is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID PIN_AddDebuggerRegisterEmulator(unsigned numRegisters, const DEBUGGER_REG_DESCRIPTION* registerDescriptions,
                                            GET_EMULATED_REGISTER_CALLBACK getFun, SET_EMULATED_REGISTER_CALLBACK setFun,
                                            GET_TARGET_DESCRIPTION_CALLBACK getDescriptionFun, VOID* val);

/*! @ingroup APPDEBUG
 * Register a handler that can interpret commands sent from an application debugger.  This API allows
 * a tool to extend the normal set of commands understood by a debugger that is connected to Pin.
 *
 * A tool may install more than one interpreter function.  Pin calls each one until it reaches an
 * interpreter that understand the command.
 *
 *  @param[in] fun  The debug interpreter function.
 *  @param[in] val  Value to pass to the interpreter function.
 *
 * @note The pin client lock is obtained during the call of this API.
 *
 * @return PIN_CALLBACK A handle to a callback that can be used to further modify this callback's properties
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern PIN_CALLBACK PIN_AddDebugInterpreter(DEBUG_INTERPRETER_CALLBACK fun, VOID* val);

/*! @ingroup APPDEBUG
 * Remove a previously installed debug interpreter function.
 *
 *  @param[in] fun  The interpreter function to remove.
 *
 * @note The pin client lock is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID PIN_RemoveDebugInterpreter(DEBUG_INTERPRETER_CALLBACK fun);

/*! @ingroup APPDEBUG
 * Register a handler that can intercept breakpoint set/delete commands sent from an application debugger.
 * This API allows a tool to take control over specific breakpoints stop behavior.
 *
 * A tool may install more than one handler function.  Pin calls each one until it reaches an
 * handler that understand the command.
 *
 *  @param[in] fun  The breakpoint handler function.
 *  @param[in] val  Value to pass to the handler function.
 *
 * @note The pin client lock is obtained during the call of this API.
 *
 * @return PIN_CALLBACK A handle to a callback that can be used to further modify this callback's properties
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows, macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern PIN_CALLBACK PIN_AddBreakpointHandler(DEBUG_BREAKPOINT_CALLBACK fun, VOID* val);

/*! @ingroup APPDEBUG
 * Remove a previously installed breakpoint handler function.
 *
 *  @param[in] fun  The breakpoint handler to remove.
 *
 * @note The pin client lock is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows, macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID PIN_RemoveBreakpointHandler(DEBUG_BREAKPOINT_CALLBACK fun);

/*! @ingroup APPDEBUG
 * Resets the breakpoint address, and returns the control back to PinADX.
 *
 *  @param[in] addr  The breakpoint address.
 *
 * @note The pin client lock is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows, macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID PIN_ResetBreakpointAt(ADDRINT addr);

/*! @ingroup APPDEBUG
 * A tool can call this API to stop execution in an application debugger as though a
 * breakpoint was hit.  The \a ctxt parameter tells the register state that the debugger
 * sees when the application stops.  If application level debugging is not enabled in this
 * Pin session, execution does not stop, but resumes immediately at \a ctxt.  Tools can
 * tell if application level debugging is enabled by calling PIN_GetDebugStatus().
 *
 * The semantics of this API are very similar to PIN_ExecuteAt().  Both APIs abandon the
 * current analysis function and resume execution at a new CONTEXT.  The only difference
 * is that PIN_ApplicationBreakpoint() also stops at a breakpoint in the application
 * debugger.
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
 * The expected format of the \a msg string may depend on which debugger is connected to Pin.
 * Tools can call PIN_GetDebuggerType() to find the debugger type.
 *
 * When used with GDB, the \a msg string is displayed verbatim to the user when the debugger
 * stops.  The debugger adds a newline to the end of the string before displaying it.
 *
 *  @param[in] ctxt                 The register state that is reported to the debugger.  When the
 *                                   debugger resumes this thread, it resumes execution at this
 *                                   register state (unless the debugger changes the register state).
 *  @param[in] tid                  The ID of the calling thread.
 *  @param[in] waitIfNoDebugger     If \a waitIfNoDebugger is TRUE and the status is DEBUG_STATUS_UNCONNECTED,
 *                                   PIN_ApplicationBreakpoint() blocks until a debugger connects.  Tools
 *                                   can call PIN_GetDebugStatus() to get the status.  If \a waitIfNoDebugger
 *                                   is FALSE or if the status is DEBUG_STATUS_DISABLED or DEBUG_STATUS_UNCONNECTABLE,
 *                                   PIN_ApplicationBreakpoint() resumes immediately at the new context when no
 *                                   debugger is connected.
 *  @param[in] msg                  Tells the reason why the breakpoint was triggered.
 *
 * @return  This API never returns.
 *
 * @note The vm lock is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern VOID PIN_ApplicationBreakpoint(const CONTEXT* ctxt, THREADID tid, BOOL waitIfNoDebugger, const std::string& msg);

/*! @ingroup APPDEBUG
 * Set whether application debugging is enabled or disabled in this Pin
 * session and set the debugging mode if debugging is enabled.  This API
 * overrides the following knobs if they are specified on the command line:
 *
 *  - @ref SWITCH_APPDEBUG "-appdebug"
 *  - @ref SWITCH_APPDEBUG_ENABLE "-appdebug_enable"
 *  - @ref SWITCH_APPDEBUG_SILENT "-appdebug_silent"
 *  - @ref SWITCH_APPDEBUG_ALLOW_REMOTE "-appdebug_allow_remote"
 *  - @ref SWITCH_APPDEBUG_CONNECTION "-appdebug_connection"
 *  - @ref SWITCH_APPDEBUG_EXCLUDE "-appdebug_exclude"
 *
 * If the tool calls this API, it must be called before PIN_StartProgram().
 *
 *  @param[in] mode     Tells whether application debugging is enabled and
 *                       specifies the mode.  If mode->_tcpClient->_ip is set,
 *                       this method makes a copy of the string.
 *
 * @return  TRUE on success, FALSE on failure or if called after PIN_StartProgram().
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern BOOL PIN_SetDebugMode(const DEBUG_MODE* mode);

/*! @ingroup APPDEBUG
 * This function tells whether application level debugging is enabled in this Pin session.  If so,
 * it tells whether an application debugger is currently connected to Pin.
 *
 * @return  A code telling the status of application level debugging.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern DEBUG_STATUS PIN_GetDebugStatus();

/*! @ingroup APPDEBUG
 * This function retrieves the information that an application level debugger will need in order
 * to connect to this Pin session.
 *
 *  @param[out] info    Receives the connection information.
 *
 * @return  TRUE if application level debugging is enabled for this Pin session.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern BOOL PIN_GetDebugConnectionInfo(DEBUG_CONNECTION_INFO* info);

/*! @ingroup APPDEBUG
 * This function tells the type of application level debugger (if any) that is connected to
 * Pin.  If no debugger is connected, returns DEBUGGER_TYPE_UNKNOWN.
 *
 * @return  The type of the application level debugger that is connected to Pin.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern DEBUGGER_TYPE PIN_GetDebuggerType();

/*! @ingroup APPDEBUG
 * Waits for an application level debugger to connect to this Pin session.  This function
 * may only be called after PIN_StartProgram().  If the debugger status is DEBUG_STATUS_DISABLED
 * or DEBUG_STATUS_UNCONNECTABLE, it returns FALSE immediately.
 *
 * After a successful return, an application level debugger is connected to Pin.  The debugger
 * will stop the application soon, but there is no guarantee that this will happen immediately
 * after this API returns.  If the tool wants to guarantee an immediate stop, it should call
 * PIN_ApplicationBreakpoint().
 *
 *  @param[in] timeout  A timeout value (milliseconds).  This function returns (with FALSE) if
 *                       a debugger has not connected by the end of the timeout period.  A
 *                       \a timeout value of zero means wait forever.
 *
 * @return  TRUE if an application level debugger is connected.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern BOOL PIN_WaitForDebuggerToConnect(unsigned timeout);

/*! @ingroup APPDEBUG
 * Tells whether a stopped thread has called PIN_ApplicationBreakpoint(), but the breakpoint has
 * NOT yet been reported to the debugger.  For example, this can occur if two threads call
 * PIN_ApplicationBreakpoint() simultaneously and the debugger has asked Pin to report one
 * debugger event at a time.  In this case, Pin reports one breakpoint to the debugger and
 * leaves the other breakpoint pending.
 *
 *  @param[in] tid      Pin ID of a stopped thread.
 *  @param[out] msg     If there is a pending breakpoint and if \a msg is not NULL,
 *                       \a msg receives the breakpoint message.
 *
 * @return  TRUE if thread \a tid is stopped and has a pending breakpoint from
 *           PIN_ApplicationBreakpoint().
 */
extern BOOL PIN_GetStoppedThreadPendingToolBreakpoint(THREADID tid, std::string* msg);

/*! @ingroup APPDEBUG
 * If the given stopped thread has a pending tool breakpoint, this function can change
 * the message associated with that breakpoint request or it can squash the breakpoint
 * request entirely.  The debugger will see the effect of the changed breakpoint after
 * it resumes execution of the thread.  If the tool changes the breakpoint message, the
 * debugger will receive the breakpoint event with the new message.  If the tool squashes
 * the breakpoint request, the thread will not stop at the breakpoint at all.  Instead,
 * it continues executing at the \i ctxt parameter that was passed to PIN_ApplicationBreakpoint().
 *
 *  @param[in] tid      Pin ID of a stopped thread.
 *  @param[in] squash   If TRUE, the breakpoint request is squashed.  The \a msg parameter
 *                       is ignored in this case.
 *  @param[in] msg      The new breakpoint message for this breakpoint request.
 *
 * @return  TRUE if thread \a tid is stopped and has a pending breakpoint from
 *           PIN_ApplicationBreakpoint().
 */
extern BOOL PIN_ChangePendingToolBreakpointOnStoppedThread(THREADID tid, BOOL squash, const std::string& msg);

#endif // PIN_G_DEBUGGER_CLIENT_PH
