/*
 * Copyright (C) 2016-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

#include <iostream>

#include "pin.H"
using std::cerr;
using std::endl;

static ADDRINT rtn_address      = 0;
static size_t rtn_size          = 0;
static unsigned char* rtn_bytes = NULL;

void SaveRtnBytes(RTN rtn)
{
    rtn_address = RTN_Address(rtn);
    rtn_size    = (size_t)RTN_Size(rtn);
    rtn_bytes   = (unsigned char*)malloc(rtn_size);

    if (rtn_bytes)
    {
        memcpy((void*)rtn_bytes, (const void*)rtn_address, rtn_size);
    }
}

void ValidateProbeSize(int nbytes)
{
    if (rtn_bytes)
    {
        int compare_probe_bytes = memcmp((const void*)rtn_bytes, (const void*)rtn_address, nbytes);
        int compare_rest_of_rtn =
            memcmp((const void*)(rtn_bytes + nbytes), (const void*)(rtn_address + nbytes), rtn_size - nbytes);

        delete[] rtn_bytes;

        if ((compare_probe_bytes == 0) || (compare_rest_of_rtn != 0))
        {
            cerr << "Invalid probe size. Probe seems to be bigger than the expected " << nbytes << " bytes." << endl;
            PIN_ExitProcess(1);
        }
    }
}

VOID foo_before(void)
{
#ifdef TARGET_IA32E
    int expected_probe_size = 6;
#else
    int expected_probe_size = 5;
#endif

    ValidateProbeSize(expected_probe_size);
}

VOID ImageLoad(IMG img, VOID* v)
{
    RTN rtn = RTN_FindByName(img, "foo");

    if (RTN_Valid(rtn))
    {
        SaveRtnBytes(rtn);
        RTN_InsertCallProbed(rtn, IPOINT_BEFORE, AFUNPTR(foo_before), IARG_END);
    }
}

/* ===================================================================== */

int main(INT32 argc, CHAR* argv[])
{
    PIN_InitSymbols();

    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);
    PIN_StartProgramProbed();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
