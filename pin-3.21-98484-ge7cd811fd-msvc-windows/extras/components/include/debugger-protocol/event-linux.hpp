/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: debugger-protocol
// <FILE-TYPE>: component public header

#ifndef DEBUGGER_PROTOCOL_EVENT_LINUX_HPP
#define DEBUGGER_PROTOCOL_EVENT_LINUX_HPP

#include "debugger-protocol.hpp"

namespace DEBUGGER_PROTOCOL
{
/*!
 * The Linux signal numbers.  These are the same values defined in the system <signal.h> header.
 */
enum LINUX_SIGNAL
{
    LINUX_SIGNAL_HUP    = 1,
    LINUX_SIGNAL_INT    = 2,
    LINUX_SIGNAL_QUIT   = 3,
    LINUX_SIGNAL_ILL    = 4,
    LINUX_SIGNAL_TRAP   = 5,
    LINUX_SIGNAL_ABRT   = 6,
    LINUX_SIGNAL_BUS    = 7,
    LINUX_SIGNAL_FPE    = 8,
    LINUX_SIGNAL_KILL   = 9,
    LINUX_SIGNAL_USR1   = 10,
    LINUX_SIGNAL_SEGV   = 11,
    LINUX_SIGNAL_USR2   = 12,
    LINUX_SIGNAL_PIPE   = 13,
    LINUX_SIGNAL_ALRM   = 14,
    LINUX_SIGNAL_TERM   = 15,
    LINUX_SIGNAL_STKFLT = 16,
    LINUX_SIGNAL_CHLD   = 17,
    LINUX_SIGNAL_CONT   = 18,
    LINUX_SIGNAL_STOP   = 19,
    LINUX_SIGNAL_TSTP   = 20,
    LINUX_SIGNAL_TTIN   = 21,
    LINUX_SIGNAL_TTOU   = 22,
    LINUX_SIGNAL_URG    = 23,
    LINUX_SIGNAL_XCPU   = 24,
    LINUX_SIGNAL_XFSZ   = 25,
    LINUX_SIGNAL_VTALRM = 26,
    LINUX_SIGNAL_PROF   = 27,
    LINUX_SIGNAL_WINCH  = 28,
    LINUX_SIGNAL_POLL   = 29,
    LINUX_SIGNAL_PWR    = 30,
    LINUX_SIGNAL_SYS    = 31,
    LINUX_SIGNAL_RT32   = 32,
    LINUX_SIGNAL_RT33   = 33,
    LINUX_SIGNAL_RT34   = 34,
    LINUX_SIGNAL_RT35   = 35,
    LINUX_SIGNAL_RT36   = 36,
    LINUX_SIGNAL_RT37   = 37,
    LINUX_SIGNAL_RT38   = 38,
    LINUX_SIGNAL_RT39   = 39,
    LINUX_SIGNAL_RT40   = 40,
    LINUX_SIGNAL_RT41   = 41,
    LINUX_SIGNAL_RT42   = 42,
    LINUX_SIGNAL_RT43   = 43,
    LINUX_SIGNAL_RT44   = 44,
    LINUX_SIGNAL_RT45   = 45,
    LINUX_SIGNAL_RT46   = 46,
    LINUX_SIGNAL_RT47   = 47,
    LINUX_SIGNAL_RT48   = 48,
    LINUX_SIGNAL_RT49   = 49,
    LINUX_SIGNAL_RT50   = 50,
    LINUX_SIGNAL_RT51   = 51,
    LINUX_SIGNAL_RT52   = 52,
    LINUX_SIGNAL_RT53   = 53,
    LINUX_SIGNAL_RT54   = 54,
    LINUX_SIGNAL_RT55   = 55,
    LINUX_SIGNAL_RT56   = 56,
    LINUX_SIGNAL_RT57   = 57,
    LINUX_SIGNAL_RT58   = 58,
    LINUX_SIGNAL_RT59   = 59,
    LINUX_SIGNAL_RT60   = 60,
    LINUX_SIGNAL_RT61   = 61,
    LINUX_SIGNAL_RT62   = 62,
    LINUX_SIGNAL_RT63   = 63
};

#if defined(DEBUGGER_PROTOCOL_BUILD) // Library clients should NOT define this.

/*!
 * When GDB connects to a Linux process, it expects that these signals are initially not
 * intercepted.  If a back-end connects to a debugger of type FRONTEND_TYPE_GDB, the
 * back-end should arrange for these signals to be handled internally by the process,
 * without notifying GDB.  Note that these are only an initial default setting.  GDB may
 * explicitly ask to intercept these signals later via IEVENT_INTERCEPTION.
 */
DEBUGGER_PROTOCOL_API EVENT DefaultLinuxSignalsNotInterceptedByGdb[] = {
    LINUX_SIGNAL_ALRM,  LINUX_SIGNAL_URG,  LINUX_SIGNAL_CHLD, LINUX_SIGNAL_VTALRM, LINUX_SIGNAL_PROF,
    LINUX_SIGNAL_WINCH, LINUX_SIGNAL_POLL, LINUX_SIGNAL_RT32, LINUX_SIGNAL_RT33};

/*!
 * Number of entries in DefaultLinuxSignalsNotInterceptedByGdb.
 */
DEBUGGER_PROTOCOL_API unsigned DefaultLinuxSignalsNotInterceptedByGdbCount =
    sizeof(DefaultLinuxSignalsNotInterceptedByGdb) / sizeof(DefaultLinuxSignalsNotInterceptedByGdb[0]);

#else

/*!
 * Signals not initially intercepted by GDB.
 */
DEBUGGER_PROTOCOL_API extern EVENT DefaultLinuxSignalsNotInterceptedByGdb[];

/*!
 * Number entries in DefaultLinuxSignalsNotInterceptedByGdb.
 */
DEBUGGER_PROTOCOL_API extern unsigned DefaultLinuxSignalsNotInterceptedByGdbCount;

#endif

/*!
 * In the future, new fields may be added to the end of the EVENT_INFO_LINUX32
 * or EVENT_INFO_LINUX64 structures.  If this happens, clients can use the
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
enum EVENT_INFO_LINUX_VERSION
{
    EVENT_INFO_LINUX_VERSION_0 ///< This is the only defined version currently.
};

/*!
 * This has the same layout as the 32-bit system siginfo_t.  If you are compiling on that
 * system, you can safely cast to that type.
 */
struct /*<POD>*/ SIGINFO32
{
    INT32 _signo;
    INT32 _errno;
    INT32 _code;

    union
    {
        UINT32 _pad[(128 / sizeof(UINT32)) - 3]; // Total size of SIGINFO32 is 128 bytes
    };
};

/*!
 * This has the same layout as the 64-bit system siginfo_t.  If you are compiling on that
 * system, you can safely cast to that type.
 */
struct /*<POD>*/ SIGINFO64
{
    INT32 _signo;
    INT32 _errno;
    INT32 _code;
    INT32 _pad0;

    union
    {
        UINT32 _pad1[(128 / sizeof(UINT32)) - 4]; // Total size of SIGINFO64 is 128 bytes
    };
};

/*!
 * Information about a signal received on a 32-bit Linux target.
 */
struct /*<POD>*/ EVENT_INFO_LINUX32
{
    EVENT_INFO_LINUX_VERSION _version; ///< Tells which fields in this structure are valid.
    SIGINFO32 _siginfo;                ///< Signal information.
};

/*!
 * Information about a signal received on a 64-bit Linux target.
 */
struct /*<POD>*/ EVENT_INFO_LINUX64
{
    EVENT_INFO_LINUX_VERSION _version; ///< Tells which fields in this structure are valid.
    SIGINFO64 _siginfo;                ///< Signal information.
};

} // namespace DEBUGGER_PROTOCOL
#endif // file guard
