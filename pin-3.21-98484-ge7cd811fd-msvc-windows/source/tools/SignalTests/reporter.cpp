/*
 * Copyright (C) 2010-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool prints a log message whenever a signal is recieved or whenever
 * a context-change happens.
 */

#include <fstream>
#include <sstream>
#include "pin.H"
#include <signal.h>
using std::string;

static void OnSig(THREADID, CONTEXT_CHANGE_REASON, const CONTEXT*, CONTEXT*, INT32, VOID*);
static BOOL InterceptSig(THREADID, INT32, CONTEXT*, BOOL, const EXCEPTION_INFO*, VOID*);
static void OnEnd(INT32, VOID*);
static std::string SignalName(INT32);

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "reporter.out", "output file");
std::ofstream Out;

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    Out.open(KnobOutputFile.Value().c_str());

    for (INT32 sig = 1; sig < 32; sig++)
    {
        if (sig != SIGKILL && sig != SIGSTOP && sig != SIGCONT && sig != SIGTSTP) PIN_InterceptSignal(sig, InterceptSig, 0);
    }
    PIN_AddContextChangeFunction(OnSig, 0);
    PIN_AddFiniFunction(OnEnd, 0);

    PIN_StartProgram();
    return 0;
}

static void OnSig(THREADID, CONTEXT_CHANGE_REASON reason, const CONTEXT*, CONTEXT*, INT32 sig, VOID*)
{
    switch (reason)
    {
        case CONTEXT_CHANGE_REASON_SIGNAL:
            Out << "Got signal " << SignalName(sig) << "\n";
            break;

        case CONTEXT_CHANGE_REASON_FATALSIGNAL:
            Out << "Got fatal signal " << SignalName(sig) << "\n";
            break;

        case CONTEXT_CHANGE_REASON_SIGRETURN:
            Out << "Returning from signal handler\n";
            break;
        default:
            Out << "Unexpected CONTEXT_CHANGE_REASON " << reason << ", exiting the test." << std::endl;
            PIN_ExitProcess(1);
            break;
    }
}

static BOOL InterceptSig(THREADID, INT32 sig, CONTEXT*, BOOL hasHandler, const EXCEPTION_INFO* pExceptInfo, VOID*)
{
    Out << "Intercepting signal " << SignalName(sig);
    if (pExceptInfo) Out << ", which is an exception";
    if (hasHandler) Out << ", application handles";
    Out << "\n";

    return TRUE;
}

static void OnEnd(INT32 code, VOID* v)
{
    Out << "Program exited\n";
    Out.close();
}

static std::string SignalName(INT32 sig)
{
    switch (sig)
    {
        case SIGHUP:
            return "SIGHUP";
        case SIGINT:
            return "SIGINT";
        case SIGILL:
            return "SIGILL";
        case SIGTRAP:
            return "SIGTRAP";
        case SIGABRT:
            return "SIGABRT";
        case SIGFPE:
            return "SIGFPE";
        case SIGKILL:
            return "SIGKILL";
        case SIGUSR1:
            return "SIGUSR1";
        case SIGUSR2:
            return "SIGUSR2";
        case SIGSEGV:
            return "SIGSEGV";
        case SIGALRM:
            return "SIGALRM";
        case SIGTERM:
            return "SIGTERM";
        default:
        {
            std::ostringstream str;
            str << "<signal " << std::dec << sig << ">";
            return str.str();
        }
    }
}
