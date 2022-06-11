/*
 * Copyright (C) 2019-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iomanip>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include "pin.H"

using namespace std;

KNOB< string > KnobInputFile(KNOB_MODE_WRITEONCE, "pintool", "i", "<imagename>", "specify an image to read");

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "img_open_debug_info_win.out", "specify output file name");

// TODO - Remove the knob and perform unconditional check once IMG_Close() is properly implemented on Windows (#mantis 4742)
KNOB< BOOL > KnobCheckImageRemoved(
    KNOB_MODE_WRITEONCE, "pintool", "r", "0",
    "Check image load callback is not called after startProgramCalled() for little_malloc image since it was closed");

ofstream OutFile;

BOOL debugInfoFound     = FALSE;
BOOL startProgramCalled = FALSE;

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

// This is a utility function for acquiring and printing the source information.
static void output(ADDRINT address, INS ins)
{
    string filename; // This will hold the source file name.
    INT32 line = 0;  // This will hold the line number within the file.

    // In this example, we don't print the column number so there is no reason to obtain it.
    // Simply pass a NULL pointer instead. Also, acquiring the client lock is not required in
    // instrumentation functions, only in analysis functions.
    //
    PIN_GetSourceLocation(address, NULL, &line, &filename);
    // Prepare the output strings.
    string asmOrFuncName;

    assert(INS_Valid(ins));
    asmOrFuncName = INS_Disassemble(ins);

    // For output cleanliness, print only if source was found.
    if (!filename.empty())
    {
        OutFile << "0x" << address << " " << asmOrFuncName << " # " << filename << ":" << line << endl;
        if (!debugInfoFound) debugInfoFound = TRUE;
    }
    else
    {
        OutFile << "0x" << address << " " << asmOrFuncName << " # "
                << "No debug info" << endl;
    }
}
INT32 Usage()
{
    cerr << "This tool disassembles an image." << endl << endl;
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

VOID OpenImage()
{
    IMG img = IMG_Open(KnobInputFile);

    if (!IMG_Valid(img))
    {
        OutFile << "Could not open " << KnobInputFile.Value() << endl;
        exit(1);
    }

    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        // For each section, process all RTNs.
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            // Many RTN APIs require that the RTN be opened first.
            RTN_Open(rtn);

            OutFile << "RTN address: 0x" << RTN_Address(rtn) << ", RTN name: " << RTN_Name(rtn) << endl;

            if (RTN_InsHead(rtn) == INS_Invalid())
            {
                OutFile << "No instructions in routine" << endl;
            }

            // Call PIN_GetSourceLocation for all the instructions of the RTN.
            for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
            {
                output(INS_Address(ins), ins); // Calls PIN_GetSourceLocation for a single instruction.
            }
            OutFile << endl;
            RTN_Close(rtn); // Don't forget to close the RTN once you're done.
        }
    }
    IMG_Close(img);
}

VOID ImageLoad(IMG img, VOID* v)
{
    // We should not get image load callback on little malloc image which we open statically from the tool main() function
    // or any other image load call before PIN_StartProgram() is called
    assert(startProgramCalled);

    OutFile << "Loading " << IMG_Name(img) << ", Image id = " << IMG_Id(img) << endl;
    if (KnobCheckImageRemoved)
    {
        ASSERT(IMG_Name(img).find("little_malloc") == string::npos,
               "little_malloc was closed during tool's main() but tool just got image load callback for it ");
    }
}

int main(INT32 argc, CHAR** argv)
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    OutFile.open(KnobOutputFile.Value().c_str());

    // Register ImageLoad to be called when an image is loaded
    IMG_AddInstrumentFunction(ImageLoad, 0);

    OpenImage();

    ASSERT(debugInfoFound, "PIN_GetSourceLocation() didn't find debug information for little_malloc image");

    startProgramCalled = TRUE;

    // Start the program, never returns
    PIN_StartProgram();
}
