/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: debugger-protocol
// <FILE-TYPE>: component public header

#ifndef DEBUGGER_PROTOCOL_THREAD_LINUX_HPP
#define DEBUGGER_PROTOCOL_THREAD_LINUX_HPP

namespace DEBUGGER_PROTOCOL
{
/*!
 * In the future, new fields may be added to the end of the THREAD_INFO_LINUX
 * structure.  If this happens, clients can use the \e _version field to retain
 * backward compatibility.
 *
 * When a client writes information to this structure, it should set \e _version
 * to the latest version that it supports.
 *
 * When a client reads this structure, it should use \e _version to tell which
 * fields are valid.  A client should allow that \e _version may be greater than
 * the newest version it knows about, which happens if an older front-end runs
 * with a newer back-end or vice-versa.
 */
enum THREAD_INFO_LINUX_VERSION
{
    THREAD_INFO_LINUX_VERSION_0 ///< This is the only defined version currently.
};

/*!
 * Information about a thread running on a Linux target.
 */
struct /*<POD>*/ THREAD_INFO_LINUX
{
    THREAD_INFO_LINUX_VERSION _version; ///< Tells which fields in this structure are valid.

    /* There are no fields defined for this version. */
};

} // namespace DEBUGGER_PROTOCOL
#endif // file guard
