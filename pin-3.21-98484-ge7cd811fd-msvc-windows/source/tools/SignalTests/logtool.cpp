/*
 * Copyright (C) 2006-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <fstream>
#include <sstream>
#include <signal.h>
#include "pin.H"
using std::string;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "logtool.out", "log file");
std::ofstream Out;

static void OnSig(THREADID, CONTEXT_CHANGE_REASON, const CONTEXT*, CONTEXT*, INT32, VOID*);
static void Fini(INT32, VOID*);
static std::string StrSig(int);

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    Out.open(KnobOutputFile.Value().c_str());

    PIN_AddFiniFunction(Fini, 0);
    PIN_AddContextChangeFunction(OnSig, 0);

    PIN_StartProgram();
    return 0;
}

static void OnSig(THREADID threadIndex, CONTEXT_CHANGE_REASON reason, const CONTEXT* ctxtFrom, CONTEXT* ctxtTo, INT32 sig,
                  VOID* v)
{
    switch (reason)
    {
        case CONTEXT_CHANGE_REASON_FATALSIGNAL:
            Out << "Received fatal signal " << StrSig(sig) << " on thread " << threadIndex << std::endl;
            break;
        case CONTEXT_CHANGE_REASON_SIGNAL:
            Out << "About to handle signal " << StrSig(sig) << " on thread " << threadIndex << std::endl;
            break;
        case CONTEXT_CHANGE_REASON_SIGRETURN:
            Out << "Returning from handler on thread " << threadIndex << std::endl;
            break;
        default:
            Out << "Unexpected CONTEXT_CHANGE_REASON " << reason << ", exiting the test." << std::endl;
            PIN_ExitProcess(1);
            break;
    }
}

static void Fini(INT32 code, VOID* v) { Out << "Exiting" << std::endl; }

static std::string StrSig(int sig)
{
    switch (sig)
    {
        case SIGUSR1:
            return "SIGUSR1";
        case SIGUSR2:
            return "SIGUSR2";
        case SIGSEGV:
            return "SIGSEGV";
        case SIGCONT:
            return "SIGCONT";
        case SIGINT:
            return "SIGINT";
        default:
            break;
    }

    std::ostringstream os;
    os << "<" << std::dec << sig << ">";
    return os.str();
}
