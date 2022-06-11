/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <iostream>
#include "pin.H"
using std::cerr;
using std::endl;

static BOOL HandleSig(THREADID, INT32, CONTEXT*, BOOL hndlr, const EXCEPTION_INFO*, VOID*);
static BOOL ParseCmdLine(int, char**);
static void Usage();

// These parameters are set from the tool's command line

int Signal     = SIGINT; // Signal to intercept
unsigned Count = 1;      // Tool exits after intercepting signal this many times
BOOL PassToApp = FALSE;  // Whether intercepted signals should be passed to application handler

int main(int argc, char** argv)
{
    PIN_Init(argc, argv);
    if (!ParseCmdLine(argc, argv)) return 1;

    PIN_InterceptSignal(Signal, HandleSig, 0);
    PIN_UnblockSignal(Signal, TRUE);

    PIN_StartProgram();
    return 0;
}

static BOOL HandleSig(THREADID tid, INT32 sig, CONTEXT* ctxt, BOOL hndlr, const EXCEPTION_INFO* exception, VOID* v)
{
    std::cerr << "Thread : " << tid << " Intercepting signal " << sig << std::endl;
    if (exception)
        std::cerr << "Signal is an exception" << std::endl;
    else
        std::cerr << "Signal is not an exception" << std::endl;
    if (--Count == 0) exit(0);
    return PassToApp;
}

static BOOL ParseCmdLine(int argc, char** argv)
{
    // Skip over the Pin arguments.
    //
    int i;
    for (i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-t") == 0 && (i + 2 < argc))
        {
            i += 2;
            break;
        }
    }

    bool done = false;
    for (; i < argc && !done; i++)
    {
        if (argv[i][0] != '-')
        {
            Usage();
            return FALSE;
        }

        switch (argv[i][1])
        {
            case 's':
            {
                if (i + 1 >= argc)
                {
                    Usage();
                    return FALSE;
                }
                Signal = strtol(argv[i + 1], 0, 10);
                if (Signal < 1 || Signal > 64)
                {
                    cerr << "Invalid signal number: " << argv[i + i] << endl;
                    Usage();
                    return FALSE;
                }
                i++;
                break;
            }

            case 'c':
            {
                if (i + 1 >= argc)
                {
                    Usage();
                    return FALSE;
                }
                unsigned long count = strtoul(argv[i + 1], 0, 10);
                if (count == 0 || count > UINT_MAX)
                {
                    cerr << "Invalid count, must be greater than zero: " << argv[i + 1] << endl;
                    Usage();
                    return FALSE;
                }
                Count = static_cast< unsigned >(count);
                i++;
                break;
            }

            case 'p':
            {
                PassToApp = TRUE;
                break;
            }

            case '-':
            {
                done = TRUE;
                break;
            }

            default:
            {
                cerr << "Invalid argument: " << argv[i] << endl;
                Usage();
                return FALSE;
            }
        }
    }

    return TRUE;
}

static void Usage()
{
    cerr << endl;
    cerr << "pin -t intercepttool [-s <signal>] [-c <count>] [-p]" << endl;
    cerr << "    -s <signal>  - Specifies signal to intercept (default is SIGINT)" << endl;
    cerr << "    -c <count>   - Tool exits after intercepting signal this many times (default 1)" << endl;
    cerr << "    -p           - Tool forwards signal to application after intercepting" << endl;
    cerr << endl;
}
