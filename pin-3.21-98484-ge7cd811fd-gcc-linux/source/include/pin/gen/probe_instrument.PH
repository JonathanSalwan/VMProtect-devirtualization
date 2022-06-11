/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_PROBE_INSTRUMENT_PH
#error duplicate inclusion of probe_instrument
#else
#define PIN_G_PROBE_INSTRUMENT_PH
/*! @file
 Unix-specific instrumentation in probe mode
*/
#if defined(TARGET_LINUX) || defined(TARGET_MAC)
/*! @ingroup PIN_CONTROL
 * Call-back function in probe mode for fork notification.
 *
 *    @param[in] childPid  Process id of the child process, available after fork in parent
 *    @param[in] v         The tool's call-back value.
 */
typedef VOID (*FORK_PROBE_MODE_CALLBACK)(UINT32 childPid, VOID* v);

/*! @ingroup PIN_CONTROL
 * Register a notification handler that is called when the application
 * forks a new process.
 * If vfork() is called in application, Pin replaces it with fork().
 * This replacement is needed to keep Pin/Tool state consistent and prevent memory
 * overlap in callbacks following the fork/vfork. The function can only be used on Linux, and macOS*.
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
 * @note The pin client lock is obtained during the call of this API.
 *
 * @par Availability:
 * \b Mode:  Probe\n
 * \b O/S:   Linux, and macOS*\n
 * \b CPU:   All\n
 */
extern PIN_CALLBACK PIN_AddForkFunctionProbed(FPOINT point, FORK_PROBE_MODE_CALLBACK fun, VOID* val);

#endif //defined(TARGET_LINUX)
#endif // PIN_G_PROBE_INSTRUMENT_PH
