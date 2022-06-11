/*
 * Copyright (C) 2016-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

/*!
 * @defgroup OS_APIS_SIGNALS Signals
 * @brief Contains signals related os apis
 */

#ifndef OS_APIS_SIGNALS_CORE_H
#define OS_APIS_SIGNALS_CORE_H

#ifdef TARGET_MAC
typedef UINT32 SIGSET_T;
#else
typedef UINT64 SIGSET_T;
#endif

/*! @ingroup OS_APIS_SIGNALS
* Represents a set of signals
*/

/*!
* Initialize the set to contain none of the signals.
*
*  @param[in] set     The Set
*/
static inline void OS_SigSetEmpty(SIGSET_T* set) { *set = 0; }

/*!
* Return true if the specified signal set doesn't contain any signals, false otherwise.
*
*  @param[in] set     The Set
*/
static inline int OS_SigIsEmpty(SIGSET_T* set) { return (*set == 0); }

/*!
* Add Signal to the set
*
*  @param[in] set     The Set
*
*  @param[in] signal  The Signal we want to add to set
*/
static inline void OS_SigSetAdd(SIGSET_T* set, int signal)
{
    UINT64 mask = (UINT64)1 << (signal - 1);
    *set |= mask;
}

/*!
* Check if signal is part of the set
*
*  @param[in] set     The Set
*
*  @param[in] signal  The Signal we want to check
*/
static inline int OS_SigSetIsMember(const SIGSET_T* set, int signal)
{
    UINT64 mask = (UINT64)1 << (signal - 1);
    return ((*set & mask) != 0);
}

/*!
* Check if at least one signal in \a other is part of the \a set
*
*  @param[in] set   The Set
*
*  @param[in] other Another signals set
*/
static inline int OS_SigSetAnyMember(const SIGSET_T* set, const SIGSET_T* other) { return ((*set & *other) != 0); }

/*!
* Add all signals from \a other to this set.
*
*  @param[in] set      The Set
*
*  @param[in] other    The signal set.
*/
static inline void OS_SigSetAddSet(SIGSET_T* set, const SIGSET_T* other) { *set |= *other; }

/*!
* Remove all signals from \a other from this set.
*
*  @param[in] set      The Set
*
*  @param[in] other    The signal set.
*/
static inline void OS_SigSetRemoveSet(SIGSET_T* set, const SIGSET_T* other) { *set &= ~*other; }

/*!
* Remove signals from \a set which don't exist in \a other (Intersection of \a set and \a other into set)
*
*  @param[in] set      The Set
*
*  @param[in] other    The signal set.
*/
static inline void OS_SigIntersect(SIGSET_T* set, const SIGSET_T* other) { *set &= *other; }

/*!
* Return one word of the mask (representing 32 signals).
*
*  @param[in] set     The Set
*
*  @param[in] i    Tells which word to return: 0 reprsents signals 1-32,
*                   1 represents signals 33-64, etc.  The lowest signal
*                   number corresponds to the least significant bit.
*
* @return  The mask word.
*/
static inline UINT32 OS_SigSetGetMaskWord(const SIGSET_T* set, unsigned i)
{
    return ((i < sizeof(SIGSET_T) / 4) ? *((UINT32*)set + i) : 0);
}

/*!
* Set one word of the mast (representing 32 signals).
*
*  @param[in] set     The Set
*
*  @param[in] i        Tells which word to set: 0 corresponds to signals 1-32,
*                       1 to signals 33-64, etc.
*  @param[in] word     The mask word to set.  The lowest bit represents
*                       the smallest signal.
*/
static inline void OS_SigSetSetMaskWord(SIGSET_T* set, unsigned i, UINT32 word)
{
    if (i < sizeof(SIGSET_T) / 4)
    {
        *((UINT32*)set + i) = word;
    }
}

/*!
* Add all the signals to the set.
*
*/
static inline void OS_SigSetFill(SIGSET_T* set) { *set = (8 == sizeof(SIGSET_T)) ? ~(UINT64)0 : ~(UINT32)0; }

/*!
* Remove signal from the set.
*
*  @param[in] signal   The signal.
*
*/
static inline void OS_SigSetRemove(SIGSET_T* set, int signal)
{
    UINT64 mask = (UINT64)1 << (signal - 1);
    *set &= ~mask;
}

/*! @ingroup OS_APIS_SIGNALS
* Send signal to a process.
* This function is compatible with POSIX kill().
*
* @param[in]  pid          The process to send signal to.
* @param[in]  signal       The signal to send.
*
* @return     Operation status code.
*
* @par Availability:
*   @b O/S:   Linux & macOS*\n
*   @b CPU:   All\n
*/
OS_RETURN_CODE OS_SendSignalToProcess(NATIVE_PID pid, INT signal);

/*! @ingroup OS_APIS_SIGNALS
* Change the signal mask, the set of currently blocked signals for the current thread.
* This function is compatible with POSIX sigprocmask().
*
* @param[in]  how          Specifies how to alter the blocked signals mask:
*                          SIG_BLOCK: The set of blocked signals is the union of the current set and the set argument.
SIG_UNBLOCK: The signals in set are removed from the current set of blocked signals.
It is legal to attempt to unblock a signal which is not blocked.
SIG_SETMASK: The set of blocked signals is set to the argument set.
* @param[in]  set          The signal set to alter.
* @param[in]  oldset       The previous blocked signals set.
*
* @return     Operation status code.
*
* @par Availability:
*   @b O/S:   Linux & macOS*\n
*   @b CPU:   All\n
*/
OS_RETURN_CODE OS_SigProcMask(INT how, const SIGSET_T* set, SIGSET_T* oldset);

#endif // OS_APIS_SIGNALS_EXT_H
