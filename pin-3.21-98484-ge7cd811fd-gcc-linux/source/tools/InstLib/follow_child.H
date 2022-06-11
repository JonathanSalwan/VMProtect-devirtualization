/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#ifndef FOLLOW_CHILD_H
#define FOLLOW_CHILD_H

using std::cerr;
using std::cout;
using std::endl;

#ifndef TARGET_WINDOWS

#if defined(TARGET_LINUX)
#include <syscall.h>
#endif

#if defined(TARGET_MAC)
#include <sys/syscall.h>
#endif

namespace INSTLIB
{
/*! @defgroup FOLLOW_CHILD
 *
 * Instrumentation for injecting pin in child processes. Pin will always be
 * in the child and parent after a fork. By default, pin will not be in a
 * process after an exec system call. This tool intercepts the exec system
 * call and inserts a Pin command line prefix so pin will also be present
 * after exec.
 *
 */

/*! @ingroup FOLLOW_CHILD
 *
 * The example below can be found in InstLibExamples/follow_child.cpp
 *
  \include follow_child.cpp
*/
class FOLLOW_CHILD
{
  public:
    /*! @ingroup FOLLOW_CHILD
     *
     * Constructor
     */
    FOLLOW_CHILD()
    {
        _prefix = 0;
        _active = FALSE;
    };

    /*! @ingroup FOLLOW_CHILD
     *
     * Set the prefix to be used for the next child. The prefix is the full
     * pathname to the pin binary followed by everything up to and
     * including the --. It is stored as array of pointers to tokens. Most
     * users can simply use the argv array that is passed to the main of
     * the tool.
     */
    VOID SetPrefix(CHAR* prefix[])
    {
        ASSERTX(_active == TRUE);
        _prefix = prefix;
    }

    /*! @ingroup FOLLOW_CHILD
     *
     * Activate, must be called before PIN_StartProgram
     */
    VOID Activate()
    {
        ASSERTX(_active == FALSE);
        _active = TRUE;
        INS_AddInstrumentFunction(Instruction, this);
    }

  private:
    /*
     * Instrumentation function
     *
     * Instrument all the system calls to look for exec
     */
    static VOID Instruction(INS ins, VOID* v)
    {
        if (!static_cast< FOLLOW_CHILD* >(v)->_active || !INS_IsSyscall(ins)) return;

        INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(StaticFollowExec), IARG_PTR, v, IARG_SYSCALL_NUMBER, IARG_SYSARG_REFERENCE, 0,
                       IARG_SYSARG_REFERENCE, 1, IARG_SYSARG_REFERENCE, 2, IARG_END);
    }

    /*
     * Analysis function must be static member functions. We pass the
     * object and use this wrapper to get to the member function
     */
    static VOID StaticFollowExec(FOLLOW_CHILD* me, int syscallNumber, CHAR const** filename, CHAR const** argv[], VOID* envp)
    {
        me->FollowExec(syscallNumber, filename, argv, envp);
    }

    /*
     * Analysis function.
     *
     * If this is an exec system call, rewrite the arguments to insert the pin prefix
     */
    VOID FollowExec(int syscallNumber, CHAR const** filename, CHAR const** argv[], VOID* envp)
    {
        if (syscallNumber != SYS_execve) return;

        CHAR const* const appFilenameKnob = "-app_filename";

        // Construct a new argv array

        // Compute size of pin prefix, ends with '--'
        INT32 newArgvSize = 0;
        for (INT32 i = 0; strcmp(_prefix[i], "--") != 0; i++)
        {
            if (strcmp(_prefix[i], appFilenameKnob) == 0)
            {
                // Don't count the appFilenameKnob
                i++;
            }
            else
            {
                newArgvSize++;
            }
        }

        // Add space for "-app_filename <file> --"
        newArgvSize += 3;

        // Compute size of application argv, ends with 0
        for (INT32 i = 0; (*argv)[i] != 0; i++)
        {
            newArgvSize++;
        }

        // Add space for 0
        newArgvSize++;

        // We are about to do an exec, so this is not a true memory leak
        CHAR const** newArgv = new CHAR const*[newArgvSize];
        INT32 newArgc        = 0;

        // Add Pin binary name
        newArgv[newArgc++] = PIN_VmFullPath();

        // Add "-app_filename <file>"
        newArgv[newArgc++] = appFilenameKnob;
        newArgv[newArgc++] = *filename;

        // Add rest of pin prefix, skipping binary name
        for (INT32 i = 1; strcmp(_prefix[i], "--") != 0; i++)
        {
            if (strcmp(_prefix[i], appFilenameKnob) == 0)
            {
                // Delete the appFilenameKnob
                i++;
            }
            else
            {
                newArgv[newArgc++] = _prefix[i];
            }
        }

        // Add "--"
        newArgv[newArgc++] = "--";

        // Add application argv
        for (INT32 i = 0; (*argv)[i] != 0; i++)
        {
            newArgv[newArgc++] = (*argv)[i];
        }
        // Add terminating 0
        newArgv[newArgc++] = 0;

        ASSERTX(newArgc == newArgvSize);

        // Change the system call arguments
        *filename = newArgv[0];
        *argv     = newArgv;

        const BOOL debug = FALSE;

        if (debug)
        {
            cout << "New argv filename: " << *filename << endl;
            for (INT32 i = 0; newArgv[i] != 0; i++)
            {
                cout << "  " << newArgv[i];
            }
            cout << endl;
        }
    }

    BOOL _active;
    CHAR const* const* _prefix;
};

} // namespace INSTLIB

#endif
#endif
