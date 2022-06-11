/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_OPIN_CLIENT_PH
#error duplicate inclusion of opin_client
#else
#define PIN_G_OPIN_CLIENT_PH
/*! @file

*/

/*! @ingroup PIN_CONTROL
 * @returns Return TRUE if -probe on command line or if Tool started application calling
 * @ref PIN_StartProgramProbed().\n
 *
 * @note Tool can run in Probe mode without specifying -probe on the command line. This switch is
 * provided as a convenience. In that case, @ref PIN_IsProbeMode() will return FALSE before
 * @ref PIN_StartProgramProbed() was called (Since the default running mode is JIT).
 *
 * @par Availability:
 * \b Mode:  JIT & Probe\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   All\n
 */
extern BOOL PIN_IsProbeMode();

/*! @ingroup PIN_CONTROL
 * @returns TRUE if Pin is being attached to a running application process
 */
extern BOOL PIN_IsAttaching();

/*! @ingroup RTN
  Return TRUE if the given RTN is a candidate for
  function insertion using probes, and FALSE otherwise.  This API should
  be called before attempting to insert a call to an analysis function
  using @ref RTN_InsertCallProbed().
  If you want to replace the given RTN with @ref RTN_ReplaceSignatureProbed()
  or @ref RTN_ReplaceProbed() you should use @ref RTN_IsSafeForProbedReplacement()


  @param     rtn                the application routine to be replaced.
  @return    TRUE if the function can be instrumented, FALSE if it cannot.

  @ref PIN_StartProgramProbed() must be used when using this API.

  @par Availability:
  \b Mode:  Probe\n
  \b O/S:   Linux, Windows\n
  \b CPU:   All\n
*/
extern BOOL RTN_IsSafeForProbedInsertion(RTN rtn);

/*! @ingroup RTN
  Return TRUE if the given RTN is a candidate for
  function insertion using probes, and FALSE otherwise.  This API should
  be called before attempting to insert a call to an analysis function
  using @ref RTN_InsertCallProbedEx().
  If you want to replace the given RTN with @ref RTN_ReplaceSignatureProbedEx()
  or @ref RTN_ReplaceProbedEx() you should use @ref RTN_IsSafeForProbedReplacementEx()


  @param     rtn      the application routine to be replaced.
  @param     mode     instrumentation mode, see @ref PROBE_MODE
  @return    TRUE if the function can be instrumented, FALSE if it cannot.

  @ref PIN_StartProgramProbed() must be used when using this API.

  @par Availability:
  \b Mode:  Probe\n
  \b O/S:   Linux, Windows\n
  \b CPU:   All\n
*/
extern BOOL RTN_IsSafeForProbedInsertionEx(RTN rtn, PROBE_MODE mode);

/*! @ingroup RTN
  Return TRUE if the given RTN is a candidate for
  probed function replacement, and FALSE otherwise.  This API should
  be called before attempting to replace a function using
  @ref RTN_ReplaceSignatureProbed() or @ref RTN_ReplaceProbed().
  Note that this routine does not guarantee it is safe to place a probe,
  it merely indicates that certain conditions are not present.

  @param     rtn          the application routine to be replaced.
  @return    TRUE if the function can be replaced, FALSE if it cannot.

  @ref PIN_StartProgramProbed() must be used when using this API.

  @par Availability:
  \b Mode:  Probe\n
  \b O/S:   Linux, Windows\n
  \b CPU:   All\n
*/
extern BOOL RTN_IsSafeForProbedReplacement(RTN rtn);

/*! @ingroup RTN
  Return TRUE if the given RTN is a candidate for
  probed function replacement, and FALSE otherwise.  This API should
  be called before attempting to replace a function using
  @ref RTN_ReplaceSignatureProbedEx().
  Note that this routine does not guarantee it is safe to place a probe,
  it merely indicates that certain conditions are not present.

  @param     rtn          the application routine to be replaced.
  @param     mode         instrumentation mode, see @ref PROBE_MODE
  @return    TRUE if the function can be replaced, FALSE if it cannot.

  @ref PIN_StartProgramProbed() must be used when using this API.

  @par Availability:
  \b Mode:  Probe\n
  \b O/S:   Linux, Windows\n
  \b CPU:   All\n
*/
extern BOOL RTN_IsSafeForProbedReplacementEx(RTN rtn, PROBE_MODE mode);

/*! @ingroup RTN
 Replace a routine in the application (orgRtn) by another function defined in
 the Pintool (replacementFunptr) using probes. The replacement function is not instrumented.
 Replacement functions typically need to call the replaced routines. However, calls to
 RTN_Funptr(orgRtn) will be redirected to replacementFunptr. Replacement functions
 must instead call the returned function pointer, which is a copy of the entry point
 that is not redirected.
 The replacement function signature does not have to be the
 same as the replaced function. In fact while the replaced function may have the CALLINGSTD_REGPARMS
 calling convention, the replacement function calling convention must not be PIN_FAST_ANALYSIS_CALL
 (i.e. the replaced function may have register parameters, the replacement function must not).
 The replacement function arguments must be passed to the replacement function using the Pin IARG_TYPEs,
 in the same way as RTN_InsertCall().
 A prototype of the routine in the application must also be passed in as an argument.
 See @ref PROTO_Allocate for more information.
 @param     orgRtn                the application routine to be replaced.
 @param     replacementFunptr     the replacement function
 @param     ...                   @ref IARG_TYPE.  One IARG_TYPE must be IARG_PROTOTYPE,
 and the list must end with IARG_END.
 @return    a function pointer to the relocated application function entry point.
 This allows the replacement routine to execute the replaced routine.

 @ref PIN_StartProgramProbed() must be used when using this API.

 Use @ref RTN_IsSafeForProbedReplacement() to determine if a function is a
 suitable candidate for probed function replacement.

 Some restrictions apply when using IARG_CONTEXT.  See @ref INST_ARGS for more information.
 IARG_THREAD_ID is not supported.

 @par Availability:
 \b Mode:  Probe\n
 \b O/S:   Linux, Windows\n
 \b CPU:   All\n
 */
extern AFUNPTR RTN_ReplaceSignatureProbed(RTN replacedRtn, AFUNPTR replacementFun, ...);

/*! @ingroup RTN
 Replace a routine in the application (orgRtn) by another function defined in
 the Pintool (replacementFunptr) using probes.

 @param     replacedRtn           the application routine to be replaced.
 @param     mode                  instrumentation mode, see @ref PROBE_MODE
 @param     replacementFun        the replacement function
 @param     ...                   @ref IARG_TYPE.  One IARG_TYPE must be IARG_PROTOTYPE,
 and the list must end with IARG_END.
 @return    a function pointer to the relocated application function entry point.
 This allows the replacement routine to execute the replaced routine.

 Use @ref RTN_IsSafeForProbedReplacementEx() to determine if a function is a
 suitable candidate for probed function replacement.


 @par Availability:
 \b Mode:  Probe\n
 \b O/S:   Linux, Windows\n
 \b CPU:   All\n
 */
extern AFUNPTR RTN_ReplaceSignatureProbedEx(RTN replacedRtn, PROBE_MODE mode, AFUNPTR replacementFun, ...);

/*! @ingroup RTN
  Insert a call to an analysis routine relative to a RTN.

  @param     orgRtn                the application routine to instrument
  @param     action                use IPOINT_BEFORE or IPOINT_AFTER to call
                                   funptr before or after execution.
  @param     funptr                the analysis function
  @param     ...                   @ref IARG_TYPE.  If using IPOINT_AFTER,
                                   one IARG_TYPE must be IARG_PROTOTYPE.
                                   The list must end with IARG_END.

  @ref PIN_StartProgramProbed() must be used when using this API.

  Use @ref RTN_IsSafeForProbedInsertion() to determine if a function is a
  suitable candidate for probed function insertion.

  Some restrictions apply when using IARG_CONTEXT.  See @ref INST_ARGS for more information.
  IARG_THREAD_ID is not supported.

  @par Availability:
  \b Mode:  Probe\n
  \b O/S:   All\n
  \b CPU:   All\n
*/
extern VOID RTN_InsertCallProbed(RTN orgRtn, IPOINT action, AFUNPTR funptr, ...);

/*! @ingroup RTN
  Insert a call to an analysis routine relative to a RTN.

  @param     orgRtn                the application routine to instrument
  @param     action                use IPOINT_BEFORE or IPOINT_AFTER to call
                                   funptr before or after execution.
  @param     mode
  @param     funptr                the analysis function
  @param     ...                   @ref IARG_TYPE.  If using IPOINT_AFTER,
                                   one IARG_TYPE must be IARG_PROTOTYPE.
                                   The list must end with IARG_END.

  @ref PIN_StartProgramProbed() must be used when using this API.

  Use @ref RTN_IsSafeForProbedInsertionEx() to determine if a function is a
  suitable candidate for probed function insertion.

  Some restrictions apply when using IARG_CONTEXT.  See @ref INST_ARGS for more information.
  IARG_THREAD_ID is not supported.

  @par Availability:
  \b Mode:  Probe\n
  \b O/S:   All\n
  \b CPU:   All\n
*/
extern VOID RTN_InsertCallProbedEx(RTN orgRtn, IPOINT action, PROBE_MODE mode, AFUNPTR funptr, ...);

/*! @ingroup PIN_CONTROL
  Check if the given location is safe for probe insertion.
  This means that the location contains an instruction which
  is large enough to be replaced entirely with a jump instruction
  (i.e. 5/7 bytes long), it is not a control flow instruction,
  and it doesn't have memory operand.

  @param     address          the application address to instrument
  @return                     TRUE/FALSE if the location is safe

  @ref PIN_StartProgramProbed() must be used when using this API.

  @par Availability:
  \b Mode:  Probe\n
  \b O/S:   All\n
  \b CPU:   IA-32 and Intel(R) 64 architectures\n
*/
extern BOOL PIN_IsSafeForProbedInsertion(ADDRINT addr);

/*! @ingroup PIN_CONTROL
  Insert a call to an analysis routine relative to a location.
  In this location we expect to find a safe instruction, i.e. an
  instruction which is large enough to be replaced entirely with
  a jump instruction (i.e. 5/7 bytes long), it is not a control
  flow instruction, and it doesn't have memory operand.

  @param     address          the application address to instrument
  @param     funptr           the analysis function
  @param     ...              @ref IARG_TYPE.  The list must end with IARG_END.

  @ref PIN_StartProgramProbed() must be used when using this API.

  Some restrictions apply when using IARG_CONTEXT.  See @ref INST_ARGS for more information.
  IARG_THREAD_ID is not supported.

  @note The pin client lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  Probe\n
  \b O/S:   All\n
  \b CPU:   IA-32 and Intel(R) 64 architectures\n
*/
extern VOID PIN_InsertCallProbed(ADDRINT addr, AFUNPTR funptr, ...);

/*! @ingroup PROTO
  Allocate and initialize a function prototype.
  @param returnArg The return argument type and size. See @ref PIN_PARG.
  @param cstype The calling standard used for compiling this function
  (CALLINGSTD_DEFAULT recommended).
  @param name The name of the function.
  @param ... List of function argument type and size.
  See @ref PIN_PARG for the recommended method of passing function arguments.
  * @note  currently PARG_FLOAT, PARG_DOUBLE are supported as return type only and can not
  be used as function argument types.
  Any other PRG_* that does not fit in an integer register are not supported neither as
  function argument nor as return type of replacement functions.

  @ref PIN_PARG_END() must end the argument list, even if the function has no arguments.
  @return proto A pointer to a function prototype.  This is an opaque type.

  @par Availability:
  \b Mode:  JIT & Probe\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern PROTO PROTO_Allocate(PARG_T returnArg, CALLINGSTD_TYPE cstype, const char* name, ...);

/*! @ingroup PROTO
  Free the specified function prototype.
  @param proto A pointer to a function prototype.

  @par Availability:
  \b Mode:  JIT & Probe\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
*/
extern VOID PROTO_Free(PROTO proto);

#endif // PIN_G_OPIN_CLIENT_PH
