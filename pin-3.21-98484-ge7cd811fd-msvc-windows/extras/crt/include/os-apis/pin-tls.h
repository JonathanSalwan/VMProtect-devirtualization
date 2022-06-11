/*
 * Copyright (C) 2015-2018 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

#ifndef OS_APIS_PIN_TLS_H_
#define OS_APIS_PIN_TLS_H_

#include "types.h"

/*!
 * @defgroup OS_APIS_PIN_TLS Thread local storage
 * @brief Contains API for thread local storage (TLS).
 */

/*! @ingroup OS_APIS_PIN_TLS
 * Prototype for TLS value destructor function.
 * The only argument for this function is the value stored in TLS.
 */
typedef VOID (*TLS_DESTRUCTOR)(VOID*);

/*! @ingroup OS_APIS_PIN_TLS
 * Type for index inside the dynamic TLS.
 */
typedef ADDRINT PIN_TLS_INDEX;

#define OS_APIS_INVALID_PIN_TLS_INDEX ((PIN_TLS_INDEX)-1)

/*! @ingroup OS_APIS_PIN_TLS
 * Return the base address for the application-wide TLS data structure.
 * The TLS data structure is used to implement a TLS which is not based
 * on segment registers.
 *
 * @return      Pointer to the base address of the TLS data structure.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
VOID* OS_TlsGetBaseAddress();

/*! @ingroup OS_APIS_PIN_TLS
 * Allocates a new slot in the dynamic TLS.
 *
 * @param[in]  dest         Optional destructor function that will be called on each value stores in TLS
 *                          when its corresponding thread dies.
 *
 * @return      Index for the newly allocated slot or OS_APIS_INVALID_PIN_TLS_INDEX on error.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
PIN_TLS_INDEX OS_TlsAlloc(TLS_DESTRUCTOR dest);

/*! @ingroup OS_APIS_PIN_TLS
 * Free a previously allocated slot in the dynamic TLS.
 *
 * @param[in]  idx          The index to free.
 *
 * @return      True on success.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
BOOL_T OS_TlsFree(PIN_TLS_INDEX idx);

/*! @ingroup OS_APIS_PIN_TLS
 * Get the value associated with a particular index from the current thread's
 * dynamic TLS.
 *
 * @param[in]  idx          The index of the value to get.
 *
 * @return      Pointer to the value, or NULL otherwise.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
VOID* OS_TlsGetValue(PIN_TLS_INDEX idx);

/*! @ingroup OS_APIS_PIN_TLS
 * Get the value associated with a particular index of a particular thread's
 * dynamic TLS.
 *
 * @param[in]  idx          The index of the value to get.
 * @param[in]  ntid         The thread ID for which the TLS value will be retrieved.
 *
 * @return      Pointer to the value, or NULL otherwise.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
VOID* OS_TlsGetValueForThread(PIN_TLS_INDEX idx, NATIVE_TID ntid);

/*! @ingroup OS_APIS_PIN_TLS
 * Return the address of the static TLS for the current thread
 *
 * @return      Address of the static TLS for the current threads (which is a VOID* array).
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
VOID** OS_TlsGetArrayAddress(void);

/*! @ingroup OS_APIS_PIN_TLS
 * Return the address of the static TLS for a particular thread
 *
 * @param[in]  ntid         The thread ID for which the static TLS will be retrieved.
 *
 * @return      Address of the static TLS for the specified threads (which is a VOID* array).
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
VOID** OS_TlsGetArrayAddressForThread(NATIVE_TID ntid);

/*! @ingroup OS_APIS_PIN_TLS
 * Set the value associated with a particular index in the current thread's
 * dynamic TLS.
 *
 * @param[in]  idx          The index of the value to set.
 * @param[in]  val          The new value to set.
 *
 * @return      True on success.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
BOOL_T OS_TlsSetValue(PIN_TLS_INDEX idx, VOID* val);

/*! @ingroup OS_APIS_PIN_TLS
 * Set the value associated with a particular index in of a particular thread's
 * dynamic TLS.
 *
 * @param[in]  idx          The index of the value to set.
 * @param[in]  ntid         The thread ID for which the value will be set in its TLS.
 * @param[in]  val          The new value to set.
 *
 * @return      True on success.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
BOOL_T OS_TlsSetValueForThread(PIN_TLS_INDEX idx, NATIVE_TID ntid, VOID* val);

/*! @ingroup OS_APIS_PIN_TLS
 * Free all resources that are used for a particular thread's TLS.
 * This function is usually called when a thread is about to exit.
 *
 * @param[in]  ntid         The thread ID for which the TLS need to be freed.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
VOID OS_TlsFreeThreadTls(NATIVE_TID ntid);

/*! @ingroup OS_APIS_PIN_TLS
 * Add a destructor for all threads' static TLS.
 * The destructor for static TLS is called on each thread termination, provided
 * that the thread had an allocated static TLS.
 *
 * @param[in]  destructor   Destructor function that will be called with the pointer
 *                          to the fixed TLS when a thread terminates.
 *
 * @par Availability:
 *   @b O/S:   Windows, Linux & macOS*\n
 *   @b CPU:   All\n
 */
BOOL_T OS_TlsAddStaticTlsDestructor(TLS_DESTRUCTOR destructor);

#endif // file guard
