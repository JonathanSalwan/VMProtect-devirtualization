/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This is a test application for the extended debugger commands in the
 * "debugger-shell" instrumenation library.  Since those commands are
 * non-symbolic, the input commands must reference raw addresses in this
 * application not symbol names.  It would be difficult to keep the
 * addresses in the input commands in sync with the addresses in this
 * application, so the application itself prints out the debugger commands.
 * To run this test, we run the application twice.  The first run generates
 * the debugger command script, and the second run executes under the debugger.
 */

#include <iostream>
#include <fstream>
#include <cstring>

#if defined(_MSC_VER)
typedef unsigned __int64 ADDR;
#elif defined(__GNUC__)
#include <stdint.h>
typedef uint64_t ADDR;
#endif

#if defined(TARGET_IA32)
#define REGAX "$eax"
#elif defined(TARGET_IA32E)
#define REGAX "$rax"
#endif

volatile unsigned Value = 0;
volatile unsigned Index = 0;
volatile unsigned Max   = 10;

static void GenerateBreakpointScripts(const char*, const char*);
static void GenerateTracepointScripts(const char*, const char*);
static void RunTest();
extern "C" unsigned AssemblyReturn(unsigned);
extern "C" char Label_WriteAx;

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
    if (argc == 4 && strcmp(argv[1], "breakpoints") == 0)
    {
        GenerateBreakpointScripts(argv[2], argv[3]);
        return 0;
    }
    if (argc == 4 && strcmp(argv[1], "tracepoints") == 0)
    {
        GenerateTracepointScripts(argv[2], argv[3]);
        return 0;
    }

    // When run with no arguments, execute the test code.
    //
    RunTest();
    return 0;
}

static void RunTest()
{
    for (unsigned i = 0; i < Max; i++)
    {
        Index = i;
        Value = Index;
    }
    for (unsigned i = 0; i < Max; i++)
    {
        Index = i;
        Value = AssemblyReturn(Index);
    }
}

static void GenerateBreakpointScripts(const char* inFile, const char* compareFile)
{
    std::ofstream in(inFile);
    std::ofstream compare(compareFile);

    in << "monitor break if icount 100\n";
    in << "monitor break if mcount 100\n";
    in << "monitor break if jump to 0x" << std::hex << reinterpret_cast< ADDR >(&RunTest) << "\n";
    in << "monitor break if load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << "\n";
    in << "monitor break if store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << "\n";
    in << "monitor list breakpoints\n";

    compare << "Breakpoint #1:  break thread 0 if icount 100\n";
    compare << "Breakpoint #2:  break thread 0 if mcount 100\n";
    compare << "Breakpoint #3:\\s+break if jump to 0x" << std::hex << reinterpret_cast< ADDR >(&RunTest) << "\n";
    compare << "Breakpoint #4:\\s+break if load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << "\n";
    compare << "Breakpoint #5:\\s+break if store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << "\n";

    in << "cont\n"; /* stop at icount 100 */
    in << "cont\n"; /* stop at mcount 100 */
    in << "cont\n"; /* stop at RunTest */
    in << "cont\n"; /* stop at Index = 0 */
    in << "cont\n"; /* stop at Value = 0 */
    in << "print i\n";
    in << "cont\n"; /* stop at Index = 1 */
    in << "cont\n"; /* stop at Value = 1 */
    in << "print i\n";

    in << "monitor delete breakpoint 4\n"; /* delete "break if load from <Value>" */
    in << "monitor delete breakpoint 5\n"; /* delete "break if store to <Value>" */
    compare << "Triggered breakpoint #1:\n";
    compare << "Triggered breakpoint #2:\n";
    compare << "Triggered breakpoint #3:\n";
    compare << "Triggered breakpoint #4:\n";
    compare << "Triggered breakpoint #5:\n";
    compare << ".*= 0\n";
    compare << "Triggered breakpoint #4:\n";
    compare << "Triggered breakpoint #5:\n";
    compare << ".*= 1\n";

    in << "monitor break before load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << " == 5\n";
    in << "monitor break after store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << " == 5\n";
    in << "cont\n"; /* stop at Index = 5 */
    in << "cont\n"; /* stop at Value = 5 */
    in << "print i\n";
    in << "monitor break at 0x" << std::hex << reinterpret_cast< ADDR >(&Label_WriteAx) << " if " REGAX " == 2\n";
    in << "cont\n"; /* stop in AssemblyReturn(2) */
    in << "print " REGAX "\n";
    in << "cont\n"; /* stop at Index = 5 */
    in << "cont\n"; /* stop at Value = 5 */
    in << "cont\n"; /* program terminates */
    in << "quit\n";

    compare << "Triggered breakpoint #6:\n";
    compare << "Triggered breakpoint #7:\n";
    compare << ".*= 5\n";
    compare << "Triggered breakpoint #8:\n";
    compare << ".*= 2\n";
    compare << "Triggered breakpoint #6:\n";
    compare << "Triggered breakpoint #7:\n";
    compare << "(Program exited normally|\\[Inferior 1 \\(Remote target\\) exited normally\\])\n";
}

static void GenerateTracepointScripts(const char* inFile, const char* compareFile)
{
    std::ofstream in(inFile);
    std::ofstream compare(compareFile);

    in << "monitor trace at 0x" << std::hex << reinterpret_cast< ADDR >(&main) << "\n";
    in << "break RunTest\n";
    in << "cont\n"; /* stop at RunTest */
    in << "monitor trace print\n";

    compare << "Tracepoint #1:\\s+trace at 0x" << std::hex << reinterpret_cast< ADDR >(&main) << "\n";
    compare << "Breakpoint 1,\\s*(.*::)*RunTest\n";
    compare << "0x0*" << std::hex << reinterpret_cast< ADDR >(&main) << "\n";

    in << "monitor trace clear\n";
    in << "monitor trace if store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << "\n";
    in << "monitor break if store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << "\n";
    in << "monitor trace if load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << "\n";
    in << "monitor break if load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << "\n";
    in << "cont\n"; /* stop at Index = 0 (before trace occured) */
    in << "cont\n"; /* stop at Value = 0 (before trace occured) */
    in << "monitor trace print\n";

    compare << "Tracepoint #2:\\s*trace if store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << "\n";
    compare << "Breakpoint #3:\\s*break if store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << "\n";
    compare << "Tracepoint #4:\\s*trace if load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << "\n";
    compare << "Breakpoint #5:\\s*break if load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << "\n";
    compare << "Triggered breakpoint #5:\n";
    compare << "Triggered breakpoint #3:\n";
    /* no trace records printed */

    in << "monitor delete breakpoint 3\n";
    in << "monitor delete breakpoint 5\n";

    in << "monitor break before load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << " == 0x" << std::dec << Max - 1
       << "\n";
    in << "monitor break after store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << " == 0x" << std::dec << Max - 1
       << "\n";
    in << "cont\n"; /* stop after Index = Max (end of first loop) */
    in << "cont\n"; /* stop after Value = Max (end of first loop) */
    in << "monitor trace print\n";

    compare << "Breakpoint #6:\n";
    compare << "Breakpoint #7:\n";
    compare << "Triggered breakpoint #6:\n";
    compare << "Triggered breakpoint #7:\n";
    compare << "0x[0-9,a-f]+:\\s*if load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << "\n";
    compare << "0x[0-9,a-f]+:\\s*if store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << "\n";
    compare << "0x[0-9,a-f]+:\\s*if load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << "\n";
    compare << "0x[0-9,a-f]+:\\s*if store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << "\n";
    compare << "0x[0-9,a-f]+:\\s*if load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << "\n";
    compare << "0x[0-9,a-f]+:\\s*if store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << "\n";
    compare << "0x[0-9,a-f]+:\\s*if load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << "\n";
    compare << "0x[0-9,a-f]+:\\s*if store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << "\n";
    compare << "0x[0-9,a-f]+:\\s*if load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << "\n";
    compare << "0x[0-9,a-f]+:\\s*if store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << "\n";
    compare << "0x[0-9,a-f]+:\\s*if load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << "\n";
    compare << "0x[0-9,a-f]+:\\s*if store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << "\n";
    compare << "0x[0-9,a-f]+:\\s*if load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << "\n";
    compare << "0x[0-9,a-f]+:\\s*if store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << "\n";
    compare << "0x[0-9,a-f]+:\\s*if load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << "\n";
    compare << "0x[0-9,a-f]+:\\s*if store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << "\n";
    compare << "0x[0-9,a-f]+:\\s*if load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << "\n";
    compare << "0x[0-9,a-f]+:\\s*if store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << "\n";
    compare << "0x[0-9,a-f]+:\\s*if load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << "\n";
    compare << "0x[0-9,a-f]+:\\s*if store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << "\n";

    in << "monitor trace disable\n";
    in << "monitor list tracepoints\n";
    in << "monitor trace clear\n";

    compare << "#1:\\s*trace at 0x" << std::hex << reinterpret_cast< ADDR >(&main) << "\\s*\\(disabled\\)\n";
    compare << "#2:\\s*trace if store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << "\\s*\\(disabled\\)\n";
    compare << "#4:\\s*trace if load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << "\\s*\\(disabled\\)\n";

    in << "monitor trace before load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << " == 2\n";
    in << "monitor trace before load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << " == 4\n";
    in << "monitor trace after store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << " == 2\n";
    in << "monitor trace after store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << " == 4\n";
    in << "cont\n"; /* stop after Index = Max (end of second loop) */
    in << "cont\n"; /* stop after Value = Max (end of second loop) */
    in << "monitor trace print\n";

    compare << "Tracepoint #8:\\s*trace before load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << " == 0x2\n";
    compare << "Tracepoint #9:\\s*trace before load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << " == 0x4\n";
    compare << "Tracepoint #10:\\s*trace after store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << " == 0x2\n";
    compare << "Tracepoint #11:\\s*trace after store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << " == 0x4\n";
    compare << "Triggered breakpoint #6:\n";
    compare << "Triggered breakpoint #7:\n";
    compare << "0x[0-9,a-f]+:\\s*before load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << " == 0x2\n";
    compare << "0x[0-9,a-f]+:\\s*after store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << " == 0x2\n";
    compare << "0x[0-9,a-f]+:\\s*before load from 0x" << std::hex << reinterpret_cast< ADDR >(&Index) << " == 0x4\n";
    compare << "0x[0-9,a-f]+:\\s*after store to 0x" << std::hex << reinterpret_cast< ADDR >(&Value) << " == 0x4\n";

    in << "quit\n";
}
