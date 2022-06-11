/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifdef PIN_G_EXCEPTION_PH
#error duplicate inclusion of exception
#else
#define PIN_G_EXCEPTION_PH
/*! @file
 *  Basic definitions pertaining to hardware and software exceptions.
 */
/*! @ingroup EXCEPTION
 *  Identifiers of exception classes. Exception class indicates the type of information 
 *  (set of attributes) that accompanies exceptions of this class.
 */
enum EXCEPTION_CLASS
{
    EXCEPTCLASS_NONE,              ///< Reserved. No real exception belongs to this class.
    EXCEPTCLASS_UNKNOWN,           ///< Unknown exception.
    EXCEPTCLASS_ACCESS_FAULT,      ///< Memory access fault.
    EXCEPTCLASS_INVALID_INS,       ///< Invalid instruction.
    EXCEPTCLASS_INT_ERROR,         ///< Erroneous integer operation.
    EXCEPTCLASS_FP_ERROR,          ///< Erroneous floating point operation.
    EXCEPTCLASS_MULTIPLE_FP_ERROR, ///< Received an ambiguous floating point exception.
    EXCEPTCLASS_DEBUG,             ///< Debugging trap.
    EXCEPTCLASS_OS                 ///< O/S specific exception.
};

/*! @ingroup EXCEPTION
 * Identifiers for all possible exception codes.  The codes below are divided into two groups.
 * The first group list all the exceptions that can be raised by @ref PIN_RaiseException().
 * Tools that want to be portable across operating systems should use codes from this first
 * group when raising exceptions via PIN_RaiseException().
 *
 * When Pin receives an exception and reports it to the tool, it may report an exception in
 * either the first or second group.  This can happen, for example, when using the @ref PIN_SafeCopyEx()
 * function.  When Pin can identify a specific exception, it reports a code from the first group.
 * Sometimes, though, the host operating system does not provide enough information for Pin to
 * determine the exact exception.  In these cases, Pin reports a code from the second group
 * (one of the EXCEPTCODE_RECEIVED_ codes).  A tool can always re-raise one of these exceptions
 * by passing it to PIN_RaiseException().  However, portable tools should avoid creating new
 * exceptions using codes from the second group, because these codes are not supported on all
 * operating systems.
 */
enum EXCEPTION_CODE
{
    EXCEPTCODE_NONE, ///< Reserved. No real exception has this code.

    // These exception codes are supported on all operating systems.  Portable tools should
    // use these codes when creating exceptions that wil be passed to PIN_RaiseException().

    EXCEPTCODE_ACCESS_INVALID_ADDRESS, ///< Virtual address not mapped (the \#PF exception).
                                       ///< Belongs to EXCEPTCLASS_ACCESS_FAULT.
    EXCEPTCODE_ACCESS_DENIED,          ///< Access not permitted due to protection violation (the \#PF exception).
                                       ///< Belongs to EXCEPTCLASS_ACCESS_FAULT.
    EXCEPTCODE_ACCESS_INVALID_PAGE,    ///< A \#PF exception for some reason other than the cases above.
                                       ///< Belongs to EXCEPTCLASS_ACCESS_FAULT.
    EXCEPTCODE_ACCESS_MISALIGNED,      ///< Misaligned memory reference (the \#AC exception).
                                       ///< Belongs to EXCEPTCLASS_ACCESS_FAULT.
    EXCEPTCODE_ILLEGAL_INS,            ///< Illegal instruction (the \#UD exception).
                                       ///< Belongs to EXCEPTCLASS_INVALID_INS.
    EXCEPTCODE_PRIVILEGED_INS,         ///< Privileged instruction (the \#GP exception).
                                       ///< Belongs to EXCEPTCLASS_INVALID_INS.
    EXCEPTCODE_INT_DIVIDE_BY_ZERO,     ///< Integer divide by zero (the \#DE exception).
                                       ///< Belongs to EXCEPTCLASS_INT_ERROR.
    EXCEPTCODE_INT_OVERFLOW_TRAP,      ///< Integer overflow trap (the \#OF trap).
                                       ///< Belongs to EXCEPTCLASS_INT_ERROR.
    EXCEPTCODE_INT_BOUNDS_EXCEEDED,    ///< Array index is out of bounds (\#BR exception).
                                       ///< Belongs to EXCEPTCLASS_INT_ERROR.
    EXCEPTCODE_X87_DIVIDE_BY_ZERO,     ///< x87 FPU divide by zero (the \#MF/\#Z exception).
                                       ///< Belongs to EXCEPTCLASS_FP_ERROR.
    EXCEPTCODE_X87_OVERFLOW,           ///< x87 FPU overflow (the \#MF/\#O exception).
                                       ///< Belongs to EXCEPTCLASS_FP_ERROR.
    EXCEPTCODE_X87_UNDERFLOW,          ///< x87 FPU underflow (the \#MF/\#U exception).
                                       ///< Belongs to EXCEPTCLASS_FP_ERROR.
    EXCEPTCODE_X87_INEXACT_RESULT,     ///< x87 FPU inexact result (the \#MF/\#P exception).
                                       ///< Belongs to EXCEPTCLASS_FP_ERROR.
    EXCEPTCODE_X87_INVALID_OPERATION,  ///< x87 FPU invalid operation (the \#MF/\#IA exception).
                                       ///< Belongs to EXCEPTCLASS_FP_ERROR.
    EXCEPTCODE_X87_DENORMAL_OPERAND,   ///< x87 FPU denormal operand (the \#MF/\#D exception).
                                       ///< Belongs to EXCEPTCLASS_FP_ERROR.
    EXCEPTCODE_X87_STACK_ERROR,        ///< x87 FPU stack overflowed or underflowed (the \#MF/\#IS exception).
                                       ///< Belongs to EXCEPTCLASS_FP_ERROR.
    EXCEPTCODE_SIMD_DIVIDE_BY_ZERO,    ///< SIMD floating point divide by zero (the \#XM/\#Z exception).
                                       ///< Belongs to EXCEPTCLASS_FP_ERROR.
    EXCEPTCODE_SIMD_OVERFLOW,          ///< SIMD floating point overflow (the \#XM/\#O exception).
                                       ///< Belongs to EXCEPTCLASS_FP_ERROR.
    EXCEPTCODE_SIMD_UNDERFLOW,         ///< SIMD floating point underflow (the \#XM/\#U exception).
                                       ///< Belongs to EXCEPTCLASS_FP_ERROR.
    EXCEPTCODE_SIMD_INEXACT_RESULT,    ///< SIMD floating point inexact result (the \#XM/\#P exception).
                                       ///< Belongs to EXCEPTCLASS_FP_ERROR.
    EXCEPTCODE_SIMD_INVALID_OPERATION, ///< SIMD invalid floating point operation (the \#XM/\#I exception).
                                       ///< Belongs to EXCEPTCLASS_FP_ERROR.
    EXCEPTCODE_SIMD_DENORMAL_OPERAND,  ///< SIMD denormal floating point operand (the \#XM/\#D exception).
                                       ///< Belongs to EXCEPTCLASS_FP_ERROR.
    EXCEPTCODE_DBG_BREAKPOINT_TRAP,    ///< Breakpoint trap (the \#BP trap).
                                       ///< Belongs to EXCEPTCLASS_DEBUG.
    EXCEPTCODE_DBG_SINGLE_STEP_TRAP,   ///< Trace trap (the \#DB trap).
                                       ///< Belongs to EXCEPTCLASS_DEBUG.

    // These exception codes can be used to raise Windows-specific exceptions.

    EXCEPTCODE_ACCESS_WINDOWS_GUARD_PAGE,     ///< Guard page access
                                              ///< (the Windows STATUS_GUARD_PAGE_VIOLATION exception).
                                              ///< Belongs to EXCEPTCLASS_ACCESS_FAULT.
    EXCEPTCODE_ACCESS_WINDOWS_STACK_OVERFLOW, ///< Thread stack overflowed
                                              ///< (the Windows STATUS_STACK_OVERFLOW exception).
                                              ///< Belongs to EXCEPTCLASS_ACCESS_FAULT.
    EXCEPTCODE_WINDOWS,                       ///< Generic Windows exception.
                                              ///< Belongs to EXCEPTCLASS_OS.

    // When Pin receives an exception, it may report the exception using one of the codes above or
    // one of the codes in the group below.

    EXCEPTCODE_RECEIVED_UNKNOWN,       ///< Unknown exception, cannot be re-raised.
                                       ///< Belongs to EXCEPTCLASS_UNKNOWN.
    EXCEPTCODE_RECEIVED_ACCESS_FAULT,  ///< General memory access fault.
                                       ///< Belongs to EXCEPTCLASS_ACCESS_FAULT.
    EXCEPTCODE_RECEIVED_AMBIGUOUS_X87, ///< Ambiguous x87 FPU exception.
                                       ///< PIN_GetFpErrorSet() tell which are possible.
                                       ///< Belongs to EXCEPTCLASS_MULTIPLE_FP_ERROR.
    EXCEPTCODE_RECEIVED_AMBIGUOUS_SIMD ///< Ambiguous SIMD FPU exception.
                                       ///< PIN_GetFpErrorSet() tell which are possible.
                                       ///< Belongs to EXCEPTCLASS_MULTIPLE_FP_ERROR.
};

/*! @ingroup EXCEPTION
 *  Types of faulty memory accesses that may cause an EXCEPTCLASS_ACCESS_FAULT exception
 */
enum FAULTY_ACCESS_TYPE
{
    FAULTY_ACCESS_TYPE_UNKNOWN, ///< Unknown access violation
    FAULTY_ACCESS_READ,         ///< Read access
    FAULTY_ACCESS_WRITE,        ///< Write access
    FAULTY_ACCESS_EXECUTE       ///< Execute access
};

/*! @ingroup EXCEPTION
 *  Possible types of FPU exceptions.
 */
enum FPERROR
{
    FPERROR_DIVIDE_BY_ZERO    = (1 << 0), ///< The #DE exception.
    FPERROR_OVERFLOW          = (1 << 1), ///< The #O exception.
    FPERROR_UNDERFLOW         = (1 << 2), ///< The #U exception.
    FPERROR_INEXACT_RESULT    = (1 << 3), ///< The #P exception.
    FPERROR_INVALID_OPERATION = (1 << 4), ///< The #IA exception on x87 or #I exception on SIMD.
    FPERROR_DENORMAL_OPERAND  = (1 << 5), ///< The #D exception.
    FPERROR_X87_STACK_ERROR   = (1 << 6)  ///< The #IS exception on x87.
};

/*! @ingroup EXCEPTION
 *  Maximum number of arguments that can be associated with an @ref EXCEPTCODE_WINDOWS exception.
 */
const UINT32 MAX_WINDOWS_EXCEPTION_ARGS = 5;

/*! @ingroup EXCEPTION
 *  Structure (POD) that describes an exception
 */
struct EXCEPTION_INFO
{
    /* ================================================================================== */
    // Generic exception information
    /* ================================================================================== */

    /*!
     * Initialize this structure with the given generic exception's attributes. Set "unknown" 
     * state for all exception-specific attributes. 
     * This function is useful when the specified exception code does not require any 
     * additional exception information or such information is not available. The function 
     * can not be used to initialize EXCEPTCODE_WINDOWS exception information.
     * @param[in]   exceptCode      exception code
     * @param[in]   exceptAddress   address of the instruction that caused the exception
     * @return reference to this structure
     */
    EXCEPTION_INFO& Init(EXCEPTION_CODE exceptCode, ADDRINT exceptAddress);

    /*!
     * @return The code of the exception
     */
    EXCEPTION_CODE GetExceptCode() const { return m_exceptCode; }

    /*!
     * @return The class of the exception
     */
    static EXCEPTION_CLASS GetExceptClass(EXCEPTION_CODE exceptCode);
    EXCEPTION_CLASS GetExceptClass() const { return GetExceptClass(m_exceptCode); }

    /*!
     * @return The address of the instruction that caused the exception
     */
    ADDRINT GetExceptAddress() const { return m_exceptAddress; }

    /*!
     * Set the address of the instruction that caused the exception
     */
    VOID SetExceptAddress(ADDRINT exceptAddress) { m_exceptAddress = exceptAddress; }

    /*!
     * @return TRUE if the specified exception is a trap. Traps are reported in the 
     *         context that represents state of registers after executing the 
     *         instruction that caused the trap. The exception address and the value
     *         of the PC register in the trap context are different.
     */
    static BOOL IsTrap(EXCEPTION_CODE exceptCode);
    BOOL IsTrap() const { return IsTrap(m_exceptCode); }

    /*!
     * Remove the exception information and initialize this structure as empty.
     */
    VOID Reset()
    {
        m_exceptCode    = EXCEPTCODE_NONE;
        m_exceptAddress = 0;
    }

    /*!
     * @return TRUE, if this structure has been reset and does not contain a valid
     *         exception information.
     */
    BOOL IsEmpty() const { return (m_exceptCode == EXCEPTCODE_NONE); }

    /*!
     * @return The string representation of this structure.
     */
    std::string ToString() const;

    /*!
     * @return The name of the exception's code.
     */
    std::string GetCodeAsString() const;

  public:
    /* ================================================================================== */
    // EXCEPTCLASS_ACCESS_FAULT information
    /* ================================================================================== */

    /*!
     * Initialize this structure with the given attributes of an EXCEPTCLASS_ACCESS_FAULT 
     * exception.
     * @param[in]   exceptCode      exception code that specifies an EXCEPTCLASS_ACCESS_FAULT 
     *                              exception
     * @param[in]   exceptAddress   address of the instruction that caused the exception
     * @param[in]   accessAddress   address of the faulty memory access
     * @param[in]   accessType      type of the faulty memory access
     * @return reference to this structure
     */
    EXCEPTION_INFO& InitAccessFault(EXCEPTION_CODE exceptCode, ADDRINT exceptAddress, ADDRINT accessAddress,
                                    FAULTY_ACCESS_TYPE accessType = FAULTY_ACCESS_TYPE_UNKNOWN);

    /*!
     * @return TRUE if this structure describes an EXCEPTCLASS_ACCESS_FAULT exception
     */
    BOOL IsAccessFault() const { return GetExceptClass() == EXCEPTCLASS_ACCESS_FAULT; }

    /*!
     * @return The type of the faulty memory access.
     * @pre    This structure should describe an EXCEPTCLASS_ACCESS_FAULT exception. 
     */
    FAULTY_ACCESS_TYPE GetFaultyAccessType() const
    {
        ASSERTX(IsAccessFault());
        return m_specific.m_access.m_type;
    }

    /*!
     * Get the address of the faulty memory access, if known.
     * @param[out] pAccessAddress   optional pointer to variable that receives the address
     *                              of the faulty memory access, if known.
     * @return TRUE, if the address of the faulty memory access is known
     * @pre    This structure should describe an EXCEPTCLASS_ACCESS_FAULT exception. 
     */
    BOOL GetFaultyAccessAddress(ADDRINT* pAccessAddress) const
    {
        ASSERTX(IsAccessFault());
        return m_specific.m_access.m_address.Get(pAccessAddress);
    }

  public:
    /* ================================================================================== */
    // EXCEPTCLASS_MULTIPLE_FP_ERROR information
    /* ================================================================================== */

    /*!
     * Initialize this structure with the given attributes of an EXCEPTCLASS_MULTIPLE_FP_ERROR 
     * exception.
     * @param[in]   exceptCode      exception code that specifies an EXCEPTCLASS_MULTIPLE_FP_ERROR 
     *                              exception
     * @param[in]   exceptAddress   address of the instruction that caused the exception
     * @param[in]   fpErrors        bit mask of FP exception codes
     * @return reference to this structure
     */
    EXCEPTION_INFO& InitMultipleFpError(EXCEPTION_CODE exceptCode, ADDRINT exceptAddress, UINT32 fpErrors);

    /*!
     * @return TRUE if this structure describes an EXCEPTCLASS_MULTIPLE_FP_ERROR exception
     */
    BOOL IsMultipleFpException() const { return GetExceptClass() == EXCEPTCLASS_MULTIPLE_FP_ERROR; }

    /*!
     * @return The exception's bit mask of FP exception codes.
     * @pre    This structure should describe an EXCEPTCLASS_MULTIPLE_FP_ERROR exception. 
     */
    UINT32 GetFpErrors() const
    {
        ASSERTX(IsMultipleFpException());
        return m_specific.m_multipleFp.m_fpErrors;
    }

  public:
    /* ================================================================================== */
    // EXCEPTCODE_WINDOWS information
    /* ================================================================================== */

    /*!
     * Initialize this structure with the given attributes of an EXCEPTCODE_WINDOWS
     * exception.
     * @param[in]   sysExceptCode   the system exception code
     * @param[in]   exceptAddress   address of the instruction that caused the exception
     * @param[in]   numArgs         number of arguments in the [pArgs] array. The number 
     *                              should not exceed the MAX_WINDOWS_EXCEPTION_ARGS value.
     * @param[in]   pArgs           pointer to an array of arguments associated with the exception 
     *                              or NULL if the number of arguments is zero
     * @return reference to this structure
     */
    EXCEPTION_INFO& InitWindowsSysException(UINT32 sysExceptCode, ADDRINT exceptAddress, UINT32 numArgs = 0,
                                            const ADDRINT* pArgs = 0);

    /*!
     * @return TRUE if this structure describes an EXCEPTCODE_WINDOWS exception
     */
    BOOL IsWindowsSysException() const { return m_exceptCode == EXCEPTCODE_WINDOWS; }

    /*!
     * @return The Windows system exception code
     * @pre    This structure should describe an EXCEPTCODE_WINDOWS exception. 
     */
    UINT32 GetWindowsSysExceptionCode() const
    {
        ASSERTX(IsWindowsSysException());
        return m_specific.m_winsys.m_exceptCode;
    }

    /*!
     * @return The number of arguments associated with the Windows system exception
     * @pre    This structure should describe an EXCEPTCODE_WINDOWS exception. 
     */
    UINT32 CountWindowsSysArguments() const
    {
        ASSERTX(IsWindowsSysException());
        return m_specific.m_winsys.m_numArgs;
    }

    /*!
     * Get the specified Windows system exception's argument.
     * @param[in]   argNum          ordinal number of the argument to be retrieved, starting 
     *                              from zero. The value of this parameter should not exceed
     *                              the value returned by the CountWindowsSysArguments() function.
     * @return The value of the specified system exception's argument
     * @pre    This structure should describe an EXCEPTCODE_WINDOWS exception. 
     */
    ADDRINT GetWindowsSysArgument(UINT32 argNum) const
    {
        ASSERTX(IsWindowsSysException());
        ASSERTX(argNum < m_specific.m_winsys.m_numArgs);
        return m_specific.m_winsys.m_args[argNum];
    }

  public:
    /* ================================================================================== */
    // Data
    /* ================================================================================== */
    EXCEPTION_CODE m_exceptCode; ///< Exception code
    ADDRINT m_exceptAddress;     ///< Address of the faulty instruction

    /*!
     * Exception-specific data
     */
    union EXCEPTION_SPECIFIC
    {
        struct ACCESS_FAULT ///< EXCEPTCLASS_ACCESS_FAULT additional information
        {
            FAULTY_ACCESS_TYPE m_type;           ///< The type of the faulty memory access
            OPTIONAL_VALUE< ADDRINT > m_address; ///< Address of the faulty memory access (optional)
        } m_access;

        struct WINDOWS_SYS ///< EXCEPTCODE_WINDOWS additional information
        {
            UINT32 m_exceptCode;                        ///< The system exception code or signal number
            UINT32 m_numArgs;                           ///< Number of arguments in the [m_args] array
            ADDRINT m_args[MAX_WINDOWS_EXCEPTION_ARGS]; ///< Array of arguments associated with the exception
        } m_winsys;

        struct MULTIPLE_FP ///< EXCEPTCLASS_MULTIPLE_FP_ERROR additional information
        {
            UINT32 m_fpErrors; ///< Bit mask of FPERROR values
        } m_multipleFp;
    } m_specific;
};

/*! @ingroup EXCEPTION
 *  Structure that describes an exception.\n
 *  The tool should not read or modify data members of this structure directly,
 *  but must treat the structure as logically opaque and use the @ref EXCEPTION
 *  functions to manage it.
 */
typedef struct EXCEPTION_INFO EXCEPTION_INFO;

/*! @ingroup EXCEPTION
 * Initialize the specified @ref EXCEPTION_INFO structure with the given generic
 * exception's attributes. Set "unknown" state for all exception-specific attributes.\n
 *
 * @note This function is useful when the specified exception code does not require any
 * additional exception information or such information is not available. The function can
 * not be used to initialize @ref EXCEPTCODE_WINDOWS exception information.\n
 * Use exception-specific initialization functions if an additional exception information
 * is available/required.
 *
 * @param[out]  pExceptInfo     pointer to the exception information structure to be initialized
 * @param[in]   exceptCode      exception code. It could specify any exception other than
 *                              @ref EXCEPTCODE_WINDOWS
 * @param[in]   exceptAddress   address of the instruction that caused the exception
 *
 * @par Availability:
 * \b Mode:  All\n
 * \b O/S:   All\n
 * \b CPU:   All\n
 */
inline VOID PIN_InitExceptionInfo(EXCEPTION_INFO* pExceptInfo, EXCEPTION_CODE exceptCode, ADDRINT exceptAddress)
{
    pExceptInfo->Init(exceptCode, exceptAddress);
}

/*! @ingroup EXCEPTION
 * Initialize the specified @ref EXCEPTION_INFO structure with the given attributes of an
 * @ref EXCEPTCLASS_ACCESS_FAULT exception.
 *
 * @param[out]  pExceptInfo     pointer to the exception information structure to be initialized
 * @param[in]   exceptCode      exception code that specifies an @ref EXCEPTCLASS_ACCESS_FAULT
 *                              exception
 * @param[in]   exceptAddress   address of the instruction that caused the exception
 * @param[in]   accessAddress   address of the faulty memory access
 * @param[in]   accessType      type of the faulty memory access
 *
 * @par Availability:
 * \b Mode:  All\n
 * \b O/S:   All\n
 * \b CPU:   All\n
 */
inline VOID PIN_InitAccessFaultInfo(EXCEPTION_INFO* pExceptInfo, EXCEPTION_CODE exceptCode, ADDRINT exceptAddress,
                                    ADDRINT accessAddress, FAULTY_ACCESS_TYPE accessType = FAULTY_ACCESS_TYPE_UNKNOWN)
{
    pExceptInfo->InitAccessFault(exceptCode, exceptAddress, accessAddress, accessType);
}

/*! @ingroup EXCEPTION
 * Initialize the specified @ref EXCEPTION_INFO structure with the given attributes of an
 * @ref EXCEPTCODE_WINDOWS exception.
 *
 * @param[out]  pExceptInfo     pointer to the exception information structure to be initialized
 * @param[in]   sysExceptCode   the system exception code
 * @param[in]   exceptAddress   address of the instruction that caused the exception
 * @param[in]   numArgs         number of arguments associated with the system exception. This is
 *                              the number of elements in the <pArgs> array; it should not exceed
 *                              the @ref MAX_WINDOWS_EXCEPTION_ARGS value.
 * @param[in]   pArgs           pointer to an array of arguments associated with the system exception
 *                              or NULL if the number of arguments is zero
 *
 * @par Availability:
 * \b Mode:  All\n
 * \b O/S:   Windows\n
 * \b CPU:   All\n
 */
inline VOID PIN_InitWindowsExceptionInfo(EXCEPTION_INFO* pExceptInfo, UINT32 sysExceptCode, ADDRINT exceptAddress,
                                         UINT32 numArgs = 0, const ADDRINT* pArgs = 0)
{
    pExceptInfo->InitWindowsSysException(sysExceptCode, exceptAddress, numArgs, pArgs);
}

/*! @ingroup EXCEPTION
 * Retrieve the exception code from the specified @ref EXCEPTION_INFO structure.
 * @param[in]   pExceptInfo     pointer to the exception information structure to be queried
 * @return The code of the exception
 *
 * @par Availability:
 * \b Mode:  All\n
 * \b O/S:   All\n
 * \b CPU:   All\n
 */
inline EXCEPTION_CODE PIN_GetExceptionCode(const EXCEPTION_INFO* pExceptInfo) { return pExceptInfo->GetExceptCode(); }

/*! @ingroup EXCEPTION
 * Given an exception code, return the corresponding exception class.
 * @param[in]   exceptCode  exception code
 * @return The class of the exception
 *
 * @par Availability:
 * \b Mode:  All\n
 * \b O/S:   All\n
 * \b CPU:   All\n
 */
inline EXCEPTION_CLASS PIN_GetExceptionClass(EXCEPTION_CODE exceptCode) { return EXCEPTION_INFO::GetExceptClass(exceptCode); }

/*! @ingroup EXCEPTION
 * Retrieve the address of the instruction that caused the specified exception.
 * @param[in]   pExceptInfo     pointer to the exception information structure to be queried
 * @return The address of the instruction that caused the exception
 *
 * @par Availability:
 * \b Mode:  All\n
 * \b O/S:   All\n
 * \b CPU:   All\n
 */
inline ADDRINT PIN_GetExceptionAddress(const EXCEPTION_INFO* pExceptInfo) { return pExceptInfo->GetExceptAddress(); }

/*! @ingroup EXCEPTION
 * Set the address of the instruction that caused the specified exception.
 * @param[in,out] pExceptInfo   pointer to the exception information structure that
 *                              receives the new exception address
 * @param[in] exceptAddress     address of the instruction that caused the exception
 */
inline VOID PIN_SetExceptionAddress(EXCEPTION_INFO* pExceptInfo, ADDRINT exceptAddress)
{
    pExceptInfo->SetExceptAddress(exceptAddress);
}

/*! @ingroup EXCEPTION
 * Retrieve the type of the faulty memory access from the specified @ref EXCEPTION_INFO
 * structure.
 * @param[in]   pExceptInfo     pointer to the exception information structure to be queried
 * @return The type of the faulty memory access that caused the exception
 * @pre    The specified exception information should describe an @ref EXCEPTCLASS_ACCESS_FAULT
 *         exception.
 *
 * @par Availability:
 * \b Mode:  All\n
 * \b O/S:   All\n
 * \b CPU:   All\n
 */
inline FAULTY_ACCESS_TYPE PIN_GetFaultyAccessType(const EXCEPTION_INFO* pExceptInfo)
{
    return pExceptInfo->GetFaultyAccessType();
}

/*! @ingroup EXCEPTION
 * Retrieve the address of the faulty memory access from the specified @ref EXCEPTION_INFO structure.
 * @param[in]   pExceptInfo     pointer to the exception information structure to be queried
 * @param[out]  pAccessAddress  pointer to variable that receives the address
 *                              of the faulty memory access, if known.
 *                              If this pointer is NULL, the function only checks if the
 *                              requested address is known but does not return its value.
 * @return TRUE, if the address of the faulty memory access is known
 * @pre    The specified exception information should describe an @ref EXCEPTCLASS_ACCESS_FAULT
 *         exception.
 *
 * @par Availability:
 * \b Mode:  All\n
 * \b O/S:   All\n
 * \b CPU:   All\n
 */
inline BOOL PIN_GetFaultyAccessAddress(const EXCEPTION_INFO* pExceptInfo, ADDRINT* pAccessAddress)
{
    return pExceptInfo->GetFaultyAccessAddress(pAccessAddress);
}

/*! @ingroup EXCEPTION
 * Retrieve the set of floating point exception codes from the specified @ref EXCEPTION_INFO
 * structure.  For EXCEPTCODE_RECEIVED_AMBIGUOUS_X87 and EXCEPTCODE_RECEIVED_AMBIGUOUS_SIMD
 * exceptions, this is the set of possible causes for the exception.
 * @param[in]   pExceptInfo     pointer to the exception information structure to be queried
 * @return A bitmask of FPERROR values.
 * @pre    The specified exception information should describe an @ref EXCEPTCLASS_MULTIPLE_FP_ERROR
 *         exception.
 *
 * @par Availability:
 * \b Mode:  All\n
 * \b O/S:   All\n
 * \b CPU:   All\n
 */
inline UINT32 PIN_GetFpErrorSet(const EXCEPTION_INFO* pExceptInfo) { return pExceptInfo->GetFpErrors(); }

/*! @ingroup EXCEPTION
 * Retrieve the Windows system exception code from the specified @ref EXCEPTION_INFO structure.
 * @param[in]   pExceptInfo     pointer to the exception information structure to be queried
 * @return The system exception code
 * @pre    The specified exception information should describe an @ref EXCEPTCODE_WINDOWS
 *         exception.
 *
 * @par Availability:
 * \b Mode:  All\n
 * \b O/S:   Windows\n
 * \b CPU:   All\n
 */
inline UINT32 PIN_GetWindowsExceptionCode(const EXCEPTION_INFO* pExceptInfo) { return pExceptInfo->GetWindowsSysExceptionCode(); }

/*! @ingroup EXCEPTION
 * Retrieve the number of Windows system exception's arguments from the specified
 * @ref EXCEPTION_INFO structure.
 * @param[in]   pExceptInfo     pointer to the exception information structure to be queried
 * @return The number of arguments associated with the system exception
 * @pre    The specified exception information should describe an @ref EXCEPTCODE_WINDOWS
 *         exception.
 *
 * @par Availability:
 * \b Mode:  All\n
 * \b O/S:   Windows\n
 * \b CPU:   All\n
 */
inline UINT32 PIN_CountWindowsExceptionArguments(const EXCEPTION_INFO* pExceptInfo)
{
    return pExceptInfo->CountWindowsSysArguments();
}

/*! @ingroup EXCEPTION
 * Retrieve the specified Windows system exception's argument from the specified
 * @ref EXCEPTION_INFO structure.
 * @param[in]   pExceptInfo     pointer to the exception information structure to be queried
 * @param[in]   argNum          ordinal number of the argument to be retrieved, starting
 *                              from zero. The value of this parameter should not exceed
 *                              the value returned by the @ref PIN_CountWindowsExceptionArguments
 *                              function.
 * @return The value of the specified system exception's argument
 * @pre    The specified exception information should describe an @ref EXCEPTCODE_WINDOWS
 *         exception.
 *
 * @par Availability:
 * \b Mode:  All\n
 * \b O/S:   Windows\n
 * \b CPU:   All\n
 */
inline ADDRINT PIN_GetWindowsExceptionArgument(const EXCEPTION_INFO* pExceptInfo, UINT32 argNum)
{
    return pExceptInfo->GetWindowsSysArgument(argNum);
}

/*! @ingroup EXCEPTION
 * Get a string representation of the specified @ref EXCEPTION_INFO structure.
 * @param[in]   pExceptInfo     pointer to the exception information structure to be stringized
 * @return The string representation of this structure.
 *
 * @par Availability:
 * \b Mode:  All\n
 * \b O/S:   All\n
 * \b CPU:   All\n
 */
inline std::string PIN_ExceptionToString(const EXCEPTION_INFO* pExceptInfo) { return pExceptInfo->ToString(); }

#endif // PIN_G_EXCEPTION_PH
