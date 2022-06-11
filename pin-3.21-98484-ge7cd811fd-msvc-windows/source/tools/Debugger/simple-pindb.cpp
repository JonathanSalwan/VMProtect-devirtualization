/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This application is a general test of features in  the 'pindb' debugger.
 * Since the debugger is not symbolic, the input commands must reference raw
 * addresses in this application not symbol names.  Therefore, it would be
 * difficult to keep the addresses in the input commands in sync with
 * addresses in this application.  To solve this problem, the application
 * itself can print out the debugger commands.  Therefore, to run the test,
 * you run this application twice.  The first run generates the debugger
 * command script and the second run executes under the debugger.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "tool_macros.h"

#if defined(_MSC_VER)
typedef unsigned __int64 ADDR;
#elif defined(__GNUC__)
#include <stdint.h>
typedef uint64_t ADDR;
#endif

static void GenerateBasicScripts(const char*, const char*);
static void GenerateDetachScripts(const char*, const char*);
static void GenerateStepCustomBreakScripts(const char*, const char*);
extern "C" void Breakpoint() ASMNAME("Breakpoint");
extern "C" void Breakpoint2() ASMNAME("Breakpoint2");
extern "C" void DoRegMemTest() ASMNAME("DoRegMemTest");
extern "C" void DoStepCustomBreakTest() ASMNAME("DoStepCustomBreakTest");
extern "C" long MemTestData[] ASMNAME("MemTestData");

int main(int argc, char** argv)
{
    if (argc != 1 && argc != 4)
    {
        std::cerr << "Must specify three arguments or none\n";
        return 1;
    }

    // When arguments are specified, just generate the
    // debugger scripts.
    //
    if (argc == 4)
    {
        if (std::strcmp(argv[1], "basic") == 0)
        {
            GenerateBasicScripts(argv[2], argv[3]);
        }
        else if (std::strcmp(argv[1], "detach") == 0)
        {
            GenerateDetachScripts(argv[2], argv[3]);
        }
        else if (std::strcmp(argv[1], "step-custom-break") == 0)
        {
            GenerateStepCustomBreakScripts(argv[2], argv[3]);
        }
        else
        {
            std::cerr << "Unknown script name '" << argv[1] << "'\n";
            return 1;
        }
        return 0;
    }

    // When run with no arguments, execute the test code.
    //
    Breakpoint();
    Breakpoint();
    Breakpoint();
    DoRegMemTest();
    DoStepCustomBreakTest();
    std::cout << "Test completed" << std::endl;
    return 0;
}

/*!
 * Generate the PinDB scripts for the "basic" test.
 *
 *  @param[in] inFile       Name of the generated PinDB command input file.
 *  @param[in] compareFile  Name of the generated PinDB expected output comparison file.
 */
static void GenerateBasicScripts(const char* inFile, const char* compareFile)
{
    std::ofstream in(inFile);
    std::ofstream compare(compareFile);

    ADDR bpAddr1     = reinterpret_cast< ADDR >(&Breakpoint);
    ADDR bpAddr2     = reinterpret_cast< ADDR >(&Breakpoint2);
    ADDR memTestAddr = reinterpret_cast< ADDR >(&DoRegMemTest);
    ADDR memDataAddr = reinterpret_cast< ADDR >(&MemTestData[0]);

    in << std::hex << std::noshowbase;
    compare << std::hex << std::noshowbase;

    compare << "Thread .* stopped at\n";

    // Test breakpoints
    //
    in << "b 0x" << bpAddr1 << "\n";
    in << "b 0x" << bpAddr2 << "\n";
    in << "c\n";
    compare << "Thread .* stopped at 0x" << bpAddr1 << " \\(at breakpoint\\)\n";
    in << "c\n";
    compare << "Thread .* stopped at 0x" << bpAddr2 << " \\(at breakpoint\\)\n";
    in << "c\n";
    compare << "Thread .* stopped at 0x" << bpAddr1 << " \\(at breakpoint\\)\n";
    in << "s\n";
    compare << "Thread .* stopped at 0x" << bpAddr2 << "\n";

    // Test disassembly
    //
    in << "dis 2 0x" << bpAddr1 << "\n";
    compare << "0x0*" << bpAddr1 << ".*nop\n";
    compare << "0x0*" << bpAddr2 << ".*ret\n";

    // Test clearing breakpoints.  Set up for register & memory tests.
    //
    in << "del 0x" << bpAddr1 << "\n";
    in << "del 0x" << bpAddr2 << "\n";
    in << "b 0x" << memTestAddr << "\n";

    // Test examine memory.
    //
    in << "x/2w 0x" << memDataAddr << "\n";
    compare << "0x0*" << memDataAddr << "  12345678 deadbeef\n";

    in << "c\n";
    compare << "Thread .* stopped at 0x" << memTestAddr << " \\(at breakpoint\\)\n";
    in << "s\n";
    in << "s\n";
#if defined(TARGET_IA32E)
    std::string reg = "rcx";
#elif defined(TARGET_IA32)
    std::string reg = "ecx";
#else
#error "Need TARGET_IA32 or TARGET_IA32E defined"
#endif
    in << "p $" << reg << "\n";
    compare << "0x0*12345678\n";
    in << "set $" << reg << "=0x123fff\n";

    in << "s\n";

    // Examine the modified memory (verifies that we really did change %cx).
    //
    in << "x/2w 0x" << memDataAddr << "\n";
    compare << "0x0*" << memDataAddr << "  00123fff deadbeef\n";

    // Test that we can modify memory.
    //
    in << "m/1w 0x" << memDataAddr << " 0xccc456\n";
    in << "x/2w 0x" << memDataAddr << "\n";
    compare << "0x0*" << memDataAddr << "  00ccc456 deadbeef\n";

    // Test that we can send pintool-commands and receive a response.
    //
    in << "monitor stacktrace on\n";
    compare << "Stack tracing enabled.\n";
    in << "monitor stacktrace off\n";
    compare << "Stack tracing disabled.\n";

    in << "c\n";
    compare << "Program exited with 0\n";
    in << "q\n";
}

/*!
 * Generate the PinDB scripts for the "detach" test.
 *
 *  @param[in] inFile       Name of the generated PinDB command input file.
 *  @param[in] compareFile  Name of the generated PinDB expected output comparison file.
 */
static void GenerateDetachScripts(const char* inFile, const char* compareFile)
{
    std::ofstream in(inFile);
    std::ofstream compare(compareFile);

    ADDR bpAddr1 = reinterpret_cast< ADDR >(&Breakpoint);
    ADDR bpAddr2 = reinterpret_cast< ADDR >(&Breakpoint2);

    in << std::hex << std::noshowbase;
    compare << std::hex << std::noshowbase;

    compare << "Thread .* stopped at\n";

    // Set some breakpoints.  We want to make sure they are not triggered after the detach.
    //
    in << "b 0x" << bpAddr1 << "\n";
    in << "b 0x" << bpAddr2 << "\n";

    // Do the detach.  The application should run to completion.
    //
    in << "detach-and-wait\n";
    compare << "Test completed\n";
    in << "q\n";
}

/*!
 * Generate the PinDB scripts for the "step-custom-break" test.
 *
 *  @param[in] inFile       Name of the generated PinDB command input file.
 *  @param[in] compareFile  Name of the generated PinDB expected output comparison file.
 */
static void GenerateStepCustomBreakScripts(const char* inFile, const char* compareFile)
{
    std::ofstream in(inFile);
    std::ofstream compare(compareFile);

    ADDR bpAddr    = reinterpret_cast< ADDR >(&DoStepCustomBreakTest);
    ADDR watchAddr = reinterpret_cast< ADDR >(&Breakpoint);

    in << std::hex << std::noshowbase;
    compare << std::hex << std::noshowbase;

    in << "b 0x" << bpAddr << "\n";
    in << "c\n";
    in << "monitor break if jump to 0x" << watchAddr << "\n";
    in << "s\n"; // step over NOP
    in << "s\n"; // try to step over CALL, but trigger "monitor break"
    in << "s\n"; // step into CALL
    in << "k\n";
    in << "q\n";

    compare << "Thread .* stopped at .* \\(after step\\)\n";         // step over NOP
    compare << "Thread .* stopped at .* \\(at tool breakpoint\\)\n"; // attempt to step over CALL
    compare << "Triggered breakpoint #1: break if jump to 0x" << watchAddr << "\n";
    compare << "Thread .* stopped at 0x" << watchAddr << " \\(after step\\)\n"; // step into CALL
}
