/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

/*!
 * @defgroup OS_APIS_SIGNALS Signals
 * @brief Contains signals related os apis
 */

#ifndef OS_APIS_SIGNALS_H
#define OS_APIS_SIGNALS_H

#include "os-apis.h"
#include "signals-core.h"

/*!
* Retrun the Max number of signals possible
*/
int OS_MaxSignalNumber();

/*!
* This enum is used only for the OS_PinSigactionToKernelSigaction function
*  you cannot use these values directly in other OS-APIs functions or system calls
*/
typedef enum
{
    SIGACTION_SIGINFO,
    SIGACTION_NODEFER,
    SIGACTION_RESETHAND,
    SIGACTION_ONSTACK,
    SIGACTION_NOCLDSTOP,
    SIGACTION_RESTART,
    SIGACTION_RESTORER
} SigactionDefines;

/*
* This function is used to convert the defines into the right
* value according to the OS (bit map value)
*/
UINT64 OS_PinSigactionToKernelSigaction(SigactionDefines input);

#ifdef TARGET_MAC
typedef void (*OS_SIGTRAP_PTR)(void*, unsigned int, int, void*, void*);

void OS_SigReturn(void* uctx, int infostyle);

#endif

/*! @ingroup OS_APIS_SIGNALS
 * Specifies an action to OS_SigAction()
 */
struct SIGACTION
{
    union
    {
        void (*_sa_handler)(int);                 //! Signal handle function (old way)
        void (*_sa_sigaction)(int, void*, void*); //! Signal handle function (new way)
        void* _sa_handler_ptr;                    //! Convenience void* pointer to the signal handler.
    } _u;
    SIGSET_T sa_mask;       //! Mask of signals to block during the handling of the signal
    unsigned long sa_flags; //! Additional flags (OS specific).

    void (*sa_restorer)(void); //! Signal restorer.
};

#ifdef TARGET_MAC
struct SIGACTION_WITH_TRAMP
{
    struct SIGACTION act;
    void (*sa_tramp)(void*, unsigned int, int, void*, void*);
};
#endif

#ifdef TARGET_MAC
/**
 *  Same as OS_SigAction but the specified act contains a trampoline (meaning don't use the default Pin trampoline
 *  but the the one that is exists in act)
 */
OS_RETURN_CODE OS_SigActionWithTrampoline(INT signum, const struct SIGACTION_WITH_TRAMP* actWithTramp, struct SIGACTION* oldact);
#endif

/*! @ingroup OS_APIS_SIGNALS
 * Change the action taken by a process on receipt of a specific signal.
 * This function is compatible with POSIX sigaction().
 *
 * @param[in]  signum       The signal to alter its behavior.
 * @param[in]  act          The action to be taken upon signal reception.
 * @param[in]  oldact       The previous action that was taken upon signal reception.
 *
 * @return     Operation status code.
 *
 * @par Availability:
 *   @b O/S:   Linux & macOS*\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_SigAction(INT signum, const struct SIGACTION* act, struct SIGACTION* oldact);

/*! @ingroup OS_APIS_SIGNALS
 * Temporarily  replaces  the  signal  mask of the calling process with the mask given
 * by mask and then suspends the process until delivery of a signal  whose action is to
 * invoke a signal handler or to terminate a process.
 *
 * @param[in]  mask         The mask to use for the signals
 *
 * @return     Operation status code.
 *
 * @par Availability:
 *   @b O/S:   Linux & macOS*\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_SigSuspend(const SIGSET_T* mask);

/*! @ingroup OS_APIS_SIGNALS
 * Returns the set of signals that are pending for delivery to the calling thread
 * (i.e., the signals which have been raised while blocked).  The mask of
 * pending signals is returned in set.
 *
 * @param[out] set          Where to store the signal set.
 *
 * @return     Operation status code.
 *
 * @par Availability:
 *   @b O/S:   Linux & macOS*\n
 *   @b CPU:   All\n
 */
OS_RETURN_CODE OS_SigPending(const SIGSET_T* set);

/*! @ingroup OS_APIS_SIGNALS
* Send signal to a particular thread inside a process.
*
* @param[in]  pid          The process ID where the thread is running.if it's INVALID_NATIVE_PID then we
ignore the pid.
* @param[in]  tid          The thread ID to send the signal to.
* @param[in]  signal       The signal to send.
*
* @return     Operation status code.
*
* @par Availability:
*   @b O/S:   Linux & macOS*\n
*   @b CPU:   All\n
*/
OS_RETURN_CODE OS_SendSignalToThread(NATIVE_PID pid, NATIVE_TID tid, UINT32 signal);

#endif // OS_APIS_SIGNALS_H
