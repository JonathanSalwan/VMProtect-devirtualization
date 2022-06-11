/*
 * Copyright (C) 2015-2021 Intel Corporation.
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
#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <vector>
#include "pin.H"
using std::ifstream;
using std::istringstream;
using std::pair;
using std::string;
using std::vector;

FILE* fp = 0;

KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "split_image.out", "specify output file name");

KNOB< bool > KnobVerbose(KNOB_MODE_WRITEONCE, "pintool", "verbose", "0", "verbose output");

vector< pair< string, ADDRESS_RANGE > > allImages;
vector< pair< string, ADDRESS_RANGE > > allRegions;

/*
 * The application that this tool is instrumenting as part of the test is linked so there
 * is a big hole in its address space between the code region and the data region.
 * This function tries to locate the starting address of this hole
 */
void* locate_app_image_hole(void* address_in_app)
{
    ifstream infile("/proc/self/maps");
    string line;
    void* app_end = NULL;
    while (std::getline(infile, line))
    {
        istringstream iss(line);
        string range;
        if (iss >> range)
        {
            size_t pos = range.find("-");
            if (pos != string::npos)
            {
                void *start, *end;
                start = (void*)strtoll(range.substr(0, pos).c_str(), NULL, 16);
                end   = (void*)strtoll(range.substr(pos + 1).c_str(), NULL, 16);
                if ((start <= address_in_app && end > address_in_app) || app_end == start)
                    app_end = end;
                else if (app_end != NULL)
                    return app_end;
            }
        }
    }
    return app_end;
}

/*
 * This function is instrumented right before the mmap() function of the loader.
 * When the loader first tries to map the test library, we catch it and force the memory mapping
 * to be in the address we want it to.
 */
void mmap_replacement(void** start, ADDRINT flags, ADDRINT fd)
{
    if (KnobVerbose)
    {
        fprintf(stderr, "Intercepted mmap for the loader with start=%p, flags=%x, fd=%d\n", *start, (int)flags, (int)fd);
    }
    if (NULL == *start && fd >= 0 && !(flags & MAP_ANON))
    {
        static const char* toFind  = "/libsplit_image_linux_lib.so";
        static const int toFindLen = strlen(toFind);
        char buf[4096];
        sprintf(buf, "/proc/self/fd/%d", (int)fd);
        int len = readlink(buf, buf, sizeof(buf));
        if (strstr(buf, toFind) == buf + len - toFindLen)
        {
            // 0x200000 is the start address of the executable's code region - we took care of that when we linked the executable.
            *start = locate_app_image_hole((void*)0x200000);
            if (KnobVerbose)
            {
                fprintf(stderr, "The hold in the app's image is at %p\n", *start);
            }
        }
    }
}

/*
 * Check if the provided memory region collides with all other known existing regions.
 * If it does: trigger an assertion.
 * If it doesn't: Add the region to the known regions
 */
void checkRegion(string type, vector< pair< string, ADDRESS_RANGE > >& all, string newName, ADDRESS_RANGE newRange)
{
    for (vector< pair< string, ADDRESS_RANGE > >::iterator it = all.begin(); it != all.end(); it++)
    {
        ADDRESS_RANGE& other = it->second;
        if ((other._low >= newRange._low && other._low <= newRange._high) ||
            (other._high >= newRange._low && other._high <= newRange._high) ||
            (newRange._low >= other._low && newRange._low <= other._high) ||
            (newRange._high >= other._low && newRange._high <= other._high))
        {
            ASSERT(false, "Found two intersecting " + type + ":\n" + newName + " at [" + newRange.String() + "],\nand " +
                              it->first + " at [" + other.String() + "]");
        }
    }
    all.push_back(pair< string, ADDRESS_RANGE >(newName, newRange));
}

VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_Name(img).find("ld-linux") != string::npos)
    {
        RTN rtn = RTN_FindByName(img, "mmap");
        if (RTN_Valid(rtn))
        {
            RTN_Open(rtn);
            RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)mmap_replacement, IARG_FUNCARG_ENTRYPOINT_REFERENCE, 0,
                           IARG_FUNCARG_ENTRYPOINT_VALUE, 3, IARG_FUNCARG_ENTRYPOINT_VALUE, 4, IARG_END);
            RTN_Close(rtn);
        }
    }
    ASSERT(IMG_NumRegions(img) != 0, "Zero regions for image " + IMG_Name(img));

    // check that there are no intersecting images
    checkRegion("images", allImages, IMG_Name(img), ADDRESS_RANGE(IMG_LowAddress(img), IMG_HighAddress(img)));
    if (IMG_NumRegions(img) == 1)
    {
        fprintf(fp, "%s, %p-%p\n", IMG_Name(img).c_str(), Addrint2VoidStar(IMG_LowAddress(img)),
                Addrint2VoidStar(IMG_HighAddress(img)));
    }
    else
    {
        ADDRINT imgLow  = IMG_LowAddress(img);
        ADDRINT imgHigh = IMG_HighAddress(img);
        bool foundHigh = false, foundLow = false;
        for (UINT32 i = 0; i < IMG_NumRegions(img); i++)
        {
            ADDRINT high = IMG_RegionHighAddress(img, i);
            ADDRINT low  = IMG_RegionLowAddress(img, i);
            if (low <= imgLow && high >= imgLow) foundLow = true;
            if (low <= imgHigh && high >= imgHigh) foundHigh = true;
            // check that there are no intersecting regions
            checkRegion("regions", allRegions, IMG_Name(img) + " region #" + decstr(i), ADDRESS_RANGE(low, high));
            fprintf(fp, "%s, %p-%p\n", IMG_Name(img).c_str(), Addrint2VoidStar(low), Addrint2VoidStar(high));
        }

        // flush 'fp' in case the assertions below will be triggered.
        fflush(fp);
        ASSERT(foundLow, "IMG_LowAddress() 0x" + hexstr(imgLow) + " is not within any region of the image");
        ASSERT(foundHigh, "IMG_HighAddress() 0x" + hexstr(imgHigh) + " is not within any region of the image");
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
