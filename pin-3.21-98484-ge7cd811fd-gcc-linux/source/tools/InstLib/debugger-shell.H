/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef DEBUGGER_SHELL_H
#define DEBUGGER_SHELL_H

#include <string>
#include "pin.H"

/*! @defgroup DEBUGGER_SHELL
 * Pin tools that run with the "-appdebug" switch can use this class to implement
 * some common custom debugger commands.  Once enabled, a user can type these
 * commands interactively at the debugger prompt.  In GDB, type "monitor help" to
 * see a list of the available commands.
 *
 * Typical usage in the tool is as follows:
 *
 *                                                                      @code
 *  #include "debugger-shell.H"
 *
 *  int main(int argc, char **argv)
 *  {
 *      if (PIN_Init(argc,argv))
 *          return 1;
 *
 *      DEBUGGER_SHELL::ISHELL *shell = DEBUGGER_SHELL::CreateShell();
 *      DEBUGGER_SHELL::STARTUP_ARGUMENTS args;
 *      if (!shell->Enable(args))
 *          return 1;
 *
 *      PIN_StartProgram();
 *  }
 *                                                                      @endcode
 *
 * The tool must also compile and link with "debugger-shell.cpp".
 */
namespace DEBUGGER_SHELL
{
struct STARTUP_ARGUMENTS;
class ICUSTOM_INSTRUMENTOR;

/*! @ingroup DEBUGGER_SHELL
 * The public interface to the debugger shell.
 */
class ISHELL
{
  public:
    /*!
     * This method must be called once, typically from the tool's main() function.
     * It enables the Pin instrumentation which implements the debugger extensions.
     *
     *  @param[in] args     Arguments that customize the debugger shell.
     *
     * @return  TRUE on success.  If there is an error, a diagnostic is printed
     *           and this method returns FALSE.
     */
    virtual BOOL Enable(const STARTUP_ARGUMENTS& args) = 0;

    /*!
     * Tools that use the debugger shell may still implement their own extended debugger
     * commands by using PIN_AddDebugInterpreter().  The debugger shell provides a way
     * for such tools to include a help message for these extended commands in the help
     * message that the debugger shell prints.  Tools can use AddExtendedHelpCategory()
     * to define their own category of help messages.
     *
     *  @param[in] name             The name of the category.
     *  @param[in] description      Describes this category of extended commands.
     *  @param[out] alreadyExists   If not NULL, receives TRUE if a tool already added
     *                               a custom category for \a name.  In this case, the
     *                               debugger shell uses the previous description.
     *
     * @return  The ID for this custom category.  If a tool already added a category
     *           named \a name, that previous category ID is returned.
     */
    virtual unsigned AddExtendedHelpCategory(const std::string& name, const std::string& description, BOOL* alreadyExists) = 0;

    /*!
     * Adds a help message for an extended command that is implemented by the layered tool.
     * See also AddExtendedHelpCategory() for more information on adding extended debugger
     * commands.
     *
     *  @param[in] category     The category for this extended command.  This can either be
     *                           one of the predefined HELP_CATEGORY constants or a value
     *                           returned by AddExtendedHelpCategory().
     *  @param[in] cmd          The name of a tool's extended command.
     *  @param[in] description  Describes what the command does.
     */
    virtual void AddExtendedHelpMessage(unsigned category, const std::string& cmd, const std::string& description) = 0;

    /*!
     * Tools that override the default instrumentation with ICUSTOM_INSTRUMENTOR
     * may need a virtual register to hold the "skip one" flag.  This method provides the
     * register number to use for this.
     *
     * @return  The Pin virtual register to use for the "skip one" flag.  This is one
     *           of the REG_INST_Gn registers.
     */
    virtual REG GetSkipOneRegister() = 0;

    virtual inline ~ISHELL() = 0; ///< Destroys the debugger shell object.
};

ISHELL::~ISHELL() {}

/*! @ingroup DEBUGGER_SHELL
 * Create a debugger shell object.
 *
 * @return  A new debugger shell object on success.  If there is an error, a diagnostic
 *           is printed and this method returns NULL.
 */
ISHELL* CreateShell();

/*! @ingroup DEBUGGER_SHELL
 * Arguments that customize the debugger shell.
 */
struct STARTUP_ARGUMENTS
{
    STARTUP_ARGUMENTS()
        : _callOrderBefore(CALL_ORDER_DEFAULT), _callOrderAfter(CALL_ORDER_DEFAULT), _enableIcountBreakpoints(FALSE),
          _countPrefetchAsMemOp(FALSE), _countZeroRepAsMemOp(FALSE), _customInstrumentor(0)
    {}

    /*!
     * Tells the relative position of instrumentation added by the debugger shell
     * for "before" and "after" instrumentation.
     */
    CALL_ORDER _callOrderBefore;
    CALL_ORDER _callOrderAfter;

    // The following fields enable breakpoints that require instruction counting.
    // When enabled, the debugger-shell always inserts instrumentation that counts
    // the number of instructions executed by each thread.
    //
    BOOL _enableIcountBreakpoints; ///< Enable instruction-counting breakpoints.
    BOOL _countPrefetchAsMemOp;    ///< Count prefetch instructions as memory operations.
    BOOL _countZeroRepAsMemOp;     ///< Count zero-repetition REP instructions as one memory operation.

    /*!
     * Allows client to override some instrumentation.
     */
    ICUSTOM_INSTRUMENTOR* _customInstrumentor;
};

/*! @ingroup DEBUGGER_SHELL
 * Predefined categories of help commands.
 */
enum HELP_CATEGORY
{
    HELP_CATEGORY_GENERAL,     ///< General commands.
    HELP_CATEGORY_BREAKPOINTS, ///< Breakpoint commands.
    HELP_CATEGORY_TRACEPOINTS, ///< Tracepoint commands.
    HELP_CATEGORY_REGISTERS,   ///< Register names.
    HELP_CATEGORY_END          ///< Marks the last predefined help category.  Custom category
                               ///<  numbers start here.
};

/*! @ingroup DEBUGGER_SHELL
 * Some tools may need to define their own analysis routine that stops at a debugger
 * breakpoint.  Such tools can define their own class, which derives from
 * ICUSTOM_INSTRUMENTOR.  Pass a pointer to that object to
 * STARTUP_ARGUMENTS::_customInstrumentor.
 *
 * Most tools do not need to override the default instrumentation, so most tools
 * need not use this interface.
 */
class ICUSTOM_INSTRUMENTOR
{
  public:
    /*!
     * The debugger shell calls this method to insert a "then" instrumentation call
     * to an analysis routine that stops at a debugger breakpoint _before_ an instruction.
     * The default instrumentation looks like this.  Tools that implement this method should
     * insert similar instrumentation:
     *
     *                                                                                          @code
     *  VOID InsertBreakpointBefore(INST ins, BBL bbl, CALL_ORDER order, const std::string &message)
     *  {
     *      INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)TriggerBreakpointBefore,
     *          IARG_CALL_ORDER, order,
     *          IARG_CONTEXT, IARG_THREAD_ID,
     *          IARG_UINT32, static_cast<UINT32>(RegSkipOne),
     *          IARG_PTR, message.c_str(),
     *          IARG_END);
     *  }
     *
     *  VOID TriggerBreakpointBefore(CONTEXT *ctxt, THREADID tid, UINT32 regSkipOne, const char *message)
     *  {
     *      ADDRINT skipPc = PIN_GetContextReg(ctxt, static_cast<REG>(regSkipOne));
     *      ADDRINT pc = PIN_GetContextReg(ctxt, REG_INST_PTR);
     *      if (skipPc == pc)
     *          return
     *
     *      PIN_SetContextReg(ctxt, static_cast<REG>(regSkipOne), pc);
     *      PIN_ApplicationBreakpoint(ctxt, tid, FALSE, message);
     *  }
     *                                                                                          @endcode
     *
     * See the method ISHELL::GetSkipOneRegister() for the register number to use for \e RegSkipOne.
     *
     *  @param[in] ins      Insert the instrumentation before this instruction.
     *  @param[in] bbl      The basic block containing \a ins.
     *  @param[in] order    The instrumentation call order to use for the instrumentation.
     *  @param[in] message  String telling why the breakpoint is triggered.  The
     *                       string is allocated in permanent storage, so the
     *                       client can pass it directly to an analysis routine.
     *                       If the debugger shell removes instrumentation, it will
     *                       also deallocate this string.
     */
    virtual VOID InsertBreakpointBefore(INS ins, BBL bbl, CALL_ORDER order, const std::string& message) = 0;

    /*!
     * The debugger shell calls this method to insert a "then" instrumentation call
     * to an analysis routine that stops at a debugger breakpoint _after_ an instruction.
     * The default instrumentation looks like this.  Tools that implement this method should
     * insert similar instrumentation:
     *
     *                                                                                          @code
     *  VOID InsertBreakpointAfter(INST ins, BBL bbl, IPOINT ipoint, CALL_ORDER order,
     *      const std::string &message)
     *  {
     *      INS_InsertThenCall(ins, ipoint, (AFUNPTR)TriggerBreakpointAfter,
     *          IARG_CALL_ORDER, order,
     *          IARG_CONTEXT, IARG_INST_PTR, IARG_THREAD_ID,
     *          IARG_PTR, message.c_str(),
     *          IARG_END);
     *  }
     *
     *  VOID TriggerBreakpointAfter(CONTEXT *ctxt, ADDRINT pc, THREADID tid, const char *message)
     *  {
     *      std::ostringstream os;
     *      os << message << "\n";
     *      os << "Breakpoint triggered after instruction at 0x" << std::hex << pc;
     *
     *      PIN_ApplicationBreakpoint(ctxt, tid, FALSE, os.str());
     *  }
     *                                                                                          @endcode
     *
     *  @param[in] ins      Insert the instrumentation after this instruction.
     *  @param[in] bbl      The basic block containing \a ins.
     *  @param[in] ipoint   Tells whether to instrument IPOINT_AFTER or IPOINT_TAKEN_BRANCH.
     *  @param[in] order    The instrumentation call order to use for the instrumentation.
     *  @param[in] message  String telling why the breakpoint is triggered.  The
     *                       string is allocated in permanent storage, so the
     *                       client can pass it directly to an analysis routine.
     *                       If the debugger shell removes instrumentation, it will
     *                       also deallocate this string.
     */
    virtual VOID InsertBreakpointAfter(INS ins, BBL bbl, IPOINT ipoint, CALL_ORDER order, const std::string& message) = 0;

    virtual inline ~ICUSTOM_INSTRUMENTOR() = 0; ///< Destroys the custom instrumentor object.
};

ICUSTOM_INSTRUMENTOR::~ICUSTOM_INSTRUMENTOR() {}

} // namespace DEBUGGER_SHELL
#endif // file guard
