/*
 * Copyright (C) 2020-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include "pin.H"
#include <iostream>
#include <fstream>

// Print out help message.
INT32 Usage()
{
    std::cerr << "This tool searches a specific variable in argv for dumpargv.test" << std::endl;
    std::cerr << KNOB_BASE::StringKnobSummary() << std::endl;
    return -1;
}

// Tool for testing proper parsing of argv
int main(int argc, char* argv[])
{
    // Initialize symbol processing
    PIN_InitSymbols();

    // Initialize PIN library. Print help message if -h(elp) is specified
    // in the command line or the command line is invalid
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    printf("argc = %u\n", argc);
    for (int i = 0; i < argc && argv[i]; i++)
    {
        printf("argv[%u] = '%s'\n", i, argv[i]);
        fflush(stdout);
        if (!strcmp(argv[i], "\\\\isamba\\nfs\\site\\ptl\\bistro\\log"))
        {
            PIN_ExitProcess(0);
        }
    }
    PIN_ExitProcess(1);
}
