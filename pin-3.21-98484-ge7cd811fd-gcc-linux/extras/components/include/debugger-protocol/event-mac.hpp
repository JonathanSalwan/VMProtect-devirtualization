/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: debugger-protocol
// <FILE-TYPE>: component public header

#ifndef DEBUGGER_PROTOCOL_EVENT_MAC_HPP
#define DEBUGGER_PROTOCOL_EVENT_MAC_HPP

#include "debugger-protocol.hpp"
#include "debugger-protocol/event-linux.hpp"

namespace DEBUGGER_PROTOCOL
{
/*!
 * The macOS* signal numbers.  These are the same values defined in the system <signal.h> header.
 */
enum MAC_SIGNAL
{
    MAC_SIGNAL_HUP    = 1,
    MAC_SIGNAL_INT    = 2,
    MAC_SIGNAL_QUIT   = 3,
    MAC_SIGNAL_ILL    = 4,
    MAC_SIGNAL_TRAP   = 5,
    MAC_SIGNAL_ABRT   = 6,
    MAC_SIGNAL_EMT    = 7,
    MAC_SIGNAL_FPE    = 8,
    MAC_SIGNAL_KILL   = 9,
    MAC_SIGNAL_BUS    = 10,
    MAC_SIGNAL_SEGV   = 11,
    MAC_SIGNAL_SYS    = 12,
    MAC_SIGNAL_PIPE   = 13,
    MAC_SIGNAL_ALRM   = 14,
    MAC_SIGNAL_TERM   = 15,
    MAC_SIGNAL_URG    = 16,
    MAC_SIGNAL_STOP   = 17,
    MAC_SIGNAL_TSTP   = 18,
    MAC_SIGNAL_CONT   = 19,
    MAC_SIGNAL_CHLD   = 20,
    MAC_SIGNAL_TTIN   = 21,
    MAC_SIGNAL_TTOU   = 22,
    MAC_SIGNAL_IO     = 23,
    MAC_SIGNAL_XCPU   = 24,
    MAC_SIGNAL_XFSZ   = 25,
    MAC_SIGNAL_VTALRM = 26,
    MAC_SIGNAL_PROF   = 27,
    MAC_SIGNAL_WINCH  = 28,
    MAC_SIGNAL_INFO   = 29,
    MAC_SIGNAL_USR1   = 30,
    MAC_SIGNAL_USR2   = 31
};

#if defined(DEBUGGER_PROTOCOL_BUILD) // Library clients should NOT define this.

/*!
 * When LLDB connects to an macOS* process, it expects that these signals are initially not
 * intercepted.  If a back-end connects to a debugger of type FRONTEND_TYPE_LLDB, the
 * back-end should arrange for these signals to be handled internally by the process,
 * without notifying LLDB.  Note that these are only an initial default setting.  LLDB may
 * explicitly ask to intercept these signals later via IEVENT_INTERCEPTION.
 */
DEBUGGER_PROTOCOL_API EVENT DefaultMacSignalsNotInterceptedByGdb[] = {MAC_SIGNAL_ALRM,   MAC_SIGNAL_URG,  MAC_SIGNAL_CHLD,
                                                                      MAC_SIGNAL_VTALRM, MAC_SIGNAL_PROF, MAC_SIGNAL_WINCH};

/*!
 * Number of entries in DefaultMacSignalsNotInterceptedByGdb.
 */
DEBUGGER_PROTOCOL_API unsigned DefaultMacSignalsNotInterceptedByGdbCount =
    sizeof(DefaultMacSignalsNotInterceptedByGdb) / sizeof(DefaultMacSignalsNotInterceptedByGdb[0]);

#else

/*!
 * Signals not initially intercepted by GDB.
 */
DEBUGGER_PROTOCOL_API extern EVENT DefaultMacSignalsNotInterceptedByGdb[];

/*!
 * Number entries in DefaultMacSignalsNotInterceptedByGdb.
 */
DEBUGGER_PROTOCOL_API extern unsigned DefaultMacSignalsNotInterceptedByGdbCount;

#endif

/*!
 * In the future, new fields may be added to the end of the EVENT_INFO_MAC32
 * or EVENT_INFO_MAC64 structures.  If this happens, clients can use the
 * \e _version field to retain backward compatibility.
 *
 * When a client writes information to these structures, it should set \e _version
 * to the latest version that it supports.
 *
 * When a client reads these structures, it should use \e _version to tell which
 * fields are valid.  A client should allow that \e _version may be greater than
 * the newest version it knows about, which happens if an older front-end runs
 * with a newer back-end or vice-versa.
 */
enum EVENT_INFO_MAC_VERSION
{
    EVENT_INFO_MAC_VERSION_0 ///< This is the only defined version currently.
};

/*!
 * Information about a signal received on a 32-bit macOS* target.
 */
struct /*<POD>*/ EVENT_INFO_MAC32
{
    EVENT_INFO_MAC_VERSION _version; ///< Tells which fields in this structure are valid.
    SIGINFO32 _siginfo;              ///< Signal information.
};

/*!
 * Information about a signal received on a 64-bit macOS* target.
 */
struct /*<POD>*/ EVENT_INFO_MAC64
{
    EVENT_INFO_MAC_VERSION _version; ///< Tells which fields in this structure are valid.
    SIGINFO64 _siginfo;              ///< Signal information.
};

} // namespace DEBUGGER_PROTOCOL
#endif // file guard
