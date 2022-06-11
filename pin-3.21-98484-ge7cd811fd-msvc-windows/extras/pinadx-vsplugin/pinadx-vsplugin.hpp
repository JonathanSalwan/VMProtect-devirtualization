/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: pinadx-vsplugin
// <FILE-TYPE>: public header

#ifndef PINADX_VSPLUGIN_HPP
#define PINADX_VSPLUGIN_HPP

#include <windows.h>


/*!
 * This is the public header for the PinADX debugger plugin for Microsoft
 * Visual Studio 2012 and later.
 */
namespace PINADX_VSPLUGIN {

/*!
 * The PinADX debugger plugin installs a new Visual Studio debugger engine.
 * To start a debugger session using PinADX, use this debugger engine GUID
 * when calling IVsDebugger2::LaunchDebugTargets2() (or similar) in the IDE.
 *
 * WARNING: This GUID is different in every Pin kit.  Be sure to get the
 * value from this header, rather than hard-coding a value in your sources.
 * This GUID is also provided in the text file "debugger-engine-guid.txt"
 * for the benefit of automated scripts.
 *
 * {8C69E2D8-AAFA-47EE-8F97-34C247B7E53E}
 */
static const GUID EngineGuid = {0x8c69e2d8, 0xaafa, 0x47ee, {0x8f, 0x97, 0x34, 0xc2, 0x47, 0xb7, 0xe5, 0x3e}};

/*!
 * You may also define your own Visual Studio debugger engine if you want.
 * To do this, create your own debugger engine GUID and install AD7 metrics
 * in the registry under <vs11-root>\AD7Metrics\Engine\{engine-guid}.  Make
 * sure that your AD7 metrics includes an entry for 'BaseDebugMonitorId'
 * with the GUID value below.  You may then start a PinADX debugger session
 * using your own debugger engine GUID.
 *
 * WARNING: This GUID is different in every Pin kit.  Be sure to get the
 * value from this header, rather than hard-coding a value in your sources.
 * This GUID is also provided in the text file "base-debug-monitor-guid.txt"
 * for the benefit of automated scripts.
 *
 * {E8123465-F829-4326-8E73-A212AFD679DC}
 */
static const GUID BaseDebugMonitorGuid = {0xe8123465, 0xf829, 0x4326, {0x8e, 0x73, 0xa2, 0x12, 0xaf, 0xd6, 0x79, 0xdc}};

/*!
 * Use this GUID as the "source ID" when sending a DkmCustomMessage to the
 * PinADX debugger plugin.  In order to send a custom message, call
 * DkmCustomMessage::Create() using CustomMessageRequestGuid as the
 * "SourceId" parameter and using one of the CUSTOM_MESSAGE_TYPE values as
 * the "MessageCode" parameter.  Then call DkmCustomMessage::SendLower().
 *
 * {2A00D322-A6DB-401D-A389-DAE37569FAA0}
 */
static const GUID CustomMessageRequestGuid = {0x2a00d322, 0xa6db, 0x401d, {0xa3, 0x89, 0xda, 0xe3, 0x75, 0x69, 0xfa, 0xa0}};

/*!
 * All DkmCustomMessage's sent by the PinADX debugger plugin use this
 * GUID as the "source ID".  If clients receive DkmCustomMessage's from
 * multiple sources, they can use the "SourceId" attribute to tell which
 * come from the PinADX debugger plugin.
 *
 * {1353C90B-8146-4B4F-A21F-D10610EDCF4D}
 */
static const GUID CustomMessageSourceGuid = {0x1353c90b, 0x8146, 0x4b4f, {0xa2, 0x1f, 0xd1, 0x6, 0x10, 0xed, 0xcf, 0x4d}};


/*!
 * Possible "message codes" for DkmCustomMessage objects that are sent from
 * the plugin or received by the plugin.
 */
enum CUSTOM_MESSAGE_TYPE
{
    /*!
     * A custom message that represents a custom breakpoint that was raised
     * by the Pin tool via PIN_ApplicationBreakpoint().  The plugin sends
     * this message when it stops the process with a CustomStop event.  A
     * VS services can receive this message by registering to receive a
     * notification via IVsCustomDebuggerStoppingEventHandler110.
     *
     * parameter1   VT_UI4 with the O/S thread ID of the thread that
     *               raised the breakpoint.
     *
     * parameter2   VT_BSTR with the string that the tool passed to
     *               PIN_ApplicationBreakpoint().
     */
    CUSTOM_MESSAGE_TYPE_BREAKPOINT,

    /*!
     * A VS service or another debugger component can send this custom
     * message to the plugin in order to send a "custom command" to the
     * Pin tool.  If the command is successfully sent, the reply to this
     * custom message has the type CUSTOM_MESSAGE_TYPE_COMMAND_REPLY.
     *
     * parameter1   VT_UI4 with the O/S thread ID of the "focus" thread
     *               for the command.  Or,
     *              NULL to indicate that an arbitrary thread ID should
     *               be chosen.
     *
     * parameter2   VT_BSTR with the text of the command.
     */
    CUSTOM_MESSAGE_TYPE_COMMAND,

    /*!
     * When a message of type CUSTOM_MESSAGE_TYPE_COMMAND is sent to the plugin,
     * the plugin replies with this message.
     *
     * parameter1   VT_UI4 with one of the CUSTOM_COMMAND_RESULT values, telling
     *               the result of the command.
     *
     * parameter2   This parameter is only used if parameter1 is
     *               CUSTOM_COMMAND_RESULT_OK.  It is a VT_BSTR with the reply
     *               to the custom command.
     */
    CUSTOM_MESSAGE_TYPE_COMMAND_REPLY,

    /*!
     * The plugin sends this message to notify the VS service of a fatal error
     * for the debug session.  A VS service can receive this notification by
     * setting the option "error-style" to "notification" in the bstrOptions
     * parameter (see below).  The service receives the notification via
     * IVsCustomDebuggerEventHandler110.
     *
     * This error notifiation can be sent if there is a problem when launching
     * a new debug session (e.g. from IVsDebugger::LaunchDebugTargets()).  When
     * this occurs, the LaunchDebugTargets() call returns a failing HRESULT and
     * and this error notification contains a detailed description of the error.
     *
     * An error notification can also be sent after a debug session is already
     * started.  When this occurs, the debug session is terminated and the
     * error notification contains a description of the problem.
     *
     * parameter1   VT_UI4 with one of the ERROR_CODE values.
     *
     * parameter2   VT_BSTR with the arguments for the error.  Some errors
     *               have more than one argument.  In this case, each argument
     *               is separated by a NUL character.
     */
    CUSTOM_MESSAGE_TYPE_ERROR,

    /*!
     * The plugin sends this message to notify the VS service that the debugger
     * has changed the value of one or more registers in a thread in the target
     * process.  This notification is sent only when the debugger changes a
     * register value (e.g. because the user edited a variable value in the
     * Locals window).  This notification is not sent if the target process
     * changes its own register value during execution.  Moreover, this
     * notification is only sent if the debug session was started with the
     * option "send-register-change-notifications" set to "true" in the
     * bstrOptions parameter (see below).
     *
     * parameter1   VT_UI4 with the O/S thread ID whose registers may have
     *               changed.
     */
    CUSTOM_MESSAGE_TYPE_REGISTERS_CHANGED,

    /*!
     * A VS service or another debugger component can send this custom
     * message to the plugin in order to invalidate cached information about
     * the target process.  This can only be done when the target process
     * is stopped in the debugger.
     *
     * It is not normally necessary to use this command because the plugin
     * usually knows when it has to invalidate its cached information about
     * the target process.  However, if the VS service somehow changes the
     * target process state without the plugin's knowledge, the VS service
     * should use this command to invalidate the plugin's cached data.
     *
     * One scenario where this is required is when the VS service sends a
     * custom command via CUSTOM_MESSAGE_TYPE_COMMAND, which changes the
     * state of the target process.  Since the plugin doesn't know the effect
     * of the custom commands, it doesn't know that it needs to invalidate
     * its cached information.
     *
     * parameter1   VT_UI8 with a bit-mask of one or more INVALIDATE_FLAG
     *               values, telling what information to invalidate.
     *
     * parameter2   If "parameter1" invalidates information about a specific
     *               thread, "parameter2" must be VT_UI4 with the O/S thread ID
     *               of the thread whose information is invalidated.
     *              Otherwise, "parameter2" should be NULL.
     */
    CUSTOM_MESSAGE_TYPE_INVALIDATE_CACHED_STATE
};

/*!
 * Tells the result of a "custom command" that was sent to the Pin tool.
 */
enum CUSTOM_COMMAND_RESULT
{
    /*!
     * The command was understood by the tool, and "parameter2" contains a
     * string response from the tool.  Tools typically return this value even
     * when the command has some sort of syntactic error.  In that case, the
     * string response typically has a description of the error.
     */
    CUSTOM_COMMAND_RESULT_OK,

    /*!
     * The tool did not understand the command.  This value is also returned
     * if the parameters to CUSTOM_MESSAGE_TYPE_COMMAND are invalid.
     */
    CUSTOM_COMMAND_RESULT_UNRECOGNIZED,

    /*!
     * The application is currently running.  Commands can only be sent when
     * the application is stopped in the debugger.
     */
    CUSTOM_COMMAND_RESULT_NOT_STOPPED,

    /*!
     * Pin did not respond after sending the command.  This usually means that
     * the Pin tool crashed while processing the command.  The debugger session
     * is not likely to work after this error, so you should quit via
     * "Terminate All".
     */
    CUSTOM_COMMAND_RESULT_PIN_CRASH
};

/*!
 * Codes for error messages that could be reported to the VS service via
 * CUSTOM_MESSAGE_TYPE_ERROR.  Some error messages have corresponding
 * arguments which describe the error in more detail.  The comment for each
 * error below documents any arguments.  Arguments (even numeric ones) are
 * converted to a string and sent to the VS service as a VT_BSTR.
 */
enum ERROR_CODE
{
    /*!
     * Error with the option string passed via 'bstrOptions' in
     * LaunchDebugTargets().
     */
    ERROR_CODE_BAD_OPTIONS,

    /*!
     * Application executable <arg1> cannot be opened, or does not have a valid PE file format.
     *
     * <arg1>   Pathname to the application's executable file.
     */
    ERROR_CODE_BAD_PEFILE,

    /*!
     * Application executable <arg1> cannot be run on this host.  (E.g. you cannot run a
     * 64-bit binary on a 32-bit host.)
     *
     * <arg1>   Pathname to the application's executable file.
     */
    ERROR_CODE_WRONG_HOST,

    /*!
     * Could not attach to Pin on port <arg1>, check 'attach-port' option.
     *
     * <arg1>   TCP port number from the 'attach-port' option, formatted as a decimal string.
     */
    ERROR_CODE_ATTACH,

    /*!
     * Pin does not provide required interface, please upgrade.
     */
    ERROR_CODE_OLD_PIN,

    /*!
     * Error return <arg1> from CreateProcess() when launching Pin, command line is: <arg2>.
     *
     * <arg1>   Windows error code from GetLastError() after failing call to CreateProcess(),
     *           formatted as a hexadecimal string (with leading "0x").
     * <arg2>   Command line passed to CreateProcess(), including the pathname of the executable
     *           file.
     */
    ERROR_CODE_LAUNCH_PIN,

    /*!
     * Pin did not connect with debugger after <arg1> seconds.  Slow main() in Pin tool?
     * Try increasing 'timeout' option.  This could also indicate that the Pin tool
     * crashed in its main() function.
     *
     * <arg1>   Timeout value (seconds) from the 'timeout' option, formatted as a decimal
     *           string.
     */
    ERROR_CODE_LAUNCH_TIMEOUT,

    /*!
     * Pin abruptly disconnected from debugger while it was running.  Possible crash
     * in tool.
     */
    ERROR_CODE_PIN_CRASH
};

/*!
 * Flags used with CUSTOM_MESSAGE_TYPE_INVALIDATE_CACHED_STATE, which tell what
 * information about the target process should be invalidated.
 */
enum INVALIDATE_FLAG
{
    /*!
     * Invalidate any cached register values for the thread specified via
     * "parameter2".
     */
    INVALIDATE_FLAG_REGISTERS = (1<<0)
};


/*
 * Options for a Debugger Session
 *
 * When a debugger session is started with IVsDebugger2::LaunchDebugTargets2()
 * or IVsDebugger3::LaunchDebugTargets3(), you can pass various options via the
 * "bstrOptions" field .  This field is a NUL terminated UTF-16 string with the
 * following format.
 *
 * 1. Overall Structure
 *
 * The string must start with the characters "version:1\n" (where "\n" denotes
 * the newline character.  Following this version marker are a series of
 * (tag, value) pairs.  The allowed tags and their expected values are defined
 * below.  Values can be any series of Unicode characters, but the newline
 * character has special semantics (see below).  Each (tag, value) pair must
 * have one of the following two formats:
 *
 *  tag-name:length:value
 *  tag-name::value
 *
 * In the first format, "length" tells the number of Unicode characters in the
 * "value".  In the second format, the "value" is assumed to end immediately
 * before the next newline character (or until the end of the string, if there
 * is no newline).  When there is a "length", it consists of the character
 * [0-9] and is interpreted as a decimal number.  Note that the first format
 * allows the "value" to contain embedded newline characters if desired.
 *
 * Each (tag, value) pair may be separated by whitespace, which is ignored.
 * Clients are encouraged to place at least one newline character after each
 * (tag, value) pair in order to make the string easy for a human to read.
 *
 * 2. Legal Tags
 *
 * These are the (tag, value) pairs that are allowed.  If the "bstrOptions"
 * string contains an unknown tag or if the format is invalid, the launch
 * will abort with an error.  Duplicate tags are not encouraged, but the
 * last occurrence takes precedence when there are two entries with the
 * same tag.
 *
 * pin-root
 *      This tag is used only when launching an application under Pin.  (I.e.
 *      it is not used when attaching to a running process.)  It specifies the
 *      absolute pathname to the root of a Pin kit to use when launching the
 *      session.  When creating a "launch" debug session, either this tag or
 *      the "alternate-pin-command" tag are required.  The add-in should NOT
 *      add any quoting if the pathname contains embedded spaces.
 *
 * pin-arguments
 *      Specifies additional arguments that are passed to Pin when using the
 *      "pin-root" tag.  This is an optional tag.  If it is not specified, no
 *      arguments are passed to Pin.  The "-appdebug" argument is automatically
 *      added, so you should not specify this in "pin-arguments".  This string
 *      is passed directly to the CreateProcess() Win32 function, so it is the
 *      add-in's responsibility to add any necessary quoting for arguments that
 *      contains embedded spaces.
 *
 * pin-tool
 *      Specifies the absolute pathname of the Pin tool's DLL, when using the
 *      "pin-root" tag.  This is an optional tag.  If it is not specified, Pin
 *      is run with no tool.  The add-in should NOT add any quoting if the
 *      pathname contains embedded spaces.
 *
 * pin-tool-arguments
 *      Specifies additional arguments that are passed to the Pin tool when
 *      "pin-tool" is specified.  If it is not specified, the tool is run with
 *      no arguments.  This string is passed directly to the CreateProcess()
 *      Win32 function, so it is the add-in's responsibility to add any
 *      necessary quoting for arguments that contains embedded spaces.
 *
 * alternate-pin-command
 *      This tag is used only when launching an application under Pin.  (I.e.
 *      it is not used when attaching to a running process.)  It specifies an
 *      alternate command line to use instead of launching Pin.  It is expected
 *      that the alternate command will eventually launch Pin somehow.  When
 *      creating a "launch" debug session, either this tag or the "pin-root"
 *      tag are required.
 *
 *      When this tag is used, it must include all arguments that are necessary
 *      to identify the application that is launched and the application's
 *      command line arguments.  None of this is added by default.  However,
 *      the PinADX plugin does set up environment strings prior to launching
 *      the alternate command.  The alternate command line string is passed
 *      directly to the CreateProcess() Win32 function, so it must include any
 *      necessary quoting for arguments that contain embedded spaces.
 *
 *      The alternate command must somehow invoke Pin with the "-appdebug" and
 *      "-appdebug_connection :port" parameters.  Because the TCP "port" is
 *      created by the PinADX plugin, the plugin must pass this port number to
 *      the alternate command.  In order to do this, the alternate command line
 *      must contain a template parameter named %TCP_PORT%, which the PinADX
 *      plugin will replace with the actual port number.  In the unlikely case
 *      that the string "%TCP_PORT%" is an actual parameter to the alternate
 *      command, the add-in can use the "tcp-port-template" option to specify
 *      a different template.
 *
 * tcp-port-template
 *      This is an optional tag that can be used with "alternate-pin-command"
 *      to specify a different TCP port template parameter.  For example, if
 *      the value of this tag is PORT_NUMBER, the string "PORT_NUMBER" must
 *      exist in the "alternate-pin-command" string.  The PinADX plugin will
 *      replace PORT_NUMBER with the actual TCP port number.  The add-in
 *      should ensure that the port template parameter string (e.g.
 *      PORT_NUMBER) occurs exactly once in the alternate Pin command string.
 *
 * attach-port
 *      This tag is used only when attaching to a process that is already
 *      running under Pin, and it is a required tag in that case.  The tag
 *      specifies the TCP port number that the Pin process is listening on.
 *      The value should contain only the characters [0-9] and is interpreted
 *      as a decimal number.
 *
 * vs-service-guid
 *      This is an optional tag that may be used with any debug session.  It
 *      specifies the GUID of the add-in's VS service.  The format should be
 *      {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}, where the X's are characters in
 *      the range [0-9A-Fa-f] representing the hexadecimal value of the GUID.
 *      (Note that the curly braces and the dashes are required.)
 *
 *      If the add-in specifies this tag, it should also implement these two
 *      interfaces:
 *
 *          IVsCustomDebuggerStoppingEventHandler110
 *          IVsCustomDebuggerEventHandler110
 *
 *      See the description of the following CUSTOM_MESSAGE_TYPE values for
 *      information on the notifications that the PinADX plugin sends to these
 *      interfaces:
 *
 *          CUSTOM_MESSAGE_TYPE_BREAKPOINT
 *          CUSTOM_MESSAGE_TYPE_ERROR
 *          CUSTOM_MESSAGE_TYPE_REGISTERS_CHANGED
 *
 *      If the add-in does not specify the "vs-service-guid" tag, attempts
 *      by the Pin tool to stop via PIN_ApplicationBreakpoint() still cause
 *      the process to stop in the debugger.  However, the add-in is not
 *      notified of the tool's custom stop message.
 *
 * timeout
 *      This is an optional tag that is used only when launching an application
 *      under Pin.  It specifies the maximum amount of time (in seconds) that
 *      the PinADX plugin will wait when trying to connect with the Pin
 *      process.  You may need to use this tag if the Pin tool has a very slow
 *      main() function because the plugin cannot connect until the Pin tool
 *      calls PIN_StartProgram().  The value should consist of the characters
 *      [0-9] and is interpreted as a decimal number.  If this tag is not
 *      specified, the PinADX plugin uses a default timeout.
 *
 * pin-serializes-debugger-events
 *      This is an optional boolean tag that may be used with any debug
 *      session.  The value should either be "true" or "false".  If the option
 *      is not specified, the default value is "false".
 *
 *      This option controls the behavior when two or more threads try to stop
 *      in the debugger simultaneously.  For example, two threads might stop at
 *      breakpoints at the same time, or one thread might stop at a breakpoint
 *      while another stops due to an exception, etc.  When the option is
 *      "false", Pin reports all threads' simultaneous "stopping events" to the
 *      Visual Studio debugger, and then Visual Studio decides how to display
 *      them in the IDE.  It appears that Visual Studio's current behavior is
 *      to serialize the events and display them one-at-a-time in the IDE.
 *
 *      When this option is "true", Pin handles simultaneous stopping events
 *      differently.  Instead, Pin serializes the events and delivers them
 *      one-at-a-time to Visual Studio.  The end result is similar because
 *      either way they get serialized in the IDE.
 *
 *      One advantage of enabling this option is that the Pin tool can check
 *      to see if there are any pending custom breakpoints by calling
 *      PIN_GetStoppedThreadPendingToolBreakpoint().  The tool can also
 *      use PIN_ChangePendingToolBreakpointOnStoppedThread() to change any
 *      pending custom breakpoints.  These APIs don't work when this option is
 *      disabled because the breakpoints won't be pending inside Pin in that
 *      case (instead, they will be pending inside Visual Studio).
 *
 *      One possible disadvantage of enabling the option is that Visual Studio
 *      has less information about all the threads.  It's not clear if this
 *      causes any problems in Visual Studio, but it seems better to avoid
 *      enabling the option unless you need the tool APIs described above.
 *
 * send-register-change-notifications
 *      This is an optional boolean tag that may be used with any debug
 *      session.  The value should either be "true" or "false".  If the option
 *      is not specified, the default value is "false".
 *
 *      If this option is enabled, the plugin sends a notification of type
 *      CUSTOM_MESSAGE_TYPE_REGISTERS_CHANGED to the VS service whenever the
 *      debugger changes the registers in one of the threads in the target
 *      process.  This is useful if the VS services wants to update a UI
 *      element whenever a register changes.
 *
 *      When this option is "true", you must also specify a VS services GUID
 *      via "vs-service-guid".
 *
 * error-style
 *      This is an optional tag that may be used with any debug session.  It
 *      controls the way in which the plugin reports error conditions to the
 *      IDE.  The value should be one of the following strings.  If the option
 *      is not specified, the default value is "none".
 *
 *      none            - The plugin simply returns E_FAIL to the IDE when
 *                         there is a failure.  If the failure occurs during
 *                         launching or attaching a new debug session, the
 *                         call to LaunchDebugTargets() returns a failing
 *                         HRESULT, and the IDE usually displays a generic
 *                         error message.
 *
 *      notification    - The plugin sends the VS service a notification of
 *                         type CUSTOM_MESSAGE_TYPE_ERROR whenever there is
 *                         an error.  In addition, the plugin returns an error
 *                         code that prevents the IDE from printing a generic
 *                         error message, because the VS service is expected
 *                         to display its own error message.  If the failure
 *                         occurs during launching or attaching a new debug
 *                         session, the call to LaunchDebugTargets() still
 *                         returns a failing HRESULT.  When using this option,
 *                         you must also specify a VS services GUID via
 *                         "vs-service-guid".
 *
 * log-directory
 *      This is an optional tag that may be used with any debug session.  It
 *      specifies the absolute pathname of an existing directory where an
 *      internal log file will be written.  The log file is useful for
 *      debugging problems with the PinADX plugin.  The add-in should NOT add
 *      any quoting if the pathname contains embedded spaces.
 *
 * log-types
 *      This is an optional tag that may be used with any debug session.  It
 *      tells the type of logging information that is written to the file
 *      specified via log-directory.  If log-directory isn't specified, the
 *      log-types tag has no effect.  The value of the log-types tag is a
 *      comma-separated list of the following keywords:
 *
 *          errors      - Enables additional logging for error conditions.
 *          warnings    - Enables logging for warning conditions.
 *          general     - Enables general logging of the PinADX plugin.
 *          memory      - Enables logging of memory read requests.
 *          register    - Enables logging of register read requests.
 *
 *      If log-types isn't specified, the default value is "errors, warnings".
 *      To disable all logging, specify an empty value for log-types.
 *
 * 3. Example
 *
 * Here is an example "bstrOptions" string.  The first character of the string
 * is the "v" in "version".  Each linebreak in the example below denotes an
 * actual newline character (\n) in the "bstrOptions" string.
 *
 *      version:1
 *      pin-root:68:C:\Users\joe\My Documents\pin-2.10-43611-msvc10-ia32_intel64-windows
 *      vs-service-guid::{01234567-0a0b-1F3E-0123-456789ABCDEF}
 *
 * For further clarity, here is the same example string as a C++ string constant:
 *
 *      wchar_t *opts = L"version:1\n"
 *          L"pin-root:68:C:\\Users\\joe\\My Documents\\pin-2.10-43611-msvc10-ia32_intel64-windows\n"
 *          L"vs-service-guid::{01234567-0a0b-1F3E-0123-456789ABCDEF}";
 */

} // namespace
#endif // file guard
