/*
 * Copyright (C) 2011-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*! @file
 *  Pin Tool for testing the correctness of the new log message format 2.0 
 */

/* ===================================================================== */
/* includes                                                              */
/* ===================================================================== */

#include "pin.H"
#include <iostream>
#include <fstream>
using std::cerr;
using std::endl;
using std::string;

/* ===================================================================== */
/* Commandline Switches                                                  */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "error_log.out", "specify trace file name");

/* ===================================================================== */
/* Finals                                                                */
/* ===================================================================== */

#if defined(TARGET_MAC)
#define MAIN "_main"
#else
#define MAIN "main"
#endif

/* ===================================================================== */
/* Instrumentation callbacks                                             */
/* ===================================================================== */

VOID ImageLoad(IMG img, VOID* v)
{
    /*
     * Pin callback. Registered by IMG_AddInstrumentFunction
     */
    if (IMG_IsMainExecutable(img))
    {
        PIN_WriteErrorMessage("this is a fatal user specified error message", 1001, PIN_ERR_FATAL, 2, "firstArg", "secondArg");
    }
}

/* ===================================================================== */
/* Utilities                                                             */
/* ===================================================================== */

/*!
*  Print out help message.
*/

INT32 Usage()
{
    /* Knobs automate the parsing and management of command line switches. 
     * A command line contains switches for Pin, the tool, and the application. 
     * The knobs parsing code understands how to separate them. 
     */
    cerr << KNOB_BASE::StringKnobSummary() << endl; //   Print out a summary of all the knobs declare

    return -1;
}

/*!
 * The main procedure of the tool.
 * This function is called when the application image is loaded but not yet started.
 * @param[in]   argc            total number of elements in the argv array
 * @param[in]   argv            array of command line arguments, 
 *                              including pin -t <toolname> -- ...
 */

int main(int argc, char* argv[])
{
    PIN_InitSymbols();

    /*
     * Initialize PIN library.
     */
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    IMG_AddInstrumentFunction(ImageLoad, 0);

    /* 
     * Start the program, never returns
     */
    PIN_StartProgram();

    return 0;
}
