/*
 * Copyright (C) 2018-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This tool verifies that in case of linking an image with a position independent executable,
 * the address of the first instruction executed within the image (which returned by IMG_EntryAddress API)
 * is absolute and correct.
 */

#include "pin.H"
#include <iostream>
#include <fstream>

#ifdef TARGET_MAC
#define ENTRY_RTN_NAME "_main"
#endif

#ifdef TARGET_LINUX
#define ENTRY_RTN_NAME "_start"
#endif

#ifdef TARGET_WINDOWS
#define ENTRY_RTN_NAME "unnamedImageEntryPoint"
#endif

using std::endl;
using std::ofstream;
using std::string;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "pic_image.out", "specify file name");

ofstream TraceFile;

/* ===================================================================== */
// Utilities
/* ===================================================================== */

/*!
 *  Print out help message.
 */
INT32 Usage()
{
    std::cerr << "This tool retrieves the first instruction of loaded image" << endl
              << "by using IMG_EntryAddress API. Address is absolute, regardless of image type (e.g. PIE)" << endl;

    std::cerr << KNOB_BASE::StringKnobSummary() << endl;

    return -1;
}

/* ===================================================================== */
// Analysis routines
/* ===================================================================== */

VOID ImageLoad(IMG img, VOID* arg)
{
    if (!IMG_IsMainExecutable(img)) return;

    ADDRINT entry = IMG_EntryAddress(img);
    TraceFile << "Application Entry Address is at : " << std::hex << entry << std::endl;

    ADDRINT start_address = RTN_Address(RTN_FindByName(img, ENTRY_RTN_NAME));

    TraceFile << ENTRY_RTN_NAME << " Address is : " << std::hex << start_address << std::endl << std::endl;

    TraceFile << ENTRY_RTN_NAME << " Address and Application Entry Address supposed to be identical!" << std::endl;

    assert(entry == start_address);
    return;
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
    // Initialize PIN library. Print help message if -h(elp) is specified
    // in the command line or the command line is invalid
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }
    PIN_InitSymbols();

    TraceFile.open(KnobOutputFile.Value().c_str());

    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
