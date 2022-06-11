/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This is the implementation file for the "debugger-shell" tool extensions.
 * See the header file "debugger-shell.H" for a description of what these
 * extensions are and how to use them in your tool.
 */

/*
 * This tool implements a number of extended debugger breakpoints, which use
 * Pin instrumentation to trigger the breakpoint condition.  This comment
 * provides an overview of the instrumentation strategy.
 *
 * Most of the extended debugger breakpoints insert instrumentation at
 * IPOINT_BEFORE, which tests the breakpoint condition.  We use if / then
 * instrumentation, where the "if" part tests the breakpoint condition and
 * the "then" part triggers the breakpoint.  The inserted analysis code follows
 * this pattern:
 *
 *      if TestCondition(....)
 *      {
 *          if (REG_SKIP_ONE == REG_INST_PTR)
 *              return;
 *          REG_SKIP_ONE = REG_INST_PTR;
 *          PIN_ApplicationBreakpoint(....);
 *      }
 *      [Original Instruction]
 *      REG_SKIP_ONE = 0
 *
 * The TestCondition() is a short in-lined "if" analysis call that returns
 * TRUE if the breakpoint condition is true, and the remainder of the code
 * before [Original Instruction] is in the "then" analysis call.  Usually,
 * all breakpoint types share the same "then" code, but use a different
 * TestCondition().
 *
 * The usage of REG_SKIP_ONE is a bit tricky.  When the debugger continues
 * from a breakpoint, it re-executes all the instrumentation on the [Original
 * Instruction], including the TestCondition() call.  Of course, we don't want
 * the breakpoint to immediately re-trigger, though, or the application would
 * never make forward progress.  We use a Pin virtual register (denoted by
 * REG_SKIP_ONE above) to solve this by skipping the next occurence of the
 * breakpoint when the application resumes.  Clearing REG_SKIP_ONE at
 * IPOINT_AFTER / IPOINT_TAKEN_BRANCH ensures that the breakpoint will
 * re-trigger if the application loops back to the same instruction.
 *
 * If more than one breakpoint is placed on the same instruction, each
 * breakpoint inserts its own instrumentation like this:
 *
 *      if TestCondition1(....)
 *      {
 *          if (REG_SKIP_ONE == REG_INST_PTR)
 *              return;
 *          REG_SKIP_ONE = REG_INST_PTR;
 *          PIN_ApplicationBreakpoint(....);
 *      }
 *      if TestCondition2(....)
 *      {
 *          if (REG_SKIP_ONE == REG_INST_PTR)
 *              return;
 *          REG_SKIP_ONE = REG_INST_PTR;
 *          PIN_ApplicationBreakpoint(....);
 *      }
 *      [Original Instruction]
 *      REG_SKIP_ONE = 0
 *
 * One breakpoint ("break if store to <addr> == <value>")  is checked at
 * IPOINT_AFTER or IPOINT_TAKEN_BRANCH.  This instrumentation looks like this:
 *
 *      REG_RECORD_EA = IARG_MEMORYWRITE_EA
 *      [Original Store Instruction]
 *      if (REG_RECORD_EA == <addr> && *REG_RECORD_EA == <value>)
 *      {
 *          PIN_ApplicationBreakpoint(....);
 *      }
 *
 * Here, we record the value of the effective address in a Pin virtual register
 * because IARG_MEMORYWRITE_EA cannot be computed at IPOINT_AFTER.  The
 * instrumentation at IPOINT_AFTER tests the breakpoint condition and triggers
 * the breakpoint.  If the breakpoint does trigger, the PC will point to the
 * next instruction after [Original Store Instruction].  Therefore, when the
 * debugger continues, execution immediately resumes at the next instruction
 * and there is no need to use the REG_SKIP_ONE technique.
 *
 * The tool also implements tracepoints.  A tracepoint is like a breakpoint,
 * except that instead of stopping when a condition is met, a trace record is
 * recorded.  The instrumentation for a tracepoint uses the same "if"
 * instrumentation to check the condition, but the "then" instrumentation is
 * different.  A typical example looks like this:
 *
 *      if TestCondition(....)
 *      {
 *          PIN_GetLock(....);
 *          TraceLog.push_back(....);
 *          PIN_ReleaseLock(....);
 *      }
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>
#include "debugger-shell.H"

using std::string;
// These are all the registers that can be used in breakpoint conditions, etc.
//
struct REG_INFO
{
    REG _reg;
    const char* _name;
};

#if defined(TARGET_IA32E)
static const REG_INFO AllRegisters[] = {{REG_GAX, "rax"}, {REG_GBX, "rbx"}, {REG_GCX, "rcx"}, {REG_GDX, "rdx"},
                                        {REG_GSI, "rsi"}, {REG_GDI, "rdi"}, {REG_GBP, "rbp"}, {REG_RSP, "rsp"},
                                        {REG_R8, "r8"},   {REG_R9, "r9"},   {REG_R10, "r10"}, {REG_R11, "r11"},
                                        {REG_R12, "r12"}, {REG_R13, "r13"}, {REG_R14, "r14"}, {REG_R15, "r15"}};

#elif defined(TARGET_IA32)

static const REG_INFO AllRegisters[] = {{REG_GAX, "eax"}, {REG_GBX, "ebx"}, {REG_GCX, "ecx"}, {REG_GDX, "edx"},
                                        {REG_GSI, "esi"}, {REG_GDI, "edi"}, {REG_GBP, "ebp"}, {REG_ESP, "esp"}};
#endif

class SHELL : public DEBUGGER_SHELL::ISHELL
{
  private:
    BOOL _isEnabled;
    DEBUGGER_SHELL::STARTUP_ARGUMENTS _clientArgs;

    // Describes the help messages for a single command.
    //
    struct HELP
    {
        HELP(const char* cmd, const char* desc) : _command(cmd), _description(desc) {}
        HELP(const std::string& cmd, const std::string& desc) : _command(cmd), _description(desc) {}

        std::string _command;     // Name of the extended debugger command.
        std::string _description; // Help string for the command.
    };
    typedef std::vector< HELP > HELPS;

    // Describes the help messages for all commands in a category.
    //
    struct HELP_CATEGORY
    {
        HELP_CATEGORY() {}
        HELP_CATEGORY(const char* name, const char* desc, const char* intro) : _name(name), _description(desc), _intro(intro) {}
        HELP_CATEGORY(const std::string& name, const std::string& desc, const std::string& intro)
            : _name(name), _description(desc), _intro(intro)
        {}

        // These are printed when the user types "help", showing a brief description
        // of the entire category.
        //
        std::string _name;        // Name of this category of commands.
        std::string _description; // Description of this category.

        // These are printed when the user types "help <category>", describing each
        // command in the category.
        //
        std::string _intro; // Introductory paragraph for all commands in this category.
        HELPS _helpStrings; // Help messages for all commands in this category.

        // If not empty, this is a cached copy of the formatted help message
        // containing all the command descriptions in '_intro' and '_helpStrings'.
        //
        std::string _formattedHelp;
    };
    typedef std::vector< HELP_CATEGORY > HELP_CATEGORIES;
    HELP_CATEGORIES _helpCategories;

    unsigned _nextHelpCategory; // Index of the next available help category.

    // If not empty, this is a cached copy of the formatted top-level help message,
    // which describes all the command categories.
    //
    std::string _formattedCategoryHelp;

    // These are used when formatting help messages.
    //
    typedef std::pair< std::string, std::string > FORMAT_PAIR;
    typedef std::vector< FORMAT_PAIR > FORMAT_PAIRS;

    typedef std::vector< std::string > WORDS;

    // These are the register numbers for the REG_SKIP_ONE and REG_RECORD_EA
    // virtual registers.  Each is one of the REG_INST_Gn registers.
    //
    REG _regSkipOne;
    REG _regThreadData;
    REG _regRecordEa;

    // Possible trigger conditions for breakpoints or tracepoints.
    //
    enum TRIGGER
    {
        TRIGGER_AT,              // Trigger before PC.
        TRIGGER_STORE_TO,        // Trigger before store to address.
        TRIGGER_STORE_VALUE_TO,  // Trigger after store of value to address.
        TRIGGER_LOAD_FROM,       // Trigger before load from address.
        TRIGGER_LOAD_VALUE_FROM, // Trigger before load of value from address.
        TRIGGER_AT_ICOUNT,       // Trigger before instruction count
        TRIGGER_AT_MCOUNT,       // Trigger before memory instruction count
        TRIGGER_JUMP_TO,         // Trigger before jump to PC.
        TRIGGER_REG_IS           // Trigger before PC if register has value.
    };

    // Possible event types.
    //
    enum ETYPE
    {
        ETYPE_BREAKPOINT,
        ETYPE_TRACEPOINT
    };

    // Data for TRIGGER_AT_ICOUNT event
    struct AT_ICOUNT
    {                   // TRIGGER_AT_ICOUNT
        UINT64 _icount; //  instruction count
        THREADID _tid;  //  thread to trigger the event
    };

    // Data for TRIGGER_AT_MCOUNT event
    struct AT_MCOUNT
    {                   // TRIGGER_AT_MCOUNT
        UINT64 _mcount; //   memory instruction count
        THREADID _tid;  //   thread to trigger the event
    };

    struct EVENT
    {
        ETYPE _type;             // Breakpoint vs. tracepoint.
        TRIGGER _trigger;        // Trigger condition.
        std::string _listMsg;    // String printed when event is listed.
        std::string _triggerMsg; // Message printed when breakpoint triggers or when tracepoint is printed.

        // Data specific to ETYPE_TRACEPOINT.
        //
        REG _reg;        // If not REG_INVALID, trace this register.
        BOOL _isDeleted; // TRUE: User has deleted, but may be referenced in TRACEREC.
        BOOL _isEnabled; // TRUE if tracepoint is enabled.

        // Data specific to the trigger condition.
        //
        union
        {
            ADDRINT _ea; // TRIGGER_STORE_TO: EA of store.
            ADDRINT _pc; // TRIGGER_AT: PC of trigger location.
                         // TRIGGER_JUMP_TO: PC of jump.

            AT_ICOUNT _atIcount; // TRIGGER_AT_ICOUNT

            AT_MCOUNT _atMcount; // TRIGGER_AT_ICOUNT

            struct
            {                  // TRIGGER_STORE_VALUE_TO:
                ADDRINT _ea;   //   EA of store.
                UINT64 _value; //   value stored.
            } _storeValueTo;

            struct
            {                  // TRIGGER_LOAD_VALUE_FROM:
                ADDRINT _ea;   //   EA of load.
                UINT64 _value; //   value loaded.
            } _loadValueFrom;

            struct
            {                   // TRIGGER_REG_IS:
                ADDRINT _pc;    //   PC of breakpoint.
                REG _reg;       //   register ID.
                ADDRINT _value; //   value of register.
            } _regIs;
        };
    };

    // All events, indexed by their ID.
    //
    typedef std::map< unsigned, EVENT > EVENTS;
    EVENTS _events;

    unsigned _nextEventId;

    // A trace record collected when executing a tracepoint.
    //
    struct TRACEREC
    {
        unsigned _id;      // Index of EVENT in '_events'.
        ADDRINT _pc;       // PC where tracepoint triggered.
        ADDRINT _regValue; // If tracepoints traces a register, it's value.
    };

    // Log of all the tracepoint data.  Access is protected by the lock.
    //
    PIN_LOCK _traceLock;
    typedef std::vector< TRACEREC > TRACERECS;
    TRACERECS _traceLog;

    // Instruction count and memory instruction count used if any
    // TRIGGER_AT_ICOUNT or TRIGGER_AT_MCOUNT breakpoints are setup State
    // is maintained on a per thread basis. We use the _regThreadData
    // virtual register to keep a pointer to this data structure for each
    // thread.
    //
    struct THREAD_DATA
    {
        THREAD_DATA() : _tid(0), _icount(0), _mcount(0) {}

        THREADID _tid;
        UINT64 _icount;
        UINT64 _mcount;
    };

    // Help messages are formatted to be no wider than this number of characters.
    //
    static const unsigned MaxHelpWidth = 80;

  public:
    // ----- constructor -----

    /*
     * This method completes construction of the object.  We do it here,
     * so we can return an error indication more easily.
     *
     * @return  TRUE on success.
     */
    BOOL Construct()
    {
        _regSkipOne    = PIN_ClaimToolRegister();
        _regRecordEa   = PIN_ClaimToolRegister();
        _regThreadData = PIN_ClaimToolRegister();

        if (!REG_valid(_regSkipOne) || !REG_valid(_regRecordEa) || !REG_valid(_regThreadData))
        {
            PrintError("Unable to allocate Pin virtual register");
            return FALSE;
        }
        PIN_InitLock(&_traceLock);
        _nextHelpCategory = DEBUGGER_SHELL::HELP_CATEGORY_END;
        _nextEventId      = 1;
        _isEnabled        = FALSE;
        return TRUE;
    }

    // ----- DEBUGGER_SHELL::ISHELL -----

    BOOL Enable(const DEBUGGER_SHELL::STARTUP_ARGUMENTS& args)
    {
        if (_isEnabled)
        {
            PrintError("Do not call ISHELL::Enable() twice");
            return FALSE;
        }

        _clientArgs = args;
        ConstructHelpStrings();

        // Callbacks for start and end of threads.
        //
        PIN_AddThreadStartFunction(ThreadStart, this);
        PIN_AddThreadFiniFunction(ThreadFini, this);

        // Debugger interpreter, to process debugger commands.
        //
        PIN_AddDebugInterpreter(DebugInterpreter, this);

        // Trace instrumentation, to handle debugger commands.
        //
        TRACE_AddInstrumentFunction(InstrumentTrace, this);
        _isEnabled = TRUE;
        return TRUE;
    }

    unsigned AddExtendedHelpCategory(const std::string& name, const std::string& description, BOOL* alreadyExists)
    {
        // See if this category name already exists, if so return that element (ignoring the
        // new description).
        //
        HELP_CATEGORIES::iterator it = FindHelpCategory(name);
        if (it != _helpCategories.end())
        {
            if (alreadyExists) *alreadyExists = TRUE;
            return it - _helpCategories.begin();
        }

        // No existing element, so add a new one.
        //
        if (_helpCategories.size() <= _nextHelpCategory) _helpCategories.resize(_nextHelpCategory + 1);
        _formattedCategoryHelp.clear();
        _helpCategories[_nextHelpCategory] = HELP_CATEGORY(name, description, "");
        if (alreadyExists) *alreadyExists = FALSE;
        return _nextHelpCategory++;
    }

    void AddExtendedHelpMessage(unsigned category, const std::string& cmd, const std::string& description)
    {
        ASSERTX(category < _helpCategories.size());

        HELP_CATEGORY& entry = _helpCategories[category];
        entry._formattedHelp.clear();
        entry._helpStrings.push_back(HELP(cmd, description));
    }

    REG GetSkipOneRegister() { return _regSkipOne; }

  private:
    /*
     * Pin call-back that is invoked when a thread starts
     *
     *  @param[in] tid          The tid of the starting thread
     *  @param[in,out] ctxt     Register state for the thread
     *  @param[in] flags        OS specific flags
     *  @param[in] v            Any tool specific value
     */
    static VOID ThreadStart(THREADID tid, CONTEXT* ctxt, INT32 flags, VOID* v)
    {
        THREAD_DATA* td = new THREAD_DATA();
        SHELL* ds       = static_cast< SHELL* >(v);

        td->_tid = tid;
        PIN_SetContextReg(ctxt, ds->_regThreadData, ADDRINT(td));
    }

    /*
     * Pin call-back that is invoked when a thread ends. Note that in some cases this
     * callback is not invoked (e.g. windows threadpool)
     *
     *  @param[in] tid          The tid of the starting thread
     *  @param[in] ctxt         Register state for the thread
     *  @param[in] code         OS specific exit code
     *  @param[in] v            Any tool specific value
     */
    static VOID ThreadFini(THREADID tid, const CONTEXT* ctxt, INT32 code, VOID* v)
    {
        THREAD_DATA* td = NULL;
        SHELL* ds       = static_cast< SHELL* >(v);

        td = reinterpret_cast< THREAD_DATA* >(PIN_GetContextReg(ctxt, ds->_regThreadData));
        if (td) delete td;
    }

    /*
     * Pin call-back that implements an extended debugger command.
     *
     *  @param[in] tid          The debugger focus thread.
     *  @param[in,out] ctxt     Register state for the debugger's "focus" thread.
     *  @param[in] cmd          Text of the extended command.
     *  @param[out] result      Text that the debugger prints when the command finishes.
     *  @param[in] vme          Pointer to ISHELL instance.
     *
     * @return  TRUE if we recognize this extended command.
     */
    static BOOL DebugInterpreter(THREADID tid, CONTEXT* ctxt, const string& cmd, string* result, VOID* vme)
    {
        SHELL* me = static_cast< SHELL* >(vme);

        /*
         * General Commands:
         *
         *  help
         *  help <category>
         *
         * Breakpoint Commands:
         *
         *  break if store to <addr>
         *  break after store to <addr> == <value>
         *  break if load from <addr>
         *  break before load from <addr> == <value>
         *  break if icount <count>
         *  break if mcount <count>
         *  break if jump to <pc>
         *  break at <pc> if <reg> == <value>
         *  list breakpoints
         *  delete breakpoint <id>
         *
         * Tracing Commands:
         *
         *  trace [<reg>] at <pc>
         *  trace [<reg>] if load from <addr>
         *  trace [<reg>] before load from <addr> == <value>
         *  trace [<reg>] if store to <addr>
         *  trace [<reg>] after store to <addr> == <value>
         *  trace enable [<id>]
         *  trace disable [<id>]
         *  trace clear
         *  trace print [to <file>]
         *  list tracepoints
         *  delete tracepoint <id>
         *
         * Example Trace Output:
         *
         *  0x1234: rax = 0x5678
         *  0x1234:
         *  0x1234: if store to 0x89abc: rax = 0x5678
         *  0x1234: if store to 0x89abc
         *  0x1234: after store to 0x89abc = 0xdef00: rax = 0x5678
         *  0x1234: after store to 0x89abc = 0xdef00
         */

        WORDS words;
        me->SplitWords(cmd, &words);
        size_t nWords = words.size();

        if (nWords == 1 && words[0] == "help")
        {
            // help
            //
            *result = me->GetFormattedCategoryHelp();
            return TRUE;
        }
        if (nWords == 2 && words[0] == "help")
        {
            // help <category>
            //
            *result = me->GetFormattedHelp(words[1]);
            return TRUE;
        }
        else if (nWords == 2 && words[0] == "list" && words[1] == "breakpoints")
        {
            // list breakpoints
            //
            *result = me->ListBreakpoints();
            return TRUE;
        }
        else if (nWords == 2 && words[0] == "list" && words[1] == "tracepoints")
        {
            // list tracepoints
            //
            *result = me->ListTracepoints();
            return TRUE;
        }
        else if (nWords == 3 && words[0] == "delete" && words[1] == "breakpoint")
        {
            // delete breakpoint <id>
            //
            *result = me->DeleteEvent(ETYPE_BREAKPOINT, words[2]);
            return TRUE;
        }
        else if (nWords == 3 && words[0] == "delete" && words[1] == "tracepoint")
        {
            // delete tracepoint <id>
            //
            *result = me->DeleteEvent(ETYPE_TRACEPOINT, words[2]);
            return TRUE;
        }
        else if (nWords == 2 && words[0] == "trace" && words[1] == "enable")
        {
            // trace enable
            //
            *result = me->EnableDisableAllTraces(TRUE);
            return TRUE;
        }
        else if (nWords == 3 && words[0] == "trace" && words[1] == "enable")
        {
            // trace enable <id>
            //
            *result = me->EnableDisableTrace(words[2], TRUE);
            return TRUE;
        }
        else if (nWords == 2 && words[0] == "trace" && words[1] == "disable")
        {
            // trace disable
            //
            *result = me->EnableDisableAllTraces(FALSE);
            return TRUE;
        }
        else if (nWords == 3 && words[0] == "trace" && words[1] == "disable")
        {
            // trace disable <id>
            //
            *result = me->EnableDisableTrace(words[2], FALSE);
            return TRUE;
        }
        else if (nWords == 2 && words[0] == "trace" && words[1] == "clear")
        {
            // trace clear
            //
            *result = me->ClearTraceLog();
            return TRUE;
        }
        else if (nWords == 2 && words[0] == "trace" && words[1] == "print")
        {
            // trace print
            //
            *result = me->PrintTraceLog("");
            return TRUE;
        }
        else if (nWords == 4 && words[0] == "trace" && words[1] == "print" && words[2] == "to")
        {
            // trace print to <file>
            //
            *result = me->PrintTraceLog(words[3]);
            return TRUE;
        }
        else if (nWords == 3 && words[0] == "trace" && words[1] == "at")
        {
            // trace at <pc>
            //
            *result = me->ParseTriggerAtEvent(ETYPE_TRACEPOINT, words[2], "");
            return TRUE;
        }
        else if (nWords == 4 && words[0] == "trace" && words[2] == "at")
        {
            // trace <reg> at <pc>
            //
            *result = me->ParseTriggerAtEvent(ETYPE_TRACEPOINT, words[3], words[1]);
            return TRUE;
        }
        else if (nWords == 5 && words[0] == "break" && words[1] == "if" && words[2] == "store" && words[3] == "to")
        {
            // break if store to <addr>
            //
            *result = me->ParseTriggerStoreToEvent(ETYPE_BREAKPOINT, words[4], "");
            return TRUE;
        }
        else if (nWords == 5 && words[0] == "break" && words[1] == "if" && words[2] == "load" && words[3] == "from")
        {
            // break if load from <addr>
            //
            *result = me->ParseTriggerLoadFromEvent(ETYPE_BREAKPOINT, words[4], "");
            return TRUE;
        }
        else if (nWords == 5 && words[0] == "trace" && words[1] == "if" && words[2] == "store" && words[3] == "to")
        {
            // trace if store to <addr>
            //
            *result = me->ParseTriggerStoreToEvent(ETYPE_TRACEPOINT, words[4], "");
            return TRUE;
        }
        else if (nWords == 6 && words[0] == "trace" && words[2] == "if" && words[3] == "store" && words[4] == "to")
        {
            // trace <reg> if store to <addr>
            //
            *result = me->ParseTriggerStoreToEvent(ETYPE_TRACEPOINT, words[5], words[1]);
            return TRUE;
        }
        else if (nWords == 5 && words[0] == "trace" && words[1] == "if" && words[2] == "load" && words[3] == "from")
        {
            // trace if load from <addr>
            //
            *result = me->ParseTriggerLoadFromEvent(ETYPE_TRACEPOINT, words[4], "");
            return TRUE;
        }
        else if (nWords == 6 && words[0] == "trace" && words[2] == "if" && words[3] == "load" && words[4] == "from")
        {
            // trace <reg> if load from <addr>
            //
            *result = me->ParseTriggerLoadFromEvent(ETYPE_TRACEPOINT, words[5], words[1]);
            return TRUE;
        }
        else if (nWords == 7 && words[0] == "break" && words[1] == "before" && words[2] == "load" && words[3] == "from" &&
                 words[5] == "==")
        {
            // break before load from <addr> == <value>
            //
            *result = me->ParseTriggerLoadValueFromEvent(ETYPE_BREAKPOINT, words[4], words[6], "");
            return TRUE;
        }
        else if (nWords == 7 && words[0] == "break" && words[1] == "after" && words[2] == "store" && words[3] == "to" &&
                 words[5] == "==")
        {
            // break after store to <addr> == <value>
            //
            *result = me->ParseTriggerStoreValueToEvent(ETYPE_BREAKPOINT, words[4], words[6], "");
            return TRUE;
        }
        else if (nWords == 7 && words[0] == "trace" && words[1] == "after" && words[2] == "store" && words[3] == "to" &&
                 words[5] == "==")
        {
            // trace after store to <addr> == <value>
            //
            *result = me->ParseTriggerStoreValueToEvent(ETYPE_TRACEPOINT, words[4], words[6], "");
            return TRUE;
        }
        else if (nWords == 8 && words[0] == "trace" && words[2] == "after" && words[3] == "store" && words[4] == "to" &&
                 words[6] == "==")
        {
            // trace <reg> after store to <addr> == <value>
            //
            *result = me->ParseTriggerStoreValueToEvent(ETYPE_TRACEPOINT, words[5], words[7], words[1]);
            return TRUE;
        }
        else if (nWords == 7 && words[0] == "trace" && words[1] == "before" && words[2] == "load" && words[3] == "from" &&
                 words[5] == "==")
        {
            // trace before load from <addr> == <value>
            //
            *result = me->ParseTriggerLoadValueFromEvent(ETYPE_TRACEPOINT, words[4], words[6], "");
            return TRUE;
        }
        else if (nWords == 8 && words[0] == "trace" && words[2] == "before" && words[3] == "load" && words[4] == "from" &&
                 words[6] == "==")
        {
            // trace <reg> before load from <addr> == <value>
            //
            *result = me->ParseTriggerLoadValueFromEvent(ETYPE_TRACEPOINT, words[5], words[7], words[1]);
            return TRUE;
        }
        else if (me->_clientArgs._enableIcountBreakpoints && nWords == 4 && words[0] == "break" && words[1] == "if" &&
                 words[2] == "icount")
        {
            // break if icount <count>
            //
            *result = me->ParseTriggerAtCount(ETYPE_BREAKPOINT, words[3], TRIGGER_AT_ICOUNT, tid);
            return TRUE;
        }
        else if (me->_clientArgs._enableIcountBreakpoints && nWords == 4 && words[0] == "break" && words[1] == "if" &&
                 words[2] == "mcount")
        {
            // break if mcount <count>
            //
            *result = me->ParseTriggerAtCount(ETYPE_BREAKPOINT, words[3], TRIGGER_AT_MCOUNT, tid);
            return TRUE;
        }
        else if (nWords == 5 && words[0] == "break" && words[1] == "if" && words[2] == "jump" && words[3] == "to")
        {
            // break if jump to <pc>
            //
            *result = me->ParseTriggerJumpToEvent(ETYPE_BREAKPOINT, words[4], "");
            return TRUE;
        }
        else if (nWords == 7 && words[0] == "break" && words[1] == "at" && words[3] == "if" && words[5] == "==")
        {
            // break at <pc> if <reg> == <value>
            //
            *result = me->ParseTriggerRegIsEvent(ETYPE_BREAKPOINT, words[2], words[4], words[6], "");
            return TRUE;
        }
        return FALSE;
    }

    /*
     * Flush the code cache.
     */
    VOID Flush() { PIN_RemoveInstrumentation(); }

    /*
     * Split an input command into a series of whitespace-separated words.  Leading
     * and trailing whitespace is ignored.
     *
     *  @param[in] cmd      The input command.
     *  @param[out] words   An STL container that receives the parsed words.  Each
     *                       word is added with the push_back() method.
     */
    VOID SplitWords(const std::string& cmd, WORDS* words)
    {
        size_t pos = cmd.find_first_not_of(' ');
        while (pos != std::string::npos)
        {
            size_t end = cmd.find_first_of(' ', pos + 1);
            if (end == std::string::npos)
            {
                words->push_back(cmd.substr(pos));
                pos = end;
            }
            else
            {
                words->push_back(cmd.substr(pos, end - pos));
                pos = cmd.find_first_not_of(' ', end + 1);
            }
        }
    }

    /*
     * Attempt to parse an unsigned integral number from a string.  The string's prefix
     * determines the radix: "0x" for hex, "0" for octal, otherwise decimal.
     *
     *  @param[in] val      The string to parse.
     *  @param[out] number  On success, receives the parsed number.
     *
     * @return  TRUE if a number is successfully parsed.
     */
    template< typename T > BOOL ParseNumber(const std::string& val, T* number)
    {
        std::istringstream is(val);

        T num;
        if (val.compare(0, 2, "0x") == 0)
            is >> std::hex >> num;
        else if (val.compare(0, 1, "0") == 0)
            is >> std::oct >> num;
        else
            is >> std::dec >> num;

        if (is.fail() || !is.eof()) return FALSE;
        *number = num;
        return TRUE;
    }

    /*
     * Attempt to parse a "full" register name.
     *
     *  @param[in] name     String which possibly names a register.
     *
     * @return  If \a name is a register we recognize, returns that register ID.  Otherwise,
     *           returns REG_INVALID().
     */
    REG ParseRegName(const std::string& name)
    {
        if (name.empty() || (name[0] != '$' && name[0] != '%')) return REG_INVALID();

        // Translate the string to lower case and remove the leading "$" or "%".
        //
        std::string reg = name.substr(1);
        std::transform(reg.begin(), reg.end(), reg.begin(), ::tolower);

        const unsigned nRegs = sizeof(AllRegisters) / sizeof(AllRegisters[0]);
        for (unsigned i = 0; i < nRegs; i++)
        {
            if (reg == AllRegisters[i]._name) return AllRegisters[i]._reg;
        }
        return REG_INVALID();
    }

    /*
     * Get the name for a Pin register.
     *
     *  @param[in] reg  The register.
     *
     * @return  The name of \a reg.
     */
    std::string GetRegName(REG reg)
    {
        const unsigned nRegs = sizeof(AllRegisters) / sizeof(AllRegisters[0]);
        for (unsigned i = 0; i < nRegs; i++)
        {
            if (reg == AllRegisters[i]._reg) return std::string("$") + AllRegisters[i]._name;
        }

        ASSERTX(0);
        return "???";
    }

    /*
     * Construct help strings for all the extended debugger commands.
     */
    void ConstructHelpStrings()
    {
        if (_helpCategories.size() < DEBUGGER_SHELL::HELP_CATEGORY_END) _helpCategories.resize(DEBUGGER_SHELL::HELP_CATEGORY_END);
        _formattedCategoryHelp.clear();

        // General commands.
        //
        HELP_CATEGORY* category = &_helpCategories[DEBUGGER_SHELL::HELP_CATEGORY_GENERAL];
        *category               = HELP_CATEGORY("general", "General commands.", "");
        HELPS* helpCommands     = &category->_helpStrings;
        category->_formattedHelp.clear();

        helpCommands->push_back(HELP("help", "Print help summary of available commands."));
        helpCommands->push_back(HELP("help <category>", "Print help on commands in <category>."));

        // Breakpoint commands.
        //
        category     = &_helpCategories[DEBUGGER_SHELL::HELP_CATEGORY_BREAKPOINTS];
        *category    = HELP_CATEGORY("breakpoints", "Breakpoint commands.", "");
        helpCommands = &category->_helpStrings;
        category->_formattedHelp.clear();

        helpCommands->push_back(HELP("list breakpoints", "List all extended breakpoints."));
        helpCommands->push_back(HELP("delete breakpoint <id>", "Delete extended breakpoint <id>."));
        helpCommands->push_back(HELP("break if load from <addr>", "Break before any load from <addr>."));
        helpCommands->push_back(HELP("break if store to <addr>", "Break before any store to <addr>."));
        helpCommands->push_back(
            HELP("break before load from <addr> == <value>", "Break before load if <value> loaded from <addr>."));
        helpCommands->push_back(HELP("break after store to <addr> == <value>", "Break after store if <value> stored to <addr>."));

        if (_clientArgs._enableIcountBreakpoints)
        {
            helpCommands->push_back(
                HELP("break if icount <count>",
                     "Break current thread before it reaches <count> instructions from the start of execution."));
            helpCommands->push_back(
                HELP("break if mcount <count>",
                     "Break current thread before it reaches <count> memory instructions from the start of execution."));
        }

        helpCommands->push_back(HELP("break if jump to <pc>", "Break before any jump to <pc>."));
        helpCommands->push_back(HELP("break at <pc> if <reg> == <value>", "Break before <pc> if <reg> contains <value>."));

        // Tracepoint commands.
        //
        category     = &_helpCategories[DEBUGGER_SHELL::HELP_CATEGORY_TRACEPOINTS];
        *category    = HELP_CATEGORY("tracepoints", "Tracepoint commands.", "");
        helpCommands = &category->_helpStrings;
        category->_formattedHelp.clear();

        helpCommands->push_back(HELP("list tracepoints", "List all extended tracepoints."));
        helpCommands->push_back(HELP("delete tracepoint <id>", "Delete extended tracepoint <id>."));
        helpCommands->push_back(HELP("trace print [to <file>]", "Print contents of trace log to screen, or to <file>."));
        helpCommands->push_back(HELP("trace clear", "Clear contents of trace log."));
        helpCommands->push_back(HELP("trace disable [<id>]", "Disable all tracepoints, or only tracepoint <id>."));
        helpCommands->push_back(HELP("trace enable [<id>]", "Enable all tracepoints, or only tracepoint <id>."));
        helpCommands->push_back(HELP("trace [<reg>] at <pc>",
                                     "Record trace entry before executing instruction at <pc>.  If <reg> is "
                                     "specified, record that register's value too."));
        helpCommands->push_back(HELP("trace [<reg>] if store to <addr>",
                                     "Record trace entry before executing any store to <addr>.  If <reg> is "
                                     "specified, record that register's value too."));
        helpCommands->push_back(HELP("trace [<reg>] after store to <addr> == <value>",
                                     "Record trace entry after any store of <value> to <addr>.  If <reg> is "
                                     "specified, record that register's value too."));
        helpCommands->push_back(HELP("trace [<reg>] if load from <addr>",
                                     "Record trace entry before executing any load to <addr>.  If <reg> is "
                                     "specified, record that register's value too."));
        helpCommands->push_back(HELP("trace [<reg>] before load from <addr> == <value>",
                                     "Record trace entry before any load of <value> from <addr>.  If <reg> is "
                                     "specified, record that register's value too."));

        // Register names.
        //
        std::string regLower = AllRegisters[0]._name;
        std::string regUpper = regLower;
        std::transform(regUpper.begin(), regUpper.end(), regUpper.begin(), ::toupper);

        std::string intro =
            "Some of the extended debugger commands accept a <reg> parameter.  You can specify a registers name "
            "using either \"$\" or \"%\" followed by the name of the register.  For example, all of these strings "
            "specify the same register: \"$" +
            regLower + "\", \"%" + regLower + "\", \"$" + regUpper + "\", \"%" + regUpper +
            "\".  The list of available register names is:";

        category     = &_helpCategories[DEBUGGER_SHELL::HELP_CATEGORY_REGISTERS];
        *category    = HELP_CATEGORY("registers", "Register names.", intro);
        helpCommands = &category->_helpStrings;
        category->_formattedHelp.clear();

        const unsigned nRegs = sizeof(AllRegisters) / sizeof(AllRegisters[0]);
        for (unsigned i = 0; i < nRegs; i++)
            helpCommands->push_back(HELP(std::string("$") + AllRegisters[i]._name, ""));
    }

    /*
     * @return  The formatted text for the "help" command.
     */
    std::string GetFormattedCategoryHelp()
    {
        if (!_formattedCategoryHelp.empty()) return _formattedCategoryHelp;

        FORMAT_PAIRS textPairs;
        for (HELP_CATEGORIES::iterator it = _helpCategories.begin(); it != _helpCategories.end(); ++it)
            textPairs.push_back(std::make_pair(it->_name, it->_description));

        std::string text = FormatHelpText(textPairs);
        text.append("\n");
        text.append("type \"help <category>\" for help on commands in that category.\n");
        _formattedCategoryHelp = text;
        return _formattedCategoryHelp;
    }

    /*
     * Get the formatted text for the "help <category>" command.
     *
     *  @param[in] categoryName     Possibly the name of a category.
     *
     * @return  The formatted text for the command.  If \a categoryName is not valid, an
     *           error message is returned.
     */
    std::string GetFormattedHelp(const std::string& categoryName)
    {
        HELP_CATEGORIES::iterator it = FindHelpCategory(categoryName);
        if (it == _helpCategories.end()) return "Unknown category '" + categoryName + "'\n";
        HELP_CATEGORY& category = *it;

        if (!category._formattedHelp.empty()) return category._formattedHelp;

        FORMAT_PAIRS textPairs;
        for (HELPS::iterator it2 = category._helpStrings.begin(); it2 != category._helpStrings.end(); ++it2)
            textPairs.push_back(std::make_pair(it2->_command, it2->_description));

        std::string text;
        if (!category._intro.empty())
        {
            text.append(SplitToMultipleLines(category._intro, MaxHelpWidth, 0));
            text.append("\n");
            text.append("\n");
        }

        text.append(FormatHelpText(textPairs));
        if (text.empty()) text = "(none)\n";

        category._formattedHelp = text;
        return category._formattedHelp;
    }

    /*
     * Attempt to find an existing help category by name.
     *
     *  @param[in] name     Names a potential category.
     *
     * @return  An iterator to the category's entry if \a name exists, otherwise the "end" iterator.
     */
    HELP_CATEGORIES::iterator FindHelpCategory(const std::string& name)
    {
        HELP_CATEGORIES::iterator it;
        for (it = _helpCategories.begin(); it != _helpCategories.end(); ++it)
        {
            if (it->_name == name) break;
        }
        return it;
    }

    /*
     * Format text for a help message.
     *
     *  @param[in] textPairs    A container of text pairs.  The first element in the pair is
     *                           a command name, the second is a longer description.
     *
     * @return  The formatted text.
     */
    std::string FormatHelpText(const FORMAT_PAIRS& textPairs)
    {
        const size_t longCommandSize = 25; // Description for long command is printed on separate line.

        // The description text starts 2 spaces to the right of the longest "short" command.
        //
        const size_t dashColumn = longCommandSize + 2;

        std::string result;
        BOOL newLineBeforeNext = FALSE;
        for (FORMAT_PAIRS::const_iterator it = textPairs.begin(); it != textPairs.end(); ++it)
        {
            std::string thisMessage = it->first;

            BOOL isLongCommand = FALSE;
            if (it->second.empty())
            {
                // There is no description, so the line just contains the command.
            }
            else if (it->first.size() < longCommandSize)
            {
                // This is a "short" command.  The description starts on the same line as
                // the command, but may continue on subsequent lines.
                //
                size_t pad = dashColumn - it->first.size();
                thisMessage.append(pad, ' ');
                thisMessage.append("- ");
                thisMessage.append(it->second);
                if (thisMessage.size() > MaxHelpWidth)
                    thisMessage = SplitToMultipleLines(thisMessage, MaxHelpWidth, dashColumn + 2);
            }
            else
            {
                // This is a "long" command.  The description starts on the next line.
                //
                thisMessage.append("\n");
                std::string desc(dashColumn + 2, ' ');
                desc.append(it->second);
                if (desc.size() > MaxHelpWidth) desc = SplitToMultipleLines(desc, MaxHelpWidth, dashColumn + 2);
                thisMessage.append(desc);
                isLongCommand = TRUE;
            }

            // It seems more readable if there is a blank line separating "long" commands
            // from their neighbors.
            //
            if (newLineBeforeNext || isLongCommand) result.append("\n");

            result.append(thisMessage);
            result.append("\n");
            newLineBeforeNext = isLongCommand;
        }

        return result;
    }

    /*
     * Split a line of text into multiple indented lines.
     *
     *  @param[in] str          The line of text to split.
     *  @param[in] maxWidth     None of the output lines will be wider than this limit.
     *  @param[in] indent       If the line is split, all line other than the first are indented
     *                           with this many spaces.
     *
     * @return  The formated text lines.
     */
    std::string SplitToMultipleLines(const std::string& str, size_t maxWidth, size_t indent)
    {
        BOOL isFirst = TRUE;
        std::string ret;

        std::string input = str;
        while (input.size() > maxWidth)
        {
            BOOL needHyphen      = FALSE;
            size_t posBreakAfter = std::string::npos;

            // Point 'posBreakAfter' to the last character of the last word that fits
            // before 'maxWidth'.  We assume that words are separated by spaces.
            //
            size_t posSpace = input.rfind(' ', maxWidth - 1);
            if (posSpace != std::string::npos) posBreakAfter = input.find_last_not_of(' ', posSpace);

            // If there's a really long word is itself longer than 'maxWidth', break
            // the word and put a hyphen in.
            //
            if (posBreakAfter == std::string::npos)
            {
                posBreakAfter = maxWidth - 2;
                needHyphen    = TRUE;
            }

            // Split the line, add indenting for all but the first one.
            //
            if (!isFirst) ret.append(indent, ' ');
            ret.append(input.substr(0, posBreakAfter + 1));
            if (needHyphen) ret.append("-");
            ret.append("\n");

            // Strip off any spaces from the start of the next line.
            //
            size_t posNextWord = input.find_first_not_of(' ', posSpace);
            input.erase(0, posNextWord);

            // Lines after the first are indented, so this reduces the effective width
            // of the line.
            //
            if (isFirst)
            {
                isFirst = FALSE;
                maxWidth -= indent;
            }
        }

        if (input.size())
        {
            if (!isFirst) ret.append(indent, ' ');
            ret.append(input);
        }
        return ret;
    }

    /*
     * @return  A single string showing all the active breakpoints.
     */
    std::string ListBreakpoints()
    {
        std::string ret;

        for (EVENTS::iterator it = _events.begin(); it != _events.end(); ++it)
        {
            if (it->second._type == ETYPE_BREAKPOINT)
            {
                ret += it->second._listMsg;
                ret += "\n";
            }
        }
        return ret;
    }

    /*
     * @return  A single string showing all the active tracepoints.
     */
    std::string ListTracepoints()
    {
        std::string ret;

        for (EVENTS::iterator it = _events.begin(); it != _events.end(); ++it)
        {
            if (it->second._type == ETYPE_TRACEPOINT && !it->second._isDeleted)
            {
                ret += it->second._listMsg;
                if (!it->second._isEnabled) ret += " (disabled)";
                ret += "\n";
            }
        }
        return ret;
    }

    /*
     * Delete an event.
     *
     *  @param[in] type     The type of event to delete.
     *  @param[in] idStr    The event ID.
     *
     * @return  A string to return to the debugger prompt.
     */
    std::string DeleteEvent(ETYPE type, const std::string& idStr)
    {
        unsigned id;
        std::string ret = ValidateId(type, idStr, &id);
        if (!ret.empty()) return ret;

        // The trace log may contain a pointer to the tracepoint, so don't really
        // delete it if the trace log is non-empty.
        //
        if (type == ETYPE_TRACEPOINT && !_traceLog.empty())
            _events[id]._isDeleted = TRUE;
        else
            _events.erase(id);
        Flush();
        return "";
    }

    /*
     * Enable or disable all "trace" events.
     *
     *  @param[in] enable   TRUE if events should be enabled.
     *
     * @return  A string to return to the debugger prompt.
     */
    std::string EnableDisableAllTraces(BOOL enable)
    {
        BOOL needFlush = FALSE;
        for (EVENTS::iterator it = _events.begin(); it != _events.end(); ++it)
        {
            if (it->second._type == ETYPE_TRACEPOINT && !it->second._isDeleted && it->second._isEnabled != enable)
            {
                it->second._isEnabled = enable;
                needFlush             = TRUE;
            }
        }
        if (needFlush) Flush();
        return "";
    }

    /*
     * Enable or disable a single trace event.
     *
     *  @param[in] idStr    ID of the trace event to enable.
     *  @param[in] enable   TRUE if the event should be enabled.
     *
     * @return  A string to return to the debugger prompt.
     */
    std::string EnableDisableTrace(const std::string& idStr, BOOL enable)
    {
        unsigned id;
        std::string ret = ValidateId(ETYPE_TRACEPOINT, idStr, &id);
        if (!ret.empty()) return ret;

        if (_events[id]._isEnabled != enable)
        {
            _events[id]._isEnabled = enable;
            Flush();
        }
        return "";
    }

    /*
     * Clear the trace log.
     *
     * @return  A string to return to the debugger prompt.
     */
    std::string ClearTraceLog()
    {
        if (_traceLog.empty()) return "";

        _traceLog.clear();

        // Now that the trace log is cleared, there's no danger that there are any
        // references to deleted "trace" events.  So, we can really delete them.
        //
        EVENTS::iterator it = _events.begin();
        while (it != _events.end())
        {
            EVENTS::iterator thisIt = it++;
            if (thisIt->second._type == ETYPE_TRACEPOINT && thisIt->second._isDeleted) _events.erase(thisIt);
        }
        return "";
    }

    /*
     * Print the contents of the trace log.
     *
     *  @param[in] file     If not empty, the file to print the log to.  Otherwise, the
     *                       content of the log is returned (and printed to the debugger prompt).
     *
     * @return  A string to return to the debugger prompt.
     */
    std::string PrintTraceLog(const std::string& file)
    {
        std::ostringstream ss;
        std::ofstream fs;
        std::ostream* os;

        // We print the log either to a file, or to the "ss" buffer.
        //
        if (!file.empty())
        {
            fs.open(file.c_str());
            os = &fs;
        }
        else
        {
            os = &ss;
        }

        // We want to pad out the "pc" field with leading zeros.
        //
        os->fill('0');
        size_t width = 2 * sizeof(ADDRINT);

        for (TRACERECS::iterator it = _traceLog.begin(); it != _traceLog.end(); ++it)
        {
            const EVENT& evnt = _events[it->_id];
            (*os) << "0x" << std::hex << std::setw(width) << it->_pc << std::setw(0);
            if (!evnt._triggerMsg.empty()) (*os) << ": " << evnt._triggerMsg;
            if (REG_valid(evnt._reg)) *os << ": " << GetRegName(evnt._reg) << " = 0x" << std::hex << it->_regValue;
            (*os) << "\n";
        }

        // If printing to the debugger prompt, this returns the output.  If not, the output
        // is flushed to the file when the 'fs' goes out of scope and the return statement
        // returns the empty string.
        //
        return ss.str();
    }

    /*
     * Parse an event ID and check that it is valid.
     *
     *  @param[in] type     The type of event that this ID should correspond to.
     *  @param[in] idStr    The candidate ID.
     *  @param[out] id      On success, receives the ID.
     *
     * @return  On success, the empty string.  On failure, an error message.
     */
    std::string ValidateId(ETYPE type, const std::string& idStr, unsigned* id)
    {
        if (!ParseNumber(idStr, id))
        {
            std::ostringstream os;
            os << "Invalid " << GetEventName(type) << " ID " << idStr << "\n";
            return os.str();
        }

        EVENTS::iterator it = _events.find(*id);
        if (it == _events.end() || it->second._type != type || (type == ETYPE_TRACEPOINT && it->second._isDeleted))
        {
            std::ostringstream os;
            os << "Invalid " << GetEventName(type) << " ID " << idStr << "\n";
            return os.str();
        }

        return "";
    }

    /*
     * Get the name of an event type.
     *
     *  @param[in] type     The event type.
     *
     * @return  The name for \a type.
     */
    std::string GetEventName(ETYPE type)
    {
        switch (type)
        {
            case ETYPE_BREAKPOINT:
                return "breakpoint";
            case ETYPE_TRACEPOINT:
                return "tracepoint";
            default:
                ASSERTX(0);
                return "";
        }
    }

    /*
     * Parse an event with trigger type TRIGGER_AT.
     *
     *  @param[in] type     The type of event.
     *  @param[in] pcStr    The trigger's PC address.
     *  @param[in] regStr   If not empty, the register to trace.
     *
     * @return  A string to return to the debugger prompt.
     */
    std::string ParseTriggerAtEvent(ETYPE type, const std::string& pcStr, const std::string& regStr)
    {
        ADDRINT pc;
        if (!ParseNumber(pcStr, &pc))
        {
            std::ostringstream os;
            os << "Invalid address " << pcStr << "\n";
            return os.str();
        }

        REG reg = REG_INVALID();
        if (type == ETYPE_TRACEPOINT && !regStr.empty())
        {
            reg = ParseRegName(regStr);
            if (!REG_valid(reg))
            {
                std::ostringstream os;
                os << "Invalid register " << regStr << " (see \"help registers\")\n";
                return os.str();
            }
        }

        unsigned id = _nextEventId++;

        std::ostringstream os;
        os << "at 0x" << std::hex << pc;

        EVENT evnt;
        std::string ret;
        if (type == ETYPE_BREAKPOINT)
        {
            std::ostringstream osTrigger;
            osTrigger << "Triggered breakpoint #" << std::dec << id << ": break " << os.str();
            evnt._triggerMsg = osTrigger.str();

            std::ostringstream osList;
            osList << "#" << std::dec << id << ":  break " << os.str();
            evnt._listMsg = osList.str();

            ret = "Breakpoint " + osList.str() + "\n";
        }
        else
        {
            std::ostringstream osList;
            osList << "#" << std::dec << id << ":  trace";
            if (REG_valid(reg)) osList << " " << GetRegName(reg);
            osList << " " << os.str();
            evnt._listMsg = osList.str();

            evnt._triggerMsg = "";
            evnt._reg        = reg;
            evnt._isDeleted  = FALSE;
            evnt._isEnabled  = TRUE;

            ret = "Tracepoint " + osList.str() + "\n";
        }

        evnt._type    = type;
        evnt._trigger = TRIGGER_AT;
        evnt._pc      = pc;
        _events.insert(std::make_pair(id, evnt));
        Flush();
        return ret;
    }

    /*
     * Parse an event with trigger type TRIGGER_LOAD_FROM
     *
     *  @param[in] type     The type of event.
     *  @param[in] addrStr  The trigger's load address.
     *  @param[in] regStr   If not empty, the register to trace.
     *
     * @return  A string to return to the debugger prompt.
     */
    std::string ParseTriggerLoadFromEvent(ETYPE type, const std::string& addrStr, const std::string& regStr)
    {
        ADDRINT addr;
        string outStr;

        if (!ParseNumber(addrStr, &addr))
        {
            std::ostringstream os;
            os << "Invalid address " << addrStr << "\n";
            return os.str();
        }

        REG reg = REG_INVALID();
        if (type == ETYPE_TRACEPOINT && !regStr.empty())
        {
            reg = ParseRegName(regStr);
            if (!REG_valid(reg))
            {
                std::ostringstream os;
                os << "Invalid register " << regStr << " (see \"help registers\")\n";
                return os.str();
            }
        }

        unsigned id = _nextEventId++;

        std::ostringstream os;
        os << "if load from 0x" << std::hex << addr;

        EVENT evnt;
        std::string ret;
        if (type == ETYPE_BREAKPOINT)
        {
            std::ostringstream osTrigger;
            osTrigger << "Triggered breakpoint #" << std::dec << id << ": break " << os.str();
            evnt._triggerMsg = osTrigger.str();

            std::ostringstream osList;
            osList << "#" << std::dec << id << ":  break " << os.str();
            evnt._listMsg = osList.str();

            ret = "Breakpoint " + osList.str() + "\n";
        }
        else
        {
            std::ostringstream osList;
            osList << "#" << std::dec << id << ":  trace";
            if (REG_valid(reg)) osList << " " << GetRegName(reg);
            osList << " " << os.str();
            evnt._listMsg = osList.str();

            evnt._triggerMsg = os.str();
            evnt._reg        = reg;
            evnt._isDeleted  = FALSE;
            evnt._isEnabled  = TRUE;

            ret = "Tracepoint " + osList.str() + "\n";
        }

        // fill in information specific to this trigger event
        evnt._type    = type;
        evnt._trigger = TRIGGER_LOAD_FROM;
        evnt._ea      = addr;
        _events.insert(std::make_pair(id, evnt));
        Flush();
        return ret;
    }

    /*
     * Parse an event with trigger type TRIGGER_LOAD_VALUE_FROM.
     *
     *  @param[in] type     The type of event.
     *  @param[in] addrStr  The trigger's load address.
     *  @param[in] valueStr The trigger's load value.
     *  @param[in] regStr   If not empty, the register to trace.
     *
     * @return  A string to return to the debugger prompt.
     */
    std::string ParseTriggerLoadValueFromEvent(ETYPE type, const std::string& addrStr, const std::string& valueStr,
                                               const std::string& regStr)
    {
        ADDRINT addr;
        if (!ParseNumber(addrStr, &addr))
        {
            std::ostringstream os;
            os << "Invalid address " << addrStr << "\n";
            return os.str();
        }

        UINT64 value = 0;
        if (!ParseNumber(valueStr, &value))
        {
            std::ostringstream os;
            os << "Invalid value " << valueStr << "\n";
            return os.str();
        }

        REG reg = REG_INVALID();
        if (type == ETYPE_TRACEPOINT && !regStr.empty())
        {
            reg = ParseRegName(regStr);
            if (!REG_valid(reg))
            {
                std::ostringstream os;
                os << "Invalid register " << regStr << " (see \"help registers\")\n";
                return os.str();
            }
        }

        unsigned id = _nextEventId++;

        std::ostringstream os;
        os << "before load from 0x" << std::hex << addr << " == 0x" << std::hex << value;

        EVENT evnt;
        std::string ret;
        if (type == ETYPE_BREAKPOINT)
        {
            std::ostringstream osTrigger;
            osTrigger << "Triggered breakpoint #" << std::dec << id << ": break " << os.str();
            evnt._triggerMsg = osTrigger.str();

            std::ostringstream osList;
            osList << "#" << std::dec << id << ":  break " << os.str();
            evnt._listMsg = osList.str();

            ret = "Breakpoint " + osList.str() + "\n";
        }
        else
        {
            std::ostringstream osList;
            osList << "#" << std::dec << id << ":  trace";
            if (REG_valid(reg)) osList << " " << GetRegName(reg);
            osList << " " << os.str();
            evnt._listMsg = osList.str();

            evnt._triggerMsg = os.str();
            evnt._reg        = reg;
            evnt._isDeleted  = FALSE;
            evnt._isEnabled  = TRUE;

            ret = "Tracepoint " + osList.str() + "\n";
        }

        evnt._type                 = type;
        evnt._trigger              = TRIGGER_LOAD_VALUE_FROM;
        evnt._loadValueFrom._ea    = addr;
        evnt._loadValueFrom._value = value;
        _events.insert(std::make_pair(id, evnt));
        Flush();
        return ret;
    }

    /*
     * Parse an event with trigger type TRIGGER_STORE_TO.
     *
     *  @param[in] type     The type of event.
     *  @param[in] addrStr  The trigger's store address.
     *  @param[in] regStr   If not empty, the register to trace.
     *
     * @return  A string to return to the debugger prompt.
     */
    std::string ParseTriggerStoreToEvent(ETYPE type, const std::string& addrStr, const std::string& regStr)
    {
        ADDRINT addr;
        if (!ParseNumber(addrStr, &addr))
        {
            std::ostringstream os;
            os << "Invalid address " << addrStr << "\n";
            return os.str();
        }

        REG reg = REG_INVALID();
        if (type == ETYPE_TRACEPOINT && !regStr.empty())
        {
            reg = ParseRegName(regStr);
            if (!REG_valid(reg))
            {
                std::ostringstream os;
                os << "Invalid register " << regStr << " (see \"help registers\")\n";
                return os.str();
            }
        }

        unsigned id = _nextEventId++;

        std::ostringstream os;
        os << "if store to 0x" << std::hex << addr;

        EVENT evnt;
        std::string ret;
        if (type == ETYPE_BREAKPOINT)
        {
            std::ostringstream osTrigger;
            osTrigger << "Triggered breakpoint #" << std::dec << id << ": break " << os.str();
            evnt._triggerMsg = osTrigger.str();

            std::ostringstream osList;
            osList << "#" << std::dec << id << ":  break " << os.str();
            evnt._listMsg = osList.str();

            ret = "Breakpoint " + osList.str() + "\n";
        }
        else
        {
            std::ostringstream osList;
            osList << "#" << std::dec << id << ":  trace";
            if (REG_valid(reg)) osList << " " << GetRegName(reg);
            osList << " " << os.str();
            evnt._listMsg = osList.str();

            evnt._triggerMsg = os.str();
            evnt._reg        = reg;
            evnt._isDeleted  = FALSE;
            evnt._isEnabled  = TRUE;
            ret              = "Tracepoint " + osList.str() + "\n";
        }

        evnt._type    = type;
        evnt._trigger = TRIGGER_STORE_TO;
        evnt._ea      = addr;
        _events.insert(std::make_pair(id, evnt));
        Flush();
        return ret;
    }

    /*
     * Parse an event with trigger type TRIGGER_STORE_VALUE_TO.
     *
     *  @param[in] type     The type of event.
     *  @param[in] addrStr  The trigger's store address.
     *  @param[in] valueStr The trigger's store value.
     *  @param[in] regStr   If not empty, the register to trace.
     *
     * @return  A string to return to the debugger prompt.
     */
    std::string ParseTriggerStoreValueToEvent(ETYPE type, const std::string& addrStr, const std::string& valueStr,
                                              const std::string& regStr)
    {
        ADDRINT addr;
        if (!ParseNumber(addrStr, &addr))
        {
            std::ostringstream os;
            os << "Invalid address " << addrStr << "\n";
            return os.str();
        }

        UINT64 value = 0;
        if (!ParseNumber(valueStr, &value))
        {
            std::ostringstream os;
            os << "Invalid value " << valueStr << "\n";
            return os.str();
        }

        REG reg = REG_INVALID();
        if (type == ETYPE_TRACEPOINT && !regStr.empty())
        {
            reg = ParseRegName(regStr);
            if (!REG_valid(reg))
            {
                std::ostringstream os;
                os << "Invalid register " << regStr << " (see \"help registers\")\n";
                return os.str();
            }
        }

        unsigned id = _nextEventId++;

        std::ostringstream os;
        os << "after store to 0x" << std::hex << addr << " == 0x" << std::hex << value;

        EVENT evnt;
        std::string ret;
        if (type == ETYPE_BREAKPOINT)
        {
            std::ostringstream osTrigger;
            osTrigger << "Triggered breakpoint #" << std::dec << id << ": break " << os.str();
            evnt._triggerMsg = osTrigger.str();

            std::ostringstream osList;
            osList << "#" << std::dec << id << ":  break " << os.str();
            evnt._listMsg = osList.str();

            ret = "Breakpoint " + osList.str() + "\n";
        }
        else
        {
            std::ostringstream osList;
            osList << "#" << std::dec << id << ":  trace";
            if (REG_valid(reg)) osList << " " << GetRegName(reg);
            osList << " " << os.str();
            evnt._listMsg = osList.str();

            evnt._triggerMsg = os.str();
            evnt._reg        = reg;
            evnt._isDeleted  = FALSE;
            evnt._isEnabled  = TRUE;

            ret = "Tracepoint " + osList.str() + "\n";
        }

        evnt._type                = type;
        evnt._trigger             = TRIGGER_STORE_VALUE_TO;
        evnt._storeValueTo._ea    = addr;
        evnt._storeValueTo._value = value;
        _events.insert(std::make_pair(id, evnt));
        Flush();
        return ret;
    }

    /*
     * Parse an event with trigger type TRIGGER_AT_ICOUNT or TRIGGER_AT_MCOUNT
     *
     *  @param[in] type     The type of event.
     *  @param[in] countStr The trigger's count.
     *  @param[in] trigger  The type of trigger (either icount or mcount).
     *  @param[in} tid      The id of the thread to trigger the breakpoint
     *
     * @return  A string to return to the debugger prompt.
     */
    std::string ParseTriggerAtCount(ETYPE type, const std::string& countStr, TRIGGER trigger, THREADID tid)
    {
        ASSERTX(type == ETYPE_BREAKPOINT);
        ASSERTX(trigger == TRIGGER_AT_ICOUNT || trigger == TRIGGER_AT_MCOUNT);

        UINT64 count = 0;
        if (!ParseNumber(countStr, &count))
        {
            std::ostringstream os;
            os << "Invalid value " << countStr << "\n";
            return os.str();
        }

        unsigned id = _nextEventId++;

        std::ostringstream os;
        os << "thread " << std::dec << tid << " if " << (trigger == TRIGGER_AT_ICOUNT ? "icount" : "mcount") << " " << std::dec
           << count;

        EVENT evnt;
        std::string ret;

        std::ostringstream osTrigger;
        osTrigger << "Triggered breakpoint #" << std::dec << id << ": break " << os.str();
        evnt._triggerMsg = osTrigger.str();

        std::ostringstream osList;
        osList << "#" << std::dec << id << ":  break " << os.str();
        evnt._listMsg = osList.str();

        ret = "Breakpoint " + osList.str() + "\n";

        evnt._type    = type;
        evnt._trigger = trigger;
        if (trigger == TRIGGER_AT_ICOUNT)
        {
            evnt._atIcount._icount = count;
            evnt._atIcount._tid    = tid;
        }
        else
        {
            evnt._atMcount._mcount = count;
            evnt._atMcount._tid    = tid;
        }
        _events.insert(std::make_pair(id, evnt));
        Flush();
        return ret;
    }

    /*
     * Parse an event with trigger type TRIGGER_JUMP_TO.
     *
     *  @param[in] type     The type of event.
     *  @param[in] addrStr  The trigger's jump address.
     *  @param[in] regStr   If not empty, the register to trace.
     *
     * @return  A string to return to the debugger prompt.
     */
    std::string ParseTriggerJumpToEvent(ETYPE type, const std::string& addrStr, const std::string& regStr)
    {
        ADDRINT addr;
        if (!ParseNumber(addrStr, &addr))
        {
            std::ostringstream os;
            os << "Invalid address " << addrStr << "\n";
            return os.str();
        }

        REG reg = REG_INVALID();
        if (type == ETYPE_TRACEPOINT && !regStr.empty())
        {
            reg = ParseRegName(regStr);
            if (!REG_valid(reg))
            {
                std::ostringstream os;
                os << "Invalid register " << regStr << " (see \"help registers\")\n";
                return os.str();
            }
        }

        unsigned id = _nextEventId++;

        std::ostringstream os;
        os << "if jump to 0x" << std::hex << addr;

        EVENT evnt;
        std::string ret;
        if (type == ETYPE_BREAKPOINT)
        {
            std::ostringstream osTrigger;
            osTrigger << "Triggered breakpoint #" << std::dec << id << ": break " << os.str();
            evnt._triggerMsg = osTrigger.str();

            std::ostringstream osList;
            osList << "#" << std::dec << id << ":  break " << os.str();
            evnt._listMsg = osList.str();

            ret = "Breakpoint " + osList.str() + "\n";
        }
        else
        {
            std::ostringstream osList;
            osList << "#" << std::dec << id << ":  trace";
            if (REG_valid(reg)) osList << " " << GetRegName(reg);
            osList << " " << os.str();
            evnt._listMsg = osList.str();

            evnt._triggerMsg = os.str();
            evnt._reg        = reg;
            evnt._isDeleted  = FALSE;
            evnt._isEnabled  = TRUE;

            ret = "Tracepoint " + osList.str() + "\n";
        }

        evnt._type    = type;
        evnt._trigger = TRIGGER_JUMP_TO;
        evnt._pc      = addr;
        _events.insert(std::make_pair(id, evnt));
        Flush();
        return ret;
    }

    /*
     * Parse an event with trigger type TRIGGER_REG_IS.
     *
     *  @param[in] type         The type of event.
     *  @param[in] pcStr        The trigger's PC address.
     *  @param[in] regCheckStr  The trigger's register name.
     *  @param[in] valueStr     The trigger's register value.
     *  @param[in] regTraceStr  If not empty, the register to trace.
     *
     * @return  A string to return to the debugger prompt.
     */
    std::string ParseTriggerRegIsEvent(ETYPE type, const std::string& pcStr, const std::string& regCheckStr,
                                       const std::string& valueStr, const std::string& regTraceStr)
    {
        ADDRINT pc;
        if (!ParseNumber(pcStr, &pc))
        {
            std::ostringstream os;
            os << "Invalid address " << pcStr << "\n";
            return os.str();
        }

        REG regCheck = ParseRegName(regCheckStr);
        if (!REG_valid(regCheck))
        {
            std::ostringstream os;
            os << "Invalid register " << regCheckStr << " (see \"help registers\")\n";
            return os.str();
        }

        ADDRINT value;
        if (!ParseNumber(valueStr, &value))
        {
            std::ostringstream os;
            os << "Invalid value " << valueStr << "\n";
            return os.str();
        }

        REG regTrace = REG_INVALID();
        if (type == ETYPE_TRACEPOINT && !regTraceStr.empty())
        {
            regTrace = ParseRegName(regTraceStr);
            if (!REG_valid(regTrace))
            {
                std::ostringstream os;
                os << "Invalid register " << regTraceStr << " (see \"help registers\")\n";
                return os.str();
            }
        }

        unsigned id = _nextEventId++;

        std::ostringstream os;
        os << " at 0x" << std::hex << pc << " if " << GetRegName(regCheck) << " == 0x" << std::hex << value;

        EVENT evnt;
        std::string ret;
        if (type == ETYPE_BREAKPOINT)
        {
            std::ostringstream osTrigger;
            osTrigger << "Triggered breakpoint #" << std::dec << id << ": break " << os.str();
            evnt._triggerMsg = osTrigger.str();

            std::ostringstream osList;
            osList << "#" << std::dec << id << ":  break " << os.str();
            evnt._listMsg = osList.str();

            ret = "Breakpoint " + osList.str() + "\n";
        }
        else
        {
            std::ostringstream osList;
            osList << "#" << std::dec << id << ":  trace";
            if (REG_valid(regTrace)) osList << " " << GetRegName(regTrace);
            osList << " " << os.str();
            evnt._listMsg = osList.str();

            evnt._triggerMsg = os.str();
            evnt._reg        = regTrace;
            evnt._isDeleted  = FALSE;
            evnt._isEnabled  = TRUE;

            ret = "Tracepoint " + osList.str() + "\n";
        }

        evnt._type         = type;
        evnt._trigger      = TRIGGER_REG_IS;
        evnt._regIs._pc    = pc;
        evnt._regIs._reg   = regCheck;
        evnt._regIs._value = value;
        _events.insert(std::make_pair(id, evnt));
        Flush();
        return ret;
    }

    /*
     * Print an error message.
     *
     *  @param[in] message  Text of the error message.
     */
    void PrintError(const std::string& message) { PIN_WriteErrorMessage(message.c_str(), 1000, PIN_ERR_NONFATAL, 0); }

    /* -------------- Instrumentation Functions -------------- */

    /*
     * Pin call-back to instrument a trace.
     *
     *  @param[in] trace    Trace to instrument.
     *  @param[in] vme      Pointer to ISHELL instance.
     */
    static VOID InstrumentTrace(TRACE trace, void* vme)
    {
        SHELL* me = static_cast< SHELL* >(vme);

        for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
        {
            for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
            {
                // Insert breakpoints before tracepoints because we don't want a tracepoint
                // to log anything until after execution resumes from the breakpoint.
                //
                BOOL insertSkipClear = FALSE;
                BOOL insertRecordEa  = FALSE;
                me->InstrumentIns(ins, bbl, ETYPE_BREAKPOINT, &insertSkipClear, &insertRecordEa);
                me->InstrumentIns(ins, bbl, ETYPE_TRACEPOINT, &insertSkipClear, &insertRecordEa);

                // If there are any events with TRIGGER_STORE_VALUE_TO, record the store's effective address
                // at IPOINT_BEFORE.  We only need to do this once, even if there are many such events.
                //
                if (insertRecordEa)
                {
                    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)ReturnAddrint, IARG_CALL_ORDER, me->_clientArgs._callOrderBefore,
                                   IARG_FAST_ANALYSIS_CALL, IARG_MEMORYWRITE_EA, IARG_RETURN_REGS, me->_regRecordEa, IARG_END);
                }

                // If there are any "before" breakpoints, we need to clear the REG_SKIP_ONE
                // virtual register.
                //
                if (insertSkipClear) me->InsertSkipClear(ins);

                if (me->_clientArgs._enableIcountBreakpoints) me->InsertCountingInstrumentation(ins);
            }
        }
    }

    /*
     * Instrument an instruction.
     *
     *  @param[in] ins                  Instruction to instrument.
     *  @param[in] bbl                  Basic block containing \a ins.
     *  @param[in] type                 Only insert instrumentation for events of this type.
     *  @param[out] insertSkipClear     If this instructions needs instrumentation to clear the
     *                                   REG_SKIP_ONE register, \a insertSkipClear is set TRUE.
     *  @param[out] insertRecordEa      If this instructions needs instrumentation to record a
     *                                   store's effective address, \a insertRecordEa is set TRUE.
     */
    VOID InstrumentIns(INS ins, BBL bbl, ETYPE type, BOOL* insertSkipClear, BOOL* insertRecordEa)
    {
        for (EVENTS::iterator it = _events.begin(); it != _events.end(); ++it)
        {
            if (it->second._type != type) continue;

            if (type == ETYPE_TRACEPOINT && (it->second._isDeleted || !it->second._isEnabled)) continue;

            switch (it->second._trigger)
            {
                case TRIGGER_AT:
                    if (INS_Address(ins) == it->second._pc)
                    {
                        if (type == ETYPE_BREAKPOINT)
                        {
                            InsertBreakpoint(ins, bbl, FALSE, IPOINT_BEFORE, it->second);
                            *insertSkipClear = TRUE;
                        }
                        else
                        {
                            InsertTracepoint(ins, bbl, FALSE, IPOINT_BEFORE, it->first, it->second);
                        }
                    }
                    break;

                case TRIGGER_AT_ICOUNT:
                    if (type == ETYPE_BREAKPOINT)
                    {
                        InsertIcountBreakpoint(ins, bbl, it->second);
                        *insertSkipClear = TRUE;
                    }
                    break;

                case TRIGGER_AT_MCOUNT:
                    if (type == ETYPE_BREAKPOINT)
                    {
                        if (INS_IsMemoryRead(ins) || INS_IsMemoryWrite(ins))
                        {
                            InsertMcountBreakpoint(ins, bbl, it->second);
                            *insertSkipClear = TRUE;
                        }
                    }
                    break;

                case TRIGGER_LOAD_FROM:
                    if (INS_IsMemoryRead(ins))
                    {
                        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)CheckAddrint, IARG_CALL_ORDER, _clientArgs._callOrderBefore,
                                         IARG_FAST_ANALYSIS_CALL, IARG_MEMORYREAD_EA, IARG_ADDRINT, it->second._ea, IARG_END);
                        if (type == ETYPE_BREAKPOINT)
                        {
                            InsertBreakpoint(ins, bbl, TRUE, IPOINT_BEFORE, it->second);
                            *insertSkipClear = TRUE;
                        }
                        else
                        {
                            InsertTracepoint(ins, bbl, TRUE, IPOINT_BEFORE, it->first, it->second);
                        }
                    }
                    break;

                case TRIGGER_STORE_TO:
                    if (INS_IsMemoryWrite(ins))
                    {
                        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)CheckAddrint, IARG_CALL_ORDER, _clientArgs._callOrderBefore,
                                         IARG_FAST_ANALYSIS_CALL, IARG_MEMORYWRITE_EA, IARG_ADDRINT, it->second._ea, IARG_END);
                        if (type == ETYPE_BREAKPOINT)
                        {
                            InsertBreakpoint(ins, bbl, TRUE, IPOINT_BEFORE, it->second);
                            *insertSkipClear = TRUE;
                        }
                        else
                        {
                            InsertTracepoint(ins, bbl, TRUE, IPOINT_BEFORE, it->first, it->second);
                        }
                    }
                    break;

                case TRIGGER_LOAD_VALUE_FROM:
                    if (INS_IsMemoryRead(ins))
                    {
                        if (type == ETYPE_BREAKPOINT) *insertSkipClear = TRUE;

                        InstrumentLoadValueFrom(ins, bbl, it->first, it->second);
                    }
                    break;

                case TRIGGER_STORE_VALUE_TO:
                    if (INS_IsMemoryWrite(ins))
                    {
                        *insertRecordEa = TRUE;
                        InstrumentStoreValueTo(ins, bbl, it->first, it->second);
                    }
                    break;

                case TRIGGER_JUMP_TO:
                    if (INS_IsControlFlow(ins))
                    {
                        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)CheckAddrint, IARG_CALL_ORDER, _clientArgs._callOrderBefore,
                                         IARG_FAST_ANALYSIS_CALL, IARG_BRANCH_TARGET_ADDR, IARG_ADDRINT, it->second._pc,
                                         IARG_END);
                        if (type == ETYPE_BREAKPOINT)
                        {
                            InsertBreakpoint(ins, bbl, TRUE, IPOINT_BEFORE, it->second);
                            *insertSkipClear = TRUE;
                        }
                        else
                        {
                            InsertTracepoint(ins, bbl, TRUE, IPOINT_BEFORE, it->first, it->second);
                        }
                    }
                    break;

                case TRIGGER_REG_IS:
                    if (INS_Address(ins) == it->second._regIs._pc)
                    {
                        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)CheckAddrint, IARG_CALL_ORDER, _clientArgs._callOrderBefore,
                                         IARG_FAST_ANALYSIS_CALL, IARG_REG_VALUE, it->second._regIs._reg, IARG_ADDRINT,
                                         it->second._regIs._value, IARG_END);
                        if (type == ETYPE_BREAKPOINT)
                        {
                            InsertBreakpoint(ins, bbl, TRUE, IPOINT_BEFORE, it->second);
                            *insertSkipClear = TRUE;
                        }
                        else
                        {
                            InsertTracepoint(ins, bbl, TRUE, IPOINT_BEFORE, it->first, it->second);
                        }
                    }
                    break;
            }
        }
    }

    /*
     * Instrument an instruction with a TRIGGER_AT_ICOUNT event.
     *
     *  @param[in] ins                   The instruction.
     *  @param[in] bbl                   The basic block containing \a ins.
     *  @param[in] evnt                  The event descrption.
     */
    VOID InsertIcountBreakpoint(INS ins, BBL bbl, const EVENT& evnt)
    {
        ASSERTX(_clientArgs._enableIcountBreakpoints);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)CheckIcount, IARG_CALL_ORDER, _clientArgs._callOrderBefore,
                         IARG_FAST_ANALYSIS_CALL, IARG_REG_VALUE, _regThreadData, IARG_PTR, &evnt._atIcount, IARG_END);

        InsertBreakpoint(ins, bbl, TRUE, IPOINT_BEFORE, evnt);
    }

    /*
     * Instrument a memory instruction with a TRIGGER_AT_MCOUNT event.
     *
     *  @param[in] ins                   The memory instruction.
     *  @param[in] bbl                   The basic block containing \a ins.
     *  @param[in] evnt                  The event description.
     */
    VOID InsertMcountBreakpoint(INS ins, BBL bbl, const EVENT& evnt)
    {
        ASSERTX(_clientArgs._enableIcountBreakpoints);

        if (INS_HasRealRep(ins) && !_clientArgs._countZeroRepAsMemOp)
        {
            INS_InsertIfPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)CheckPredMcount, IARG_CALL_ORDER,
                                       _clientArgs._callOrderBefore, IARG_FAST_ANALYSIS_CALL, IARG_REG_VALUE, _regThreadData,
                                       IARG_PTR, &evnt._atMcount, IARG_EXECUTING, IARG_END);
        }
        else
        {
            INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)CheckMcount, IARG_CALL_ORDER, _clientArgs._callOrderBefore,
                             IARG_FAST_ANALYSIS_CALL, IARG_REG_VALUE, _regThreadData, IARG_PTR, &evnt._atMcount, IARG_END);
        }
        InsertBreakpoint(ins, bbl, TRUE, IPOINT_BEFORE, evnt);
    }

    VOID InsertCountingInstrumentation(INS ins)
    {
        BOOL isMemory = INS_IsMemoryRead(ins) || INS_IsMemoryWrite(ins);

        if (INS_IsPrefetch(ins) && !_clientArgs._countPrefetchAsMemOp) isMemory = FALSE;

        if (isMemory)
        {
            if (INS_HasRealRep(ins) && !_clientArgs._countZeroRepAsMemOp)
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IncrementIcount, IARG_CALL_ORDER, _clientArgs._callOrderBefore,
                               IARG_FAST_ANALYSIS_CALL, IARG_REG_VALUE, _regThreadData, IARG_END);

                INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)IncrementMcount, IARG_CALL_ORDER,
                                         _clientArgs._callOrderBefore, IARG_FAST_ANALYSIS_CALL, IARG_REG_VALUE, _regThreadData,
                                         IARG_END);
            }
            else
            {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IncrementIMcount, IARG_CALL_ORDER, _clientArgs._callOrderBefore,
                               IARG_FAST_ANALYSIS_CALL, IARG_REG_VALUE, _regThreadData, IARG_END);
            }
        }
        else
        {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IncrementIcount, IARG_CALL_ORDER, _clientArgs._callOrderBefore,
                           IARG_FAST_ANALYSIS_CALL, IARG_REG_VALUE, _regThreadData, IARG_END);
        }
    }

    /*
     * Instrument a load instruction with a TRIGGER_LOAD_VALUE_FROM event.
     *
     *  @param[in] ins      The load instruction.
     *  @param[in] bbl      The basic block containing \a ins.
     *  @param[in] id       The event ID.
     *  @param[in] evnt     The event descrption.
     */
    VOID InstrumentLoadValueFrom(INS ins, BBL bbl, unsigned id, const EVENT& evnt)
    {
        UINT32 readOperandSize = 0;
        for (UINT32 index = 0; index < INS_MemoryOperandCount(ins); index++)
        {
            if (INS_MemoryOperandIsRead(ins, index))
            {
                readOperandSize = INS_MemoryOperandSize(ins, index);
                break;
            }
        }
        switch (readOperandSize)
        {
            case 1:
                InstrumentLoadValueFromForSize< UINT8 >(ins, bbl, id, evnt, (AFUNPTR)CheckAddressAndValue8);
                break;
            case 2:
                InstrumentLoadValueFromForSize< UINT16 >(ins, bbl, id, evnt, (AFUNPTR)CheckAddressAndValue16);
                break;
            case 4:
                InstrumentLoadValueFromForSize< UINT32 >(ins, bbl, id, evnt, (AFUNPTR)CheckAddressAndValue32);
                break;
            case 8:
                if (sizeof(ADDRINT) >= sizeof(UINT64))
                    InstrumentLoadValueFromForSize< UINT64 >(ins, bbl, id, evnt, (AFUNPTR)CheckAddressAndValueAddrint);
                else
                    InstrumentLoadValue64HiLo(ins, bbl, id, evnt);
                break;
        }
    }

    /*
     * Instrument a store instruction with a TRIGGER_STORE_VALUE_TO event.
     *
     *  @param[in] ins      The  instruction.
     *  @param[in] bbl      The basic block containing \a ins.
     *  @param[in] id       The event ID.
     *  @param[in] evnt     The event descrption.
     */
    VOID InstrumentStoreValueTo(INS ins, BBL bbl, unsigned id, const EVENT& evnt)
    {
        UINT32 writeOperandSize = 0;
        for (UINT32 index = 0; index < INS_MemoryOperandCount(ins); index++)
        {
            if (INS_MemoryOperandIsWritten(ins, index))
            {
                writeOperandSize = INS_MemoryOperandSize(ins, index);
                break;
            }
        }
        switch (writeOperandSize)
        {
            case 1:
                InstrumentStoreValueToForSize< UINT8 >(ins, bbl, id, evnt, (AFUNPTR)CheckAddressAndValue8);
                break;
            case 2:
                InstrumentStoreValueToForSize< UINT16 >(ins, bbl, id, evnt, (AFUNPTR)CheckAddressAndValue16);
                break;
            case 4:
                InstrumentStoreValueToForSize< UINT32 >(ins, bbl, id, evnt, (AFUNPTR)CheckAddressAndValue32);
                break;
            case 8:
                if (sizeof(ADDRINT) >= sizeof(UINT64))
                    InstrumentStoreValueToForSize< UINT64 >(ins, bbl, id, evnt, (AFUNPTR)CheckAddressAndValueAddrint);
                else
                    InstrumentStoreValue64HiLo(ins, bbl, id, evnt);
                break;
        }
    }

    /*
     * Instrument a load instruction with a TRIGGER_LOAD_VALUE_FROM event.
     *
     *  @tparam UINTX           One of the UINT types, matching the size of the store.
     *                           There is an assumption that sizeof(UINTX) <= sizeof(ADDRINT).
     *  @param[in] ins          The load instruction.
     *  @param[in] bbl          The basic block containing \a ins.
     *  @param[in] id           The event ID.
     *  @param[in] evnt         The event descrption.
     *  @param[in] CheckLoadX  One of the CheckAddressAndValue() analysis functions, matching the
     *                           size of the load.
     */
    template< typename UINTX >
    VOID InstrumentLoadValueFromForSize(INS ins, BBL bbl, unsigned id, const EVENT& evnt, AFUNPTR CheckLoadX)
    {
        UINT64 value = evnt._loadValueFrom._value;
        ETYPE type   = evnt._type;

        if (static_cast< UINTX >(value) == value)
        {
            INS_InsertIfCall(ins, IPOINT_BEFORE, CheckLoadX, IARG_CALL_ORDER, _clientArgs._callOrderAfter,
                             IARG_FAST_ANALYSIS_CALL, IARG_MEMORYREAD_EA, IARG_ADDRINT, evnt._loadValueFrom._ea, IARG_ADDRINT,
                             static_cast< ADDRINT >(value), IARG_END);

            if (type == ETYPE_BREAKPOINT)
                InsertBreakpoint(ins, bbl, TRUE, IPOINT_BEFORE, evnt);
            else
                InsertTracepoint(ins, bbl, TRUE, IPOINT_BEFORE, id, evnt);
        }
    }

    /*
     * Instrument a store instruction with a TRIGGER_STORE_VALUE_TO event.
     *
     *  @tparam UINTX           One of the UINT types, matching the size of the store.
     *                           There is an assumption that sizeof(UINTX) <= sizeof(ADDRINT).
     *  @param[in] ins          The store instruction.
     *  @param[in] bbl          The basic block containing \a ins.
     *  @param[in] id           The event ID.
     *  @param[in] evnt         The event descrption.
     *  @param[in] CheckStoreX  One of the CheckAddressAndValue() analysis functions, matching the
     *                           size of the store.
     */
    template< typename UINTX >
    VOID InstrumentStoreValueToForSize(INS ins, BBL bbl, unsigned id, const EVENT& evnt, AFUNPTR CheckStoreX)
    {
        UINT64 value = evnt._storeValueTo._value;
        ETYPE type   = evnt._type;

        if (static_cast< UINTX >(value) == value)
        {
            if (INS_IsValidForIpointAfter(ins))
            {
                INS_InsertIfCall(ins, IPOINT_AFTER, CheckStoreX, IARG_CALL_ORDER, _clientArgs._callOrderAfter,
                                 IARG_FAST_ANALYSIS_CALL, IARG_REG_VALUE, _regRecordEa, IARG_ADDRINT, evnt._storeValueTo._ea,
                                 IARG_ADDRINT, static_cast< ADDRINT >(value), IARG_END);
                if (type == ETYPE_BREAKPOINT)
                    InsertBreakpoint(ins, bbl, TRUE, IPOINT_AFTER, evnt);
                else
                    InsertTracepoint(ins, bbl, TRUE, IPOINT_AFTER, id, evnt);
            }
            if (INS_IsValidForIpointTakenBranch(ins))
            {
                INS_InsertIfCall(ins, IPOINT_TAKEN_BRANCH, CheckStoreX, IARG_CALL_ORDER, _clientArgs._callOrderAfter,
                                 IARG_FAST_ANALYSIS_CALL, IARG_REG_VALUE, _regRecordEa, IARG_ADDRINT, evnt._storeValueTo._ea,
                                 IARG_ADDRINT, static_cast< ADDRINT >(value), IARG_END);
                if (type == ETYPE_BREAKPOINT)
                    InsertBreakpoint(ins, bbl, TRUE, IPOINT_TAKEN_BRANCH, evnt);
                else
                    InsertTracepoint(ins, bbl, TRUE, IPOINT_TAKEN_BRANCH, id, evnt);
            }
        }
    }

    /*
     * Instrument a 64-bit load instruction with a TRIGGER_LOAD_VALUE_FROM event.
     * The value is checked using high and low ADDRINT parts (where ADDRINT is assumed
     * to be 32-bits).
     *
     *  @param[in] ins          The load instruction.
     *  @param[in] bbl          The basic block containing \a ins.
     *  @param[in] id           The event ID.
     *  @param[in] evnt         The event descrption.
     */
    VOID InstrumentLoadValue64HiLo(INS ins, BBL bbl, unsigned id, const EVENT& evnt)
    {
        UINT64 value = evnt._loadValueFrom._value;
        ETYPE type   = evnt._type;
        ADDRINT hi   = static_cast< ADDRINT >(value >> 32);
        ADDRINT lo   = static_cast< ADDRINT >(value);

        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)CheckAddressAndValue64, IARG_CALL_ORDER, _clientArgs._callOrderAfter,
                         IARG_FAST_ANALYSIS_CALL, IARG_MEMORYREAD_EA, IARG_ADDRINT, evnt._loadValueFrom._ea, IARG_ADDRINT, hi,
                         IARG_ADDRINT, lo, IARG_END);

        if (type == ETYPE_BREAKPOINT)
            InsertBreakpoint(ins, bbl, TRUE, IPOINT_BEFORE, evnt);
        else
            InsertTracepoint(ins, bbl, TRUE, IPOINT_BEFORE, id, evnt);
    }

    /*
     * Instrument a 64-bit store instruction with a TRIGGER_STORE_VALUE_TO event.
     * The value is checked using high and low ADDRINT parts (where ADDRINT is assumed
     * to be 32-bits).
     *
     *  @param[in] ins          The store instruction.
     *  @param[in] bbl          The basic block containing \a ins.
     *  @param[in] id           The event ID.
     *  @param[in] evnt         The event descrption.
     */
    VOID InstrumentStoreValue64HiLo(INS ins, BBL bbl, unsigned id, const EVENT& evnt)
    {
        UINT64 value = evnt._storeValueTo._value;
        ETYPE type   = evnt._type;
        ADDRINT hi   = static_cast< ADDRINT >(value >> 32);
        ADDRINT lo   = static_cast< ADDRINT >(value);

        if (INS_IsValidForIpointAfter(ins))
        {
            INS_InsertIfCall(ins, IPOINT_AFTER, (AFUNPTR)CheckAddressAndValue64, IARG_CALL_ORDER, _clientArgs._callOrderAfter,
                             IARG_FAST_ANALYSIS_CALL, IARG_REG_VALUE, _regRecordEa, IARG_ADDRINT, evnt._storeValueTo._ea,
                             IARG_ADDRINT, hi, IARG_ADDRINT, lo, IARG_END);
            if (type == ETYPE_BREAKPOINT)
                InsertBreakpoint(ins, bbl, TRUE, IPOINT_AFTER, evnt);
            else
                InsertTracepoint(ins, bbl, TRUE, IPOINT_AFTER, id, evnt);
        }
        if (INS_IsValidForIpointTakenBranch(ins))
        {
            INS_InsertIfCall(ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)CheckAddressAndValue64, IARG_CALL_ORDER,
                             _clientArgs._callOrderAfter, IARG_FAST_ANALYSIS_CALL, IARG_REG_VALUE, _regRecordEa, IARG_ADDRINT,
                             evnt._storeValueTo._ea, IARG_ADDRINT, hi, IARG_ADDRINT, lo, IARG_END);
            if (type == ETYPE_BREAKPOINT)
                InsertBreakpoint(ins, bbl, TRUE, IPOINT_TAKEN_BRANCH, evnt);
            else
                InsertTracepoint(ins, bbl, TRUE, IPOINT_TAKEN_BRANCH, id, evnt);
        }
    }

    /*
     * Add the instrumentation for a call to the breakpoint analysis routine.
     *
     *  @param[in] ins      The instruction being instrumented.
     *  @param[in] bbl      The basic block containing \a ins.
     *  @param[in] isThen   TRUE if this should be a "then" instrumentation call.
     *  @param[in] ipoint   Where to place the instrumentation.
     *  @param[in] evnt     The ETYPE_BREAKPOINT event description.
     */
    VOID InsertBreakpoint(INS ins, BBL bbl, BOOL isThen, IPOINT ipoint, const EVENT& evnt)
    {
        ASSERTX(evnt._type == ETYPE_BREAKPOINT);

        // Breakpoints alwas use "then" instrumentation currently.  If that ever changes,
        // we need to extend the ICUSTOM_INSTRUMENTOR interface to communicate "then"
        // vs. non-"then" instrumentation to the client.
        //
        ASSERTX(isThen);

        if (_clientArgs._customInstrumentor)
        {
            if (ipoint == IPOINT_BEFORE)
            {
                _clientArgs._customInstrumentor->InsertBreakpointBefore(ins, bbl, _clientArgs._callOrderBefore, evnt._triggerMsg);
            }
            else
            {
                _clientArgs._customInstrumentor->InsertBreakpointAfter(ins, bbl, ipoint, _clientArgs._callOrderAfter,
                                                                       evnt._triggerMsg);
            }
            return;
        }

        if (ipoint == IPOINT_BEFORE)
        {
            INS_InsertThenCall(ins, ipoint, (AFUNPTR)TriggerBreakpointBefore, IARG_CALL_ORDER, _clientArgs._callOrderBefore,
                               IARG_CONST_CONTEXT, // IARG_CONST_CONTEXT has much lower overhead
                                                   // than IARG_CONTEX fog passing the CONTEXT*
                                                   // to the analysis routine. Note that IARG_CONST_CONTEXT
                                                   // passes a read-only CONTEXT* to the analysis routine
                               IARG_THREAD_ID, IARG_UINT32, static_cast< UINT32 >(_regSkipOne), IARG_PTR,
                               evnt._triggerMsg.c_str(), IARG_END);
        }
        else
        {
            INS_InsertThenCall(ins, ipoint, (AFUNPTR)TriggerBreakpointAfter, IARG_CALL_ORDER, _clientArgs._callOrderAfter,
                               IARG_CONST_CONTEXT, // IARG_CONST_CONTEXT has much lower overhead
                                                   // than IARG_CONTEX fog passing the CONTEXT*
                                                   // to the analysis routine. Note that IARG_CONST_CONTEXT
                                                   // passes a read-only CONTEXT* to the analysis routine
                               IARG_INST_PTR, IARG_THREAD_ID, IARG_PTR, evnt._triggerMsg.c_str(), IARG_END);
        }
    }

    /*
     * Add the instrumentation for a call to the tracepoint analysis routine.
     *
     *  @param[in] ins      The instruction being instrumented.
     *  @param[in] bbl      The basic block containing \a ins.
     *  @param[in] isThen   TRUE if this should be a "then" instrumentation call.
     *  @param[in] ipoint   Where to place the instrumentation.
     *  @param[in] id       The event ID.
     *  @param[in] evnt     The ETYPE_TRACEPOINT event description.
     */
    VOID InsertTracepoint(INS ins, BBL bbl, BOOL isThen, IPOINT ipoint, unsigned id, const EVENT& evnt)
    {
        ASSERTX(evnt._type == ETYPE_TRACEPOINT);

        CALL_ORDER order;
        if (ipoint == IPOINT_BEFORE)
            order = _clientArgs._callOrderBefore;
        else
            order = _clientArgs._callOrderAfter;

        if (REG_valid(evnt._reg))
        {
            if (isThen)
            {
                INS_InsertThenCall(ins, ipoint, (AFUNPTR)RecordTracepointAndReg, IARG_CALL_ORDER, order, IARG_PTR, this,
                                   IARG_UINT32, static_cast< UINT32 >(id), IARG_INST_PTR, IARG_REG_VALUE, evnt._reg, IARG_END);
            }
            else
            {
                INS_InsertCall(ins, ipoint, (AFUNPTR)RecordTracepointAndReg, IARG_CALL_ORDER, order, IARG_PTR, this, IARG_UINT32,
                               static_cast< UINT32 >(id), IARG_INST_PTR, IARG_REG_VALUE, evnt._reg, IARG_END);
            }
        }
        else
        {
            if (isThen)
            {
                INS_InsertThenCall(ins, ipoint, (AFUNPTR)RecordTracepoint, IARG_CALL_ORDER, order, IARG_PTR, this, IARG_UINT32,
                                   static_cast< UINT32 >(id), IARG_INST_PTR, IARG_END);
            }
            else
            {
                INS_InsertCall(ins, ipoint, (AFUNPTR)RecordTracepoint, IARG_CALL_ORDER, order, IARG_PTR, this, IARG_UINT32,
                               static_cast< UINT32 >(id), IARG_INST_PTR, IARG_END);
            }
        }
    }

    /*
     * Insert instrumentation after an instruction to clear the "skip one" flag.
     *
     *  @param[in] ins  The instruction.
     */
    VOID InsertSkipClear(INS ins)
    {
        if (INS_IsValidForIpointAfter(ins))
        {
            INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)ReturnZero, IARG_CALL_ORDER, _clientArgs._callOrderAfter,
                           IARG_FAST_ANALYSIS_CALL, IARG_RETURN_REGS, _regSkipOne, IARG_END);
        }
        if (INS_IsValidForIpointTakenBranch(ins))
        {
            INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)ReturnZero, IARG_CALL_ORDER, _clientArgs._callOrderAfter,
                           IARG_FAST_ANALYSIS_CALL, IARG_RETURN_REGS, _regSkipOne, IARG_END);
        }
    }

    /* -------------- Analysis Functions -------------- */

    /*
     * These are all analysis routines that check for a trigger condition.  They
     * should all be fast, and we expect Pin to inline them.
     */
    static ADDRINT PIN_FAST_ANALYSIS_CALL CheckAddrint(ADDRINT a, ADDRINT b) { return (a == b); }

    static ADDRINT PIN_FAST_ANALYSIS_CALL CheckAddressAndValue8(ADDRINT ea, ADDRINT expect, ADDRINT value)
    {
        return (ea == expect) && (*reinterpret_cast< UINT8* >(ea) == static_cast< UINT8 >(value));
    }

    static ADDRINT PIN_FAST_ANALYSIS_CALL CheckAddressAndValue16(ADDRINT ea, ADDRINT expect, ADDRINT value)
    {
        return (ea == expect) && (*reinterpret_cast< UINT16* >(ea) == static_cast< UINT16 >(value));
    }

    static ADDRINT PIN_FAST_ANALYSIS_CALL CheckAddressAndValue32(ADDRINT ea, ADDRINT expect, ADDRINT value)
    {
        return (ea == expect) && (*reinterpret_cast< UINT32* >(ea) == static_cast< UINT32 >(value));
    }

    static ADDRINT PIN_FAST_ANALYSIS_CALL CheckAddressAndValueAddrint(ADDRINT ea, ADDRINT expect, ADDRINT value)
    {
        return (ea == expect) && (*reinterpret_cast< ADDRINT* >(ea) == value);
    }

    static ADDRINT PIN_FAST_ANALYSIS_CALL CheckAddressAndValue64(ADDRINT ea, ADDRINT expect, ADDRINT valueHi, ADDRINT valueLo)
    {
        UINT64 value = (static_cast< UINT64 >(valueHi) << 32) | valueLo;
        return (ea == expect) && (*reinterpret_cast< UINT64* >(ea) == value);
    }

    // check if the icount and the thread match the expected by the breakpoint
    static ADDRINT PIN_FAST_ANALYSIS_CALL CheckIcount(THREAD_DATA* td, AT_ICOUNT* expected)
    {
        // bit-wise "and" because logical "and" does not produce inline-able code
        return (td->_icount == expected->_icount) & (td->_tid == expected->_tid);
    }

    // check if the mcount and the thread match the expected by the breakpoint
    static ADDRINT PIN_FAST_ANALYSIS_CALL CheckMcount(THREAD_DATA* td, AT_MCOUNT* expected)
    {
        // bit-wise "and" because logical "and" does not produce inline-able code
        return (td->_mcount == expected->_mcount) & (td->_tid == expected->_tid);
    }

    static ADDRINT PIN_FAST_ANALYSIS_CALL CheckPredMcount(THREAD_DATA* td, AT_MCOUNT* expected, BOOL executing)
    {
        // bit-wise "and" because logical "and" does not produce inline-able code
        return executing & static_cast< BOOL >(CheckMcount(td, expected));
    }

    /*
     * These are utility analysis routines that return values to be stored in a Pin virtual
     * register.  They are meant to be used with IARG_RETURN_REGS.
     */
    static ADDRINT PIN_FAST_ANALYSIS_CALL ReturnZero() { return 0; }

    static ADDRINT PIN_FAST_ANALYSIS_CALL ReturnAddrint(ADDRINT a) { return a; }

    /*
     * Analysis routine to keep track of the debugger shell state, such as instruction
     * count or memory count
     */
    static VOID PIN_FAST_ANALYSIS_CALL IncrementIcount(THREAD_DATA* td) { td->_icount++; }

    static VOID PIN_FAST_ANALYSIS_CALL IncrementMcount(THREAD_DATA* td) { td->_mcount++; }

    static VOID PIN_FAST_ANALYSIS_CALL IncrementIMcount(THREAD_DATA* td)
    {
        td->_icount++;
        td->_mcount++;
    }

    /*
     * Trigger a breakpoint that occurs before an instruction.
     *
     *  @param[in] ctxt         Register state before the instruction.
     *                          NOTE that since IARG_CONST_CONTEXT was specified
     *                          this ctxt is read-only
     *  @param[in] tid          The calling thread.
     *  @param[in] regSkipOne   The REG_SKIP_ONE Pin virtual register.
     *  @param[in] message      Tells what breakpoint was triggered.
     */
    static VOID TriggerBreakpointBefore(CONTEXT* ctxt, THREADID tid, UINT32 regSkipOne, const char* message)
    {
        // When we resume from the breakpoint, this analysis routine is re-executed.
        // This logic prevents the breakpoint from being re-triggered when we resume.
        // The REG_SKIP_ONE virtual register is cleared in the instruction's "after"
        // analysis function.
        //
        ADDRINT skipPc = PIN_GetContextReg(ctxt, static_cast< REG >(regSkipOne));
        ADDRINT pc     = PIN_GetContextReg(ctxt, REG_INST_PTR);
        if (skipPc == pc) return;

        CONTEXT writableContext;
        // since IARG_CONST_CONTEXT was specified  ctxt is read-only
        // need to copy the ctxt into a writable context in order to do
        // the following PIN_SetContextReg
        PIN_SaveContext(ctxt, &writableContext);

        PIN_SetContextReg(&writableContext, static_cast< REG >(regSkipOne), pc);
        PIN_ApplicationBreakpoint(&writableContext, tid, FALSE, message);
    }

    /*
     * Trigger a breakpoint that occurs after an instruction.
     *
     *  @param[in] ctxt     Register state after the instruction (PC points to next instruction).
     *  @param[in] pc       PC of instruction that triggered the breakpoint.
     *  @param[in] tid      The calling thread.
     *  @param[in] message  Tells what breakpoint was triggered.
     */
    static VOID TriggerBreakpointAfter(CONTEXT* ctxt, ADDRINT pc, THREADID tid, const char* message)
    {
        // Note, we don't need any special logic to prevent re-triggering this breakpoint
        // when we resume because 'ctxt' points at the next instruction.  When resuming, we
        // start executing at the next instruction, so avoid re-evaluating the breakpoint
        // condition.

        // Tell the user the PC of the instruction that triggered the breakpoint because
        // the PC in 'ctxt' points at the next instruction.  Otherwise, the triggering instruction
        // might not be obvious if it was a CALL or branch instruction.
        //
        std::ostringstream os;
        os << message << "\n";
        os << "Breakpoint triggered after instruction at 0x" << std::hex << pc;

        PIN_ApplicationBreakpoint(ctxt, tid, FALSE, os.str());
    }

    /*
     * Record a tracepoint with no register value.
     *
     *  @param[in] me   Points to our SHELL object.
     *  @param[in] id   Event ID for the tracepoint description.
     *  @param[in] pc   Trigger PC for tracepoint.
     */
    static VOID RecordTracepoint(SHELL* me, UINT32 id, ADDRINT pc)
    {
        TRACEREC rec;
        rec._id = static_cast< unsigned >(id);
        rec._pc = pc;

        PIN_GetLock(&me->_traceLock, 1);
        me->_traceLog.push_back(rec);
        PIN_ReleaseLock(&me->_traceLock);
    }

    /*
     * Record a tracepoint with a register value.
     *
     *  @param[in] me           Points to our SHELL object.
     *  @param[in] id           Event ID for the tracepoint description.
     *  @param[in] pc           Trigger PC for tracepoint.
     *  @param[in] regValue     Trigger PC for tracepoint.
     */
    static VOID RecordTracepointAndReg(SHELL* me, UINT32 id, ADDRINT pc, ADDRINT regValue)
    {
        TRACEREC rec;
        rec._id       = static_cast< unsigned >(id);
        rec._pc       = pc;
        rec._regValue = regValue;

        PIN_GetLock(&me->_traceLock, 1);
        me->_traceLog.push_back(rec);
        PIN_ReleaseLock(&me->_traceLock);
    }
};

DEBUGGER_SHELL::ISHELL* DEBUGGER_SHELL::CreateShell()
{
    SHELL* shell = new SHELL();
    if (!shell->Construct())
    {
        delete shell;
        return 0;
    }
    return shell;
}
