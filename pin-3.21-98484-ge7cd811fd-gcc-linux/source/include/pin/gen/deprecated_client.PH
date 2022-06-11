/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_DEPRECATED_CLIENT_PH
#error duplicate inclusion of deprecated_client
#else
#define PIN_G_DEPRECATED_CLIENT_PH
/*! @file
 *
 * Deprecated Pin client functions.
 */
/* CALLBACK_SetExecutionPriority is now deprecated.
 */
/*! @ingroup DEPRECATED
  Sets the execution priority of a registered callback function
  The lower the execution priority, the sooner it will be called relative to other
  similar callbacks
  @param callback  Registered Callback function returned from *_Add*Function()
  @param priority  New priority to assign to the callback

  @note The pin client lock is obtained during the call of this API.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n

  @note DEPRECATED: See  CALLBACK_SetExecutionOrder.
*/
extern PIN_DEPRECATED_API VOID CALLBACK_SetExecutionPriority(PIN_CALLBACK callback, INT32 priority);

/* CALLBACK_GetExecutionPriority is now deprecated. Returns CALLBACK_GetExecutionOrder(callback)
 */
/*! @ingroup DEPRECATED
  Gets the execution priority of a registered callback function
  The lower the execution priority, the sooner it will be called relative to other
  similar callbacks
  @param callback  Registered Callback function returned from *_Add*Function()

  @return INT32   The current priority value of the callback

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n

  @note DEPRECATED: See  CALLBACK_GetExecutionOrder.
*/
extern PIN_DEPRECATED_API INT32 CALLBACK_GetExecutionPriority(PIN_CALLBACK callback);

#endif // PIN_G_DEPRECATED_CLIENT_PH
