/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

// This tool instruments the modifyFlags() function in the application
// then get and get the flag register.
// The tool is used to verify the correctness of passing the flag
// register by reference to an analysis routine.

#include <iostream>
#include <fstream>
#include <pin.H>
using std::cerr;
using std::endl;
using std::string;

#ifdef TARGET_MAC
#define GLOBALFUN_NAME(name) "_" name
#else
#define GLOBALFUN_NAME(name) name
#endif

using std::ofstream;

/////////////////////
// GLOBAL VARIABLES
/////////////////////

// A knob for defining the output file name
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "reg_reference_flags.out",
                              "specify file name for reg_reference_flags output");

// A knob for defining which register reference to use. One of:
// 1. default   - regular REG_REFERENCE passed to the analysis routine using IARG_REG_REFERENCE.
// 2. const     - const REG_REFERENCE passed to the analysis routine using IARG_REG_CONST_REFERENCE.
KNOB< string > KnobTestReference(KNOB_MODE_WRITEONCE, "pintool", "testreference", "default",
                                 "specify which context to test. One of default|const.");

// ofstream object for handling the output.
ofstream OutFile;

/////////////////////
// UTILITY FUNCTIONS
/////////////////////

static int Usage()
{
    cerr << "This tool verifies the correctness of passing the flags register by reference to an analysis routine." << endl
         << endl
         << KNOB_BASE::StringKnobSummary() << endl;
    return 1;
}

/////////////////////
// ANALYSIS FUNCTIONS
/////////////////////

static void ChangeRegBefore(ADDRINT* val)
{
    OutFile << "Flags: " << std::hex << *val << std::endl;
    if (KnobTestReference.Value() == "default")
    {
        *val = 0xcd0;
    }
}

/////////////////////
// CALLBACKS
/////////////////////

static VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        RTN modifyFlagsRtn = RTN_FindByName(img, GLOBALFUN_NAME("modifyFlags"));
        ASSERTX(RTN_Valid(modifyFlagsRtn));
        RTN_Open(modifyFlagsRtn);
        INS ins = RTN_InsHeadOnly(modifyFlagsRtn);
        if (KnobTestReference.Value() == "default")
        {
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(ChangeRegBefore), IARG_REG_REFERENCE, REG_AppFlags(), IARG_END);
        }
        else if (KnobTestReference.Value() == "const")
        {
            INS_InsertCall(ins, IPOINT_BEFORE, AFUNPTR(ChangeRegBefore), IARG_REG_CONST_REFERENCE, REG_AppFlags(), IARG_END);
        }
        else
        {
            OutFile << "ERROR: Unknown reference requested for testing: " << KnobTestReference.Value() << endl;
            PIN_ExitApplication(2); // never returns
        }
        RTN_Close(modifyFlagsRtn);
    }
}

static VOID Fini(INT32 code, VOID* v) { OutFile.close(); }

/////////////////////
// MAIN FUNCTION
/////////////////////

int main(int argc, char* argv[])
{
    // Initialize pin
    PIN_InitSymbolsAlt(EXPORT_SYMBOLS);
    if (PIN_Init(argc, argv)) return Usage();

    OutFile.open(KnobOutputFile.Value().c_str());

    // Register Instruction to be called to instrument instructions
    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
