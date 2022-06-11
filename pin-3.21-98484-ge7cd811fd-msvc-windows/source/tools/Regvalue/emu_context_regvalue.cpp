/*
 * Copyright (C) 2013-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <fstream>
#include <iostream>
#include "context_utils.h"
using std::endl;
using std::flush;

using std::ofstream;

/////////////////////
// GLOBAL VARIABLES
/////////////////////

// A knob for defining the output file name
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "emu_context_regvalue.out", "specify output file name");

// ofstream object for handling the output
ofstream OutFile;

/////////////////////
// UTILITY FUNCTIONS
/////////////////////

bool CheckAllModifiedValues(ostream& ost)
{
    bool success       = true;
    vector< REG > regs = GetTestRegs();
    int numOfRegs      = regs.size();
    for (int r = 0; r < numOfRegs; ++r)
    {
        REG reg = regs[r];
        success &= CompareValues(GetRegval(reg), GetToolRegisterValue(reg), REG_Size(reg), ost);
    }
    return success;
}

/////////////////////
// CALLBACKS
/////////////////////

static VOID OnSyscall(THREADID tid, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    // We only need to test this once
    static bool done = false;
    if (done) return;
    done = true;

    // Save the original context
    CONTEXT copy;
    PIN_SaveContext(ctxt, &copy);

    OutFile << "Begin tests before entering system call" << endl;

    // Test the "get" mechanism of EMU_CONTEXT
    StoreContext(ctxt);
    PrintStoredRegisters(OutFile);

    // Test the "set" mechanism of EMU_CONTEXT
    ModifyContext(ctxt);
    OutFile << endl << "After modifications" << endl;
    StoreContext(ctxt);
    PrintStoredRegisters(OutFile);
    if (!CheckAllModifiedValues(OutFile))
    {
        OutFile << "ERROR: values mismatch" << endl << flush;
        PIN_ExitApplication(1); // never returns
    }

    // Restore the original context
    PIN_SaveContext(&copy, ctxt);
    OutFile << endl << "After restoring" << endl;
    StoreContext(ctxt);
    PrintStoredRegisters(OutFile);

    // Test complete
    OutFile << endl << "Done" << endl;
}

static VOID Fini(INT32 code, VOID* v) { OutFile.close(); }

/////////////////////
// MAIN FUNCTION
/////////////////////

int main(int argc, char* argv[])
{
    // Initialize Pin
    PIN_Init(argc, argv);

    // Open the output file
    OutFile.open(KnobOutputFile.Value().c_str());

    // Add instrumentation
    PIN_AddSyscallEntryFunction(OnSyscall, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Start running the application
    PIN_StartProgram(); // never returns

    return 0;
}
