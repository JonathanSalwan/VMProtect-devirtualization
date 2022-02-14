/*
 * Copyright (C) 2021-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file */
/*
 * This file contains types that are in LEVEL_VM and are part of the Pin
 * tool API. This is necessary because the pin client does not link against
 * the vm and does not include vm header files.
 *
 */

#ifndef TYPES_VMAPI_PH
#define TYPES_VMAPI_PH

/*! @ingroup APPDEBUG
 * Possible debugging events that can be intercepted with PIN_InterceptDebuggingEvent().
 */
enum DEBUGGING_EVENT
{
    /*!
     * Thread triggered a breakpoint.  This does not include breakpoints trigged via
     * PIN_ApplicationBreakpoint().
     */
    DEBUGGING_EVENT_BREAKPOINT,

    DEBUGGING_EVENT_SINGLE_STEP, ///< Thread completed a single-step.

    /*!
     * Thread stopped due to a request from the debugger or because another thread has
     * stopped in the debugger.
     */
    DEBUGGING_EVENT_ASYNC_BREAK
};

/*! @ingroup APPDEBUG
 * Describes a register that is exposed to an application-level debugger.
 */
struct DEBUGGER_REG_DESCRIPTION
{
    REG _pinReg;           ///< The Pin register ID, if this is a native Pin register.
                           ///< REG_NONE if the tool will emulate this register.
    unsigned _toolRegId;   ///< Only used if _pinReg is REG_NONE.  Gives an ID for this emulated register.
    unsigned _widthInBits; ///< Width (bits) of the register, as seen by the debugger.  Currently,
                           ///<  this must be a multiple of 8 bits.
    const char* _name;     ///< The name of the register
    int _gccId;            ///< The GCC/Dwarf equivalent register id
};

/*! @ingroup CONTEXT
 *  A constant that designates default (canonical) CONTEXT implementation.
 */
const ADDRINT DEFAULT_CONTEXT_IMPL = 0;

/*! @ingroup CONTEXT
 *  A structure that keeps architectural state of the processor.
 */
struct CONTEXT
{
    /*!
     *  Construct the CONTEXT object with the default (canonical) implementation.
     */
    CONTEXT() : _impl(DEFAULT_CONTEXT_IMPL) { memset(_state, 0, sizeof(CHAR) * ARCH_STATE_SIZE); }

    ADDRINT _impl;                ///< implementation-specific data
    CHAR _state[ARCH_STATE_SIZE]; ///< architectural state
};

/*! @ingroup PHYSICAL_CONTEXT
 *  A structure that keeps architectural state of the processor.
 */
struct PHYSICAL_CONTEXT
{
    /*!
    *  Constructor of the PHYSICAL_CONTEXT object
    */
    PHYSICAL_CONTEXT(CONTEXT* pCtxt) : _pCtxt(pCtxt) {}

    CONTEXT* _pCtxt; ///< pointer to CONTEXT
};

/*! @ingroup BUFFER
 *
 * Tool buffer ID assigned by Pin.
 */
typedef UINT32 BUFFER_ID;

/*! @ingroup BUFFER
 *
 * ID returned if defining a buffer fails.
 */
const BUFFER_ID BUFFER_ID_INVALID = 0;

typedef UINT32 DEBUG_MODE_OPTIONS; ///< A bit-wise 'or' of DEBUG_MODE_OPTION's.

/*! @ingroup CONTEXT
 * enums for queries on the supported state: PIN_SupportsProcessorState and PIN_ContextContainsState
 */
enum PROCESSOR_STATE
{
    PROCESSOR_STATE_X87, ///< is the X87 (fp stack) supported
    PROCESSOR_STATE_XMM, ///< are the xmm registers supported
    PROCESSOR_STATE_YMM, ///< are the ymm registers supported
    PROCESSOR_STATE_ZMM  ///< are the zmm registers supported
};

/*! @ingroup CONTEXT
 *  Architectural state of the processor.\n
 *  The tool should not read or modify data members of this structure directly,
 *  but must treat the structure as logically opaque and use the @ref CONTEXT
 *  functions to manage it.
 */
typedef struct CONTEXT CONTEXT;

/*! @ingroup PHYSICAL_CONTEXT
 *  Physical state of the processor.\n
 *  The tool should not read or modify data members of this structure directly,
 *  but must treat the structure as logically opaque and use the @ref PHYSICAL_CONTEXT
 *  functions to manage it.
 */
typedef struct PHYSICAL_CONTEXT PHYSICAL_CONTEXT;

/*! @ingroup INST_ARGS
 *
 * Determines where the analysis call is inserted relative to the instrumented object.
 * Instrumented object can be: INS, BBL, TRACE, RTN.
 * This is used in instrumentation functions, for example see @ref TRACE_InsertCall().
 */
typedef enum
{
    IPOINT_INVALID,

    IPOINT_BEFORE, ///< Insert a call before the first instruction of the instrumented object. Always valid.

    /*! Insert a call on the fall-through path of the last instruction of the instrumented
     *  object (if such fall-through exists).\n
     *  In case of a routine (RTN), instruments all return paths.
     *  Always valid.\n
     *  In case of an instruction (INS), valid only if @ref INS_IsValidForIpointAfter() is true.\n
     *  In case of a BBL, valid only if @ref BBL_HasFallThrough() is true.\n
     *  In case of a TRACE, valid only if @ref TRACE_HasFallThrough() is true.\n
     */
    IPOINT_AFTER,

    /*! Insert a call anywhere inside the instrumented object.\n
     *  Valid on all instrumentation functions except @ref INS_InsertIfCall(), @ref INS_InsertThenCall().
     */
    IPOINT_ANYWHERE,

    /*! Insert a call on the taken edge of control-flow instructions of the instrumented object.\n
     *  In case of an instruction, Valid only if @ref INS_IsValidForIpointTakenBranch() is true.
     */
    IPOINT_TAKEN_BRANCH
} IPOINT;

/*! @ingroup PIN_CONTROL
 * Possible reasons for an application context change.
 */
enum CONTEXT_CHANGE_REASON
{
    CONTEXT_CHANGE_REASON_FATALSIGNAL, ///< Receipt of fatal Unix signal
    CONTEXT_CHANGE_REASON_SIGNAL,      ///< Receipt of handled Unix signal
    CONTEXT_CHANGE_REASON_SIGRETURN,   ///< Return from Unix signal handler
    CONTEXT_CHANGE_REASON_APC,         ///< Receipt of Windows APC
    CONTEXT_CHANGE_REASON_EXCEPTION,   ///< Receipt of Windows exception
    CONTEXT_CHANGE_REASON_CALLBACK     ///< Receipt of Windows call-back
};

/*! @ingroup PIN_CONTROL
 * Possible return values of the @ref INTERNAL_EXCEPTION_CALLBACK exception filter function
 */
enum EXCEPT_HANDLING_RESULT
{
    EHR_HANDLED, ///< Exception is handled. Continue execution with the internal physical context

    EHR_UNHANDLED, ///< Exception is not handled.
                   ///< Execute default system procedure for unhandled exceptions (Windows) or abort the process (Unix)

    EHR_CONTINUE_SEARCH ///< Execute next (upper level) exception filter function, if any.
                        ///< For the uppermost filter, this is the same as EHR_UNHANDLED semantics
};

/*! @ingroup INST_ARGS
 *
 * Predefined values for @ref IARG_CALL_ORDER. Controls the order of analysis call
 * invocations when an instruction has multiple analysis calls.
 * CALL_ORDER_DEFAULT is the default call order when @ref IARG_CALL_ORDER
 * is not specified. You can use integral values relative to the predefined values to have a
 * fine-grained control over the instrumentation order
 * (e.g., CALL_ORDER_DEFAULT + 5, CALL_ORDER_LAST - 10, etc).
 *
 * Pin guarantees that multiple analysis calls with the same CALL_ORDER on the same instruction,
 * added in the context of the same instrumentation callback (e.g,. @ref INS_INSTRUMENT_CALLBACK),
 * will be invoked according to their insertion order. For all other cases, the invocation order
 * for analysis calls with the same CALL_ORDER on the same instruction depends on the implementation
 * and may change without notice.
 */
enum CALL_ORDER
{
    CALL_ORDER_FIRST   = 100, ///< Execute this call first. Value is 100.
    CALL_ORDER_DEFAULT = 200, ///< Default when IARG_CALL_ORDER is not specified. Value is 200.
    CALL_ORDER_LAST    = 300  ///< Execute this call last. Value is 300.
};

/*! @ingroup INST_ARGS
 *
 * Determines the arguments that are passed to the analysis call.
 * All argument lists must end with IARG_END.
 */
typedef enum
{
    IARG_INVALID,
    IARG_ADDRINT, ///< Type: ADDRINT. Constant value (additional arg required)
    IARG_PTR,     ///< Type: "VOID *". Constant value (additional pointer arg required)
    IARG_BOOL,    ///< Type: BOOL. Constant (additional BOOL arg required)
    IARG_UINT32,  ///< Type: UINT32. Constant (additional integer arg required)
    IARG_UINT64,  ///< Type: UINT64. Constant (additional UINT64 arg required)

    /*!
     * Type: ADDRINT. The address of the instrumented instruction. This value does not change at IPOINT_AFTER.
     * This is simply shorthand for <tt>IARG_ADDRINT, INS_Address(ins)</tt>.
     */
    IARG_INST_PTR,

    /*!
     * Type: ADDRINT for integer registers. Value of a register (additional register arg required) @ref REG
     * Basically, this cannot be used to retrieve the value of registers whose size is larger than ADDRINT
     * (e.g. x87 FPU/XMM/YMM/ZMM/opmask) or registers which are not architectural (REG_PIN_*), but there are some
     * exceptions to this rule.
     */
    IARG_REG_VALUE,

    /*!
     * Type: @ref PIN_REGISTER.
     *      Pointer to union holding register content up to largest vector register size (ZMM - 64 bytes).
     *      Register is specified in additional argument.
     * Type: @ref PIN_WIDE_REGISTER.
     *      Pointer to union holding tile register contents (1024 bytes). Register is specified in additional argument.
     * Register is updated back to the relevant application register (Meaning if you change values in the mentioned union they will
     * be propagated to the relevant application register). If you're not interested in modifying the register value, but only read
     * its value use IARG_REG_CONST_REFERENCE instead.
     *
     * Not supported in Probe mode or with the Buffering APIs.
     *
     * @note additional REG arg required!!!
     */
    IARG_REG_REFERENCE,

    /*!
     * Type: @ref PIN_REGISTER.
     *      Pointer to union holding register content up to largest vector register size (ZMM - 64 bytes).
     *      Register is specified in additional argument.
     * Type: @ref PIN_WIDE_REGISTER.
     *      Pointer to union holding tile register contents (1024 bytes). Register is specified in additional argument.
     * Registers are not updated back to the relevant application register (Meaning if you change values in the mentioned union
     * they will not be propagated to the relevant application register). If you're interested in modifying the register value,
     * use IARG_REG_REFERENCE instead.
     * Not supported with the Buffering APIs.
     *
     * @note additional REG arg required!!!
     */
    IARG_REG_CONST_REFERENCE,

    IARG_MEMORYREAD_EA, ///< Type: ADDRINT. Effective address of a memory read, only valid if INS_IsMemoryRead is true and at IPOINT_BEFORE
    IARG_MEMORYREAD2_EA, ///< Type: ADDRINT. Effective address of a 2nd memory read (e.g. 2nd operand in cmps on ia32), only valid at IPOINT_BEFORE
    IARG_MEMORYWRITE_EA, ///< Type: ADDRINT. Effective address of a memory write, only valid at IPOINT_BEFORE
    /*!
     * Type: UINT32. Size in bytes of memory read.
     * This IARG is applicable for all instructions for which @ref INS_hasKnownMemorySize returns TRUE.
     * For other instructions e.g. vgather/vscatter see @ref IARG_MULTI_ELEMENT_OPERAND.
     * For the varying memory size read instruction, XRSTOR, the size is based on the XSAVE area header. If the header
     * indicates compact mode it will provide the compact size. Otherwise it will provide the standard size.
     * The minimum size for the XRSTOR instruction (not including FXRSTOR) is 576 (i.e the XSAVE area size up
     * to and including the header).
     */
    IARG_MEMORYREAD_SIZE,
    /*!
     * Type: UINT32. Size in bytes of memory write.
     * This IARG is applicable for all instructions for which @ref INS_hasKnownMemorySize returns TRUE.
     * For other instructions e.g. vgather/vscatter see @ref IARG_MULTI_ELEMENT_OPERAND.
     * For varying size memory write instructions, the supported instructions are the XSAVE family of instructions.
     * For XSAVE, the size of the XSAVE area used is based on user request and supported features in the machine. This will
     * provide the exact size used.
     * For XSAVEOPT, the size is calculated the same as if XSAVE was used. This may provide in some cases an upper
     * bound to the actual used memory.
     * For XSAVEC, in IPOINT_BEFORE the size is calculated based on user request only. This may provide in some cases
     * an upper bound to the actual size.
     * If used in IPOINT_AFTER it will provide the exact compact size as defined in the XSAVE area header.
     * The minimum size for the XSAVE family write instructions (not including FXSAVE) is 576 (i.e the XSAVE area
     * size up to and including the header).
     */
    IARG_MEMORYWRITE_SIZE,

    // IARG_MEMORY*_PTR argument represents actual address of the memory access, even if the operand is rewritten
    // or a memory translation callback is registered
    /*!
     * Actual address of the memory access; same as @ref IARG_MEMORYREAD_EA, unless the memory address is translated
     * by @ref MEMORY_ADDR_TRANS_CALLBACK, or the memory operand is rewritten by @ref INS_RewriteMemoryOperand
     */
    IARG_MEMORYREAD_PTR,
    /*!
     * Actual address of the memory access; same as @ref IARG_MEMORYREAD2_EA, unless the memory address is translated
     * by @ref MEMORY_ADDR_TRANS_CALLBACK, or the memory operand is rewritten by @ref INS_RewriteMemoryOperand
     */
    IARG_MEMORYREAD2_PTR,
    /*!
     * Actual address of the memory access. Same as @ref IARG_MEMORYWRITE_EA, unless the memory address is translated
     * by @ref MEMORY_ADDR_TRANS_CALLBACK, or the memory operand is rewritten by @ref INS_RewriteMemoryOperand
     */
    IARG_MEMORYWRITE_PTR,
    /*!
     * Actual address of the memory access. Same as @ref IARG_MEMORYOP_EA, unless the memory address is translated
     * by @ref MEMORY_ADDR_TRANS_CALLBACK, or the memory operand is rewritten by @ref INS_RewriteMemoryOperand
     */
    IARG_MEMORYOP_PTR,

    /*!
     * Type: PIN_MULTI_MEM_ACCESS_INFO* the addresses read by the various vgather* instructions,
     * Also available for regular memory instructions
     */
    IARG_MULTI_MEMORYACCESS_EA,
        
    /*!
     * Type: IMULTI_ELEMENT_OPERAND* . Information about a multi element operand (implemented for IPOINT_BEFORE).
     * The operand index is required as the next argument.
     */
    IARG_MULTI_ELEMENT_OPERAND,   

    IARG_EXPLICIT_MEMORY_EA, ///< Type: ADDRINT. Effective address of the explicit memory operand. Useful for instrumenting LEA instructions

    IARG_BRANCH_TAKEN, ///< Type: BOOL. Non zero if a branch is taken. Argument is invalid for XBEGIN and XEND instructions.
    /*! Type: ADDRINT. Target address of branch instruction.\n
     *   In case of INS instrumentation, valid when @ref INS_IsControlFlow() is true.\n
     *   However, this argument is invalid if the instruction is XBEGIN or XEND.
     *   In Linux, for the case of branching into the vsyscall area (in kernel 5.3 or above), the target
     *   address that will be received in the analysis routine would be that of the vsyscall area. Note that
     *   this address is not readable (e.g. via PIN_SafeCopy() and PIN_FetchCode())
     */
    IARG_BRANCH_TARGET_ADDR,
    /*! Type: ADDRINT. Fall through address of the instrumented object.\n
     *  In case of INS instrumentation, valid only if @ref INS_HasFallThrough() is true.
     */
    IARG_FALLTHROUGH_ADDR,

    IARG_EXECUTING, ///< Type: BOOL. False if the instruction will not be executed because of predication, otherwise true.

	/*! Type: BOOL. True if INS_HasRealRep(ins) and this the first iteration of the REP sequence, otherwise false.\n
	 *  @note In case count register is 0 when calling analysis routine, the value of IARG_FIRST_REP_ITERATION is false.
	 */
    IARG_FIRST_REP_ITERATION,
    IARG_PREDICATE, ///< Reserved. Do not use

    // These are internal only
    IARG_STACK_VALUE,      ///< Reserved. Do not use
    IARG_STACK_REFERENCE,  ///< Reserved. Do not use
    IARG_MEMORY_VALUE,     ///< Reserved. IA32(e) only
    IARG_MEMORY_REFERENCE, ///< Reserved. IA32(e) only

    // Syscall stuff
    IARG_SYSCALL_NUMBER,   ///< Type: ADDRINT. System call number. Valid for IPOINT_BEFORE at the system call instruction
    IARG_SYSARG_REFERENCE, ///< Type: "ADDRINT *". Pointer to system call argument n. Valid for IPOINT_BEFORE at the system call instruction.  (First argument number is 0.)

    IARG_SYSARG_VALUE, ///< Type: ADDRINT. System call argument n. Valid for IPOINT_BEFORE at the system call instruction.  (First argument number is 0.)
    IARG_SYSRET_VALUE, ///< Type: ADDRINT. System call return value. On Linux and macOS* the value is -1 if the system call failed. (IPOINT_AFTER only)
    IARG_SYSRET_ERRNO, ///< Type: INT32. System call errno (IPOINT_AFTER_only).

    // function arguments
    /*!
     * Type: "ADDRINT *". Pointer to integer argument n. Valid only at the call site.  (First argument number is 0.)
    */
    IARG_FUNCARG_CALLSITE_REFERENCE,
    /*!
     * Type: ADDRINT. Integer argument n. Valid only at the call site.  (First argument number is 0.)
     */
    IARG_FUNCARG_CALLSITE_VALUE,
    /*!
     * Type: "ADDRINT *". Pointer to integer argument n. Valid only at the entry point of a routine.  (First argument number is 0.)
     */
    IARG_FUNCARG_ENTRYPOINT_REFERENCE,
    /*!
     * Type: ADDRINT. Integer argument n. Valid only at the entry point of a routine.  (First argument number is 0.)
     */
    IARG_FUNCARG_ENTRYPOINT_VALUE,
    IARG_FUNCRET_EXITPOINT_REFERENCE, ///< Type: "ADDRINT *". Pointer to function result. Valid only at return instruction.
    IARG_FUNCRET_EXITPOINT_VALUE,     ///< Type: ADDRINT. Function result. Valid only at return instruction.

    IARG_RETURN_IP, ///< Type: ADDRINT. Return address for function call, valid only at the function entry point.

    IARG_ORIG_FUNCPTR, ///< Type: AFUNPTR. Function pointer to the relocated entry of the original uninstrumented function.

    IARG_PROTOTYPE, ///< Type: PROTO.  The function prototype of the application function. See @ref PROTO

    IARG_THREAD_ID, ///< Type: THREADID. Application thread id.

    /*!
     * Type: @ref CONTEXT *. Handle to access a context (architectural state).  When passed at
     * @ref IPOINT_AFTER or @ref IPOINT_TAKEN_BRANCH, PC points to the next instruction.  Upon return from
     * the analysis routine, Pin ignores any changes you've made to the @ref CONTEXT.  If you want
     * to change register values, use @ref IARG_REG_REFERENCE, @ref IARG_RETURN_REGS, or @ref PIN_ExecuteAt.
     * See @ref PROBE_IARGS for probe mode restrictions.  Not supported with the Buffering APIs.
     */
    IARG_CONTEXT,
    /*!
     * Type: @ref CONTEXT *.
     * Like @ref IARG_CONTEXT, but tool receives a read-only @ref CONTEXT*.
     * @ref PIN_SetContextReg, @ref PIN_SetContextRegval and @ref PIN_SetContextFPState will not work with
     * @ref IARG_CONST_CONTEXT.
     * The overhead of @ref IARG_CONST_CONTEXT is considerably lower than that of @ref IARG_CONTEXT.
     * Tools that need a @ref CONTEXT* and only read from it should use @ref IARG_CONST_CONTEXT.
     * Tools that need a @ref CONTEXT* and only occasionally write into it should also use @ref IARG_CONST_CONTEXT.
     * One example of a tool that needs a @ref CONTEXT * and only occasionally writes into it, would be an
     * emulator that would need to write into the @ref CONTEXT * only when an exception occurs, and then
     * raise an exception with the @ref CONTEXT *.
     * @ref PIN_SaveContext can be used by the tool to get a writable copy of the @ref CONTEXT *.
     * @ref IARG_CONST_CONTEXT is available only in Jit mode.
     */
    IARG_CONST_CONTEXT,
    /*!
     * Type: @ref CONTEXT *. Additional two @ref REGSET * arguments are needed 'inSet' and 'outSet'.
     * Like @ref IARG_CONTEXT, but tool needs to define also which registers set it needs to read (inSet)
     * and which registers set it may write (outSet).
     * @ref PIN_SetContextReg, @ref PIN_SetContextRegval and @ref PIN_SetContextFPState will work only on registers
     * that are in the outSet. These functions will change the actual application registers so there is no need
     * to perform @ref PIN_ExecuteAt as need to be done with @ref IARG_CONTEXT.
     * @ref PIN_GetContextReg, @ref PIN_GetContextRegval and @ref PIN_GetContextFPState will return unexpected values
     * for registers not defined in the 'inSet'.
     * The overhead of @ref IARG_PARTIAL_CONTEXT may lower than that of @ref IARG_CONTEXT as there is no need to perform
     * @ref PIN_ExecuteAt to update registers.
     * Tools that know which application registers are needed to be read should use @ref IARG_PARTIAL_CONTEXT instead of
     * @ref IARG_CONST_CONTEXT. Thus may reduce the overhead as not fully updated context for read need to be provided.
     * Tools that know which application registers other than @ref REG_INST_PTR should use @ref IARG_PARTIAL_CONTEXT
     * instead of combination of @ref IARG_CONTEXT and the use of @ref PIN_ExecuteAt for registers update.
     * If @ref REG_INST_PTR needs to be updated @ref PIN_ExecuteAt still needs to be used.
     * Any updates to a register from the inSet of an @ref IARG_PARTIAL_CONTEXT will be propagated to the application upon
     * return from the analysis routine. Tools that want to update @ref CONTEXT registers without affecting application
     * registers should use @ref IARG_CONTEXT.
     * @ref IARG_PARTIAL_CONTEXT is available only in Jit mode.
     */
    IARG_PARTIAL_CONTEXT,

    /*!
     * Type: @ref REGSET *.
     * Used to specify registers whose values will be the same upon return from the analysis routine.
     * Should be used to specify caller-saved registers which are preserved by the analysis routine.
     * When a non-inlinable analysis routine preserves caller-saved registers, Pin can avoid generating
     * code to preserve these registers, across the analysis call, if they are specified in the IARG_PRESERVE.
     * e.g. if a non-inlinable analysis routine preserves the values in the x87 registers, then Pin
     * can avoid generating code to preserve these registers, across the analysis call, if REG_X87 is specified in
     * the @ref IARG_PRESERVE
     * e.g. if a non-inlinable analysis routine preserves the values in XMM caller saved registers, then Pin
     * can avoid generating code to preserve these registers, across the analysis call, if those preserved XMM
     * registers are specified in the @ref IARG_PRESERVE
     * @ref IARG_PRESERVE must be followed by a @ref REGSET* that points to the @ref REGSET containing the registers preserved.
     * See the @ref REGSET_AddAll "(REGSET_*)" functions defined under @ref REG for @ref REGSET construction and manipulation.
     */
    IARG_PRESERVE,

    IARG_RETURN_REGS, ///< Register to write analysis function return value (additional register arg required).  Not supported in Probe mode.

    IARG_CALL_ORDER, ///< Type: CALL_ORDER. Determine order of analysis calls. See @ref CALL_ORDER.

    IARG_IARGLIST, ///< Type: IARGLIST. List of IARGS which can be constructed one IARG at a time.

    IARG_FAST_ANALYSIS_CALL, ///< No type: Use a fast linkage to call the analysis function. See @ref PIN_FAST_ANALYSIS_CALL

    IARG_MEMORYOP_EA, ///< Type: ADDRINT. Effective address of a memory op (memory op index is next arg); only valid at IPOINT_BEFORE
    IARG_MEMORYOP_SIZE, ///< Type: UINT32. Size of a memory op (memory op index is next arg)
    IARG_MEMORYOP_MASKED_ON, ///< Type: BOOL. TRUE if the this memory op (memory op index is next arg) is masked on; only valid at IPOINT_BEFORE
    IARG_TSC,                ///< Type: UINT64. Time Stamp Counter value at the point of entering the analysis call.
    IARG_FILE_NAME, ///< Reserved for internal use only.
    IARG_LINE_NO,   ///< Reserved for internal use only.
    IARG_LAST       ///< Reserved for internal use only.

} IARG_TYPE;

// IARG_END must be the last IARG in the argument list.
// These iargs are used by Pin internally, and not passed
// to the analysis, replacement or callback routine.
// We need to be sure about the type of __LINE__, because we're passing it
// via varargs, and different compilers seem to treat it differently. By forcing
// it to UINT32 (which restricts the file to a mere 4Gi lines), we can be safe.
#ifdef OMIT_SOURCE_LOCATION
#define IARG_END IARG_LAST
#else
#define IARG_END IARG_FILE_NAME, __FILE__, IARG_LINE_NO, LEVEL_BASE::UINT32(__LINE__), IARG_LAST
#endif

static const UINT32 MAX_BYTES_PER_PIN_REG   = 64;
static const UINT32 MAX_WORDS_PER_PIN_REG   = (MAX_BYTES_PER_PIN_REG / 2);
static const UINT32 MAX_DWORDS_PER_PIN_REG  = (MAX_WORDS_PER_PIN_REG / 2);
static const UINT32 MAX_QWORDS_PER_PIN_REG  = (MAX_DWORDS_PER_PIN_REG / 2);
static const UINT32 MAX_FLOATS_PER_PIN_REG  = (MAX_BYTES_PER_PIN_REG / sizeof(float));
static const UINT32 MAX_DOUBLES_PER_PIN_REG = (MAX_BYTES_PER_PIN_REG / sizeof(double));

/*! @ingroup CONTEXT
 *  A container large enough to hold registers up to the size of the largest vector register (ZMM - 64 bytes)).
 *  Implemented as a union to allow viewing the value as different types (signed/unsigned integer or floating point)
 *  and allow access in blocks of various sizes.
 */
union PIN_REGISTER
{
    UINT8 byte[MAX_BYTES_PER_PIN_REG];
    UINT16 word[MAX_WORDS_PER_PIN_REG];
    UINT32 dword[MAX_DWORDS_PER_PIN_REG];
    UINT64 qword[MAX_QWORDS_PER_PIN_REG];

    INT8 s_byte[MAX_BYTES_PER_PIN_REG];
    INT16 s_word[MAX_WORDS_PER_PIN_REG];
    INT32 s_dword[MAX_DWORDS_PER_PIN_REG];
    INT64 s_qword[MAX_QWORDS_PER_PIN_REG];

    FLT32 flt[MAX_FLOATS_PER_PIN_REG];
    FLT64 dbl[MAX_DOUBLES_PER_PIN_REG];
};

static const UINT32 MAX_BYTES_PER_PIN_WIDE_REG   = 1024;
static const UINT32 MAX_WORDS_PER_PIN_WIDE_REG   = (MAX_BYTES_PER_PIN_WIDE_REG / 2);
static const UINT32 MAX_DWORDS_PER_PIN_WIDE_REG  = (MAX_WORDS_PER_PIN_WIDE_REG / 2);
static const UINT32 MAX_QWORDS_PER_PIN_WIDE_REG  = (MAX_DWORDS_PER_PIN_WIDE_REG / 2);
static const UINT32 MAX_FLOATS_PER_PIN_WIDE_REG  = (MAX_BYTES_PER_PIN_WIDE_REG / sizeof(float));
static const UINT32 MAX_DOUBLES_PER_PIN_WIDE_REG = (MAX_BYTES_PER_PIN_WIDE_REG / sizeof(double));

/*! @ingroup CONTEXT
 * TODO
 *  A container for tile registers (1024 bytes).
 *  Implemented as a union to allow viewing the value as different types (signed/unsigned integer or floating point)
 *  and allow access in blocks of various sizes.
 */
union PIN_WIDE_REGISTER
{
    UINT8 byte[MAX_BYTES_PER_PIN_WIDE_REG];
    UINT16 word[MAX_WORDS_PER_PIN_WIDE_REG];
    UINT32 dword[MAX_DWORDS_PER_PIN_WIDE_REG];
    UINT64 qword[MAX_QWORDS_PER_PIN_WIDE_REG];

    INT8 s_byte[MAX_BYTES_PER_PIN_WIDE_REG];
    INT16 s_word[MAX_WORDS_PER_PIN_WIDE_REG];
    INT32 s_dword[MAX_DWORDS_PER_PIN_WIDE_REG];
    INT64 s_qword[MAX_QWORDS_PER_PIN_WIDE_REG];

    FLT32 flt[MAX_FLOATS_PER_PIN_WIDE_REG];
    FLT64 dbl[MAX_DOUBLES_PER_PIN_WIDE_REG];
};

/*! @ingroup PROTO
 *
 * Type of calling standards used for compiling the function.  See @ref
 * PROTO_Allocate() for its use.  CALLINGSTD_DEFAULT is the default calling
 * standard for the platform. This is the recommended option.
 */
typedef enum
{
    CALLINGSTD_INVALID,
    CALLINGSTD_DEFAULT,
    CALLINGSTD_CDECL,
    CALLINGSTD_REGPARMS,
    CALLINGSTD_STDCALL,
    CALLINGSTD_ART
} CALLINGSTD_TYPE;

/*! @ingroup PROTO
 *
 * Function prototype argument types.  See @ref PROTO_Allocate() for its
 * use.
 */
typedef enum
{
    PARG_INVALID,
    PARG_POINTER,
    PARG_BOOL,
    PARG_CHAR,
    PARG_UCHAR,
    PARG_SCHAR,
    PARG_SHORT,
    PARG_USHORT,
    PARG_INT,
    PARG_UINT,
    PARG_LONG,
    PARG_ULONG,
    PARG_LONGLONG,
    PARG_ULONGLONG,
    PARG_FLOAT,
    PARG_DOUBLE,
    PARG_VOID,
    PARG_ENUM,
    PARG_AGGREGATE,
    PARG_END
} PARG_TYPE;

struct PARG_T
{
    PARG_TYPE _parg;
    size_t _size;
};

/*! @ingroup PROTO
 *
 * Macros used to simplify the passing of function prototype arguments.
 * The macro will pass the type and size of each argument specified.  When
 * passing an argument to @ref PROTO_Allocate(), use these macros.  As an
 * example, PIN_PARG(int) will pass the PARG_TYPE PARG_INT and the size of
 * an integer to @ref PROTO_Allocate().
 */
#define PIN_PARG(t) _PinPargClass< t >::parg()

/*! @ingroup PROTO
 *
 *  Type and size of an aggregate in @ref PROTO_Allocate()
 *  See @ref PIN_PARG
 */
#define PIN_PARG_AGGREGATE(t) _PinParg(PARG_AGGREGATE, sizeof(t))

/*! @ingroup PROTO
 *
 *  Type and size of an enum in @ref PROTO_Allocate()
 *  See @ref PIN_PARG
 */
#define PIN_PARG_ENUM(t) _PinParg(PARG_ENUM, sizeof(t))

/*! @ingroup PROTO
 *
 *  Ends a list of arguments in @ref PROTO_Allocate()
 *  See @ref PIN_PARG
 */
#define PIN_PARG_END() _PinParg(PARG_END, 0)

template< typename T > struct _PinPargClass
{
    static PARG_T parg()
    {
        PARG_T x;
        x._parg = PARG_INVALID;
        x._size = 0;
        return x;
    }
};
template< typename T > struct _PinPargClass< T* >
{
    static PARG_T parg()
    {
        PARG_T x;
        x._parg = PARG_POINTER;
        x._size = sizeof(T*);
        return x;
    }
};
template< typename T > struct _PinPargClass< T& >
{
    static PARG_T parg()
    {
        PARG_T x;
        x._parg = PARG_POINTER;
        x._size = sizeof(T&);
        return x;
    }
};
template<> struct _PinPargClass< bool >
{
    static PARG_T parg()
    {
        PARG_T x;
        x._parg = PARG_BOOL;
        x._size = sizeof(bool);
        return x;
    }
};
template<> struct _PinPargClass< char >
{
    static PARG_T parg()
    {
        PARG_T x;
        x._parg = PARG_CHAR;
        x._size = sizeof(char);
        return x;
    }
};
template<> struct _PinPargClass< unsigned char >
{
    static PARG_T parg()
    {
        PARG_T x;
        x._parg = PARG_UCHAR;
        x._size = sizeof(unsigned char);
        return x;
    }
};
template<> struct _PinPargClass< signed char >
{
    static PARG_T parg()
    {
        PARG_T x;
        x._parg = PARG_SCHAR;
        x._size = sizeof(signed char);
        return x;
    }
};
template<> struct _PinPargClass< short >
{
    static PARG_T parg()
    {
        PARG_T x;
        x._parg = PARG_SHORT;
        x._size = sizeof(short);
        return x;
    }
};
template<> struct _PinPargClass< unsigned short >
{
    static PARG_T parg()
    {
        PARG_T x;
        x._parg = PARG_USHORT;
        x._size = sizeof(unsigned short);
        return x;
    }
};
template<> struct _PinPargClass< int >
{
    static PARG_T parg()
    {
        PARG_T x;
        x._parg = PARG_INT;
        x._size = sizeof(int);
        return x;
    }
};
template<> struct _PinPargClass< unsigned int >
{
    static PARG_T parg()
    {
        PARG_T x;
        x._parg = PARG_UINT;
        x._size = sizeof(unsigned int);
        return x;
    }
};
template<> struct _PinPargClass< long >
{
    static PARG_T parg()
    {
        PARG_T x;
        x._parg = PARG_LONG;
        x._size = sizeof(long);
        return x;
    }
};
template<> struct _PinPargClass< unsigned long >
{
    static PARG_T parg()
    {
        PARG_T x;
        x._parg = PARG_ULONG;
        x._size = sizeof(unsigned long);
        return x;
    }
};
template<> struct _PinPargClass< long long >
{
    static PARG_T parg()
    {
        PARG_T x;
        x._parg = PARG_LONGLONG;
        x._size = sizeof(long long);
        return x;
    }
};
template<> struct _PinPargClass< unsigned long long >
{
    static PARG_T parg()
    {
        PARG_T x;
        x._parg = PARG_ULONGLONG;
        x._size = sizeof(unsigned long long);
        return x;
    }
};
template<> struct _PinPargClass< float >
{
    static PARG_T parg()
    {
        PARG_T x;
        x._parg = PARG_FLOAT;
        x._size = sizeof(float);
        return x;
    }
};
template<> struct _PinPargClass< double >
{
    static PARG_T parg()
    {
        PARG_T x;
        x._parg = PARG_DOUBLE;
        x._size = sizeof(double);
        return x;
    }
};
template<> struct _PinPargClass< void >
{
    static PARG_T parg()
    {
        PARG_T x;
        x._parg = PARG_VOID;
        x._size = 0;
        return x;
    }
};

static inline PARG_T _PinParg(PARG_TYPE p, size_t s)
{
    PARG_T x;
    x._parg = p;
    x._size = s;
    return x;
}

/*
 * Opaque types in api
 */
class IARGLIST_CLASS;
class PROTO_CLASS;

/*! @ingroup INST_ARGS
 *
 * List of IARG_TYPE elements. See @ref IARGLIST_Alloc() for its use.
 */
typedef IARGLIST_CLASS* IARGLIST;

/*! @ingroup INST_ARGS
 *
 * Use a faster linkage for calls to analysis functions. Add
 * PIN_FAST_ANALYSIS_CALL to the declaration between the return type and
 * the function name. You must also add @ref IARG_FAST_ANALYSIS_CALL to the
 * InsertCall. For example:

 @begincode
VOID PIN_FAST_ANALYSIS_CALL docount(int j);

 INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(docount), IARG_FAST_ANALYSIS_CALL, IARG_UINT32, 3, IARG_END);
 @endcode
 *
 * You will gain the most benefit from using PIN_FAST_ANALYSIS_CALL if
 * - you are instrumenting a 32 bit process (the 64 bit calling conventions already pass
 *   arguments in registers)
 * - your analysis routine is not inlined (if it is inlined there is no
 *   call to optimize)
 * - your analysis routine is relatively small (if it is large the extra overhead
 *   imposed by the poor calling convention won't be significant anyway)
 *
 */
#if defined(TARGET_IA32) && defined(TARGET_LINUX)
#define PIN_FAST_ANALYSIS_CALL __attribute__((regparm(3)))
#elif defined(TARGET_IA32) && defined(TARGET_MAC)
#define PIN_FAST_ANALYSIS_CALL __attribute__((regparm(3)))
#elif defined(TARGET_IA32) && defined(TARGET_WINDOWS)
#define PIN_FAST_ANALYSIS_CALL __fastcall
#else
#define PIN_FAST_ANALYSIS_CALL
#endif

/*! @ingroup INST_ARGS
 *
 * used in the Type whose address is passed to analysis routine when IARG_MULTI_MEMORYACCESS_EA is requested
 */
typedef enum
{
    PIN_MEMOP_LOAD,
    PIN_MEMOP_STORE
} PIN_MEMOP_ENUM;

/*! @ingroup INST_ARGS
 *
 * used in the Type whose address is passed to analysis routine when IARG_MULTI_MEMORYACCESS_EA is requested
 */
typedef struct
{
    ADDRINT memoryAddress;
    PIN_MEMOP_ENUM memopType;
    UINT32 bytesAccessed;
    BOOL maskOn;
} PIN_MEM_ACCESS_INFO;

/*! @ingroup INST_ARGS
 *
 */
const int MAX_MULTI_MEMOPS = 16;

/*! @ingroup INST_ARGS
 *
 * address of this Type is passed to analysis routine when IARG_MULTI_MEMORYACCESS_EA is requested
 */
typedef struct
{
    UINT32 numberOfMemops; // only the first numberOfMemops elements in the memop[] array
                           // are valid
    PIN_MEM_ACCESS_INFO memop[MAX_MULTI_MEMOPS];
} PIN_MULTI_MEM_ACCESS_INFO;

/*! @ingroup INST_ARGS
 *
 * Element Access type used in IMULTI_ELEMENT_OPERAND
 */
typedef enum
{
   PIN_OP_ELEMENT_ACCESS_READ,
   PIN_OP_ELEMENT_ACCESS_WRITE,
   PIN_OP_ELEMENT_ACCESS_READWRITE
} PIN_OP_ELEMENT_ACCESS;

/*! @ingroup INST_ARGS
 *
 * An object implementing this interface is passed to analysis routine when 
 * IARG_MULTI_ELEMENT_OPERAND is requested.
 * This interface allows inspecting multi element operands (register or memory).
 *
 */
class IMULTI_ELEMENT_OPERAND {
public:
    virtual ~IMULTI_ELEMENT_OPERAND(){}
    /** 
    * Is this a Memory operand
    */
    virtual BOOL IsMemory() = 0;
    /** 
    * Is this a Register operand
    */
    virtual BOOL IsRegister() = 0;
    /** 
    * Full operand size. For operands that do not have a size.
    * (for example, the memory operand of vscatter/vgather) - 0 is returned)
    */
    virtual USIZE Size() = 0;
    /** 
    * Effective address of the full operand.
    * For memory operands that do not have an address (for example, the memory 
    * operand of vscatter/vgather) is returned.
    * This function should be used only for Memory operands and will ASSERT if used 
    * for Register operands.
    */
    virtual ADDRINT Address() = 0;
    /** 
    * Number of operand elements
    */
    virtual UINT32 NumOfElements() = 0;
    /** 
    * Element size
    */
    virtual USIZE ElementSize(UINT32 element_index) = 0;
    /** 
    * Element effective address
    */
    virtual ADDRINT ElementAddress( UINT32 element_index ) = 0;
    /** 
    * Element mask value - 0 or 1
    */
    virtual UINT32 ElementMaskValue( UINT32 element_index ) = 0;
    /** 
    * Element access type (read/write/readwrite)
    */
    virtual PIN_OP_ELEMENT_ACCESS ElementAccessType( UINT32 element_index) = 0;
};

/*! @ingroup PROTO
 *
 * List of function prototype arguments. See @ref PROTO_Allocate() for its
 * use.
 */
typedef PROTO_CLASS* PROTO;

/*! @ingroup THREADS
 *  Thread ID assigned by PIN.
 */
typedef UINT32 THREADID;

/*! @ingroup THREADS
 *  Invalid value of the THREADID type.
 */
const THREADID INVALID_THREADID = static_cast< THREADID >(-1);

/*! @ingroup THREADS
 * Unique thread ID which, unlike THREADID identifier, is not reused by Pin after
 * the thread termination. The uniqueness of this identifier allows to use it
 * in the @ref PIN_WaitForThreadTermination() function which monitors the thread's state.
 */
typedef UINT64 PIN_THREAD_UID;

/*! @ingroup THREADS
 *  Invalid value of the PIN_THREAD_UID type.
 */
const PIN_THREAD_UID INVALID_PIN_THREAD_UID = static_cast< PIN_THREAD_UID >(-1);

/*! @ingroup THREADS
 *  Thread ID assigned by OS.
 */
typedef NATIVE_TID OS_THREAD_ID;

/*! @ingroup THREADS
 *  Invalid value of the OS_THREAD_ID type.
 */
const OS_THREAD_ID INVALID_OS_THREAD_ID = INVALID_NATIVE_TID;

/*! @ingroup THREADS
 *  Main (starting) function of a thread.
 *  @param[in] arg  argument of the main thread function, as specified by the
 *                  thread creator.
 */
typedef VOID ROOT_THREAD_FUNC(VOID* arg);

/*! @ingroup THREADS
 *  Process ID assigned by OS.
 */
typedef UINT32 OS_PROCESS_ID;

/*! @ingroup APPDEBUG
 * Possible status codes telling the state of application level debugging.
 */
enum DEBUG_STATUS
{
    DEBUG_STATUS_DISABLED,      ///< Application debugging is not enabled in this Pin session.
    DEBUG_STATUS_UNCONNECTABLE, ///< Application debugging is enabled, but it is too early to allow a debugger to connect.
    DEBUG_STATUS_UNCONNECTED,   ///< Application debugging is enabled, but no debugger is connected yet.
    DEBUG_STATUS_CONNECTED      ///< Application debugging is enabled and a debugger is connected.
};

/*! @ingroup APPDEBUG
 * Possible connection types for an application debugger. The connection type
 * can be specified either via the -appdebug knobs or by PIN_SetDebugMode().
 */
enum DEBUG_CONNECTION_TYPE
{
    DEBUG_CONNECTION_TYPE_NONE,       ///< Application debugging is disabled in this session.
    DEBUG_CONNECTION_TYPE_TCP_SERVER, ///< Pin opens a TCP port and waits for a debugger to connect.
    DEBUG_CONNECTION_TYPE_TCP_CLIENT  ///< Pin connects to a TCP port opened by the debugger.
};

/*! @ingroup APPDEBUG
 * Application debugger types that could be connected to Pin.
 */
enum DEBUGGER_TYPE
{
    DEBUGGER_TYPE_UNKNOWN,             ///< No debugger connected, or type is unknown.
    DEBUGGER_TYPE_GDB,                 ///< The GNU debugger.
    DEBUGGER_TYPE_LLDB,                ///< The LLVM debugger.
    DEBUGGER_TYPE_IDB,                 ///< The Intel debugger.
    DEBUGGER_TYPE_VISUAL_STUDIO_VSDBG, ///< Visual Studio via VSDBG.
    DEBUGGER_TYPE_VISUAL_STUDIO        ///< Visual Studio via native connection.
};

/*! @ingroup APPDEBUG
 * Options which affect application debugging.
 */
enum DEBUG_MODE_OPTION
{
    DEBUG_MODE_OPTION_NONE = 0, ///< No options specified.

    /*!
     * If this option is set, Pin stops the application at the first instruction
     * and execution remains stopped until a debugger connects and continues
     * the application. If this option is cleared, the application immediately
     * runs when PIN_StartProgram() is called.
     */
    DEBUG_MODE_OPTION_STOP_AT_ENTRY = (1 << 0),

    /*!
     * If debugging is enabled Pin normally prints a message to the console when
     * PIN_StartProgram() is called which tells the user how to connect a debugger.
     * This option suppresses the message.
     */
    DEBUG_MODE_OPTION_SILENT = (1 << 1),

    /*!
     * By default, Pin only listens for a debugger's TCP connection on the local
     * machine.  If this option is enabled, Pin will also listen for a connection
     * from a remote machine.
     */
    DEBUG_MODE_OPTION_ALLOW_REMOTE = (1 << 2)
};

/*!
 * Information for DEBUG_CONNECTION_TYPE_TCP_SERVER.
 */
struct _tcpServerStruct
{
    int _tcpPort; ///< TCP port that Pin listens on waiting for a debugger connection.
};

/*! @ingroup APPDEBUG
 * Information that an external application level debugger needs in order to connect to Pin.
 */
struct DEBUG_CONNECTION_INFO
{
    DEBUG_CONNECTION_TYPE _type; ///< Tells the type of debugger connection.
    BOOL _stopAtEntry;           ///< TRUE if Pin stops in debugger at first instruction (-appdebug).

    // The following data is dependent on the connection type.
    //
    union
    {
        struct _tcpServerStruct _tcpServer;
    };
};

/*!
 * Information for DEBUG_CONNECTION_TYPE_TCP_CLIENT.
 */
struct _tcpClientStruct
{
    const char* _ip; ///< IP address of machine running debugger (dot decimal format).  NULL for local machine.
    int _tcpPort;    ///< TCP port that debugger is listening on, waiting for a connection from Pin.
};

/*! @ingroup APPDEBUG
 * Information about the application debugging mode.
 */
struct DEBUG_MODE
{
    DEBUG_CONNECTION_TYPE _type; ///< Tells the connection type to the application debugger.
    DEBUG_MODE_OPTIONS _options; ///< Options which affect application debugging.

    // The following data is dependent on the connection type.
    //
    union
    {
        /*!
         * Information for DEBUG_CONNECTION_TYPE_TCP_CLIENT.
         */
        struct _tcpClientStruct _tcpClient;
    };
};

/*! @ingroup INST_ARGS
 *
 * used in for memory address translation callback
 */

typedef union
{
    UINT32 data;
    struct
    {
        UINT32 isAtomic : 1;
        UINT32 isRmw : 1;
        UINT32 isPrefetch : 1;
        UINT32 isFromPin : 1;
    } bits;
} PIN_MEM_TRANS_FLAGS;

typedef struct
{
    // because this structure is allocated on the stack before
    // the call to CallMemoryAddrTrans and a function may
    // write into the 48bytes above it's rsp
    UINT8 dummySpace[32];
    THREADID threadIndex;
    ADDRINT addr;
    size_t bytes;
    PIN_MEMOP_ENUM memOpType;
    ADDRINT ip;
    PIN_MEM_TRANS_FLAGS flags;
} PIN_MEM_TRANS_INFO;

/*! @ingroup PIN_CONTROL
* Additional parameters to define how exactly to perform @ref PIN_CallApplicationFunction ()
*/
typedef struct
{
    UINT32 native : 1; ///< When set, PIN executes the function without instrumenting it (default: FALSE)
} CALL_APPLICATION_FUNCTION_PARAM;

/*! @ingroup PIN_CONTROL
* Types of Pin callbacks.
* These type could be used with the function @ref PIN_SetAllowedConcurrentCallbacks()
*/
enum PIN_CALLBACK_TYPE
{
    PIN_CALLBACK_TYPE_NONE    = 0, ///< None of the callback types.
    PIN_CALLBACK_TYPE_SYSCALL = 1  ///< Syscall entry/exit callback.
};

#endif
