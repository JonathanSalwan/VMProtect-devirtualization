/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
Pin Tool for testing the correctness of INS_MemoryDisplacement(INS ins).
INS_MemoryDisplacement computes the memory displacement, which is a sign number. 
Tested only on 64 Linux architecture.
When running the tool with the application: "memory_displacment_app", the output, which is the displacement value, should be -24.
When running the application natively( without the tool), the output, which is the displacement value, should be 4.
*/

#include "pin.H"
#include <string.h>
#include <iostream>
#include <fstream>
using std::cerr;
using std::cout;
using std::endl;
using std::ios;
using std::ofstream;
using std::string;

/* ===================================================================== */
//  Commandline Switches
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "w_malloctrace.out", "specify trace file name");

/* ===================================================================== */
// Finals
/* ===================================================================== */

#if defined(TARGET_MAC)
#define NEG_DISP "_neg_disp"
#else
#define NEG_DISP "neg_disp"
#endif

/* ================================================================== */
// Global variables
/* ================================================================== */

ADDRDELTA disp; // hold memory displacement value of the first instruction of the instrumented function.

/* ===================================================================== */
// Instrumentation callbacks
/* ===================================================================== */

VOID ImageLoad(IMG img, VOID* v)
{   // Pin callback. Registered by IMG_AddInstrumentFunction
    // Instrument the neg_disp () function which resides in memory_displacement_app.s

    // Find the negDispRtn() function.
    RTN negDispRtn = RTN_FindByName(img, NEG_DISP);
    if (RTN_Valid(negDispRtn))
    {
        RTN_Open(negDispRtn);
        INS ins;
        ins = RTN_InsHead(negDispRtn); // First instruction of negDispRtn
        if (INS_Valid(ins))
        {
            disp = INS_MemoryDisplacement(ins);
        }
        RTN_Close(negDispRtn);
    }
}

/*
 * Print out the displacement of the first instruction of the function: "neg_disp()".
 * When running this tool with the application:"memory_displacement_app", should print -24
 * This function is called when the application exits.
 * @param[in]   code            exit code of the application
 * @param[in]   v               value specified by the tool in the 
 *                              PIN_AddFiniFunction function cal
 */

VOID Fini(INT32 code, VOID* v)
{
    // Write to a file
    ofstream OutFile;
    OutFile.open(KnobOutputFile.Value().c_str());
    OutFile.setf(ios::showbase);
    cout << disp << endl;
    OutFile.close();
}

/* ===================================================================== */
// Utilities
/* ===================================================================== */

/*
 *  Print out help message.
 */

INT32 Usage()
{
    cerr << "This tool check the correctness of INS_MemoryDisplacement(INS ins) function, which return the displacement as a "
            "sign number"
         << endl;

    //Knobs automate the parsing and management of command line switches. A command line contains switches for Pin, the tool, and the application. The knobs parsing code understands how to separate them.
    cerr << KNOB_BASE::StringKnobSummary() << endl; //   Print out a summary of all the knobs declare

    return -1;
}

/*
 * The main procedure of the tool.
 * This function is called when the application image is loaded but not yet started.
 * @param[in]   argc            total number of elements in the argv array
 * @param[in]   argv            array of command line arguments, 
 *                              including pin -t <toolname> -- ...
 */

int main(int argc, char* argv[])
{
    PIN_InitSymbols();

    // Initialize PIN library.
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }
    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Register function to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();
    return 0;
}
