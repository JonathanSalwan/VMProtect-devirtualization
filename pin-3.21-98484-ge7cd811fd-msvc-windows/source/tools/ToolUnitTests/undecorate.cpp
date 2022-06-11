/*
 * Copyright (C) 2009-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * Check interface PIN_UndecorateSymbolName()
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <iostream>

#include "pin.H"
using std::cout;
using std::endl;
using std::ofstream;
using std::string;

KNOB< string > KnobOutput(KNOB_MODE_WRITEONCE, "pintool", "o", "undecorate.out", "Name for log file");
KNOB< BOOL > KnobVerbose(KNOB_MODE_WRITEONCE, "pintool", "v", "0", "Verbose output to log file");

static ofstream out;

// Define the names we expect to demangle
static struct
{
    const string fullName;
    const string noArgsName;
} testNames[] = {{"A MyMethod<A>(A&)", "MyMethod<A>"},
                 {"A::MyMethod1(std::vector<int, std::allocator<int> > const&)", "A::MyMethod1"},
                 {"A::MyMethod2(std::string)", "A::MyMethod2"},
                 {"MyMethod1<A>::MyMethod1()", "MyMethod1<A>::MyMethod1"},
                 {"A::MyMethod3(std::string) const", "A::MyMethod3"},
                 {"Foo()", "Foo"},
                 {"int my_operator<int>(int const&)", "my_operator<int>"}};

VOID ImageLoad(IMG img, VOID* v)
{
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            string rtnName = RTN_Name(rtn);
            if (rtnName.find("MyMethod") != string::npos || rtnName.find("Foo") != string::npos ||
                rtnName.find("my_operator") != string::npos)
            {
                // The tested application has class "A" with interface "MyMethod"
                // We just check that A::MyMethod is successfully demangled
                string demangledName         = PIN_UndecorateSymbolName(rtnName, UNDECORATION_COMPLETE);
                string demangledNameNoParams = PIN_UndecorateSymbolName(rtnName, UNDECORATION_NAME_ONLY);

                out << "Mangled name: " << rtnName << endl;
                out << "Full demangled name: " << demangledName << endl;
                out << "Demangled name w/o parameters: " << demangledNameNoParams << endl;

                BOOL matched = FALSE;
                for (UINT32 i = 0; i < sizeof(testNames) / sizeof(testNames[0]); i++)
                {
                    if (demangledName == testNames[i].fullName && demangledNameNoParams == testNames[i].noArgsName)
                    {
                        matched = TRUE;
                        break;
                    }
                }

                if (!matched)
                {
                    cout << "Error in demangling: " << endl;
                    cout << "Mangled name: " << rtnName << endl;
                    cout << "Demangled name: " << demangledName << endl;
                    cout << "Demangled name, no parameters: " << demangledNameNoParams << endl;
                    exit(-1);
                }
                continue;
            }
            if (rtnName.find("@@") != string::npos)
            {
                // Check C and C++ names demangling with version (Linux)
                // Like
                //     _ZNSt11char_traitsIcE2eqERKcS2_@@GLIBCXX_3.4.5
                //     localeconv@@GLIBC_2.2
                string name = PIN_UndecorateSymbolName(rtnName, UNDECORATION_COMPLETE);

                if (rtnName.find("_Z") == 0)
                {
                    out << "C++ name with version: " << rtnName << endl;
                    out << "Demangled name: " << name << endl;
                }
                else
                {
                    out << "C name with version: " << rtnName << endl;
                    out << "Demangled name: " << name << endl;
                }
                if (name.find("@@") != string::npos)
                {
                    printf("Error: undecorated name should not include \"@@\"\n");
                    exit(-1);
                }
                continue;
            }

            // Otherwise just demangle the name both ways but normally don't bother to print them.
            // We can't easily tell what the results should be, but throwing a lot more
            // names at our demangler must be a good thng to do, and this should include
            // all the names from the standard C++ runtime, is a reasonable stress test.
            string complete = PIN_UndecorateSymbolName(rtnName, UNDECORATION_COMPLETE);
            string nameOnly = PIN_UndecorateSymbolName(rtnName, UNDECORATION_NAME_ONLY);

            if (KnobVerbose)
            {
                out << rtnName << " => " << endl;
                out << "   " << complete << endl;
                out << "   " << nameOnly << endl;
            }
        }
    }
}

int main(INT32 argc, CHAR** argv)
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    out.open(KnobOutput.Value().c_str());

    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
