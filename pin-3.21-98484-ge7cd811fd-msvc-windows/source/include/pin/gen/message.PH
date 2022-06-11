/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_MESSAGE_PH
#error duplicate inclusion of message
#else
#define PIN_G_MESSAGE_PH
/*! @file
  This module contains routines and classes for the handling error messages
 */
/*! @ingroup MESSAGE
 * A class for Pin log file
 */
class LOGFILE
{
  public:
    /*!
     * Create or open the specified file. If the file does not exist,
     * create the new file.
     *
     *  @param[in] name     Name of the file.
     *  @param[in] append   If TRUE, new log messages are appended to the file
     *                       if it already exists.  If FALSE, the new file
     *                       file overwrites any existing file.
     *
     *  @note   MESSAGE_TYPE::MessageLock is locked when calling this API, therefore MESSAGE_TYPE::MessageInternal()
     *          should not be called from this function.
     */
    virtual VOID Open(const CHAR* name, BOOL append) = 0;

    virtual BOOL IsOpen() const = 0;
    virtual VOID Close()        = 0;

    /*!
     * Write the specified message to the file.
     */
    virtual VOID Write(const CHAR* message, size_t length) = 0;
    VOID Write(const std::string& message);
    virtual VOID WriteAtPosition(const CHAR* message, size_t length, size_t offset) = 0;

    virtual ~LOGFILE() {}
};

/*! @ingroup MESSAGE
 * Type that represents where to put the log.
 */
enum LOGTYPE
{
    LOGTYPE_CONSOLE,            //!< log only to the console
    LOGTYPE_LOGFILE,            //!< log only to the log file
    LOGTYPE_CONSOLE_AND_LOGFILE //!< log to both the log file and console
};

/*! @ingroup MESSAGE
 * A call-back function that is called whenever a MESSAGE_TYPE object prints
 * a message.
 *
 *  @param[in] messageName  Name of the MESSAGE_TYPE object.  This is empty
 *                          for messages printed via PrintMessageAndExit().
 *  @param[in] message      The text of the message.
 *  @param[in] type         The type of the message.
 *  @param[in] ap           A variable list of additional arguments for the
 *                          message.
 *
 * @return  When TRUE is returned, the message is also printed to the console
 *           if the MESSAGE_TYPE would normally do this.  When FALSE is returned,
 *           the message is not printed to the console.
 */
typedef BOOL (*MESSAGE_CALLBACK)(const std::string& message, PIN_ERRTYPE type, INT32 userType, INT32 severity, INT32 numArgs,
                                 va_list ap);

/*! @ingroup MESSAGE
 * class describing a message object.
 * a message can be emitted using the Message() member function
 */
class MESSAGE_TYPE
{
  private:
    STATIC VOID (*_panic_callback)();
    STATIC VOID (*_terminate_callback)();
    STATIC std::string _log_fname;
    STATIC MESSAGE_TYPE* _list;
    STATIC LOGFILE* _logstream;
    STATIC BOOL _logstreamAppend;
    STATIC MESSAGE_CALLBACK _message_callback;

  private:
    const std::string _name;
    const std::string _prefix;
    const std::string _comment;
    BOOL _terminate;
    BOOL _trace;
    BOOL _enabled;
    BOOL _resource;
    LOGTYPE _log;

    MESSAGE_TYPE* _next;

  public:
    STATIC VOID RegisterPanicCallBack(VOID (*cb)());
    STATIC VOID RegisterTerminateCallBack(VOID (*cb)());
    STATIC VOID RegisterLogFile(const std::string& name, BOOL append);
    STATIC std::string GetLogFileName();
    STATIC MESSAGE_TYPE* Find(const std::string& name);
    STATIC BOOL Disable(const std::string& name);
    STATIC BOOL Enable(const std::string& name);
    STATIC UINT32 Number();
    STATIC std::string StringLongAll();
    STATIC BOOL IsTerminating();
    STATIC NORETURN VOID PrintMessageAndExit(const char* message, const char* message2, PIN_ERRTYPE errType = PIN_ERR_NONE,
                                             INT32 numArgs = 0, ...);
    STATIC VOID RegisterMessageCallBack(MESSAGE_CALLBACK fun);
    STATIC VOID UnregisterMessageCallBack();
    STATIC MESSAGE_CALLBACK MessageCallBack();
    STATIC BOOL ErrorFileSpecified();

  private:
    STATIC VOID DumpTrace();

  public:
    MESSAGE_TYPE(std::string name, std::string prefix, BOOL terminate, BOOL trace, BOOL enabled, BOOL resource, LOGTYPE log,
                 std::string comment);

    VOID Message(const std::string& message, BOOL printPrefix = TRUE, PIN_ERRTYPE errType = PIN_ERR_NONE, INT32 numArgs = 0, ...);

    VOID NORETURN MessageNoReturn(const std::string& message, BOOL printPrefix = TRUE, PIN_ERRTYPE errType = PIN_ERR_NONE,
                                  INT32 numArgs = 0, ...);

    BOOL inline on() const { return _enabled; }
    VOID disable() { _enabled = false; }
    VOID enable() { _enabled = true; }

    int Compare(const MESSAGE_TYPE& k2) const;

  private:
    VOID MessageInternal(const std::string& message, BOOL printPrefix, PIN_ERRTYPE errType, va_list va, INT32 numArgs);
};

/*! @ingroup MESSAGE
 * This type of message cannot be turned off.
 * It is used for PIN's assertion mechanism to signal to the compiler
 * that once MessageNoReturn() is called, then the application exits
 * @see MESSAGE_TYPE
 */
class MESSAGE_TYPE_ALWAYS_ON : public MESSAGE_TYPE
{
  public:
    inline MESSAGE_TYPE_ALWAYS_ON(std::string name, std::string prefix, BOOL terminate, BOOL trace, BOOL resource, LOGTYPE log,
                                  std::string comment)
        : MESSAGE_TYPE(name, prefix, terminate, trace, true, resource, log, comment)
    {}
    BOOL inline on() const { return true; }
};

/*! @ingroup MESSAGE
 *  Error message object for non-fatal error.
 */
extern MESSAGE_TYPE MessageTypeNonFatalError;

/*! @ingroup MESSAGE
  Error message object (terminates) (Always on message, using mesgoff from command line has no effect)
*/
extern MESSAGE_TYPE_ALWAYS_ON MessageTypeError;

/*! @ingroup MESSAGE
 *  Error message object for fatal errors that require post-logging handling.
 *  These errors do not cause the process termination in the MESSAGE_TYPE::Message()
 *  function.
 */
extern MESSAGE_TYPE MessageTypeCriticalError;

/*! @ingroup MESSAGE
  Warning message object
*/
extern MESSAGE_TYPE MessageTypeWarning;

/*! @ingroup MESSAGE
  Console message object
*/
extern MESSAGE_TYPE MessageTypeConsole;

/*! @ingroup MESSAGE
  Console message object  - No prefix
*/
extern MESSAGE_TYPE MessageTypeConsoleNoPrefix;

/*! @ingroup MESSAGE
  Assert message object
*/
extern MESSAGE_TYPE_ALWAYS_ON MessageTypeAssert;

/*! @ingroup MESSAGE
  Phase message object
*/
extern MESSAGE_TYPE MessageTypePhase;

/*! @ingroup MESSAGE
  Known-warning message object
*/
extern MESSAGE_TYPE MessageTypeKnown;

/*! @ingroup MESSAGE
  Info message object
*/
extern MESSAGE_TYPE MessageTypeInfo;

/*! @ingroup MESSAGE
  Debug message object
*/
extern MESSAGE_TYPE MessageTypeDebug;

/*! @ingroup MESSAGE
  Optimization opportunity message object
*/
extern MESSAGE_TYPE MessageTypeOpportunity;

/*! @ingroup MESSAGE
  Statistcis message object
*/
extern MESSAGE_TYPE MessageTypeStats;

/*! @ingroup MESSAGE
  Log message object
*/
extern MESSAGE_TYPE MessageTypeLog;

/*! @ingroup MESSAGE
  MilliSeconds elapsed since last invocation of this function
*/
extern UINT64 MilliSecondsElapsed();

/*! @ingroup MESSAGE
 * Create a string for an assertion failure.
 *
 *  @param[in] fileName     Name of the file containing the assertion failure or the empty string.
 *  @param[in] functionName Name of the function containing the assertion failure.
 *  @param[in] line         Line number of the assertion failure.
 *  @param[in] message      Message describing the failure.
 */
extern std::string AssertString(const char* fileName, const char* functionName, unsigned line, const std::string& message);

/*! @ingroup MESSAGE
 *
 */
extern VOID BreakMe();

#define QMESSAGE(t, m)            \
    do                            \
    {                             \
        if (t.on()) t.Message(m); \
    }                             \
    while (0)
#define QMESSAGE0(t, m, p, e)              \
    do                                     \
    {                                      \
        if (t.on()) t.Message(m, p, e, 0); \
    }                                      \
    while (0)
#define QMESSAGE1(t, m, p, e, a1)              \
    do                                         \
    {                                          \
        if (t.on()) t.Message(m, p, e, 1, a1); \
    }                                          \
    while (0)
#define QMESSAGE2(t, m, p, e, a1, a2)              \
    do                                             \
    {                                              \
        if (t.on()) t.Message(m, p, e, 2, a1, a2); \
    }                                              \
    while (0)
#define QMESSAGEP(t, m, p)           \
    do                               \
    {                                \
        if (t.on()) t.Message(m, p); \
    }                                \
    while (0)
#define QMESSAGE_LIMIT(t, m, max) \
    do                            \
    {                             \
        static int qqq = 0;       \
        if (qqq++ >= max) break;  \
        if (t.on()) t.Message(m); \
    }                             \
    while (0)

#ifdef __KLOCWORK__
#define QMESSAGE0_NORETURN(t, m, p, e) \
    do                                 \
    {                                  \
        abort();                       \
    }                                  \
    while (0)
#define QMESSAGE1_NORETURN(t, m, p, e, a1) \
    do                                     \
    {                                      \
        abort();                           \
    }                                      \
    while (0)
#define QMESSAGE2_NORETURN(t, m, p, e, a1, a2) \
    do                                         \
    {                                          \
        abort();                               \
    }                                          \
    while (0)
#else
#define QMESSAGE0_NORETURN(t, m, p, e)             \
    do                                             \
    {                                              \
        if (t.on()) t.MessageNoReturn(m, p, e, 0); \
    }                                              \
    while (0)
#define QMESSAGE1_NORETURN(t, m, p, e, a1)             \
    do                                                 \
    {                                                  \
        if (t.on()) t.MessageNoReturn(m, p, e, 1, a1); \
    }                                                  \
    while (0)
#define QMESSAGE2_NORETURN(t, m, p, e, a1, a2)             \
    do                                                     \
    {                                                      \
        if (t.on()) t.MessageNoReturn(m, p, e, 2, a1, a2); \
    }                                                      \
    while (0)
#endif

#define ASSERTQ(message) QMESSAGE0_NORETURN(LEVEL_BASE::MessageTypeAssert, message, TRUE, PIN_ERR_ASSERT)

#if defined(PIN_ASSERT_NO_FILE_NAME)
#define PIN_ASSERT_FILE ""
#else
#define PIN_ASSERT_FILE __FILE__
#endif

#define ASSERT(condition, message)                                                                                               \
    do                                                                                                                           \
    {                                                                                                                            \
        if (!(condition)) ASSERTQ(LEVEL_BASE::AssertString(PIN_ASSERT_FILE, __FUNCTION__, __LINE__, std::string("") + message)); \
    }                                                                                                                            \
    while (0)

#define ASSERTX(condition) ASSERT(condition, "assertion failed: " #condition "\n")

#define NYI() ASSERT(0, "Not Yet Implemented\n")

/*
 for errors related to debugger integration
 */
#define PIN_ERROR_DEBUGGER(message) QMESSAGE0_NORETURN(MessageTypeError, message, TRUE, PIN_ERR_DEBUGGER)

#define PIN_ERROR(message) QMESSAGE0_NORETURN(MessageTypeError, message, TRUE, PIN_ERR_INTERNAL)
#define PIN_ERROR0(message, type) QMESSAGE0_NORETURN(MessageTypeError, message, TRUE, type)
#define PIN_ERROR1(message, type, arg1) QMESSAGE1_NORETURN(MessageTypeError, message, TRUE, type, arg1)
#define PIN_ERROR2(message, type, arg1, arg2) QMESSAGE2_NORETURN(MessageTypeError, message, TRUE, type, arg1, arg2)

/*
 for calling even before the MessageType* constructors may have executed
 */
#define PIN_FATAL_ERROR(message)                                             \
    {                                                                        \
        PIN_ERROR(message);                                                  \
        MESSAGE_TYPE::PrintMessageAndExit(message, "E: ", PIN_ERR_INTERNAL); \
    }

/*
 * Non-fatal errors.
 */
#define PIN_NON_FATAL_ERROR(message) QMESSAGE0(MessageTypeNonFatalError, message, TRUE, PIN_ERR_INTERNAL)
#define PIN_NON_FATAL_ERROR0(message, type) QMESSAGE0(MessageTypeNonFatalError, message, TRUE, type)
#define PIN_NON_FATAL_ERROR1(message, type, arg1) QMESSAGE1(MessageTypeNonFatalError, message, TRUE, type, arg1)

/*
 * Fatal errors that require post-logging handling.
 * These errors do not cause the process termination in the MESSAGE_TYPE::Message() function.
 */
#define PIN_CRITICAL_ERROR(message) QMESSAGE0(MessageTypeCriticalError, message, TRUE, PIN_ERR_INTERNAL)
#define PIN_CRITICAL_ERROR0(message, type) QMESSAGE0(MessageTypeCriticalError, message, TRUE, type)
#define PIN_CRITICAL_ERROR1(message, type, arg1) QMESSAGE1(MessageTypeCriticalError, message, TRUE, type, arg1)
#define PIN_CRITICAL_ERROR2(message, type, arg1, arg2) QMESSAGE2(MessageTypeCriticalError, message, TRUE, type, arg1, arg2)

/*
 * The special message dumper. It copies the message to the log file,
 * sends it to stderr and exits pin with -1.
 * @param - message of type const char* to dump
 */
#define PIN_OUT_OF_MEMORY(message) MESSAGE_TYPE::PrintMessageAndExit(message, "Pin is out of memory: ", PIN_ERR_OUT_OF_MEMORY)

#define CONSOLE(message) QMESSAGE(MessageTypeConsole, message)

#define CONSOLE_NOPREFIX(message) QMESSAGE(MessageTypeConsoleNoPrefix, message)

#define PHASE(message) QMESSAGE(MessageTypePhase, message)

#define WARNING(message) QMESSAGE(MessageTypeWarning, message)
#define WARNING_LIMIT(message, n) QMESSAGE_LIMIT(MessageTypeWarning, message, n)

#define KNOWN(message) QMESSAGE(MessageTypeKnown, message)
#define KNOWN_LIMIT(message, n) QMESSAGE_LIMIT(MessageTypeKnown, message, n)

#define LOG(message) QMESSAGE(MessageTypeLog, message)

#define INFO(message) QMESSAGE(MessageTypeInfo, message)
#define INFO_LIMIT(message, n) QMESSAGE_LIMIT(MessageTypeInfo, message, n)

#define OPPORTUNITY(message) QMESSAGE(MessageTypeOpportunity, message)
#define OPPORTUNITY_LIMIT(message, n) QMESSAGE_LIMIT(MessageTypeOpportunity, message, n)
#define PIN_DEBUG(message) QMESSAGE(MessageTypeDebug, message)
#define STATS(message) QMESSAGE(MessageTypeStats, message)
#endif // PIN_G_MESSAGE_PH
