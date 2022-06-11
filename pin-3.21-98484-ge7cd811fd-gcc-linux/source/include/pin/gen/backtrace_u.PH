/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_BACKTRACE_U_PH
#error duplicate inclusion of backtrace_u
#else
#define PIN_G_BACKTRACE_U_PH
/*! @file
 libbacktrace equivalent implementation
*/
/*! @ingroup CONTEXT
 * This function is the equivalent Pin version of unw_getcontext():
 * While unw_getcontext() generates a context for libunwind based on the native context,
 * PIN_GetInitialContextForUnwind() generates a context for libunwind based on Pin CONTEXT.
 *
 * @param[in]       ctxt   Pin context to convert to libunwind's context
 * @param[out]      cursor Points to valid object of type unw_cursor_t which was initialized using unw_init_local().
 * @return   UNW_ESUCCESS in case of success.
 *
 * @par Availability:
 * \b Mode:  All\n
 * \b O/S:   Linux & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern int PIN_GetInitialContextForUnwind(const CONTEXT* ctxt, void* cursor);

/*! @ingroup CONTEXT
 * This function is the equivalent Pin version of backtrace():
 * While backtrace() generates a backtrace for the Pin (and Pin tool) call stack,
 * PIN_Backtrace() generates a backtrace for the application that Pin is instrumenting.
 *
 * @param[in]       ctxt   context of the application - the backtrace will be generated based upon it.
 * @param[out]      buffer Points to an array on which the backtrace will be stored.
 * @param[in]       size   specifies the maximum number of addresses that can be stored in buffer.
 * @return   the number of addresses returned in buffer, which is not greater than @b size.
 *
 * @par Availability:
 * \b Mode:  All\n
 * \b O/S:   Linux & macOS*\n
 * \b CPU:   IA-32 and Intel(R) 64 architectures\n
 */
extern int PIN_Backtrace(const CONTEXT* ctxt, void** buffer, int size);

#endif // PIN_G_BACKTRACE_U_PH
