/*
 * Copyright (C) 2015-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

//
// This tool tests that if you set an image load offset using
// the Pin will shift all of the image's RTNs according to the new load offset.
//

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cstring>
#include "pin.H"

static int knownImages = 0;

void checkImageRange(const IMG& img, ADDRINT low, ADDRINT high)
{
    // Check that all RTNs are in place
    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec))
    {
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn))
        {
            ADDRINT addr = RTN_Address(rtn);
            ASSERT(addr >= low && addr < high, "RTN " + RTN_Name(rtn) + " is at " + hexstr(addr));
        }
    }
}

// Trace an image load event
static VOID TraceImageLoad(IMG img, VOID* v)
{
    if (IMG_Name(img) == "MainImage")
    {
        knownImages++;
        // Check that all RTNs are in place
        checkImageRange(img, 0x4000000, 0x4010000);
    }
    else if (IMG_Name(img) == "SecondImage")
    {
        knownImages++;
        // Check that all RTNs are in place
        checkImageRange(img, 0x6000000, 0x6001000);
    }
    else if (IMG_Name(img) == "ThirdImage")
    {
        knownImages++;
        // Check that all RTNs are in place
        checkImageRange(img, 0x7000000, 0x7001000);
    }
    else
        ASSERT(FALSE, "Unknown image " + IMG_Name(img));
}

// This function is called when the application exits
static VOID Fini(INT32 code, VOID* v) { ASSERTX(3 == knownImages); }

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

static INT32 Usage()
{
    PIN_ERROR("This tool tests that if you set an image load offset, Pin will shift all of\n"
              "its RTNs according to the new load offset\n" +
              KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize symbol processing
    PIN_InitSymbols();

    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    // We will handle image load operations.
    PIN_SetReplayMode(REPLAY_MODE_IMAGEOPS);

    // Creates artificial main image
    IMG img1 = IMG_CreateAt("MainImage", 0x4000000, 0x10000, 0, TRUE);
    ASSERT(IMG_Valid(img1), "IMG_CreateAt for main image failed");

    // Creates secondary image
    IMG img2 = IMG_CreateAt("SecondImage", 0x6000000, 0x1000, 0, FALSE);
    ASSERT(IMG_Valid(img2), "IMG_CreateAt for seconday image failed");

    // Creates secondary image
    IMG img3 = IMG_CreateAt("ThirdImage", 0x7000000, 0x1000, 0, FALSE);
    ASSERT(IMG_Valid(img3), "IMG_CreateAt for third image failed");

    PIN_LockClient();

    // Populate the IMG object with RTN
    RTN rtn = RTN_CreateAt(0x6000100, "FakeRtn");
    ASSERT(RTN_Valid(rtn), "Failed to create FakeRtn at address 0x6000100");

    // Populate the IMG object with RTN
    rtn = RTN_CreateAt(0x70001f0, "FakierRtn");
    ASSERT(RTN_Valid(rtn), "Failed to create FakierRtn at address 0x70001f0");

    // Now change the load offset
    LINUX_LOADER_IMAGE_INFO li;
    bzero(&li, sizeof(li));
    li.name   = (char*)"SecondImage";
    li.l_addr = 0x6000000;
    IMG_SetLoaderInfo(img2, &li);

    // Now change the load offset
    bzero(&li, sizeof(li));
    li.name   = (char*)"ThirdImage";
    li.l_addr = 0x1000000;
    IMG_SetLoaderInfo(img3, &li);

    // And, finally, inform Pin that it is all there, which will invoke
    // image load callbacks.
    IMG_ReplayImageLoad(img1);
    IMG_ReplayImageLoad(img2);
    IMG_ReplayImageLoad(img3);

    PIN_UnlockClient();

    // Register image load callback
    IMG_AddInstrumentFunction(TraceImageLoad, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
