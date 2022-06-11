/*
 * Copyright (C) 2004-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>
#include <fstream>
#include <string>
#include "pin.H"
using std::endl;
using std::ofstream;
using std::string;

/* ===================================================================== */
/* Global Variables and Definitions */
/* ===================================================================== */

// A knob for defining the output file name
KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "line_tool.out",
                              "specify file name for line information output");

// ofstream object for handling the output.
ofstream trace;

#if defined(TARGET_MAC)
#define MAINNAME "_main"
#else
#define MAINNAME "main"
#endif

/* ===================================================================== */

VOID ImageLoad(IMG img, VOID* v)
{
    // Looking for main symbol only in main image

    if (IMG_IsMainExecutable(img))
    {
        for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
        {
            for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
            {
                if (RTN_Name(rtn) == MAINNAME)
                {
                    string filePath;
                    INT32 line;
                    PIN_GetSourceLocation(RTN_Address(rtn), NULL, &line, &filePath);

                    if (filePath != "")
                    {
                        string::size_type index = filePath.find("hello");
                        if (index != string::npos)
                        {
                            string file = filePath.substr(index);
                            trace << "File " << file << " line " << line << endl;
                        }
                    }

                    //Test the case where all output parameters are NULL
                    PIN_GetSourceLocation(RTN_Address(rtn), NULL, NULL, NULL);
                }
            }
        }
    }
}

VOID Fini(INT32 code, VOID*) { trace.close(); }

int main(INT32 argc, CHAR** argv)
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    trace.open(KnobOutputFile.Value().c_str());

    IMG_AddInstrumentFunction(ImageLoad, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
