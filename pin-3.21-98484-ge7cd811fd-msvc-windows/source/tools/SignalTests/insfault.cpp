/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*!
 * This application causes various "complex" instructions to fault, and then
 * verifies that the fault was received at the expected location.  It also
 * sets all the registers to known values before the fault, and then verifies
 * that the signal context contains the expected values of those registers.
 *
 * The "complex" instructions are chosen to match those instructions that
 * Pin needs to translate into instruction sequences.  Therefore, this is a
 * good test that Pin correctly propagated the instruction attributes when
 * doing the translation, so that the fault is later recognized as being on
 * an "application" instruction (and not a Pin internal or tool instruction).
 *
 * It's also a good test of Pin's support for recovering the original program
 * state at the location of the fault.
 */

#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <cassert>
#include <signal.h>
#include <setjmp.h>
#include <sys/mman.h>

// Possible results from running a single test.
//
enum TSTATUS
{
    TSTATUS_NOFAULT, // Test did not raise a fault
    TSTATUS_DONE     // There are no more tests to run
};

static sigjmp_buf JumpBuffer;
static unsigned TestNumber = 0;
static bool IsError        = false;

static bool Initialize();
static void Handle(int, siginfo_t*, void*);
static TSTATUS DoTest(unsigned);

extern bool CheckUContextRegisters(void*);
extern "C" void DoFaultRetSp();
extern "C" void DoFaultRetTarg();
extern "C" void DoFaultRetImmSp();
extern "C" void DoFaultRetImmTarg();
extern "C" void DoFaultCallSp();
extern "C" void DoFaultCallTarg();
extern "C" void DoFaultCallRegSp();
extern "C" void DoFaultCallRegTarg();
extern "C" void DoFaultCallMemSp();
extern "C" void DoFaultCallMemTarg();
extern "C" void DoFaultCallMemBadMem();
extern "C" void DoFaultSegMov();
extern "C" void DoFaultStringOp();
extern "C" void DoFaultPushF();
extern "C" void DoFaultPopF();
extern "C" void DoFaultPush();
extern "C" void DoFaultPop();
extern "C" void DoFaultPushMem();
extern "C" void DoFaultPopMem();
extern "C" void DoFaultEnter();
extern "C" void DoFaultLeave();
extern "C" void DoFaultMaskmovdqu();
extern "C" void DoFaultBitTest();
extern "C" void DoFaultMovSegSelector();
extern "C" void DoFaultJumpMemBadMem();
extern "C" void DoFaultBadLoad();
extern "C" void DoFaultBadStore();
extern "C" void DoFaultCmov();
extern "C" char Unmapped;

int main(int argc, char** argv)
{
    if (!Initialize())
    {
        std::cerr << "Error during initializations\n";
        return 1;
    }

    // You can choose to run just a single test by selecting the test
    // number on the command line.
    //
    bool doOneTest = false;
    unsigned oneTest;
    if (argc > 2)
    {
        std::cerr << "Usage: " << argv[0] << " [test-number]\n";
        return 1;
    }
    if (argc == 2)
    {
        errno   = 0;
        oneTest = std::strtoul(argv[1], 0, 10);
        if (errno)
        {
            std::cerr << "Invalid test number " << argv[1] << "\n";
            return 1;
        }
        doOneTest  = true;
        TestNumber = oneTest;
    }

    // Since each test causes a fault, save the state here.  The fault
    // handler jumps back here after each test.
    //
    sigsetjmp(JumpBuffer, 1);
    if (doOneTest && TestNumber != oneTest) return (IsError) ? 1 : 0;

    // Run all the tests.
    //
    for (;;)
    {
        std::cout << "Starting test " << std::dec << TestNumber << " ..." << std::endl;
        switch (DoTest(TestNumber))
        {
            case TSTATUS_DONE:
                std::cout << "  Last test" << std::endl;
                return (IsError) ? 1 : 0;
            case TSTATUS_NOFAULT:
                std::cout << "  *** Failed to raise signal" << std::endl;
                IsError = true;
                break;
            default:
                assert(0);
                break;
        }

        TestNumber++;
        if (doOneTest) return (IsError) ? 1 : 0;
    }

    assert(0);
    return 1;
}

static bool Initialize()
{
    // Set up a signal handler using an alternate stack.  Some of the
    // tests corrupt $SP, so we can't use the normal stack for the
    // handler.
    //
    stack_t ss;
    ss.ss_sp    = new char[SIGSTKSZ];
    ss.ss_flags = 0;
    ss.ss_size  = SIGSTKSZ;
    if (sigaltstack(&ss, 0) != 0) return false;

    struct sigaction act;
    act.sa_sigaction = Handle;
    act.sa_flags     = SA_SIGINFO | SA_ONSTACK;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGSEGV, &act, 0) != 0) return false;

    // There is a page in the assembly file that should be unmapped.
    //
    void* ptr = &Unmapped;
    if (munmap(ptr, 4096) != 0) return false;

    return true;
}

// This is the fault handler.
//
static void Handle(int sig, siginfo_t*, void* uctxt)
{
    // Make sure we got the expected signal.
    //
    if (sig == SIGSEGV)
    {
        std::cout << "  Received SEGV";
    }
    else
    {
        std::cout << "  *** Received unexpected signal " << std::dec << sig;
        IsError = true;
    }
    std::cout << "\n";

    // Make sure the fault happened at the expected PC.  Make sure the
    // other regiters in the fault context have the expected values.
    //
    if (!CheckUContextRegisters(uctxt)) IsError = true;

    // Advance to the next test.
    //
    TestNumber++;
    siglongjmp(JumpBuffer, 1);
}

// Do a specific test.
//
static TSTATUS DoTest(unsigned num)
{
    switch (num)
    {
        case 0:
            std::cout << "  Fault on RET (bad SP)\n";
            DoFaultRetSp();
            return TSTATUS_NOFAULT;

        case 1:
            std::cout << "  Fault on RET (bad target)\n";
            DoFaultRetTarg();
            return TSTATUS_NOFAULT;

        case 2:
            std::cout << "  Fault on RET <imm> (bad SP)\n";
            DoFaultRetImmSp();
            return TSTATUS_NOFAULT;

        case 3:
            std::cout << "  Fault on RET <imm> (bad targ)\n";
            DoFaultRetImmTarg();
            return TSTATUS_NOFAULT;

        case 4:
            std::cout << "  Fault on CALL <imm> (bad SP)\n";
            DoFaultCallSp();
            return TSTATUS_NOFAULT;

        case 5:
            std::cout << "  Fault on CALL <imm> (bad target)\n";
            DoFaultCallTarg();
            return TSTATUS_NOFAULT;

        case 6:
            std::cout << "  Fault on CALL *%rx (bad SP)\n";
            DoFaultCallRegSp();
            return TSTATUS_NOFAULT;

        case 7:
            std::cout << "  Fault on CALL *%rx (bad target)\n";
            DoFaultCallRegTarg();
            return TSTATUS_NOFAULT;

        case 8:
            std::cout << "  Fault on CALL *[mem] (bad SP)\n";
            DoFaultCallMemSp();
            return TSTATUS_NOFAULT;

        case 9:
            std::cout << "  Fault on CALL *[mem] (bad target)\n";
            DoFaultCallMemTarg();
            return TSTATUS_NOFAULT;

        case 10:
            std::cout << "  Fault on CALL *[mem] (bad mem location)\n";
            DoFaultCallMemBadMem();
            return TSTATUS_NOFAULT;

        case 11:
            std::cout << "  Fault on MOV %gs:[mem], %rx (bad segment selector)\n";
            DoFaultSegMov();
            return TSTATUS_NOFAULT;

        case 12:
            std::cout << "  Fault on MOVS\n";
            DoFaultStringOp();
            return TSTATUS_NOFAULT;

        case 13:
            std::cout << "  Fault on PUSHF\n";
            DoFaultPushF();
            return TSTATUS_NOFAULT;

        case 14:
            std::cout << "  Fault on POPF\n";
            DoFaultPopF();
            return TSTATUS_NOFAULT;

        case 15:
            std::cout << "  Fault on PUSH\n";
            DoFaultPush();
            return TSTATUS_NOFAULT;

        case 16:
            std::cout << "  Fault on POP\n";
            DoFaultPop();
            return TSTATUS_NOFAULT;

        case 17:
            std::cout << "  Fault on PUSH [mem] (bad mem location)\n";
            DoFaultPushMem();
            return TSTATUS_NOFAULT;

        case 18:
            std::cout << "  Fault on POP [mem] (bad mem location)\n";
            DoFaultPopMem();
            return TSTATUS_NOFAULT;

        case 19:
            std::cout << "  Fault on ENTER\n";
            DoFaultEnter();
            return TSTATUS_NOFAULT;

        case 20:
            std::cout << "  Fault on LEAVE\n";
            DoFaultLeave();
            return TSTATUS_NOFAULT;

        case 21:
            std::cout << "  Fault on MASKMOVDQU\n";
            DoFaultMaskmovdqu();
            return TSTATUS_NOFAULT;

        case 22:
            std::cout << "  Fault on BT [mem]\n";
            DoFaultBitTest();
            return TSTATUS_NOFAULT;

        case 23:
            std::cout << "  Fault on MOV %seg, [mem] (bad mem location)\n";
            DoFaultMovSegSelector();
            return TSTATUS_NOFAULT;

        case 24:
            std::cout << "  Fault on JMP *[mem] (bad mem location)\n";
            DoFaultJumpMemBadMem();
            return TSTATUS_NOFAULT;

        case 25:
            std::cout << "  Fault on MOV [mem], %rx (bad load location)\n";
            DoFaultBadLoad();
            return TSTATUS_NOFAULT;

        case 26:
            std::cout << "  Fault on MOV %rx, [mem] (bad store location)\n";
            DoFaultBadStore();
            return TSTATUS_NOFAULT;

        case 27:
            std::cout << "  Fault on CMOV [mem], %rx (bad load location)\n";
            DoFaultCmov();
            return TSTATUS_NOFAULT;

        default:
            return TSTATUS_DONE;
    }
}
