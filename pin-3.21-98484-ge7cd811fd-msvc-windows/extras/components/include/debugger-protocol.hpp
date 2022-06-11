/*
 * Copyright (C) 2008-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: debugger-protocol
// <FILE-TYPE>: component public header

#ifndef DEBUGGER_PROTOCOL_HPP
#define DEBUGGER_PROTOCOL_HPP

#include <string>
#include "util.hpp"
#include "debugger-protocol/forward.hpp"

#if defined(DEBUGGER_PROTOCOL_BUILD_SHARED)
#define DEBUGGER_PROTOCOL_API _STLP_EXPORT_DECLSPEC
#elif defined(DEBUGGER_PROTOCOL_USE_SHARED)
#define DEBUGGER_PROTOCOL_API _STLP_IMPORT_DECLSPEC
#else
#define DEBUGGER_PROTOCOL_API
#endif

/*! @mainpage DEBUGGER_PROTOCOL library
 *
 * The DEBUGGER_PROTOCOL library provides a low-level interface for communicating
 * between the back-end of a debugger and the front-end of a debugger.  The
 * library supports front-end and back-end users.
 *
 * The DEBUGGER_PROTOCOL library currently supports the GDB protocol, which means
 * a back-end using this API can communicate with GDB.  Likewise, a debugger
 * front-end using this API can communicate with a GDB back-end (target stub).
 * In the future, support may be added for other protocols.
 *
 * To get started writing a debugger front-end, see IFRONTEND.
 *
 * To get started writing a debugger back-end, see IBACKEND.
 */

/*! @brief The DEBUGGER_PROTOCOL library. */
namespace DEBUGGER_PROTOCOL
{
/*!
 * Identifier representing a target thread, typically an O/S identifier.  The value zero is reserved.
 * Thread ID's may be reused once a thread exits.
 */
typedef unsigned long THREAD;

/*!
 * Identifier representing a register.  This is either one of the "generic" registers
 * (e.g. REG_PC) or a value 'REG_END + i' where \e i is an index into the \e regs
 * parameter to IBACKEND::SetRegisters() or IFRONTEND::SetRegisters().
 */
typedef unsigned REG;

/*!
 * Represents an ID to an "image", which is a loadable module in the target process.
 * An image can represent a DLL, shared library, or the main executable program.  Image
 * ID's may be reused once an image is unloaded.
 */
typedef unsigned long IMAGE;

/*!
 * An identifier representing a target process, typically an O/S process ID.  Process
 * ID's may be reused once a process exits.
 */
typedef unsigned long PROCESS;

/*!
 * O/S specific information about an image.  Pointers to this type must be cast according
 * to the \e os parameter that is passed to CreateBackEnd() or returned from
 * IFRONTEND::GetBackEndOs().
 *
 * - For OS_LINUX32 or OS_LINUX64, cast to IMAGE_INFO_LINUX, which is defined in
 *   "debugger-protocol/image-linux.hpp".
 *
 * - For OS_WINDOWS32 or OS_WINDOWS64, cast to IMAGE_INFO_WINDOWS, which is defined in
 *   "debugger-protocol/image-windows.hpp".
 */
typedef void IMAGE_INFO;

/*!
 * Images list required by GDB (in System V R4 standard).
 * Implemented only for Linux GDB.
 *
 */
typedef void SVR4_IMAGES_LIST;

/*!
 * Image info required by GDB (in System V R4 standard).
 * Implemented only for Linux GDB.
 *
 */
typedef void SVR4_IMAGE_INFO;

/*!
 * Identifies an O/S specific "event", according to the \e os parameter passed to
 * CreateBackEnd() or returned from IFRONTEND::GetBackEndOs().
 *
 * - For OS_LINUX32 and OS_LINUX64, this is a signal number.  In API's where the
 *   back-end reports an event to the front-end, the value zero indicates that the
 *   target process received an unknown signal.
 *
 * - For OS_WINDOWS32 and OS_WINDOWS64, this is an exception code.
 */
typedef unsigned long EVENT;

/*!
 * O/S specific additional information about an event, which is used when the back-end
 * supports BACKEND_FEATURE_EVENT_INFO.  Pointers to this type must be cast according
 * to the \e os parameter that is passed to CreateBackEnd() or returned from
 * IFRONTEND::GetBackEndOs().
 *
 * - For OS_LINUX32, cast to EVENT_INFO_LINUX32, which is defined in "debugger-protocol/event-linux.hpp".
 * - For OS_LINUX64, cast to EVENT_INFO_LINUX64, which is defined in "debugger-protocol/event-linux.hpp".
 *
 * - For OS_WINDOWS32, cast to EVENT_INFO_WINDOWS32 which is defined in "debugger-protocol/event-windows.hpp".
 * - For OS_WINDOWS64, cast to EVENT_INFO_WINDOWS64 which is defined in "debugger-protocol/event-windows.hpp".
 */
typedef void EVENT_INFO;

/*!
 * O/S specific information about a thread, which is used when the back-end supports
 * BACKEND_FEATURE_THREAD_INFO.  Pointers to this type must be cast according
 * to the \e os parameter that is passed CreateBackEnd() or returned from
 * IFRONTEND::GetBackEndOs().
 *
 * - For OS_LINUX32 or OS_LINUX64, cast to THREAD_INFO_LINUX, which is defined in
 *   "debugger-protocol/thread-linux.hpp".
 *
 * - For OS_WINDOWS32 or OS_WINDOWS64, cast to THREAD_INFO_WINDOWS, which is defined in
 *   "debugger-protocol/thread-windows.hpp".
 */
typedef void THREAD_INFO;

/*!
 * Flags that can be used with the Initialize() function.
 */
enum INITIALIZE_FLAG
{
    INITIALIZE_FLAG_NONE = 0,
    INITIALIZE_FLAG_TCP  = (1 << 0) ///< Initialize for a TCP front- or back-end connection.
};
typedef UINT32 INITIALIZE_FLAGS; ///< Bit mask of INITIALIZE_FLAG's.

/*!
 * Possible O/S types.
 */
enum OS
{
    OS_INVALID,
    OS_LINUX32,
    OS_LINUX64,
    OS_WINDOWS32,
    OS_WINDOWS64,
    OS_MAC32,
    OS_MAC64
};

/*!
 * Generic register ID's that are common for all targets.
 */
enum
{
    REG_INVALID,
    REG_PC,    ///< The program counter.
    REG_FP,    ///< The frame pointer.
    REG_SP,    ///< The stack pointer.
    REG_FLAGS, ///< The flags.
    REG_END
};

/*!
 * Possible types of the debugger front-end.
 */
enum FRONTEND_TYPE
{
    FRONTEND_TYPE_UNKNOWN,             ///< Type not known.
    FRONTEND_TYPE_GDB,                 ///< The GNU debugger.
    FRONTEND_TYPE_IDB,                 ///< The Intel debugger.
    FRONTEND_TYPE_VISUAL_STUDIO_VSDBG, ///< Visual Studio via VSDBG.
    FRONTEND_TYPE_VISUAL_STUDIO,       ///< Visual Studio via native connection.
    FRONTEND_TYPE_LLDB                 ///< The LLVM debugger.
};

/*!
 * Possible types of the debugger back-end.
 */
enum BACKEND_TYPE
{
    BACKEND_TYPE_UNKNOWN, ///< Type not known.
    BACKEND_TYPE_PIN      ///< The Pin dynamic instrumentation system.
};

/*!
 * Possible features supported by a debugger front-end.
 */
enum FRONTEND_FEATURE
{
    FRONTEND_FEATURE_NONE = 0, ///< No extended features supported.

    /*!
     * The front-end will dynamically change the expected register set based on the
     * target description returned by the back-end.  (See
     * ITARGET_DESCRIPTION::GetTargetDescription(), IFRONTEND::SetRegisters(), and
     * IBACKEND::SetRegisters().)
     */
    FRONTEND_FEATURE_DYNAMIC_REGISTERS = (1 << 0),

    /*!
     * This option controls the behavior when two or more threads stop simultaneously.
     * For example, two threads might each trigger a breakpoint at the same time, or
     * one thread might trigger a breakpoint while another raises an exception.
     *
     * With the default behavior, the back-end reports each stop-reason separately.  The
     * front-end gets notified via INOTIFICATIONS::NotifyStopped() and one thread will
     * report a stop-reason via ICOMMANDS::GetThreadStopReason().  The back-end keeps
     * the other thread's stop-reason pending.  When the front-end continues execution,
     * the back-end calls NotifyStopped() again and reports the pending stop-reason
     * without actually resuming the target process.  In this mode, the back-end uses
     * various heuristics to maintain the consistency of the pending stop-reasons.  For
     * example, if there is a pending STOP_REASON_BREAKPOINT when the front-end removes
     * the associated breakpoint, the back-end will implicitly squash the pending
     * STOP_REASON_BREAKPOINT notification.
     *
     * If the front-end enables FRONTEND_FEATURE_MULTIPLE_STOP_REASONS, the algorithm is
     * different.  Instead, the back-end reports all stop-reasons simultaneously.  The
     * front-end is notified with a single call to INOTIFICATIONS::NotifyStopped() and
     * each thread reports its own stop-reason via ICOMMANDS::GetThreadStopReason().
     */
    FRONTEND_FEATURE_MULTIPLE_STOP_REASONS = (1 << 1),

    /*!
     * The front-end supports the 'N' stop reply packet.
     * The 'N' packet is sent to the front-end in the special case when all resumed
     * threads had been terminated, but the process is still running (i.e. has additional
     * active threads which are stopped by the debugger).
     */
    FRONTEND_FEATURE_NO_RESUMED = (1 << 2)
};
typedef UINT32 FRONTEND_FEATURES; ///< Bit mask of FRONTEND_FEATURE's.

/*!
 * Possible features supported by a debugger back-end.
 */
enum BACKEND_FEATURE
{
    BACKEND_FEATURE_NONE = 0, ///< No extended features supported.

    /*!
     * The back-end supports the IBREAKPOINTS interface via
     * ICOMMANDS::GetInterface(INTERFACE_ID_BREAKPOINTS).  If a back-end does not
     * support this feature, the front-end will set breakpoints by overwriting
     * a target instruction with a software trap.  When such a breakpoint
     * triggers, the back-end stops the target with STOP_REASON_EVENT.
     */
    BACKEND_FEATURE_BREAKPOINTS = (1 << 0),

    /*!
     * The back-end supports the ICUSTOM_BREAKPOINTS interface via
     * ICOMMANDS::GetInterface(INTERFACE_ID_CUSTOM_BREAKPOINTS).
     */
    BACKEND_FEATURE_CUSTOM_BREAKPOINTS = (1 << 1),

    /*!
     * The back-end supports the ICUSTOM_COMMANDS interface via
     * ICOMMANDS::GetInterface(INTERFACE_ID_CUSTOM_COMMANDS).
     */
    BACKEND_FEATURE_CUSTOM_COMMANDS = (1 << 2),

    /*!
     * The back-end supports the IKILL_WITH_NOTIFICATION interface via
     * ICOMMANDS::GetInterface(INTERFACE_ID_KILL_WITH_NOTIFICATION).
     */
    BACKEND_FEATURE_KILL_WITH_NOTIFICATION = (1 << 3),

    /*!
     * The back-end supports the ITARGET_DESCRIPTION interface via
     * ICOMMANDS::GetInterface(INTERFACE_ID_TARGET_DESCRIPTION).
     */
    BACKEND_FEATURE_TARGET_DESCRIPTION = (1 << 4),

    /*!
     * The back-end supports the IEVENT_INTERCEPTION interface via
     * ICOMMANDS::GetInterface(INTERFACE_ID_EVENT_INTERCEPTION).
     */
    BACKEND_FEATURE_EVENT_INTERCEPTION = (1 << 5),

    /*!
     * The back-end supports the IIMAGE_EXTENSIONS interface via
     * ICOMMANDS::GetInterface(INTERFACE_ID_IMAGE_EXTENSIONS).
     */
    BACKEND_FEATURE_IMAGE_EXTENSIONS = (1 << 6),

    /*!
     * The back-end supports the ITHREAD_EXTENSIONS interface via
     * ICOMMANDS::GetInterface(INTERFACE_ID_THREAD_EXTENSIONS).
     */
    BACKEND_FEATURE_THREAD_EXTENSIONS = (1 << 7),

    /*!
     * The back-end supports additional event information via EVENT_INFO.  Back-ends that do
     * not support this feature can ignore the EVENT_INFO parameters in all methods.
     */
    BACKEND_FEATURE_EVENT_INFO = (1 << 8),

    /*!
     * The back-end supports the ability to resume a thread with CONTINUE_MODE_GO_NEW_EVENT
     * or CONTINUE_MODE_STEP_NEW_EVENT.  Back-ends that do not support this feature will
     * never receives these modes in calls to ICOMMANDS::SetContinueMode().
     */
    BACKEND_FEATURE_CONTINUE_WITH_NEW_EVENT = (1 << 9),

    /*!
     * The back-end supports the IPROCESS_INFO interface via
     * ICOMMANDS::GetInterface(INTERFACE_ID_PROCESS_INFO).
     */
    BACKEND_FEATURE_PROCESS_INFO = (1 << 10),

    /*!
     * The back-end supports System V R4 libraries extension
     */
    BACKEND_FEATURE_SVR4_LIBRARIES = (1 << 11)
};
typedef UINT32 BACKEND_FEATURES; ///< Bit mask of BACKEND_FEATURE's.

/*!
 * Various properties for debugger transport connection
 */
struct /*<POD>*/ DEBUGGER_CONNECTION_PROPERTIES
{
    enum DEBUGGER_CONNECTION_TYPE
    {
        NONE,
        TCP_SERVER,
        TCP_CLIENT
    };
    /*!
     * Underlying connection type for the debugger transport
     */
    DEBUGGER_CONNECTION_TYPE _type;

    /*!
     * TCP port of the debugger connection
     * Available only on TCP_SERVER or TCP_CLIENT connection types
     */
    int _tcpPort;

    /*!
     * TCP host of the debugger connection
     * Available only on TCP_CLIENT connection type
     */
    std::string _tcpHost;
};

/*!
 * Possible options for a communication endpoint.
 */
enum ENDPOINT_OPTION
{
    ENDPOINT_OPTION_NONE = 0, ///< No options specified.

    /*!
     * Do not use run-length compression for response data sent by the back-end.
     * This option is ignored by the front-end because only the back-end will
     * attempt to compress the data it sends.  This option is useful because it
     * makes the ILOG_NOTIFICATIONS messages easier for a human to read.
     */
    ENDPOINT_OPTION_NO_COMPRESSION = (1 << 0),

    /*!
     * This option only applies to endpoints that act as a server.  By default,
     * a server-like endpoint only allows a client to connect from the same
     * machine (i.e. via the TCP loopback address).  This option allows a server
     * to accept a connection even from a different machine (i.e. using the
     * INADDR_ANY local address).
     */
    ENDPOINT_OPTION_ALLOW_REMOTE_CLIENTS = (1 << 1),

    /*!
     * This options requires from the backend to always send the thread id in the
     * reply packets.
     */
    ENDPOINT_OPTION_ALWAYS_SEND_THREAD = (1 << 2),

    /*!
     * This option enables the backend to send textual representation of the stop
     * reason in the packet.
     */
    ENDPOINT_OPTION_SEND_STOP_REASON = (1 << 3),

    /*!
     * This option enables the backend to send unlimited size packets to the
     * frontend.
     */
    ENDPOINT_OPTION_UNLIMITED_LENGTH_PACKETS = (1 << 4)
};
typedef UINT32 ENDPOINT_OPTIONS; ///< Bit mask of ENDPOINT_OPTION's.

/*!
 * Back-ends can provide some optional features via interfaces that are obtained
 * by calling ICOMMANDS::GetInterface().  This enumeration defines the
 * set of possible optional interfaces.
 */
enum INTERFACE_ID
{
    INTERFACE_ID_BREAKPOINTS,            ///< Back-end defines IBREAKPOINTS.
    INTERFACE_ID_CUSTOM_BREAKPOINTS,     ///< Back-end defines ICUSTOM_BREAKPOINTS.
    INTERFACE_ID_CUSTOM_COMMANDS,        ///< Back-end defines ICUSTOM_COMMANDS.
    INTERFACE_ID_KILL_WITH_NOTIFICATION, ///< Back-end defines IKILL_WITH_NOTIFICATION.
    INTERFACE_ID_TARGET_DESCRIPTION,     ///< Back-end defines ITARGET_DESCRIPTION.
    INTERFACE_ID_EVENT_INTERCEPTION,     ///< Back-end defines IEVENT_INTERCEPTION.
    INTERFACE_ID_IMAGE_EXTENSIONS,       ///< Back-end defines IIMAGE_EXTENSIONS.
    INTERFACE_ID_THREAD_EXTENSIONS,      ///< Back-end defines ITHREAD_EXTENSIONS.
    INTERFACE_ID_PROCESS_INFO,           ///< Back-end defines IPROCESS_INFO.
    INTERFACE_ID_SVR4_LIBRARIES          ///< Back-end defines ISVR4_LIBRARIES.
};

/*!
 * Possible notifications that can be enabled with IIMAGE_EXTENSIONS::SetImageNotifications().
 */
enum IMAGE_NOTIFICATION
{
    IMAGE_NOTIFICATION_NONE = 0, ///< Do not enable any image notifications.

    /*!
     * Enable STOP_REASON_IMAGE_LOAD and STOP_REASON_IMAGE_MULTIPLE notifications when an image is loaded.
     */
    IMAGE_NOTIFICATION_LOAD = (1 << 0),

    /*!
     * Enable STOP_REASON_IMAGE_UNLOAD and STOP_REASON_IMAGE_MULTIPLE notifications when an image is unloaded.
     */
    IMAGE_NOTIFICATION_UNLOAD = (1 << 1)
};
typedef UINT32 IMAGE_NOTIFICATIONS; ///< Bit mask of IMAGE_NOTIFICATION's.

/*!
 * Possible notifications that can be enabled with ITHREAD_EXTENSIONS::SetThreadNotifications().
 */
enum THREAD_NOTIFICATION
{
    THREAD_NOTIFICATION_NONE  = 0,        ///< Do not enable any thread notifications.
    THREAD_NOTIFICATION_START = (1 << 0), ///< Enable STOP_REASON_THREAD_START notifications.
    THREAD_NOTIFICATION_EXIT  = (1 << 1)  ///< Enable STOP_REASON_THREAD_EXIT notifications.
};
typedef UINT32 THREAD_NOTIFICATIONS; ///< Bit mask of THREAD_NOTIFICATION's.

/*!
 * Possible continuation modes when debugger front-end continues a target thread.
 */
enum CONTINUE_MODE
{
    CONTINUE_MODE_FROZEN, ///< Thread should remain frozen.

    /*!
     * Thread should resume execution.  If it had previously stopped with STOP_REASON_EVENT,
     * that event is not propagated back to the application.  The thread resumes as though
     * the event did not occur.
     */
    CONTINUE_MODE_GO_SQUASH_EVENT,

    /*!
     * Thread should resume execution.  If it had previously stopped with STOP_REASON_EVENT,
     * it continues executing with that event (e.g. by executing the application's handler).
     * If the application did not stop due to an event, the behavior is the same as with
     * CONTINUE_MODE_GO_SQUASH_EVENT.
     */
    CONTINUE_MODE_GO_PASS_EVENT,

    /*!
     * Thread should resume execution.  If it had previously stopped with STOP_REASON_EVENT,
     * that event is discarded.  Regardless of how the thread previously stopped, it resumes
     * as though it had received the specified event.  This mode is only supported by back-ends
     * that have the BACKEND_FEATURE_CONTINUE_WITH_NEW_EVENT feature.
     */
    CONTINUE_MODE_GO_NEW_EVENT,

    /*!
     * This is exactly like CONTINUE_MODE_GO_SQUASH_EVENT except the thread stops
     * after executing a single instruction.
     */
    CONTINUE_MODE_STEP_SQUASH_EVENT,

    /*!
     * This is exactly like CONTINUE_MODE_GO_PASS_EVENT except the thread stops
     * after executing one instruction.  If the event causes the thread to enter
     * a handler, execution stops before the first instruction of that handler.
     */
    CONTINUE_MODE_STEP_PASS_EVENT,

    /*!
     * This is exactly like CONTINUE_MODE_GO_NEW_EVENT except the thread stops
     * after executing one instruction.  If the event causes the thread to enter
     * a handler, execution stops before the first instruction of that handler.
     * This mode is only supported by back-ends that have the
     * BACKEND_FEATURE_CONTINUE_WITH_NEW_EVENT feature.
     */
    CONTINUE_MODE_STEP_NEW_EVENT
};

/*!
 * Possible reasons why an application thread might stop.
 */
enum STOP_REASON
{
    STOP_REASON_INVALID,

    /*!
     * Thread triggered a breakpoint.  This can only happen for breakpoints set
     * via IBREAKPOINTS::SetBreakpoint(), not for breakpoints set by overwriting an
     * instruction with a software trap.
     */
    STOP_REASON_BREAKPOINT,

    /*!
     * Thread triggered a breakpoint that was initiated by the target.  Such breakpoints
     * do not correspond to IBREAKPOINTS::SetBreakpoint() nor do they correspond to
     * breakpoints set when the debugger front-end overwrites an instruction with a
     * software trap.  The front-end can get additional information about these
     * breakpoints via ICUSTOM_BREAKPOINTS::GetThreadStopMessage().
     */
    STOP_REASON_CUSTOM_BREAK,

    /*!
     * Thread was stopped by the back-end, for example in response to a call
     * to ICOMMANDS::Break() or because some other thread triggered a breakpoint.
     */
    STOP_REASON_STOPPED,

    STOP_REASON_STEP,  ///< Thread completed a single-step.
    STOP_REASON_EVENT, ///< Thread received an O/S event, see EVENT.

    /* Notifications enabled by ITHREAD_EXTENSIONS::SetThreadNotifications() */

    /*!
     * New thread started and is stopped at the PC of its first instruction.
     */
    STOP_REASON_THREAD_START,

    /*!
     * A thread is about to exit and is stopped at the PC of its last instruction
     * (usually a system call which terminates the thread).  The thread will truly
     * exit when it is next continued or single-stepped.  Any attempt to change the
     * thread's registers will be ignored.
     */
    STOP_REASON_THREAD_EXIT,

    /* Notifications enabled by IIMAGE_EXTENSIONS::SetImageNotifications() */

    STOP_REASON_IMAGE_LOAD,   ///< Image loaded, see IIMAGE_EXTENSIONS::GetThreadStopImage().
    STOP_REASON_IMAGE_UNLOAD, ///< Image unloaded, see IIMAGE_EXTENSIONS::GetThreadStopImage().

    /*!
     * More than one image was loaded or unloaded at the same time.  Use
     * IIMAGE_EXTENSIONS::GetImageCount() and IIMAGE_EXTENSIONS::GetImageId() to get the new
     * set of loaded images.
     */
    STOP_REASON_IMAGE_MULTIPLE
};

/*!
 * Return status for some methods that block.
 */
enum RESULT
{
    RESULT_OK,         ///< Method completed successfully.
    RESULT_ERROR,      ///< Method failed.
    RESULT_INTERRUPTED ///< Method was interrupted.
};

/*!
 * Possible return values from IBACKEND::HandleCommands() when a debugger
 * session completes.
 */
enum SESSION_RESULT
{
    SESSION_RESULT_TERMINATED,  ///< Target process terminated or was killed by the front-end.
    SESSION_RESULT_DETACHED,    ///< Front-end detached via ICOMMANDS::Detach().
    SESSION_RESULT_INTERRUPTED, ///< Session interrupted via IBACKEND::InterruptCommandThread().
    SESSION_RESULT_ERROR        ///< Communication failure with the front-end.
};

/*!
 * Flags used with IFRONTEND::InvalidateCachedState() to tell what information should be
 * invalidated.
 */
enum INVALIDATE_FLAG
{
    INVALIDATE_FLAG_REGISTERS = (1 << 0) ///< Invalidate the register values for a single thread.
};
typedef UINT64 INVALIDATE_FLAGS; ///< Bit mask of INVALIDATE_FLAG's.

/*!
 * Information describing how to connect to an endpoint using a TCP transport.
 */
struct /*<POD>*/ TCP_INFO
{
    int _localPort; ///< The local port of a socket listening for a connection.
};

/*!
 * Describes a single register in the target program.
 */
struct /*<POD>*/ REG_DESCRIPTION
{
    unsigned _widthInBits;     ///< Width (bits) of the register.  Currently, the width
                               ///<  must be a multiple of 8.
    REG _correspondingGeneric; ///< The corresponding "generic" register ID, if any.  Or,
                               ///<  REG_INVALID, if there is no corresponding generic.
                               ///<  There should be exactly one register that corresponds
                               ///<  to each generic.  (E.g. there should be exactly one
                               ///<  REG_PC register.)
    bool _accelerated;         ///< If TRUE, the back-end automatically sends the value
                               ///<  of this register whenever the target program stops.
                               ///<  This saves a protocol request if the front-end
                               ///<  usually needs this register when the target stops.
    const char* _name;         ///< The textual name of the register
    int _gccId;                ///< The GCC/DWARF compiler registers number for this
                               ///< register (used for EH frame and other compiler
                               ///< information that is encoded in the executable files).
};

/*!
 * This function must be called before using any other function in the debugger-protocol
 * library.  Typically, it is called once from the application's start-up code, but it
 * is legal to call it more than once so long as each call has a matching call to
 * Cleanup().  The Initialize() and Cleanup() functions have no internal locking, so
 * callers must provide synchronization if they are called from multi-threaded code.
 *
 *  @param[in] flags    A bit-mask of flags indicating which library features the caller
 *                       intends to use.
 *
 * @return  TRUE on success.
 */
DEBUGGER_PROTOCOL_API bool Initialize(INITIALIZE_FLAGS flags);

/*!
 * This function should be called to clean up resources when the caller is done using
 * the debugger-protocol library.  The Initialize() and Cleanup() functions have no
 * internal locking, so callers must provide synchronization if they are called from
 * multi-threaded code.
 *
 *  @param[in] flags    The same flags that were passed to the matching call to
 *                       Initialize().
 *
 * @return  TRUE on success.
 */
DEBUGGER_PROTOCOL_API bool Cleanup(INITIALIZE_FLAGS flags);

/*!
 * Represents a communication endpoint for a debugger front-end.
 *
 * Clients using this interface need to be aware that the debugger alternates between
 * two modes: "run mode" and "command mode".  When the target application is running,
 * the debugger is in "run mode", and most attempts to send commands to the target
 * will fail.  Once the target application stops, the debugger enters "command mode"
 * and the user can send commands to the target process.
 */
class /*<INTERFACE>*/ IFRONTEND
{
  public:
    /*!
     * If this front-end is a server, retrieve the communication information that
     * the back-end needs in order to connect.
     *
     *  @param[out] info    Receives the connection information.
     *
     * @return  TRUE on success (only if the front-end is a server).
     */
    virtual bool GetServerInfo(TCP_INFO* info) = 0;

    /*!
     * The front-end and back-end must communicate with each other using
     * the same set of target registers.  The front-end calls this method
     * to define the register set that it will use.  The back-end should call
     * IBACKEND::SetRegisters() with the same set of registers.
     *
     * This method must be called before the front-end calls any of the
     * following ICOMMANDS methods:
     *
     *  - GetRegisterValue()
     *  - SetRegisterValue()
     *  - ContinueThreads()
     *  - Any of the GetThreadStop*() methods
     *  - GetThreadExitStatus()
     *
     * A front-end can dynamically choose the register set if the back-end
     * supports the feature BACKEND_FEATURE_TARGET_DESCRIPTION.  To do this, the
     * front-end can call ITARGET_DESCRIPTION::GetTargetDescription() to query
     * the supported registers, and then dynamically create \a regs based on the
     * returned description.  Front-ends that support this ability should define
     * the feature FRONTEND_FEATURE_DYNAMIC_REGISTERS.
     *
     *  @param[in] regs     Describes the register set to use when
     *                       communicating with the back-end.  This
     *                       can either be a predefined description from
     *                       "debugger-protocol/regs-XXX.hpp" or it can
     *                       be a custom description.
     *  @param[in] nregs    The number of entries in \a regs.
     *
     * @return  TRUE on success.  FALSE if there is an error with \a regs or
     *           if registers have already been accessed via one of the ICOMMANDS
     *           methods listed above.
     */
    virtual bool SetRegisters(const REG_DESCRIPTION* regs, unsigned nregs) = 0;

    /*!
     * If this front-end is a client, connect to a back-end which must be a server.
     * This method should not be called for server-like front-ends.
     *
     * After connecting, the target process is stopped and the debugger is in
     * "command mode".
     *
     *  @param[in] ip        The IP address of the machine running the back-end.
     *  @param[in] port      The port number for the back-end.
     *
     * @return  On success, an object that can be used to send commands to the back-end.
     *           NULL on failure.
     */
    virtual ICOMMANDS* ConnectToServer(const std::string& ip, int port) = 0;

    /*!
     * If this front-end is a server, wait for a connection from a back-end which
     * must be a client.  This method should not be called for client-like front-ends.
     *
     * After connecting, the target process is stopped and the debugger is in
     * "command mode".
     *
     *  @param[in] timeout  A timeout value (milliseconds).  This method fails if
     *                       the back-end doesn't connect in the timeout period.
     *                       If \a timeout is zero, wait forever.
     *
     * @return  On success, an object that can be used to send commands to the back-end.
     *           NULL on failure.
     */
    virtual ICOMMANDS* ConnectFromClient(unsigned timeout) = 0;

    /*!
     * Tells the type of the back-end client.  This is only useful after
     * a successful call to ConnectToServer() or ConnectFromClient().
     *
     * @return  The type of the back-end client, or BACKEND_TYPE_UNKNOWN if not
     *           connected.
     */
    virtual BACKEND_TYPE GetBackEndType() = 0;

    /*!
     * Tells the set of features supported by the back-end.  This is only useful after
     * a successful call to ConnectToServer() or ConnectFromClient().
     *
     * @return  If called after successfully connecting to the back-end, returns a
     *           bitwise 'or' of features supported by the back end.  If called before
     *           a successful connect, returns BACKEND_FEATURE_NONE.
     */
    virtual BACKEND_FEATURES GetBackEndFeatures() = 0;

    /*!
     * @return  If called after successfully connecting to the back-end, returns the
     *           O/S type of the target process.
     */
    virtual OS GetBackEndOs() = 0;

    /*!
     * This method should be called when the debugger is in "run mode" to check for
     * notifications from the back-end.  If \a block is TRUE, this call waits until
     * a notification is sent from the back-end.  If \a block is FALSE, this call
     * checks for a notification and returns immediately.
     *
     * When there is a notification, this methods calls exactly one of the methods
     * in \a receiver to indicate why the target stopped or exited.
     *
     *  @param[in] receiver     An object that receives notifications from
     *                           the debugger back-end.
     *  @param[in] block        If TRUE, call blocks until there is a notification.
     *
     * @return  TRUE on success, FALSE on communication error.  Note that this method
     *           returns TRUE even when there is no notification (e.g. when \a block
     *           is FALSE or when InterruptNotificationThread() interrupts this method).
     */
    virtual bool HandleNotifications(INOTIFICATIONS* receiver, bool block) = 0;

    /*!
     * Interrupt the thread executing the HandleNotifications() method, causing it to
     * return with TRUE status.  The InterruptNotificationThread() method returns
     * immediately, without waiting for HandleNotifications() to return.
     *
     * Calling this method enables a "sticky" interrupt flag.  Therefore, if no thread
     * is blocked in HandleNotifications(), the next call to that method will return
     * immediately.  The sticky interrupt flag is cleared by calling ClearInterruptFlag()
     * or ICOMMANDS::ContinueThreads().
     *
     * The following two usage models are supported.  The front-end may call this method
     * from a separate thread while another thread is blocked in HandleNotifications().
     * Or, when used on Unix, the front-end thread that is blocked in HandleNotifications()
     * may set up a signal handler that calls InterruptNotificationThread().
     */
    virtual void InterruptNotificationThread() = 0;

    /*!
     * Clears the "sticky" interrupt flag that is set by InterruptNotificationThread().
     * This method has no effect if the flag is not set.
     */
    virtual void ClearInterruptFlag() = 0;

    /*!
     * The front-end object may keep some information about the state of the target process
     * cached in order to get this information more quickly in the future.  The front-end
     * object will automatically invalidate this cached information whenever the target
     * resumes execution, so normally clients using the front-end don't need to be aware
     * of this caching.  However, if the client changes the target state in a way that
     * the front-end can't observe, the client should call InvalidateCachedState() to
     * explicitly invalidate any cached information.
     *
     * One scenario where this is required is when the client uses ICUSTOM_COMMANDS to
     * change the state of the target process.  Since the front-end object doesn't know
     * the side-effect of commands sent this way, the client is responsible for calling
     * InvalidateCachedState() if the custom command changes the target's state.
     *
     *  @param[in] thread   Tells which thread's information to invalidate.  This parameter
     *                       may be ignored, depending on the value of \a flags.
     *  @param[in] flags    Bit-mask of flags telling what information to invalidate.
     */
    virtual void InvalidateCachedState(THREAD thread, INVALIDATE_FLAGS flags) = 0;

    /*!
     * Register a listener that receives log messages about the debugger protocol.  This
     * is useful for debugging the debugger-protocol itself.
     *
     *  @param[in] receiver     Object that receives notifications.
     */
    virtual void AddDebugLogReceiver(ILOG_NOTIFICATIONS* receiver) = 0;

    /*!
     * Unregister a listener, so it no longer receives log messages.
     *
     *  @param[in] receiver     Object previously registered via AddDebugLogReceiver().
     */
    virtual void RemoveDebugLogReceiver(ILOG_NOTIFICATIONS* receiver) = 0;

    /*!
     * The destructor automatically disconnects from the back-end and unregisters any
     * listeners registered via AddDebugLogReceiver().
     */
    virtual ~IFRONTEND() {}
};

/*!
 * Create a new debugger front-end communication endpoint, which uses the GDB
 * remote protocol over a TCP transport.  A front-end typically connects to a
 * back-end in another process.  Either the front-end or the back-end must act
 * as a communication "server" and the other must act as a "client".  The
 * server establishes the communication channel and waits for the client to
 * connect to it.
 *
 * Before calling this function, Initialize() must be called with the flag
 * INITIALIZE_FLAG_TCP.
 *
 *  @param[in] isServer     Tells if the front-end acts as a server.
 *  @param[in] type         The type of the front-end client.
 *  @param[in] features     A bitwise 'or' of features that the front-end supports.
 *  @param[in] options      A bitwise 'or' of options that affect the front-end.
 *
 * @return  A new debugger front-end object.
 */
DEBUGGER_PROTOCOL_API IFRONTEND* CreateFrontEnd(bool isServer, FRONTEND_TYPE type, FRONTEND_FEATURES features,
                                                ENDPOINT_OPTIONS options);

/*!
 * Represents a communication endpoint for a debugger back-end.
 *
 * The IBACKEND object has only limited thread safety.  Aside from the following cases,
 * the caller must ensure that only one thread at a time calls into the IBACKEND object:
 *
 *  - It is permissible for a thread to call IBACKEND::InterruptCommandThread() while
 *    another thread calls any IBACKEND method.
 *  - It is permissible for a thread to call any INOTIFICATIONS method while another
 *    thread calls any IBACKEND method.
 */
class /*<INTERFACE>*/ IBACKEND
{
  public:
    /*!
     * If this back-end is a server, retrieve the communication information that
     * the front-end needs in order to connect.
     *
     *  @param[out] info    Receives the connection information.
     *
     * @return  TRUE on success (only if the back-end is a server).
     */
    virtual bool GetServerInfo(TCP_INFO* info) = 0;

    /*!
     * Tell the backend to disable some features.
     * Note that this must be called before the frontend was connected to
     * this backend.
     *
     *  @param[out] maskFeatures   Mask of backend features to disable
     *
     */
    virtual void MaskFeatures(BACKEND_FEATURES maskFeatures) = 0;

    /*!
     * This method defines the register set used to communicate with the
     * front-end.  It must be called before calling HandleCommands().
     *
     * If a back-end does NOT support BACKEND_FEATURE_TARGET_DESCRIPTION,
     * it can call SetRegisters() even before connecting with the front-end.
     *
     * If a back-end DOES support BACKEND_FEATURE_TARGET_DESCRIPTION, it
     * should wait until after connecting with the front-end.  Once connected,
     * query the front-end's feature with GetFrontEndFeatures() and see if
     * it supports FRONTEND_FEATURE_DYNAMIC_REGISTERS.  If not, call
     * SetRegisters() with a default set of registers.  If it does, call
     * SetRegisters() where \a regs matches the target description sent
     * to the front-end.
     *
     *  @param[in] regs     Describes the register set to use when
     *                       communicating with the front-end.  This
     *                       can either be a predefined description from
     *                       "debugger-protocol/regs-XXX.hpp" or it can
     *                       be a custom description.
     *  @param[in] nregs    The number of entries in \a regs.
     *
     * @return  TRUE on success.  FALSE if there is an error with \a regset
     *           or if SetRegisters() is called too late.
     */
    virtual bool SetRegisters(const REG_DESCRIPTION* regs, unsigned nregs) = 0;

    /*!
     * Retrieve an interface that can be used to send notifications to the debugger
     * front-end.  This method may be called even before the back-end is connected
     * to a front-end.
     *
     * @return  An interface that can be used to send notifications to the debugger
     *           front-end.
     */
    virtual INOTIFICATIONS* GetNotificationListener() = 0;

    /*!
     * This method is the first step when connecting to a front-end from a client-like
     * back-end.  When you use this method, the front-end must be a server.  The
     * connection sequence is as follows:
     *
     *  - Call ConnectToServer() to connect with the front-end.
     *
     *  - Tell the target process to stop and then call WaitForStopAfterConnect().
     *    The target process should call an INOTIFICATIONS method to indicate
     *    that it has stopped or exited.  If the target process is already stopped,
     *    you may call INOTIFICATIONS before calling WaitForStopAfterConnect().
     *
     *  - This is a good time to do any initialization that depends on the type
     *    or features of the front-end.  In particular, this is a good time to call
     *    SetRegisters() if you have not already done so.
     *
     *  - Call HandleCommands() to handle commands from the front-end.
     *
     *  @param[in] ip           The IP address of the machine running the front-end.
     *  @param[in] port         The port number for the front-end.
     *
     * @return  TRUE on success, FALSE on communication error.
     */
    virtual bool ConnectToServer(const std::string& ip, int port) = 0;

    /*!
     * This method is the first step when connecting to a front-end from a server-like
     * back-end.  When you use this method, the front-end must be a client.  The
     * connection sequence is as follows:
     *
     *  - Call ConnectFromClient() to connect with the front-end.
     *
     *  - Tell the target process to stop and then call WaitForStopAfterConnect().
     *    The target process should call an INOTIFICATIONS method to indicate
     *    that it has stopped or exited.  If the target process is already stopped,
     *    you may call INOTIFICATIONS before calling WaitForStopAfterConnect().
     *
     *  - This is a good time to do any initialization that depends on the type
     *    or features of the front-end.  In particular, this is a good time to call
     *    SetRegisters() if you have not already done so.
     *
     *  - Call HandleCommands() to handle commands from the front-end.
     *
     * This method might block indefinitely while waiting for the front-end to
     * connect, but it can be interrupted when another thread calls any of the
     * following methods:
     *
     *  - InterruptCommandThread()
     *  - INOTIFICATIONS::NotifyTerminatedExit()
     *  - INOTIFICATIONS::NotifyTerminatedEvent()
     *
     * After being interrupted, you may call ConnectFromClient() again to resume
     * waiting for the front-end to connect.
     *
     * @return  RESULT_OK on success, RESULT_ERROR on communication error, or
     *           RESULT_INTERRUPTED if interrupted.
     */
    virtual RESULT ConnectFromClient() = 0;

    /*!
     * Tells the type of the front-end client.  This is only useful after connecting
     * to the front-end via ConnectToServer() or ConnectFromClient().
     *
     * @return  The type of the front-end client, or FRONTEND_TYPE_UNKNOWN if not
     *           connected.
     */
    virtual FRONTEND_TYPE GetFrontEndType() = 0;

    /*!
     * Tells the set of features supported by the front-end.  This is only useful after
     * connecting to the front-end via ConnectToServer() or ConnectFromClient().
     *
     * @return  If called after successfully connecting to the front-end, returns a
     *           bitwise 'or' of features supported by the front-end.  If called before
     *           a successful connect, returns FRONTEND_FEATURE_NONE.
     */
    virtual FRONTEND_FEATURES GetFrontEndFeatures() = 0;

    /*!
     * This is the second step when connecting the back-end to a front-end.  See
     * ConnectToServer() or ConnectFromClient() for the connection sequence.  This
     * method blocks until one of the following occurs:
     *
     *  - The target process stops or exits and calls one of the INOTIFICATIONS methods.
     *  - Another thread calls InterruptCommandThread().
     *  - The timeout expires.
     *
     * If this method is interrupted via InterruptCommandThread(), you may call
     * WaitForStopAfterConnect() again to resume waiting for the application to stop.
     *
     *  @param[in] timeout  Timeout period (milliseconds).  The value zero means wait
     *                       forever.
     *
     * @return  RESULT_OK if the process stops or exits, RESULT_ERROR on timeout, or
     *           RESULT_INTERRUPTED if interrupted.
     */
    virtual RESULT WaitForStopAfterConnect(unsigned timeout) = 0;

    /*!
     * This is the last step when connecting the back-end to a front-end.  See
     * ConnectToServer() or ConnectFromClient() for the connection sequence.  This
     * method enters a loop that handles commands sent from the front-end.  Since
     * it doesn't return until the debugger session completes, it is typically called
     * from a dedicated thread.
     *
     * This method returns when one of the following occurs:
     *
     *  - The process exits and sends a notification via INOTIFICATIONS.
     *    If the process is already exited when HandleCommands() is called,
     *    this method returns immediately.
     *
     *  - The front-end kills the target process.
     *
     *  - The front-end detaches from the target process.
     *
     *  - Another thread calls InterruptCommandThread().
     *
     *  - There is a communication failure with the front-end (e.g. the
     *    front-end terminates unexpectedly).
     *
     * If this method is interrupted via InterruptCommandThread(), you may call
     * HandleCommands() again to resume handling commands from the front-end.
     *
     *  @param[in] receiver     An object that receives and responds to commands
     *                           from the debugger front-end.
     *
     * @return  A return code which tells why the debugger session completed.
     */
    virtual SESSION_RESULT HandleCommands(ICOMMANDS* receiver) = 0;

    /*!
     * Reset the debugger communication channel after a fork.  If the debugger
     * back-end process does a fork(), this must be called by the child of the
     * fork().  Since forking is specific to Unix hosts, this method is only
     * relevant on Unix.
     *
     * If this is a server-like back-end, the \a followChild parameter controls
     * whether a new debugger communication channel is created for the child process.
     * If \a followChild is TRUE, the child process may connect to a new front-end
     * via ConnectFromClient().  Note that the back-end in the child process is
     * distinct from the back-end in the parent process, and the child process
     * should call GetServerInfo() to get the new connection information.
     *
     * Client-like back-ends do not support the \a followChild parameter, and
     * passing TRUE will result in an error return from ResetAfterFork().
     *
     * Regardless of the setting of \a followChild, the child process should
     * eventually call the ~IBACKEND() destructor in order to clean up resources.
     *
     * Note that there are generally no multi-thread safety issues when calling
     * ResetAfterFork() because a Unix process has only a single thread after a fork().
     *
     *  @param[in] followChild      If TRUE, a new communication endpoint is created
     *                               in the child process.  If FALSE, the endpoint
     *                               is disabled in the child process.
     */
    virtual bool ResetAfterFork(bool followChild) = 0;

    /*!
     * Interrupt another thread blocked in ConnectFromClient(), WaitForStopAfterConnect(),
     * or HandleCommands().  The InterruptCommandThread() method returns immediately,
     * without waiting for the blocked thread to return.
     */
    virtual void InterruptCommandThread() = 0;

    /*!
     * If this is a server-like back-end, you may call this method to reset the back-end,
     * allowing it to connect to a new front-end.  This is typically called after HandleCommands()
     * returns with SESSION_RESULT_DETACHED in order to allow the back-end to connect to a
     * new front-end (or to reconnect to the same front-end).
     *
     * This method does not change the back-end's connection information, so GetServerInfo()
     * will return the same information as before this call.
     *
     * This method must be serialized with any other thread's usage of this IBACKEND object.
     * No other thread may call InterruptCommandThread() or any INOTIFICATIONS method while
     * this thread calls ResetServerConnection().
     *
     * @return  TRUE on success.
     */
    virtual bool ResetServerConnection() = 0;

    /*!
     * Register a listener that receives log messages about the debugger protocol.  This
     * is useful for debugging the debugger-protocol itself.
     *
     *  @param[in] receiver     Object that receives notifications.
     */
    virtual void AddDebugLogReceiver(ILOG_NOTIFICATIONS* receiver) = 0;

    /*!
     * Unregister a listener, so it no longer receives log messages.
     *
     *  @param[in] receiver     Object previously registered via AddDebugLogReceiver().
     */
    virtual void RemoveDebugLogReceiver(ILOG_NOTIFICATIONS* receiver) = 0;

    /*!
     * If there is an active connection to the front-end when the destructor is called,
     * the connection is ungracefully closed.
     */
    virtual ~IBACKEND() {}
};

/*!
 * Create a new debugger back-end communication endpoint, which uses the GDB
 * remote protocol over a TCP transport.  A back-end typically connects to a
 * front-end in another process.  Either the front-end or the back-end must act
 * as a communication "server" and the other must act as a "client".  The
 * server establishes the communication channel and waits for the client to
 * connect to it.
 *
 * Before calling this function, Initialize() must be called with the flag
 * INITIALIZE_FLAG_TCP.
 *
 *  @param[in] isServer     Tells if the back-end acts as a server.
 *  @param[in] os           The O/S type of the target program.
 *  @param[in] type         The type of the back-end client.
 *  @param[in] features     A bitwise 'or' of features supported by this back-end.
 *  @param[in] options      A bitwise 'or' of options that affect the back-end.
 *  @param[in] props        Debugger connection properties (depends on the debugger type).
 *
 * @return  A new debugger back-end object.
 */
DEBUGGER_PROTOCOL_API IBACKEND* CreateBackEnd(bool isServer, OS os, BACKEND_TYPE type, BACKEND_FEATURES features,
                                              ENDPOINT_OPTIONS options, const DEBUGGER_CONNECTION_PROPERTIES& props);

/*!
 * This interface represents the basic commands that a debugger front-end can send to
 * the back-end.  Most of the methods in this interface may only be called when the
 * debugger is in "command mode", when all threads in the target process are stopped.
 * There are two different usage models.
 *
 * When used by a debugger front-end, the DEBUGGER_PROTOCOL library implements this
 * interface, and the front-end makes calls to it in order to send commands to the
 * back-end.  In this model, the front-end should expect that a call to a method in this
 * interface may fail due to a communication error with the back-end.  The "Error Returns"
 * section of each method describes this.
 *
 * When used by a back-end, the back-end implements this interface and the DEBUGGER_PROTOCOL
 * library calls it in order to notify the back-end of commands received from the front-end.
 * The DEBUGGER_PROTOCOL library will never incorrectly call a method when not in "command
 * mode", so the back-end need not check for this error condition.
 */
class /*<INTERFACE>*/ ICOMMANDS
{
  public:
    /*!
     * @return  The current number of threads in the application.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns zero on communication error, or if called during "run mode".
     */
    virtual unsigned GetThreadCount() = 0;

    /*!
     * Retrieves the ID of a thread in the application.
     *
     *  @param[in] iThread  An index in the range [0, n-1], where \e n is the value
     *                       returned by GetThreadCount().
     *
     * @return  The ID of the indexed thread, invalid thread Id if \a iThread is out of range.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns invalid thread Id if called during "run mode".
     */
    virtual THREAD GetThreadId(unsigned iThread) = 0;

    /*!
     * Retrieves the ID of a thread in the application that caused current stop
     * or invalid thread Id if the focus thread can not be safely determined,
     * like when all stopped threads are blocked in OS.
     *
     * @return  The ID of the focus thread as determined by the back-end.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns invalid thread Id if called during "run mode".
     */
    virtual THREAD GetFocusThreadId() = 0;

    /*!
     * Tells if a thread still exists in the application.
     *
     *  @param[in] thread   ID of a thread.
     *
     * @return  TRUE if the thread still exists.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE on communication error, or if called during "run mode".
     */
    virtual bool DoesThreadExist(THREAD thread) = 0;

    /*!
     * Tells how a thread should continue on the next call to ContinueThreads().
     * These settings remain in effect only until the next call to ContinueThreads().
     * Any thread with no explicit setting behaves as though \a mode is CONTINUE_MODE_FROZEN.
     *
     *  @param[in] thread       ID of a thread.
     *  @param[in] mode         Tells how to continue the thread.
     *  @param[in] evnt         If \a mode is CONTINUE_MODE_GO_NEW_EVENT or CONTINUE_MODE_STEP_NEW_EVENT,
     *                           \a evnt specifies the new event.
     *  @param[in] szEvntInfo   Size (bytes) of the \a evntInfo structure, if that parameter is non-NULL.
     *  @param[in] evntInfo     If the back-end supports BACKEND_FEATURE_EVENT_INFO and \a mode is
     *                           CONTINUE_MODE_GO_NEW_EVENT or CONTINUE_MODE_STEP_NEW_EVENT, \a evntInfo
     *                           gives additional information for the new event.  If \a evntInfo is
     *                           NULL, the back-end chooses a "generic" event of type \a evnt.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE if:
     *      - Communication error with back-end.
     *      - Called during "run mode".
     *      - The \a evntInfo parameter is non-NULL and the back-end does not support
     *        BACKEND_FEATURE_EVENT_INFO.
     *      - The \a mode is CONTINUE_MODE_GO_NEW_EVENT or CONTINUE_MODE_STEP_NEW_EVENT,
     *        and the back-end does not support BACKEND_FEATURE_CONTINUE_WITH_NEW_EVENT.
     */
    virtual bool SetContinueMode(THREAD thread, CONTINUE_MODE mode, EVENT evnt, size_t szEvntInfo,
                                 const EVENT_INFO* evntInfo) = 0;

    /*!
     * Let all threads continue execution according to the settings made via
     * SetContinueMode().  It is an error to call this method if all threads
     * are set to CONTINUE_MODE_FROZEN.  This method returns immediately, without
     * waiting for the threads to stop again.  Calling this method puts the
     * debugger in "run mode".  When the target process stops again, the debugger
     * front-end is notified via one of the INOTIFICATIONS methods.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE on communication error, of if called during "run mode".
     */
    virtual bool ContinueThreads() = 0;

    /*!
     * Tells why a thread stopped.  If a thread was frozen for the last call to ContinueThreads(),
     * its stop reason remains unchanged.
     *
     *  @param[in] thread       The ID of a thread.
     *  @param[out] reason      Tells why the thread stopped.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE on communication error, or if called during "run mode".
     */
    virtual bool GetThreadStopReason(THREAD thread, STOP_REASON* reason) = 0;

    /*!
     * Tells the O/S event that caused a thread to stop.
     *
     *  @param[in] thread       The ID of a thread.
     *  @param[out] evnt        Receives the O/S specific event that caused the thread
     *                           to stop.
     *  @param[in] szEvntInfo   Size (bytes) of the \a evntInfo structure if that parameter
     *                           is non-NULL.
     *  @param[out] evntInfo    If the back-end supports BACKEND_FEATURE_EVENT_INFO,
     *                           \a evntInfo may be non-NULL, in which case it receives
     *                           additional information about the event.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE if:
     *      - Communication error with back-end.
     *      - Called during "run mode".
     *      - The \a thread did not stop with STOP_REASON_EVENT.
     *      - The \a evntInfo parameter is non-NULL and the back-end does not
     *        support BACKEND_FEATURE_EVENT_INFO.
     */
    virtual bool GetThreadStopEvent(THREAD thread, EVENT* evnt, size_t szEvntInfo, EVENT_INFO* evntInfo) = 0;

    /*!
     * This is the only method that may be called during "run mode".  It tells
     * the target process to stop, but does not wait for it to do so.  When the
     * target does stop, the debugger front-end is notified via one of the
     * INOTIFICATIONS methods.
     *
     * This method has no effect if called during "command mode".
     *
     * When called from a debugger front-end, the following two usage models are
     * supported.  The front-end may call this method from a separate thread while
     * another thread is blocked in IFRONTEND::HandleNotifications().  Or, when used
     * on Unix, the front-end thread that is blocked in IFRONTEND::HandleNotifications()
     * may set up a signal handler that calls Break().
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE on communication error.
     */
    virtual bool Break() = 0;

    /*!
     * Force all threads in the target application to terminate, killing the process.
     * There is NO notification sent via INOTIFICATIONS.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE on communication error, or if called during "run mode".
     */
    virtual bool Kill() = 0;

    /*!
     * Detach the debugger from the target application, and resume all threads.  Threads no
     * longer stop at breakpoints or notify the debugger front-end when an O/S event is received,
     * etc.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE on communication error, or if called during "run mode".
     */
    virtual bool Detach() = 0;

    /*!
     * Get the value of a register.
     *
     *  @param[in] thread   An ID for a thread.
     *  @param[in] reg      An ID for a register.  When called from a debugger front-end
     *                       \a reg may be a generic register ID.  However, the
     *                       DEBUGGER_PROTOCOL library never passes a generic ID, so the
     *                       back-end need not support this.
     *  @param[out] value   Receives the register's value.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE on communication error, or if called during "run mode".
     */
    virtual bool GetRegisterValue(THREAD thread, REG reg, UTIL::REGVALUE* value) = 0;

    /*!
     * Set the value of a register.
     *
     *  @param[in] thread   An ID for a thread.
     *  @param[in] reg      An ID for a register.  When called from a debugger front-end
     *                       \a reg may be a generic register ID.  However, the
     *                       DEBUGGER_PROTOCOL library never passes a generic ID, so the
     *                       back-end need not support this.
     *  @param[in] value    The new value for the register.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE on communication error, or if called during "run mode".
     */
    virtual bool SetRegisterValue(THREAD thread, REG reg, const UTIL::REGVALUE& value) = 0;

    /*!
     * Get the value of a memory region.
     *
     *  @param[in] range    Address range of the memory region.
     *  @param[out] value   Value and size of the memory range (target byte order).  The
     *                       size may be less than the specified range.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE on communication error, or if called during "run mode".
     */
    virtual bool GetMemoryValue(const UTIL::ANYRANGE& range, UTIL::DATA* value) = 0;

    /*!
     * Set the value of a memory range.
     *
     *  @param[in] addr             Starting address of the memory range.
     *  @param[in] value            Value and size of the data to write at that memory
     *                               range (target byte order).
     *  @param[out] sizeWritten     Receives the number of bytes successfully written,
     *                               which might be less than the size of \a value.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE on communication error, or if called during "run mode".
     */
    virtual bool SetMemoryValue(ANYADDR addr, const UTIL::DATA& value, size_t* sizeWritten) = 0;

    /*!
     * A back-end can provide some optional functionality through interfaces returned
     * by this method.  To define an optional interface, the back-end should also define
     * the corresponding BACKEND_FEATURE for that interface.
     *
     *  @param[in] id   An ID that tells which interface to return.
     *
     * @return  A pointer to an object that implements the requested interface.  Callers
     *           should cast this pointer to the appropriate interface.  If the back-end
     *           does not implement the interface, this method returns NULL.
     */
    virtual void* GetInterface(INTERFACE_ID id) = 0;

  protected:
    virtual ~ICOMMANDS() {} ///< Do not call delete on ICOMMANDS.
};

/*!
 * An optional interface that can be returned from ICOMMANDS::GetInterface() if the
 * back-end defines BACKEND_FEATURE_BREAKPOINTS.  This interface allows a front-end
 * to set breakpoints in a target-independent way.  The back-end decides how to implement
 * the breakpoint and notifies the front-end via STOP_REASON_BREAKPOINT.
 */
class /*<INTERFACE>*/ IBREAKPOINTS
{
  public:
    /*!
     * Ask the back-end to set a breakpoint.
     *
     * The breakpoint may have either hardware or software semantics.  A non-zero
     * value for \a bpSize indicates a software semantic breakpoint.  This type of
     * breakpoint behaves as though the instruction at address \a addr was overwritten
     * by a trap instruction whose size is \a bpSize bytes.  After hitting a software
     * semantic breakpoint, the PC points \a bpSize bytes after the breakpoint location.
     * When resuming, the debugger front-end should clear the breakpoint, back up the
     * PC by \a bpSize bytes, and single-step over the breakpoint location.
     *
     * A zero value of \a bpSize indicates a hardware semantic breakpoint.  After
     * hitting this type of breakpoint, the PC points at the breakpoint location.  The
     * debugger front-end may resume from a hardware semantic breakpoint simply by
     * single-stepping or continuing.  It need not clear the breakpoint first.
     *
     * At most, only one breakpoint may be set at an address.  If there is a previous
     * breakpoint at \a addr, SetBreakpoint() overwrites it.
     *
     *  @param[in] addr     Address of the breakpoint.
     *  @param[in] bpSize   If zero, breakpoint has hardware semantics.  Otherwise,
     *                       it has software semantics, and the PC is bumped \a bpSize
     *                       bytes forward when the breakpoint is triggered.
     *  @param[out] wasSet  Receives TRUE if the breakpoint was successfully set.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE on communication error, or if called during "run mode".
     */
    virtual bool SetBreakpoint(ANYADDR addr, size_t bpSize, bool* wasSet) = 0;

    /*!
     * Ask the back-end to clear a breakpoint that was previously set via SetBreakpoint().
     *
     *  @param[in] addr         Address of the breakpoint.
     *  @param[out] wasCleared  Receives TRUE if \a addr corresponds to a previous
     *                           breakpoint and it was successfully cleared.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE on communication error, or if called during "run mode".
     */
    virtual bool ClearBreakpoint(ANYADDR addr, bool* wasCleared) = 0;

  protected:
    virtual ~IBREAKPOINTS() {} ///< Do not call delete on IBREAKPOINTS.
};

/*!
 * An optional interface that can be returned from ICOMMANDS::GetInterface() if the
 * back-end defines BACKEND_FEATURE_CUSTOM_BREAKPOINTS.  Back-ends that support this
 * interface support the concept of a "custom breakpoint", a breakpoint that the
 * back-end can decide to trigger based on some condition.  The back-end notifies
 * the front-end that such a breakpoint has triggered via STOP_REASON_CUSTOM_BREAK.
 * The front-end can determine why the breakpoint triggered by calling
 * ICUSTOM_BREAKPOINTS::GetThreadStopMessage().  Typically, a front-end can enable or
 * disable custom a breakpoint by sending a custom command via ICUSTOM_COMMANDS.
 */
class /*<INTERFACE>*/ ICUSTOM_BREAKPOINTS
{
  public:
    /*!
     * Tells why a thread triggered a "custom breakpoint".  This is useful for threads
     * that stop with STOP_REASON_CUSTOM_BREAK.  The format of \a msg is not defined by
     * this API (it can be any format that the front-end and back-end agree upon).
     *
     * If the front-end is GDB, \a msg is a string with no trailing newline character,
     * which GDB displays to the user when process stops.  There can be embedded newline
     * characters in \a msg to print a multi-line description.
     *
     *  @param[in] thread   The ID of a thread.
     *  @param[out] msg     If the thread stopped with STOP_REASON_CUSTOM_BREAK, \a msg
     *                       describes the reason the breakpoint triggered.  If the thread
     *                       did not stop with STOP_REASON_CUSTOM_BREAK, \a msg is empty.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE on communication error, or if called during "run mode".
     */
    virtual bool GetThreadStopMessage(THREAD thread, std::string* msg) = 0;

  protected:
    virtual ~ICUSTOM_BREAKPOINTS() {} ///< Do not call delete on ICUSTOM_BREAKPOINTS.
};

/*!
 * An optional interface that can be returned from ICOMMANDS::GetInterface() if the
 * back-end defines BACKEND_FEATURE_CUSTOM_COMMANDS.
 */
class /*<INTERFACE>*/ ICUSTOM_COMMANDS
{
  public:
    /*!
     * Ask the back-end to execute a custom command.  The formats of \a cmd and
     * \a response are not defined by this API (they can be any format that the
     * front-end and back-end agree upon).
     *
     * When the front-end is GDB, \a cmd is the text string that the user types
     * after the "monitor" command and \a response is displayed verbatim as a
     * response to the command.
     *
     *  @param[in] thread           The ID of the debugger's current "focus" thread.
     *  @param[in] cmd              The command string.
     *  @param[out] cmdRecognized   Receives TRUE if the back-end recognizes \a cmd.
     *                               If the back-end feels the command has some sort
     *                               of semantic error, it should set \a cmdRecognized
     *                               to TRUE and set \a response to a description of
     *                               the error.
     *  @param[out] response        Receives a response to the command.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE on communication error, or if called during "run mode".
     */
    virtual bool DoCustomCommand(THREAD thread, const std::string& cmd, bool* cmdRecognized, std::string* response) = 0;

  protected:
    virtual ~ICUSTOM_COMMANDS() {} ///< Do not call delete on ICUSTOM_COMMANDS.
};

/*!
 * An optional interface that can be returned from ICOMMANDS::GetInterface() if the
 * back-end defines BACKEND_FEATURE_KILL_WITH_NOTIFICATION.
 */
class /*<INTERFACE>*/ IKILL_WITH_NOTIFICATION
{
  public:
    /*!
     * Tell the target application to terminate.
     *
     * This method returns immediately, putting the debugger in "run mode".  When
     * the target process actually terminates, the debugger front-end is notified
     * via INOTIFICATIONS::NotifyTerminatedExit().
     *
     *  @param[in] code     The application terminates with this exit code.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE on communication error, or if called during "run mode".
     */
    virtual bool KillWithNotification(INT64 code) = 0;

  protected:
    virtual ~IKILL_WITH_NOTIFICATION() {} ///< Do not call delete on IKILL_WITH_NOTIFICATION.
};

/*!
 * An optional interface that can be returned from ICOMMANDS::GetInterface() if the
 * back-end defines BACKEND_FEATURE_TARGET_DESCRIPTION.
 */
class /*<INTERFACE>*/ ITARGET_DESCRIPTION
{
  public:
    /*!
     * Ask the back-end for a description of the target machine's features.  The format of the
     * returned information is not defined by this API (it can be any format that the front-end
     * and back-end agree upon).  However, the intent is that the information should describe
     * the target machine's registers, and possibly other information.
     *
     * When the front-end is GDB, \a name is the name of a "target feature" XML document.
     * By convention, the first name that GDB asks for is "target.xml".  However, that
     * document may include other documents, and GDB will make subsequent calls where
     * \a name references the included documents.
     *
     *  @param[in] name         Name of the description document to return.
     *  @param[out] content     On success, receives the requested description.  If the
     *                           document \a name is unknown, \a content is set
     *                           to be empty.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE on communication error, or if called during "run mode".
     */
    virtual bool GetTargetDescription(const std::string& name, UTIL::DATA* content) = 0;

  protected:
    virtual ~ITARGET_DESCRIPTION() {} ///< Do not call delete on ITARGET_DESCRIPTION.
};

/*!
 * An optional interface that can be returned from ICOMMANDS::GetInterface() if the
 * back-end defines BACKEND_FEATURE_EVENT_INTERCEPTION.
 */
class /*<INTERFACE>*/ IEVENT_INTERCEPTION
{
  public:
    /*!
     * Notifies the back-end that the application need not notify the front-end if the
     * application receives an O/S specific event.  Instead, the application can handle
     * the event as it normally would.  This setting takes effect on the next call to
     * ContinueThreads().  These preferences are "sticky", so they remain in effect until
     * explicitly changed.
     *
     * Some targets may not support this request, or may only support it for certain
     * events.  The back-end is free to ignore the request and provide a notification for
     * the event anyway.
     *
     *  @param[in] evnt     An O/S specific event.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE if called during "run mode".
     */
    virtual bool MarkEventNotIntercepted(EVENT evnt) = 0;

    /*!
     * Notifies the back-end that the application should stop and notify the
     * front-end whenever it receives any of this target's O/S events.  This is
     * the default behavior, so this method is only needed to override a previous
     * setting made via MarkEventNotIntercepted().  This setting takes effect on the
     * next call to ContinueThreads().
     *
     * Some targets may not support intercepting all events.  In such cases, this
     * method applies only to those events that can be intercepted.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE if called during "run mode".
     */
    virtual bool MarkAllEventsIntercepted() = 0;

  protected:
    virtual ~IEVENT_INTERCEPTION() {} ///< Do not call delete on IEVENT_INTERCEPTION.
};

/*!
 * An optional interface that can be returned from ICOMMANDS::GetInterface() if the
 * back-end defines BACKEND_FEATURE_IMAGE_EXTENSIONS.
 */
class /*<INTERFACE>*/ IIMAGE_EXTENSIONS
{
  public:
    /*!
     * Set the notifications that will be triggered when the target application loads or
     * unloads images.  By default, the back-end does not trigger any notifications when
     * images are loaded or unloaded.
     *
     *  @param[in] notifications    A bit-mask indicating which events should trigger
     *                               notifications.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE if:
     *      - Communication error with back-end.
     *      - Called during "run mode".
     */
    virtual bool SetImageNotifications(IMAGE_NOTIFICATIONS notifications) = 0;

    /*!
     * Tells the number of images currently loaded in the application.
     *
     *  @param[out] count   Receives the number of images.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE if:
     *      - Communication error with back-end.
     *      - Called during "run mode".
     */
    virtual bool GetImageCount(unsigned* count) = 0;

    /*!
     * Retrieves the ID of an image in the application.
     *
     *  @param[in] iImage       An index in the range [0, n-1], where \e n is the value
     *                           provided by GetImageCount().
     *  @param[out] image       Receives the image identifier.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE if:
     *      - Communication error with back-end.
     *      - Called during "run mode".
     *      - The \a iImage is out of range.
     *
     * @par Error Returns (when implemented by debugger back-end)
     *  Returns FALSE if \a iImage is out of range.
     */
    virtual bool GetImageId(unsigned iImage, IMAGE* image) = 0;

    /*!
     * Retrieves information about an image that is currently loaded in the application.
     * Note that this may not be used to retrieve information for an image that has
     * been reported via STOP_REASON_IMAGE_UNLOAD.
     *
     *  @param[in] image    The image.
     *  @param[in] szInfo   Size (bytes) of the \a info structure.
     *  @param[out] info    Receives information about the image.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE if:
     *      - Communication error with back-end.
     *      - Called during "run mode".
     *      - The \a image is not valid.
     */
    virtual bool GetImageInfo(IMAGE image, size_t szInfo, IMAGE_INFO* info) = 0;

    /*!
     * Tells the image that was loaded or unloaded for a thread that stopped with
     * STOP_REASON_IMAGE_LOAD or STOP_REASON_IMAGE_UNLOAD.
     *
     *  @param[in] thread       The ID of a thread.
     *  @param[out] image       Receives the image identifier.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE if:
     *      - Communication error with back-end.
     *      - Called during "run mode".
     *      - The \a thread did not stop with STOP_REASON_IMAGE_LOAD or STOP_REASON_IMAGE_UNLOAD.
     *
     * @par Error Returns (when implemented  by debugger back-end)
     *  Returns FALSE if \a thread did not stop with STOP_REASON_IMAGE_LOAD or
     *  STOP_REASON_IMAGE_UNLOAD.
     */
    virtual bool GetThreadStopImage(THREAD thread, IMAGE* image) = 0;

    /*!
     * Gets the loaded images list of the application according to the SVR4 format.
     *
     *  @param[out] info        Receives the image list
     *
     * @return  TRUE on success.
     *
     */
    virtual bool GetSvr4ImageList(SVR4_IMAGES_LIST* info) = 0;

  protected:
    virtual ~IIMAGE_EXTENSIONS() {} ///< Do not call delete on IIMAGE_EXTENSIONS.
};

/*!
 * An optional interface that can be returned from ICOMMANDS::GetInterface() if the
 * back-end defines BACKEND_FEATURE_THREAD_EXTENSIONS.
 */
class /*<INTERFACE>*/ ITHREAD_EXTENSIONS
{
  public:
    /*!
     * Set the notifications that will be triggered when threads start or exit in the
     * target application.  By default, the back-end does not trigger any notifications
     * when start or exit..
     *
     *  @param[in] notifications    A bit-mask indicating which events should trigger
     *                               notifications.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE if:
     *      - Communication error with back-end.
     *      - Called during "run mode".
     */
    virtual bool SetThreadNotifications(THREAD_NOTIFICATIONS notifications) = 0;

    /*!
     * Retrieves information about a thread in the application.
     *
     *  @param[in] thread   ID of a thread.
     *  @param[in] szInfo   Size (bytes) of the \a info structure.
     *  @param[out] info    Receives information about the thread.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE if:
     *      - Communication error with back-end.
     *      - Called during "run mode".
     */
    virtual bool GetThreadInfo(THREAD thread, size_t szInfo, THREAD_INFO* info) = 0;

    /*!
     * Tells the thread exit status for a thread that stopped with STOP_REASON_THREAD_EXIT.
     *
     *  @param[in] thread   The ID of a thread.
     *  @param[out] status  Receives the O/S specific thread exit status.
     *
     * @return  TRUE on success.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE if:
     *      - Communication error with back-end.
     *      - Called during "run mode".
     *      - The \a thread did not stop with STOP_REASON_THREAD_EXIT.
     *
     * @par Error Returns (when implemented  by debugger back-end)
     *  Returns FALSE if \a thread did not stop with STOP_REASON_THREAD_EXIT.
     */
    virtual bool GetThreadExitStatus(THREAD thread, INT64* status) = 0;

  protected:
    virtual ~ITHREAD_EXTENSIONS() {} ///< Do not call delete on ITHREAD_EXTENSIONS.
};

/*!
 * An optional interface that can be returned from ICOMMANDS::GetInterface() if the
 * back-end defines BACKEND_FEATURE_PROCESS_INFO.
 */
class /*<INTERFACE>*/ IPROCESS_INFO
{
  public:
    /*!
     * Get the O/S process ID of the target application.
     *
     *  @param[out] pid     Receives the process ID.
     *
     * @par Error Returns (when used by debugger front-end)
     *  Returns FALSE if:
     *      - Communication error with back-end.
     *      - Called during "run mode".
     */
    virtual bool GetProcessId(PROCESS* pid) = 0;

    /*!
     * Retrieves the address of the loader inside the target process memory layout.
     *
     * @param[out] addr    The address of the loader.
     *
     * @return  TRUE on success.
     *
     *  Returns FALSE if:
     *      - Not supported (client interface doesn't exist).
     */
    virtual bool GetLoaderInfoAddr(ANYADDR* addr) = 0;

    /*!
     * Retrieves the target application architecture description.
     *
     * @param[out] desc    The description of the architecture (OS specific).
     *
     * @return  TRUE on success.
     *
     *  Returns FALSE if:
     *      - Not supported (client interface doesn't exist).
     */
    virtual bool GetTargetApplicationArch(std::string& desc) = 0;

  protected:
    virtual ~IPROCESS_INFO() {} ///< Do not call delete on IPROCESS_INFO.
};

/*!
 * This interface represents notifications sent from the debugger back-end to the front-end.
 * There are two different usage models.
 *
 * When used by a debugger front-end, the front-end implements this interface and the
 * DEBUGGER_PROTOCOL library calls it in order to notify the front-end of events sent from
 * the back-end.
 *
 * When used by a back-end, the DEBUGGER_PROTOCOL library implements this interface, and the
 * back-end makes calls to it when it wants to send a notification to the front-end.  In this
 * model, the back-end should expect that a call to a method may fail due to a communication
 * error with the front-end.  The "Error Returns" section of each method describes this.
 */
class /*<INTERFACE>*/ INOTIFICATIONS
{
  public:
    /*!
     * Indicates that all threads in the application stopped and the debugger back-end is
     * ready to accept commands from the front-end.  Use ICOMMANDS::GetThreadStopReason()
     * to determine why each thread stopped.  When this notification is sent, the debugger
     * changes to "command mode".
     *
     *  @param[in] focus    If the application stops because one of the threads triggered a
     *                       breakpoint, received an O/S event, or completed a single-step;
     *                       \a focus specifies that thread.  If there is more than one such
     *                       thread, one is chosen arbitrarily.  If there is no such thread,
     *                       \a focus is one arbitrarily chosen thread in the application.
     *
     * @return  TRUE.
     *
     * @par Error Returns (when used by debugger back-end)
     *  Returns FALSE on communication error, or if called during "command mode".
     */
    virtual bool NotifyStopped(THREAD focus) = 0;

    /*!
     * Indicates that the application terminated due to calling exit() or some similar
     * O/S service.  Since the application no longer exists, the back-end may not be
     * able to accept further commands.
     *
     *  @param[in] code     The application's exit code (e.g. value passed to exit()).
     *
     * @return  TRUE.
     *
     * @par Error Returns (when used by debugger back-end)
     *  Returns FALSE on communication error, or if called during "command mode".
     */
    virtual bool NotifyTerminatedExit(INT64 code) = 0;

    /*!
     * Indicates that the application terminated due to some O/S event (e.g. receiving an
     * unhandled Unix signal).  Note that some O/S's (e.g. Windows) may instead report a
     * fatal event via NotifyStopped() and then report that the application terminated via
     * a separate call to NotifyTerminatedExit().
     *
     *  @param[in] evnt         Identifies the terminating event.
     *  @param[in] szEvntInfo   Size (bytes) of the \a evntInfo structure, if that parameter
     *                           is non-NULL.
     *  @param[in] evntInfo     If not NULL, tells additional information about the terminating
     *                           event.  This parameter will always be NULL when the back-end
     *                           does not support BACKEND_FEATURE_EVENT_INFO.
     *
     * @return  TRUE.
     *
     * @par Error Returns (when used by debugger back-end)
     *  Returns FALSE on communication error, or if called during "command mode".
     */
    virtual bool NotifyTerminatedEvent(EVENT evnt, size_t szEvntInfo, const EVENT_INFO* evntInfo) = 0;

  protected:
    virtual ~INOTIFICATIONS() {} ///< Do not call delete on INOTIFICATIONS.
};

/*!
 * Clients of the DEBUGGER_PROTOCOL library can implement this interface to receive
 * a notification whenever a protocol message is sent or received.  This is useful for
 * debugging the debugger-protocol itself.
 */
class /*<INTERFACE>*/ ILOG_NOTIFICATIONS
{
  public:
    /*!
     * Indicates that a protocol message was sent or received.
     *
     *  @param[in] message  A textual description of the protocol message.
     */
    virtual void NotifyLogMessage(const std::string& message) = 0;

  protected:
    virtual ~ILOG_NOTIFICATIONS() {} ///< Do not call delete on ILOG_NOTIFICATIONS.
};

} // namespace DEBUGGER_PROTOCOL
#endif // file guard
