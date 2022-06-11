/*
 * Copyright (C) 2007-2021 Intel Corporation.
 * SPDX-License-Identifier: MIT
 */

/*
 * This test verifies that we can call PIN_LockClient / PIN_UnlockClient
 * from a static analysis tool, either directly or indirectly via
 * RTN_FindNameByAddress(), which happens to call these functions internally.
 * Note, this is just a unit test.  There's no real need to call the lock
 * functions directly from a static analysis tool.
 */

#include <iostream>
#include "pin.H"
using std::endl;
using std::string;

KNOB< string > KnobInputFile(KNOB_MODE_WRITEONCE, "pintool", "i", "<imagename>", "specify an image to read");

int main(INT32 argc, CHAR** argv)
{
    PIN_InitSymbols();
    if (PIN_Init(argc, argv)) return 1;

    IMG img = IMG_Open(KnobInputFile);

    if (!IMG_Valid(img))
    {
        std::cout << "Could not open " << KnobInputFile.Value() << endl;
        return 1;
    }

    RTN_FindNameByAddress(0x123);

    PIN_LockClient();
    PIN_UnlockClient();

    IMG_Close(img);
    return 0;
}
