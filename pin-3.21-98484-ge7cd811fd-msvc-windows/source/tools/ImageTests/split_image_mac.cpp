/*
 * Copyright (C) 2014-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/**
 * This tool writes the region/s of every image loaded during the lifetime
 * of the application.
 * This information is then compared with the actual load addresses of the 
 * segments as the application views them (split_image_app).
 * The comparison is done using the python script region_compare.py.
 */

#include <cstdio>
#include <cstdlib>
#include "pin.H"

using std::fprintf;
using std::string;

FILE* fp = 0;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "split_image.out", "specify output file name");

// check if each image area is really covered by its region
VOID ImageLoad(IMG img, VOID* v)
{
    for (int i = 0; i < IMG_NumRegions(img); i++)
    {
        ADDRINT high = IMG_RegionHighAddress(img, i);
        ADDRINT low  = IMG_RegionLowAddress(img, i);
        fprintf(fp, "%s, %p-%p\n", IMG_Name(img).c_str(), Addrint2VoidStar(low), Addrint2VoidStar(high));
    }
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v) { fclose(fp); }

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);

    fp = fopen(KnobOutputFile.Value().c_str(), "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Couldn't open %s for output\n", KnobOutputFile.Value().c_str());
        exit(1);
    }

    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
