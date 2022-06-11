/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_TLS_CLIENT_PH
#error duplicate inclusion of tls_client
#else
#define PIN_G_TLS_CLIENT_PH
/*! @file

 */

/*! @ingroup THREADS
  Keys to access client data in the thread local storage
*/
enum
{
    // Reserved for internal exception handling
    TLS_KEY_INTERNAL_EXCEPTION,

    TLS_KEY_CLIENT_FIRST,
    // Keys in the [TLS_KEY_CLIENT_FIRST, TLS_KEY_CLIENT_LAST] range can be
    // allocated by tool
    TLS_KEY_CLIENT_LAST = TLS_KEY_CLIENT_FIRST + 63
};

/*! @ingroup THREADS
  Maximum number of TLS keys that can be allocated by tool
*/
const UINT32 MAX_CLIENT_TLS_KEYS = (TLS_KEY_CLIENT_LAST - TLS_KEY_CLIENT_FIRST + 1);

/*! @ingroup THREADS
 * Allocate a new TLS key and associate it with a given data destruction function.
 * Upon successful return, any thread of the process can use PIN_SetThreadData() and
 * PIN_GetThreadData() functions to store and retrieve values in its own slot,
 * referenced by the allocated key. The initial value associated with the key in
 * all threads is NULL. At the thread exit, if a key has a non-NULL pointer to the
 * cleanup (destruction) function, and the thread has a non-NULL value in the
 * corresponding slot, the function is called with the slot's value as its sole
 * argument. The order in which destructors are invoked is undefined.
 * @param[in]    destructFun     optional parameter that specifies data destruction
 *                               function to be associated with the new key.
 *                               This function is called just after notifying the
 *                               client about VM thread exit. By default (NULL destructor),
 *                               the data is not cleaned up.
 * @return   allocated TLS key, upon success;
 *           -1, if number of already allocated keys reached the MAX_CLIENT_TLS_KEYS limit.

  @par Availability:
  \b Mode:  JIT\n
  \b O/S:   Linux, Windows & macOS*\n
  \b CPU:   All\n
 */
extern TLS_KEY PIN_CreateThreadDataKey(DESTRUCTFUN destruct_func);

/*! @ingroup THREADS
 * Release TLS key, allocated by a previous call to the PIN_CreateThreadDataKey() function.
 * The function has no effect if specified key is not allocated/already released.
 * After the key is released, a client should not attempt to use it for any further
 * TLS access. Doing otherwise results in undefined behavior.
 * @param[in]    key         TLS key to be released
 * @return   TRUE, upon success;
 *           FALSE, if if specified key is invalid
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   All\n
 */
extern BOOL PIN_DeleteThreadDataKey(TLS_KEY key);

/* This version of PIN_SetThreadData is now deprecated. Please use the version of this function which takes
 * a THREADID argument.
 */
/*! @ingroup DEPRECATED
 * Store specified value in the specified TLS slot of the thread.
 *
 * @note DEPRECATED and NO LONGER SUPPORTED. Please use the version of this function which takes a THREADID argument.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   All\n
 */
inline BOOL PIN_SetThreadData(TLS_KEY key, const VOID* data)
{
    ASSERT(FALSE, "This version of PIN_SetThreadData is now deprecated. Please use the version of this function which "
                  "takes a THREADID argument.");
    return FALSE;
}

/*! @ingroup THREADS
 * Store specified value in the specified TLS slot of the thread.
 * @param[in]    key                 TLS key, referencing the slot, where the value
 *                                   will be stored upon successful return. The key
 *                                   should be allocated by a previous call to the
 *                                   PIN_CreateThreadDataKey() function.
 * @param[in]    data                value to be stored in the TLS slot
 *                                   of the current thread.
 * @param[in]    threadId            Thread ID assigned by pin of the thread for which
 *                                   TLS access is desired, not necessarily the current
 *                                   thread.
 *                                   This ID can be obtained by the PIN_ThreadId()
 *                                   function call or received as the value of the
 *                                   IARG_THREAD_ID argument.
 * @return   TRUE, if specified key is currently allocated;
 *           FALSE, if specified key is invalid or the given thread is not yet
 *                  registered in the pin thread database;
 *           undefined, if specified key is valid, but it is not currently allocated
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   All\n
 */
extern BOOL PIN_SetThreadData(TLS_KEY key, const VOID* data, THREADID threadId);

/* This version of PIN_GetThreadData is now deprecated. Please use the version of this function which takes
 * a THREADID argument.
 */
/*! @ingroup DEPRECATED
 * Get the value stored in the specified TLS slot of the thread.
 *
 * @note DEPRECATED and NO LONGER SUPPORTED. Please use the version of this function which takes a THREADID argument.
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   All\n
 */
inline VOID* PIN_GetThreadData(TLS_KEY key)
{
    ASSERT(FALSE, "This version of PIN_GetThreadData is now deprecated. Please use the version of this function which "
                  "takes a THREADID argument.");
    return FALSE;
}

/*! @ingroup THREADS
 * Get the value stored in the specified TLS slot of the thread.
 * @param[in]    key                 TLS key, referencing the slot, whose value is
 *                                   requested. The key should be allocated by a
 *                                   previous call to the PIN_CreateThreadDataKey()
 *                                   function.
 * @param[in]    threadId            Thread ID assigned by pin of the thread for which
 *                                   TLS access is desired, not necessarily the current
 *                                   thread.
 *                                   This ID can be obtained by the PIN_ThreadId()
 *                                   function call or received as the value of the
 *                                   IARG_THREAD_ID argument.
 * @return   value stored in the specified slot of the thread, if specified key is
 *           currently allocated;
 *           NULL,  if specified key is invalid or the given thread is not yet
 *                  registered in the pin thread database;
 *           undefined, if specified key is valid, but it is not currently allocated
 *
 * @par Availability:
 * \b Mode:  JIT\n
 * \b O/S:   Linux, Windows & macOS*\n
 * \b CPU:   All\n
 */
extern VOID* PIN_GetThreadData(TLS_KEY key, THREADID threadId);

#endif // PIN_G_TLS_CLIENT_PH
